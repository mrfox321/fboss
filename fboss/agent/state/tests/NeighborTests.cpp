/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <fboss/agent/state/ArpResponseEntry.h>
#include "fboss/agent/test/TestUtils.h"

#include "fboss/agent/state/ArpEntry.h"
#include "fboss/agent/state/NdpEntry.h"
#include "fboss/agent/state/NeighborEntry.h"
#include "fboss/agent/state/NeighborResponseTable.h"
#include "fboss/agent/state/Vlan.h"

#include <gtest/gtest.h>

namespace {
auto constexpr kState = "state";
}

using namespace facebook::fboss;
using folly::IPAddressV4;
using folly::IPAddressV6;
using folly::MacAddress;

template <typename NeighborEntryT>
void serializeTest(const NeighborEntryT& entry) {
  auto serialized = entry.toFollyDynamic();
  auto entryBack = NeighborEntryT::fromFollyDynamic(serialized);

  EXPECT_EQ(entry, *entryBack);
}

TEST(ArpEntry, serialize) {
  auto entry = std::make_unique<ArpEntry>(
      IPAddressV4("192.168.0.1"),
      MacAddress("01:01:01:01:01:01"),
      PortDescriptor(PortID(1)),
      InterfaceID(10),
      NeighborState::REACHABLE,
      std::nullopt,
      42,
      false);
  validateThriftyMigration(*entry);
  serializeTest(*entry);
}

TEST(NdpEntry, serialize) {
  auto entry = std::make_unique<NdpEntry>(
      IPAddressV6("2401:db00:21:70cb:face:0:96:0"),
      MacAddress("01:01:01:01:01:01"),
      PortDescriptor(PortID(10)),
      InterfaceID(10),
      NeighborState::REACHABLE,
      std::nullopt,
      42,
      false);
  validateThriftyMigration(*entry);
  serializeTest(*entry);
}

TEST(ArpTable, serialize) {
  ArpTable table;
  table.addEntry(
      IPAddressV4("192.168.0.1"),
      MacAddress("01:01:01:01:01:01"),
      PortDescriptor(PortID(10)),
      InterfaceID(10),
      NeighborState::REACHABLE);
  table.addEntry(
      IPAddressV4("192.168.0.2"),
      MacAddress("01:01:01:01:01:02"),
      PortDescriptor(PortID(11)),
      InterfaceID(11),
      NeighborState::PENDING);

  validateThriftyMigration(table);
  serializeTest(table);
}

TEST(NdpTable, serialize) {
  NdpTable table;
  table.addEntry(
      IPAddressV6("2401:db00:21:70cb:face:0:96:0"),
      MacAddress("01:01:01:01:01:01"),
      PortDescriptor(PortID(10)),
      InterfaceID(10),
      NeighborState::REACHABLE);
  table.addEntry(
      IPAddressV6("2401:db00:21:70cb:face:0:96:1"),
      MacAddress("01:01:01:01:01:02"),
      PortDescriptor(PortID(11)),
      InterfaceID(11),
      NeighborState::PENDING);

  validateThriftyMigration(table);
  serializeTest(table);
}

TEST(NeighborResponseEntry, serialize) {
  auto entry = std::make_unique<ArpResponseEntry>(
      IPAddressV4("192.168.0.1"),
      MacAddress("01:01:01:01:01:01"),
      InterfaceID(0));

  auto serialized = entry->toFollyDynamic();
  auto entryBack = ArpResponseEntry::fromFollyDynamic(serialized);

  EXPECT_TRUE(*entry == *entryBack);
}

TEST(NeighborResponseTableTest, modify) {
  auto ip1 = IPAddressV4("192.168.0.1"), ip2 = IPAddressV4("192.168.0.2");
  auto mac1 = MacAddress("01:01:01:01:01:01"),
       mac2 = MacAddress("01:01:01:01:01:02");

  auto state = std::make_shared<SwitchState>();
  auto vlan = std::make_shared<Vlan>(VlanID(2001), "vlan1");
  auto arpResponseTable = std::make_shared<ArpResponseTable>();
  arpResponseTable->setEntry(ip1, mac1, InterfaceID(0));
  vlan->setArpResponseTable(arpResponseTable);
  state->getVlans()->addVlan(vlan);

  // modify unpublished state
  EXPECT_EQ(vlan.get(), vlan->modify(&state));

  arpResponseTable = std::make_shared<ArpResponseTable>();
  arpResponseTable->setEntry(ip2, mac2, InterfaceID(1));
  vlan->setArpResponseTable(arpResponseTable);

  // modify unpublished state
  EXPECT_EQ(vlan.get(), vlan->modify(&state));

  vlan->publish();
  auto modifiedVlan = vlan->modify(&state);

  arpResponseTable = std::make_shared<ArpResponseTable>();
  arpResponseTable->setEntry(ip1, mac1, InterfaceID(0));
  modifiedVlan->setArpResponseTable(arpResponseTable);

  EXPECT_NE(vlan.get(), modifiedVlan);

  EXPECT_FALSE(vlan->getArpResponseTable()->getEntry(ip1) != nullptr);
  EXPECT_TRUE(vlan->getArpResponseTable()->getEntry(ip2) != nullptr);
  EXPECT_TRUE(modifiedVlan->getArpResponseTable()->getEntry(ip1) != nullptr);
  EXPECT_FALSE(modifiedVlan->getArpResponseTable()->getEntry(ip2) != nullptr);
}
