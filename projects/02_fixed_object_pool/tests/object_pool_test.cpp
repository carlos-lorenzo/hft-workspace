#include <gtest/gtest.h>
#include <fixed_object_pool.hpp>

#include <cstddef>
#include <type_traits>

// Sample POD event type for testing constraints
struct MockOrder { uint64_t id; double price; uint32_t qty; };

TEST(FixedObjectPoolTest, MemoryReuse) {
    FixedObjectPool<MockOrder, 1024> pool;

    auto ptr1 = pool.allocate(1, 2, 3);

    pool.deallocate(ptr1);
    auto ptr2 = pool.allocate(1, 2, 3);

    EXPECT_EQ(ptr1, ptr2);

}

TEST(FixedObjectPoolTest, CapacityExhaustion) {
    FixedObjectPool<MockOrder, 1024> pool;
    while (!pool.full())
    {
        auto ptr = pool.allocate(1, 2, 3);
        EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % alignof(MockOrder), 0);
    }
    auto ptr = pool.allocate(1, 2, 3);
    EXPECT_EQ(ptr, nullptr);

}

TEST(FixedObjectPoolTest, AlligmentVerification) {
    FixedObjectPool<MockOrder, 1024> pool;

    auto ptr = pool.allocate(1, 2, 3);

    EXPECT_EQ(reinterpret_cast<uintptr_t>(ptr) % alignof(MockOrder), 0);

}