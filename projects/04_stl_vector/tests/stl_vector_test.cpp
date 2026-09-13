#include <gtest/gtest.h>
#include <order_book_l2.hpp>
#include <print>
#include <cstddef>

constexpr auto base_price_ticks = 10000;


TEST(OrderBookL2Test, SameWindow) {


    OrderBookSideL2<uint32_t, false> side {base_price_ticks - (capacity/2 - 1)};
    side.apply_update(base_price_ticks, 1, 1);
    EXPECT_EQ(side.best_index(), (capacity/2 - 1));
    side.apply_update(base_price_ticks + 1, 1, 1);
    side.apply_update(base_price_ticks + 10, 1, 1);
    // std::cout << side.get_bbo();
    side.apply_update(base_price_ticks + 10, 0, 0);
    // std::cout << side.get_bbo();
    EXPECT_EQ(side.best_index(), (capacity/2 - 1) + 1);

    //
    // side.apply_update(base_price_ticks + 167, 1, 1);
    // std::cout << side.get_bbo();
}

TEST(OrderBookL2Test, WindowReset) {
    OrderBookSideL2<uint32_t, true> side {base_price_ticks - (capacity/2 - 1)};

    side.apply_update(base_price_ticks, 1, 1);
    side.apply_update(base_price_ticks + capacity * 2, 1, 1);
    EXPECT_EQ(side.get_bbo().price_ticks, base_price_ticks);

    side.apply_update(base_price_ticks - capacity * 2, 1, 1);
    EXPECT_EQ(side.get_bbo().price_ticks, base_price_ticks - capacity * 2);
    side.apply_update(base_price_ticks - capacity * 2, 0, 1);
    EXPECT_EQ(side.get_bbo().quantity, 0);

}

TEST(OrderBookL2Test, WindowShift) {
    OrderBookSideL2<uint32_t, false> side {base_price_ticks - (capacity/2 - 1)};
    side.apply_update(base_price_ticks, 1, 1);
    side.apply_update(510, 1, 1);
    side.apply_update(base_price_ticks + 513, 1, 1);
    EXPECT_EQ(side.get_bbo().price_ticks, base_price_ticks + 513);
    side.apply_update(base_price_ticks + 513, 0, 1);
    EXPECT_EQ(side.get_bbo().price_ticks, base_price_ticks);
    side.apply_update(base_price_ticks, 0, 1);
    EXPECT_EQ(side.get_bbo().price_ticks, 0);
}