/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/cpp2/reflection/folly_dynamic.h>
#include <thrift/lib/cpp2/reflection/reflection.h>
#include <thrift/lib/cpp2/reflection/testing.h>
#include "fboss/agent/gen-cpp2/switch_config_fatal_types.h"
#include "fboss/fsdb/if/gen-cpp2/fsdb_oper_types.h"
#include "fboss/thrift_cow/nodes/Serializer.h"
#include "fboss/thrift_cow/nodes/Types.h"
#include "fboss/thrift_cow/nodes/tests/gen-cpp2/test_fatal_types.h"

#include <gtest/gtest.h>
#include <type_traits>

using namespace facebook::fboss;
using namespace facebook::fboss::thrift_cow;

using k = test_tags::strings;
using sk = cfg::switch_config_tags::strings;

namespace {

cfg::L4PortRange buildPortRange(int min, int max) {
  cfg::L4PortRange portRange;
  portRange.min() = min;
  portRange.max() = max;
  return portRange;
}

} // namespace

TEST(ThriftListNodeTests, ThriftListFieldsPrimitivesSimple) {
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields;
  ASSERT_EQ(fields.size(), 0);
}

TEST(ThriftListNodeTests, ThriftListFieldsPrimitivesGetSet) {
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields;
  fields.emplace_back(3);
  ASSERT_EQ(fields.size(), 1);
  ASSERT_EQ(fields.at(0), 3);

  fields.ref(0) = 10;
  ASSERT_EQ(fields.at(0), 10);
}

TEST(ThriftListNodeTests, ThriftListFieldsPrimitivesConstructFromThrift) {
  std::vector<int> data = {1, 2, 5, 99};
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields(data);

  ASSERT_EQ(fields.size(), 4);
  ASSERT_EQ(fields.at(0), 1);
  ASSERT_EQ(fields.at(1), 2);
  ASSERT_EQ(fields.at(2), 5);
  ASSERT_EQ(fields.at(3), 99);

  ASSERT_EQ(fields.toThrift(), data);
}

TEST(ThriftListNodeTests, ThriftListFieldsStructsSimple) {
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields;
  ASSERT_EQ(fields.size(), 0);
}

TEST(ThriftListNodeTests, ThriftListFieldsStructsGetSet) {
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields;

  auto portRange1 = buildPortRange(100, 999);
  auto portRange2 = buildPortRange(1000, 9999);

  fields.emplace_back(portRange1);
  ASSERT_EQ(fields.size(), 1);
  ASSERT_EQ(fields.at(0)->toThrift(), portRange1);

  fields.emplace_back(portRange2);
  ASSERT_EQ(fields.size(), 2);
  ASSERT_EQ(fields.at(1)->toThrift(), portRange2);

  fields.ref(0)->template set<sk::min>(500);
  ASSERT_EQ(fields.ref(0)->template get<sk::min>(), 500);
}

TEST(ThriftListNodeTests, ThriftListFieldsStructsConstructFromThrift) {
  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};
  ThriftListFields<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields(data);

  ASSERT_EQ(fields.size(), 2);
  ASSERT_EQ(fields.at(0)->toThrift(), data[0]);
  ASSERT_EQ(fields.at(1)->toThrift(), data[1]);

  ASSERT_EQ(fields.toThrift(), data);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesSimple) {
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      node;
  ASSERT_EQ(node.size(), 0);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesGetSet) {
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields;
  fields.emplace_back(3);
  ASSERT_EQ(fields.size(), 1);
  ASSERT_EQ(fields.at(0), 3);

  fields.ref(0) = 10;
  ASSERT_EQ(fields.at(0), 10);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesConstructFromThrift) {
  std::vector<int> data = {1, 2, 5, 99};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields(data);

  ASSERT_EQ(fields.size(), 4);
  ASSERT_EQ(fields.at(0), 1);
  ASSERT_EQ(fields.at(1), 2);
  ASSERT_EQ(fields.at(2), 5);
  ASSERT_EQ(fields.at(3), 99);

  ASSERT_EQ(fields.toThrift(), data);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesVisit) {
  std::vector<int> data = {1, 2, 5, 99};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields(data);

  folly::dynamic out;
  auto f = [&out](auto& node) { out = node.toFollyDynamic(); };

  std::vector<std::string> path = {"0"};
  auto result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 1);

  path = {"0", "test"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::NON_EXISTENT_NODE);

  path = {"3"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 99);

  // also test cvisit
  result = fields.cvisitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 99);

  // Now create const node and test cvisit
  const ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fieldsConst(data);
  result = fieldsConst.cvisitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 99);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesVisitMutable) {
  std::vector<int> data = {1, 2, 5, 99};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>
      fields(data);

  folly::dynamic toWrite, out;
  auto write = [&toWrite](auto& node) { node.fromFollyDynamic(toWrite); };
  auto read = [&out](auto& node) { out = node.toFollyDynamic(); };

  std::vector<std::string> path = {"0"};
  auto result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 1);

  path = {"0", "test"};
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::NON_EXISTENT_NODE);

  toWrite = 1001;
  path = {"3"};
  result = fields.visitPath(path.begin(), path.end(), write);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(out, 1001);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesClone) {
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>;

  std::vector<int> data = {1, 2, 5, 99};

  auto node = std::make_shared<TestNodeType>(data);

  ASSERT_FALSE(node->isPublished());

  node->publish();

  ASSERT_TRUE(node->isPublished());

  auto newNode = node->clone();
  ASSERT_FALSE(newNode->isPublished());
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesModify) {
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>;

  std::vector<int> data = {1, 2, 5, 99};

  auto node = std::make_shared<TestNodeType>(data);

  ASSERT_FALSE(node->isPublished());

  node->publish();

  ASSERT_TRUE(node->isPublished());

  TestNodeType::modify(&node, "0");

  ASSERT_FALSE(node->isPublished());

  // now try modifying a nonexistent index
  ASSERT_EQ(node->size(), 4);
  TestNodeType::modify(&node, "6");
  ASSERT_EQ(node->size(), 7);
}

