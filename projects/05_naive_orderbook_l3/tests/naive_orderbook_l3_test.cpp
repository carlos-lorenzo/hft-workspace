
#include <gtest/gtest.h>

#include <cstdint>
#include <naive_orderbook_l3.hpp>

namespace NaiveL3Test {

    struct Order {
        uint32_t uuid;
        uint32_t qty;
        uint32_t original_qty;
        bool side;
    };

    using BidBook = NaiveOrderbookL3Side<
        Order,
        uint32_t,
        uint32_t,
        uint32_t,
        true
    >;

    using AskBook = NaiveOrderbookL3Side<
        Order,
        uint32_t,
        uint32_t,
        uint32_t,
        false
    >;

}

// ---------------------------------------------------------
// Basic insertion
// ---------------------------------------------------------

TEST(NaiveOrderbookL3, EmptyBookReturnsZero) {
    NaiveL3Test::BidBook book;

    EXPECT_EQ(book.best_price(), 0);
}

TEST(NaiveOrderbookL3, AddSingleBid) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    EXPECT_EQ(book.best_price(), 50);
}

TEST(NaiveOrderbookL3, AddSingleAsk) {
    NaiveL3Test::AskBook book;

    book.add(1, 100, 50);

    EXPECT_EQ(book.best_price(), 50);
}

TEST(NaiveOrderbookL3, AddMultipleOrdersAtSamePrice) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 200, 50);
    book.add(3, 300, 50);

    EXPECT_EQ(book.best_price(), 50);
}

// ---------------------------------------------------------
// Price priority
// ---------------------------------------------------------

TEST(NaiveOrderbookL3, BidReturnsHighestPrice) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 55);
    book.add(3, 100, 45);

    EXPECT_EQ(book.best_price(), 55);
}

TEST(NaiveOrderbookL3, AskReturnsLowestPrice) {
    NaiveL3Test::AskBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 55);
    book.add(3, 100, 45);

    EXPECT_EQ(book.best_price(), 45);
}

TEST(NaiveOrderbookL3, BestPriceUpdatesAfterAddingBetterBid) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    EXPECT_EQ(book.best_price(), 50);

    book.add(2, 100, 60);
    EXPECT_EQ(book.best_price(), 60);

    book.add(3, 100, 55);
    EXPECT_EQ(book.best_price(), 60);
}

TEST(NaiveOrderbookL3, BestPriceUpdatesAfterAddingBetterAsk) {
    NaiveL3Test::AskBook book;

    book.add(1, 100, 50);
    EXPECT_EQ(book.best_price(), 50);

    book.add(2, 100, 40);
    EXPECT_EQ(book.best_price(), 40);

    book.add(3, 100, 45);
    EXPECT_EQ(book.best_price(), 40);
}

// ---------------------------------------------------------
// Execution
// ---------------------------------------------------------

TEST(NaiveOrderbookL3, PartialExecutionKeepsPriceLevel) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.execute(1, 40);

    EXPECT_EQ(book.best_price(), 50);
}

TEST(NaiveOrderbookL3, FullExecutionRemovesOrder) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 40);

    book.execute(1, 100);

    EXPECT_EQ(book.best_price(), 40);
}

TEST(NaiveOrderbookL3, ExecutionRemovesEmptyPriceLevel) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 40);

    book.execute(1, 100);

    EXPECT_EQ(book.best_price(), 40);

    book.execute(2, 100);

    EXPECT_EQ(book.best_price(), 0);
}

TEST(NaiveOrderbookL3, ExecutingUnknownOrderDoesNothing) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.execute(999, 100);

    EXPECT_EQ(book.best_price(), 50);
}

TEST(NaiveOrderbookL3, ExecutingSameOrderTwiceDoesNotCorruptBook) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 40);

    book.execute(1, 100);
    book.execute(1, 100);

    EXPECT_EQ(book.best_price(), 40);
}

// ---------------------------------------------------------
// Cancellation
// ---------------------------------------------------------


TEST(NaiveOrderbookL3, FullCancellationRemovesOrder) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);
    book.add(2, 100, 40);

    book.cancel(1);

    EXPECT_EQ(book.best_price(), 40);
}

TEST(NaiveOrderbookL3, CancellingUnknownOrderDoesNothing) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.cancel(999);

    EXPECT_EQ(book.best_price(), 50);
}

TEST(NaiveOrderbookL3, CancellingLastOrderEmptiesBook) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.cancel(1);

    EXPECT_EQ(book.best_price(), 0);
}

// ---------------------------------------------------------
// Independence between book instances
// ---------------------------------------------------------

TEST(NaiveOrderbookL3, BidAndAskBooksAreIndependent) {
    NaiveL3Test::BidBook bids;
    NaiveL3Test::AskBook asks;

    bids.add(1, 100, 50);
    asks.add(2, 100, 60);

    EXPECT_EQ(bids.best_price(), 50);
    EXPECT_EQ(asks.best_price(), 60);

    bids.execute(1, 100);

    EXPECT_EQ(bids.best_price(), 0);
    EXPECT_EQ(asks.best_price(), 60);
}

// ---------------------------------------------------------
// Different price levels
// ---------------------------------------------------------

TEST(NaiveOrderbookL3, RemovingNonBestBidDoesNotChangeBestPrice) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 60);
    book.add(2, 100, 50);

    book.execute(2, 100);

    EXPECT_EQ(book.best_price(), 60);
}

TEST(NaiveOrderbookL3, RemovingNonBestAskDoesNotChangeBestPrice) {
    NaiveL3Test::AskBook book;

    book.add(1, 100, 40);
    book.add(2, 100, 50);

    book.execute(2, 100);

    EXPECT_EQ(book.best_price(), 40);
}

TEST(NaiveOrderbookL3, PartialExecutionUpdatesQuantity) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.execute(1, 40);

    ASSERT_TRUE(book.contains(1));

    EXPECT_EQ(book.remaining_qty(1), 60);
    EXPECT_EQ(book.order_price(1), 50);
    EXPECT_EQ(book.quantity_at(50), 60);
    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(book.level_count(), 1);
}


TEST(NaiveOrderbookL3, FullExecutionRemovesOrderAndLevel) {
    NaiveL3Test::BidBook book;

    book.add(1, 100, 50);

    book.execute(1, 100);

    EXPECT_FALSE(book.contains(1));
    EXPECT_FALSE(book.remaining_qty(1).has_value());

    EXPECT_EQ(book.order_count(), 0);
    EXPECT_EQ(book.level_count(), 0);
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.best_price(), 0);
}