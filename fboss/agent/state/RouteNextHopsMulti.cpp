/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/state/RouteNextHopsMulti.h"
#include "fboss/agent/AddressUtil.h"
#include "fboss/agent/FbossError.h"
#include "fboss/agent/Utils.h"
#include "fboss/agent/gen-cpp2/switch_state_types.h"
#include "fboss/agent/state/RouteNextHopEntry.h"
#include "fboss/agent/state/StateUtils.h"

namespace {
constexpr auto kNexthopDelim = "@";
}

namespace facebook::fboss {

//
// RouteNextHop Class
//

folly::dynamic RouteNextHopsMulti::toFollyDynamicLegacy() const {
  // Store the clientid->RouteNextHopEntry map as a dynamic::object
  folly::dynamic obj = folly::dynamic::object();
  for (auto const& row : map()) {
    int clientid = static_cast<int>(row.first);
    const auto& entry = RouteNextHopEntry::fromThrift(row.second);
    obj[folly::to<std::string>(clientid)] = entry.toFollyDynamicLegacy();
  }
  return obj;
}

RouteNextHopsMulti RouteNextHopsMulti::fromFollyDynamicLegacy(
    const folly::dynamic& json) {
  RouteNextHopsMulti nh;
  for (const auto& pair : json.items()) {
    nh.update(
        ClientID(pair.first.asInt()),
        RouteNextHopEntry::fromFollyDynamicLegacy(pair.second));
  }
  return nh;
}

std::vector<ClientAndNextHops> RouteNextHopsMulti::toThriftLegacy() const {
  std::vector<ClientAndNextHops> list;
  for (const auto& srcPair : map()) {
    ClientAndNextHops destPair;
    *destPair.clientId() = static_cast<int>(srcPair.first);
    for (const auto& nh : *srcPair.second.nexthops()) {
      destPair.nextHops()->push_back(nh);
    }
    list.push_back(destPair);
  }
  return list;
}

std::string RouteNextHopsMulti::strLegacy() const {
  std::string ret = "";
  for (auto const& row : map()) {
    ClientID clientid = row.first;
    auto entry = RouteNextHopEntry::fromThrift(row.second);
    RouteNextHopSet const& nxtHps = entry.getNextHopSet();

    ret.append(folly::to<std::string>("(client#", clientid, ": "));
    for (const auto& nh : nxtHps) {
      ret.append(folly::to<std::string>(nh.str(), ", "));
    }
    ret.append(")");
  }
  return ret;
}

void RouteNextHopsMulti::update(ClientID clientId, RouteNextHopEntry nhe) {
  RouteNextHopsMulti::update(clientId, writableData(), nhe.toThrift());
}

ClientID RouteNextHopsMulti::findLowestAdminDistance() {
  return RouteNextHopsMulti::findLowestAdminDistance(data());
}

void RouteNextHopsMulti::delEntryForClient(ClientID clientId) {
  RouteNextHopsMulti::delEntryForClient(clientId, writableData());
}

const state::RouteNextHopEntry* RouteNextHopsMulti::getEntryForClient(
    ClientID clientId) const {
  return RouteNextHopsMulti::getEntryForClient(clientId, data());
}

bool RouteNextHopsMulti::isSame(ClientID id, const RouteNextHopEntry& nhe)
    const {
  auto entry = getEntryForClient(id);
  return entry && (*entry == nhe.toThrift());
}

std::pair<ClientID, const state::RouteNextHopEntry*>
RouteNextHopsMulti::getBestEntry() const {
  return RouteNextHopsMulti::getBestEntry(data());
}

state::RouteNextHopsMulti RouteNextHopsMulti::toThrift() const {
  state::RouteNextHopsMulti thriftMulti{};
  thriftMulti.lowestAdminDistanceClientId() = lowestAdminDistanceClientId();
  for (auto entry : map()) {
    thriftMulti.client2NextHopEntry()->emplace(entry.first, entry.second);
  }
  return thriftMulti;
}

RouteNextHopsMulti RouteNextHopsMulti::fromThrift(
    const state::RouteNextHopsMulti& multi) {
  RouteNextHopsMulti routeNextHopMulti{};
  routeNextHopMulti.setLowestAdminDistanceClientId(
      *multi.lowestAdminDistanceClientId());
  for (auto entry : *multi.client2NextHopEntry()) {
    routeNextHopMulti.map().emplace(entry.first, entry.second);
  }
  return routeNextHopMulti;
}

folly::dynamic RouteNextHopsMulti::migrateToThrifty(folly::dynamic const& dyn) {
  folly::dynamic newDyn = folly::dynamic::dynamic::object;
  folly::dynamic client2NextHopEntryDyn = folly::dynamic::object;
  auto multi = fromFollyDynamicLegacy(dyn);
  for (auto [key, value] : dyn.items()) {
    client2NextHopEntryDyn[key] = RouteNextHopEntry::migrateToThrifty(value);
  }
  newDyn["client2NextHopEntry"] = client2NextHopEntryDyn;
  newDyn["lowestAdminDistanceClientId"] =
      static_cast<int>(multi.lowestAdminDistanceClientId());
  return newDyn;
}

void RouteNextHopsMulti::migrateFromThrifty(folly::dynamic& dyn) {
  for (auto [key, value] : dyn["client2NextHopEntry"].items()) {
    auto clientID = key.asString();
    auto multiDynamic = value;
    RouteNextHopEntry::migrateFromThrifty(multiDynamic);
    dyn[clientID] = multiDynamic;
  }
  dyn.erase("client2NextHopEntry");
  dyn.erase("lowestAdminDistanceClientId");
}

std::pair<ClientID, const state::RouteNextHopEntry*>
RouteNextHopsMulti::getBestEntry(
    const state::RouteNextHopsMulti& nexthopsmulti) {
  auto clientID = *nexthopsmulti.lowestAdminDistanceClientId();
  auto entry = RouteNextHopsMulti::getEntryForClient(clientID, nexthopsmulti);
  if (entry) {
    return std::make_pair(clientID, entry);
  } else {
    // Throw an exception if the map is empty. Shall not happen
    throw FbossError("Unexpected empty RouteNextHopsMulti");
  }
}

const state::RouteNextHopEntry* FOLLY_NULLABLE
RouteNextHopsMulti::getEntryForClient(
    ClientID clientId,
    const state::RouteNextHopsMulti& nexthopsmulti) {
  const auto& map = *(nexthopsmulti.client2NextHopEntry());
  auto iter = map.find(clientId);
  if (iter == map.end()) {
    return nullptr;
  }
  return &iter->second;
}

void RouteNextHopsMulti::update(
    ClientID clientId,
    state::RouteNextHopsMulti& nexthopsmulti,
    state::RouteNextHopEntry nhe) {
  auto& map = *(nexthopsmulti.client2NextHopEntry());
  auto iter = map.find(clientId);
  if (iter == map.end()) {
    map.insert(std::make_pair(clientId, nhe));
  } else {
    iter->second = nhe;
  }

  // Let's check whether this has a preferred admin distance
  if (map.size() == 1) {
    nexthopsmulti.lowestAdminDistanceClientId() = clientId;
    return;
  }
  auto lowestAdminDistanceClientId =
      *nexthopsmulti.lowestAdminDistanceClientId();
  auto entry = RouteNextHopsMulti::getEntryForClient(
      lowestAdminDistanceClientId, nexthopsmulti);
  if (!entry) {
    nexthopsmulti.lowestAdminDistanceClientId() =
        RouteNextHopsMulti::findLowestAdminDistance(nexthopsmulti);
  } else if (*(nhe.adminDistance()) < *(entry->adminDistance())) {
    // Arbritary choice to use the newest one if we have multiple
    // with the same admin distance
    nexthopsmulti.lowestAdminDistanceClientId() = clientId;
  }
}

ClientID RouteNextHopsMulti::findLowestAdminDistance(
    const state::RouteNextHopsMulti& nexthopsmulti) {
  const auto& map = *nexthopsmulti.client2NextHopEntry();
  if (map.size() == 0) {
    // We'll set it on the next add
    return ClientID(-1);
  }
  auto lowest = map.begin();
  auto it = map.begin();
  while (it != map.end()) {
    if (*(it->second.adminDistance()) < *(lowest->second.adminDistance())) {
      lowest = it;
    } else if (it->second == lowest->second && it->first < lowest->first) {
      // If we have two clients with the same admin distance, pick the lowest
      // client id - this is an arbritary, but deterministic, choice
      lowest = it;
    }
    ++it;
  }
  return lowest->first;
}

void RouteNextHopsMulti::delEntryForClient(
    ClientID clientId,
    state::RouteNextHopsMulti& nexthopsmulti) {
  auto& map = *(nexthopsmulti.client2NextHopEntry());
  map.erase(clientId);
  // Let's regen the next best entry
  if (*nexthopsmulti.lowestAdminDistanceClientId() == clientId) {
    nexthopsmulti.lowestAdminDistanceClientId() =
        RouteNextHopsMulti::findLowestAdminDistance(nexthopsmulti);
  }
}

} // namespace facebook::fboss