TEST(ThriftListNodeTests, ThriftListFieldsPrimitivesEncode) {
  using TC =
      apache::thrift::type_class::list<apache::thrift::type_class::integral>;
  std::vector<int> data = {1, 2, 5, 99};
  ThriftListFields<TC, std::vector<int>> fields(data);

  ASSERT_EQ(fields.toThrift(), data);

  auto encoded = fields.encode(fsdb::OperProtocol::COMPACT);
  auto recovered =
      deserialize<TC, std::vector<int>>(fsdb::OperProtocol::COMPACT, encoded);
  ASSERT_EQ(recovered, data);

  encoded = fields.encode(fsdb::OperProtocol::SIMPLE_JSON);
  recovered = deserialize<TC, std::vector<int>>(
      fsdb::OperProtocol::SIMPLE_JSON, encoded);
  ASSERT_EQ(recovered, data);

  encoded = fields.encode(fsdb::OperProtocol::BINARY);
  recovered =
      deserialize<TC, std::vector<int>>(fsdb::OperProtocol::BINARY, encoded);
  ASSERT_EQ(recovered, data);
}

TEST(ThriftListNodeTests, ThriftListNodePrimitivesEncode) {
  using TC =
      apache::thrift::type_class::list<apache::thrift::type_class::integral>;
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::integral>,
      std::vector<int>>;

  std::vector<int> data = {1, 2, 5, 99};

  auto node = std::make_shared<TestNodeType>(data);
  ASSERT_EQ(node->toThrift(), data);

  auto encoded = node->encode(fsdb::OperProtocol::COMPACT);
  auto recovered =
      deserialize<TC, std::vector<int>>(fsdb::OperProtocol::COMPACT, encoded);
  ASSERT_EQ(recovered, data);

  encoded = node->encode(fsdb::OperProtocol::SIMPLE_JSON);
  recovered = deserialize<TC, std::vector<int>>(
      fsdb::OperProtocol::SIMPLE_JSON, encoded);
  ASSERT_EQ(recovered, data);

  encoded = node->encode(fsdb::OperProtocol::BINARY);
  recovered =
      deserialize<TC, std::vector<int>>(fsdb::OperProtocol::BINARY, encoded);
  ASSERT_EQ(recovered, data);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsSimple) {
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields;
  ASSERT_EQ(fields.size(), 0);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsGetSet) {
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields;

  auto portRange1 = buildPortRange(100, 999);
  auto portRange2 = buildPortRange(1000, 9999);

  fields.emplace_back(portRange1);
  ASSERT_EQ(fields.size(), 1);
  ASSERT_EQ(fields.at(0)->toThrift(), portRange1);

  fields.emplace_back(portRange2);
  ASSERT_EQ(fields.size(), 2);
  ASSERT_EQ(fields.at(1)->toThrift(), portRange2);

  fields.ref(0)->template set<sk::min>(500);
  ASSERT_EQ(fields.ref(0)->template get<sk::min>(), 500);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsConstructFromThrift) {
  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields(data);

  ASSERT_EQ(fields.size(), 2);
  ASSERT_EQ(fields.at(0)->toThrift(), data[0]);
  ASSERT_EQ(fields.at(1)->toThrift(), data[1]);

  ASSERT_EQ(fields.toThrift(), data);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsVisit) {
  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields(data);

  folly::dynamic out;
  auto f = [&out](auto& node) { out = node.toFollyDynamic(); };

  std::vector<std::string> path = {"0"};
  auto result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  folly::dynamic expected;
  apache::thrift::to_dynamic(
      expected, data[0], apache::thrift::dynamic_format::JSON_1);
  ASSERT_EQ(out, expected);

  path = {"1"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  apache::thrift::to_dynamic(
      expected, data[1], apache::thrift::dynamic_format::JSON_1);
  ASSERT_EQ(out, expected);

  path = {"0", "nonexistent"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::INVALID_STRUCT_MEMBER);

  path = {"0", "min"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 100);

  path = {"1", "min"};
  result = fields.visitPath(path.begin(), path.end(), f);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 1000);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsVisitMutable) {
  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};
  ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>
      fields(data);

  folly::dynamic toWrite, out;
  auto write = [&toWrite](auto& node) { node.fromFollyDynamic(toWrite); };
  auto read = [&out](auto& node) { out = node.toFollyDynamic(); };

  std::vector<std::string> path = {"0"};
  auto result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  folly::dynamic expected;
  apache::thrift::to_dynamic(
      expected, data[0], apache::thrift::dynamic_format::JSON_1);
  ASSERT_EQ(out, expected);

  path = {"1"};
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  apache::thrift::to_dynamic(
      expected, data[1], apache::thrift::dynamic_format::JSON_1);
  ASSERT_EQ(out, expected);

  apache::thrift::to_dynamic(
      toWrite, buildPortRange(1, 2), apache::thrift::dynamic_format::JSON_1);

  path = {"0"};
  result = fields.visitPath(path.begin(), path.end(), write);
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, toWrite);

  path = {"0", "min"};
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 1);
  path = {"0", "max"};
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 2);

  path = {"1", "min"};
  result = fields.visitPath(path.begin(), path.end(), read);
  ASSERT_EQ(result, ThriftTraverseResult::OK);
  ASSERT_EQ(out, 1000);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsClone) {
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>;

  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};

  auto node = std::make_shared<TestNodeType>(data);

  ASSERT_FALSE(node->isPublished());
  ASSERT_FALSE(node->cref(0)->isPublished());

  node->publish();

  ASSERT_TRUE(node->isPublished());
  ASSERT_TRUE(node->cref(0)->isPublished());

  auto newNode = node->clone();
  ASSERT_FALSE(newNode->isPublished());
  ASSERT_TRUE(node->cref(0)->isPublished());
}

