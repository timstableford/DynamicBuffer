#ifdef __LINUX_BUILD

#include <cstring>
#include <cstdio>
#include <functional>

#include <gtest/gtest.h>

#include <DynamicBuffer.h>

struct TestStruct {
    int32_t field1;
    int32_t field2;
};

TEST(IndependentMethod, TestDynamicBufferFragmentation) {
    DynamicBuffer<uint8_t> dynBuffer(128, 32);
    ASSERT_EQ(4096, dynBuffer.getFree());

    int8_t slot1 = dynBuffer.allocate(512);
    ASSERT_EQ(3584, dynBuffer.getFree());
    ASSERT_EQ(512, dynBuffer.getBuffer(slot1).size());

    int8_t slot2 = dynBuffer.allocate(256);
    ASSERT_EQ(3328, dynBuffer.getFree());
    ASSERT_EQ(256, dynBuffer.getBuffer(slot2).size());

    dynBuffer.free(slot1);
    ASSERT_EQ(3840, dynBuffer.getFree());

    int8_t slot3 = dynBuffer.allocate(1024);
    DynamicBuffer<uint8_t>::Buffer bufferObj = dynBuffer.getBuffer(slot3);
    ASSERT_EQ(2816, dynBuffer.getFree());
    ASSERT_EQ(1024, dynBuffer.getBuffer(slot3).size());

    for (uint16_t i = 0; i < bufferObj.size(); i++) {
        bufferObj[i] = i % 0xff;
    }

    for (uint16_t i = 0; i < bufferObj.size(); i++) {
        EXPECT_EQ(i % 0xff, bufferObj[i]);
    }
}

TEST(IndependentMethod, TestAsGenericBuffer) {
    DynamicBuffer<uint8_t> dynBuffer(128, 32);
    ASSERT_EQ(4096, dynBuffer.getFree());

    int8_t slot1 = dynBuffer.allocate(512);
    ASSERT_EQ(3584, dynBuffer.getFree());
    ASSERT_EQ(512, dynBuffer.getBuffer(slot1).size());

    DynamicBuffer<uint8_t>::Buffer b1 = dynBuffer.getBuffer(slot1);
    GenericBuffer<uint8_t> &b2 = b1;
    ASSERT_EQ(512, b2.size());
}

TEST(IndependentMethod, TestArrayBufferWrapper) {
    uint8_t arr[512];
    ArrayBufferWrapper<uint8_t> wrapper(arr, 512);
    arr[10] = 10;
    ASSERT_EQ(10, arr[10]);
}

TEST(IndependentMethod, TestCustomType) {
    DynamicBuffer<TestStruct> buffer(1, 10);
    ASSERT_EQ(10, buffer.getFree());
}

#endif // __LINUX_BUILD
