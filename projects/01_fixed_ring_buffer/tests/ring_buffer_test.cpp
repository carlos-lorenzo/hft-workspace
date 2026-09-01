#include <gtest/gtest.h>
#include <fixed_ring_buffer.hpp>

// Sample POD event type for testing constraints
struct MockOrder { uint64_t id; double price; uint32_t qty; };

TEST(FixedRingBufferTest, CapacityAndAlignment) {
    // Basic alignment and capacity static assertions / runtime checks
    constexpr size_t capacity = 64;
    FixedRingBuffer<MockOrder, capacity> rb;
    EXPECT_EQ(capacity, rb.capacity());
    EXPECT_EQ(rb.size(), 0);
}

TEST(FixedRingBufferTest, PushPopBehavior) {
    // Invariant validation: push until full, pop until empty
    constexpr size_t capacity = 4;
    FixedRingBuffer<MockOrder, capacity> rb;
    while (!rb.full())
    {
        ASSERT_TRUE(rb.try_push({rb.size(), 2, 3}));
    }

    EXPECT_EQ(rb.size(), capacity);

    while (!rb.empty())
    {
        MockOrder order{};
        ASSERT_TRUE(rb.try_pop(order));
    }

    EXPECT_EQ(rb.size(), 0);

}

TEST(FixedRingBufferTest, IndexWrapAround) {
    // Push and pop across boundary to verify power-of-two mask indexing
    constexpr size_t capacity = 4;

    FixedRingBuffer<MockOrder, capacity> rb;

    // Repeat 10 * capacity + 1 to ensure index wrap-around properly works

    for (auto i = 0u; i < 10*capacity+1; ++i)
    {
        ASSERT_TRUE(rb.try_push({0,0,0}));
        MockOrder order{};
        ASSERT_TRUE(rb.try_pop(order));
    }
    ASSERT_TRUE(rb.try_push({0,0,0}));

    ASSERT_EQ(rb.size(), 1);

    MockOrder order{};
    ASSERT_TRUE(rb.try_pop(order));



}