TEST(ThriftListNodeTests, ThriftListNodeStructsModify) {
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>;

  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};

  auto node = std::make_shared<TestNodeType>(data);

  ASSERT_FALSE(node->isPublished());
  ASSERT_FALSE(node->cref(0)->isPublished());

  node->publish();

  ASSERT_TRUE(node->isPublished());
  ASSERT_TRUE(node->cref(0)->isPublished());

  TestNodeType::modify(&node, "0");

  ASSERT_FALSE(node->isPublished());
  ASSERT_FALSE(node->cref(0)->isPublished());

  // now try modifying a nonexistent index
  ASSERT_EQ(node->size(), 2);
  TestNodeType::modify(&node, "4");
  ASSERT_EQ(node->size(), 5);
}

TEST(ThriftListNodeTests, ThriftListNodeStructsRemove) {
  using TestNodeType = ThriftListNode<
      apache::thrift::type_class::list<apache::thrift::type_class::structure>,
      std::vector<cfg::L4PortRange>>;

  std::vector<cfg::L4PortRange> data = {
      buildPortRange(100, 999), buildPortRange(1000, 9999)};

  auto node = std::make_shared<TestNodeType>(data);

  // test remove by index
  ASSERT_TRUE(node->remove(1));
  ASSERT_EQ(node->size(), 1);
  ASSERT_EQ(*node->cref(0)->toThrift().min(), 100);

  // reset list, then remove by string
  node->fromThrift(data);
  ASSERT_EQ(node->size(), 2);
  ASSERT_TRUE(node->remove("0"));
  ASSERT_EQ(node->size(), 1);
  ASSERT_EQ(*node->cref(0)->toThrift().min(), 1000);

  // verify remove non-existent index fails
  ASSERT_FALSE(node->remove("123"));
  ASSERT_FALSE(node->remove("15"));

  // verify incompatible key string also fails
  ASSERT_FALSE(node->remove("incompatible"));
}
