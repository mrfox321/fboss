/*
 *  Copyright (c) 2004-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "fboss/agent/hw/sai/api/BufferApi.h"
#include "fboss/agent/hw/sai/fake/FakeSai.h"
#include "fboss/agent/hw/sai/store/SaiObject.h"
#include "fboss/agent/hw/sai/store/SaiStore.h"
#include "fboss/agent/hw/sai/store/tests/SaiStoreTest.h"

#include <folly/logging/xlog.h>

#include <gtest/gtest.h>

#include <variant>

using namespace facebook::fboss;

class BufferStoreTest : public SaiStoreTest {
 public:
  SaiBufferPoolTraits::CreateAttributes createPoolAttrs() const {
    SaiBufferPoolTraits::Attributes::Type type{SAI_BUFFER_POOL_TYPE_EGRESS};
    SaiBufferPoolTraits::Attributes::Size size{42};
    SaiBufferPoolTraits::Attributes::ThresholdMode mode{
        SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC};
    return {type, size, mode};
  }
  BufferPoolSaiId createBufferPool() const {
    auto& bufferApi = saiApiTable->bufferApi();
    return bufferApi.create<SaiBufferPoolTraits>(createPoolAttrs(), 0);
  }

  SaiBufferProfileTraits::CreateAttributes createProfileAttrs(
      BufferPoolSaiId _pool) const {
    SaiBufferProfileTraits::Attributes::PoolId pool{_pool};
    std::optional<SaiBufferProfileTraits::Attributes::ReservedBytes>
        reservedBytes{42};
    std::optional<SaiBufferProfileTraits::Attributes::ThresholdMode> mode{
        SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC};
    std::optional<SaiBufferProfileTraits::Attributes::SharedDynamicThreshold>
        dynamicThresh{24};
    std::optional<SaiBufferProfileTraits::Attributes::XoffTh> xoffTh{293624};
    std::optional<SaiBufferProfileTraits::Attributes::XonTh> xonTh{0};
    std::optional<SaiBufferProfileTraits::Attributes::XonOffsetTh> xonOffsetTh{
        4826};
    return SaiBufferProfileTraits::CreateAttributes{
        pool, reservedBytes, mode, dynamicThresh, xoffTh, xonTh, xonOffsetTh};
  }
  BufferProfileSaiId createBufferProfile(BufferPoolSaiId _pool) {
    auto& bufferApi = saiApiTable->bufferApi();
    return bufferApi.create<SaiBufferProfileTraits>(
        createProfileAttrs(_pool), 0);
  }
};

TEST_F(BufferStoreTest, loadBufferPool) {
  auto poolId = createBufferPool();
  SaiStore s(0);
  s.reload();
  auto& store = s.get<SaiBufferPoolTraits>();
  auto got = store.get(SAI_BUFFER_POOL_TYPE_EGRESS);
  EXPECT_EQ(got->adapterKey(), poolId);
  EXPECT_EQ(
      std::get<SaiBufferPoolTraits::Attributes::ThresholdMode>(
          got->attributes()),
      SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC);
}

TEST_F(BufferStoreTest, loadBufferProfile) {
  auto poolId = createBufferPool();
  auto profileId = createBufferProfile(poolId);
  SaiStore s(0);
  s.reload();
  auto& store = s.get<SaiBufferProfileTraits>();
  auto got = store.get(createProfileAttrs(poolId));
  EXPECT_EQ(got->adapterKey(), profileId);
  EXPECT_EQ(
      GET_OPT_ATTR(BufferProfile, ThresholdMode, got->attributes()),
      SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC);
}

TEST_F(BufferStoreTest, loadBufferPoolFromJson) {
  auto poolId = createBufferPool();
  SaiStore s(0);
  s.reload();
  auto json = s.adapterKeysFollyDynamic();
  SaiStore s2(0);
  s2.reload(&json);
  auto& store = s2.get<SaiBufferPoolTraits>();
  auto got = store.get(SAI_BUFFER_POOL_TYPE_EGRESS);
  EXPECT_EQ(got->adapterKey(), poolId);
  EXPECT_EQ(
      std::get<SaiBufferPoolTraits::Attributes::ThresholdMode>(
          got->attributes()),
      SAI_BUFFER_POOL_THRESHOLD_MODE_DYNAMIC);
}

TEST_F(BufferStoreTest, loadBufferProfileFromJson) {
  auto poolId = createBufferPool();
  auto profileId = createBufferProfile(poolId);
  SaiStore s(0);
  s.reload();
  auto json = s.adapterKeysFollyDynamic();
  SaiStore s2(0);
  s2.reload(&json);
  auto& store = s2.get<SaiBufferProfileTraits>();
  auto got = store.get(createProfileAttrs(poolId));
  EXPECT_EQ(got->adapterKey(), profileId);
  EXPECT_EQ(
      GET_OPT_ATTR(BufferProfile, ThresholdMode, got->attributes()),
      SAI_BUFFER_PROFILE_THRESHOLD_MODE_DYNAMIC);
}

TEST_F(BufferStoreTest, bufferPoolLoadCtor) {
  auto poolId = createBufferPool();
  SaiObject<SaiBufferPoolTraits> obj = createObj<SaiBufferPoolTraits>(poolId);
  EXPECT_EQ(obj.adapterKey(), poolId);
  EXPECT_EQ(GET_ATTR(BufferPool, Size, obj.attributes()), 42);
}

TEST_F(BufferStoreTest, bufferProfileLoadCtor) {
  auto poolId = createBufferPool();
  auto profileId = createBufferProfile(poolId);
  SaiObject<SaiBufferProfileTraits> obj =
      createObj<SaiBufferProfileTraits>(profileId);
  EXPECT_EQ(obj.adapterKey(), profileId);
  EXPECT_EQ(GET_OPT_ATTR(BufferProfile, ReservedBytes, obj.attributes()), 42);
}

TEST_F(BufferStoreTest, bufferPoolCreateCtor) {
  SaiObject<SaiBufferPoolTraits> obj =
      createObj<SaiBufferPoolTraits>(24, createPoolAttrs(), 0);
  EXPECT_EQ(GET_ATTR(BufferPool, Size, obj.attributes()), 42);
}

TEST_F(BufferStoreTest, bufferProfileCreateCtor) {
  auto c = createProfileAttrs(createBufferPool());
  SaiObject<SaiBufferProfileTraits> obj =
      createObj<SaiBufferProfileTraits>(c, c, 0);
  EXPECT_EQ(GET_OPT_ATTR(BufferProfile, ReservedBytes, obj.attributes()), 42);
}

TEST_F(BufferStoreTest, serDeserBufferPool) {
  auto poolId = createBufferPool();
  verifyAdapterKeySerDeser<SaiBufferPoolTraits>({poolId});
}

TEST_F(BufferStoreTest, toStrBufferPool) {
  std::ignore = createBufferPool();
  verifyToStr<SaiBufferPoolTraits>();
}

TEST_F(BufferStoreTest, serDeserBufferProfile) {
  auto poolId = createBufferPool();
  auto profileId = createBufferProfile(poolId);
  verifyAdapterKeySerDeser<SaiBufferProfileTraits>({profileId});
}

TEST_F(BufferStoreTest, toStrBufferProfile) {
  auto poolId = createBufferPool();
  std::ignore = createBufferProfile(poolId);
  verifyToStr<SaiBufferProfileTraits>();
}
