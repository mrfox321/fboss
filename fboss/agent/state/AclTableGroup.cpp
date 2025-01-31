/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */
#include "fboss/agent/state/AclTableGroup.h"
#include <folly/Conv.h>
#include <thrift/lib/cpp/util/EnumUtils.h>
#include "fboss/agent/state/AclEntry.h"
#include "fboss/agent/state/AclTable.h"
#include "fboss/agent/state/NodeBase-defs.h"
#include "fboss/agent/state/StateUtils.h"

using apache::thrift::TEnumTraits;
using folly::IPAddress;

namespace {
constexpr auto kAclStage = "aclStage";
constexpr auto kName = "name";
constexpr auto kAclTableMap = "aclTableMap";
} // namespace

namespace facebook::fboss {

folly::dynamic AclTableGroupFields::toFollyDynamic() const {
  folly::dynamic aclTableGroup = folly::dynamic::object;
  aclTableGroup[kAclStage] = static_cast<int>(*data().stage());
  aclTableGroup[kName] = *data().name();
  if (aclTableMap_) {
    aclTableGroup[kAclTableMap] = aclTableMap_->toFollyDynamic();
  }
  return aclTableGroup;
}

AclTableGroupFields AclTableGroupFields::fromFollyDynamic(
    const folly::dynamic& aclTableGroupJson) {
  std::shared_ptr<AclTableMap> aclTableMap;
  if (aclTableGroupJson.find(kAclTableMap) != aclTableGroupJson.items().end()) {
    aclTableMap =
        AclTableMap::fromFollyDynamic(aclTableGroupJson[kAclTableMap]);
  }
  AclTableGroupFields aclTableGroup(
      cfg::AclStage(aclTableGroupJson[kAclStage].asInt()),
      aclTableGroupJson[kName].asString(),
      aclTableMap);

  return aclTableGroup;
}

AclTableGroup::AclTableGroup(cfg::AclStage stage) : NodeBaseT(stage) {}

template class NodeBaseT<AclTableGroup, AclTableGroupFields>;

} // namespace facebook::fboss
