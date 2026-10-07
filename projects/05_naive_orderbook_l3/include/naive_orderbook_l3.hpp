#pragma once

#include <algorithm>
#include <cstdint>
#include <deque>
#include <map>
#include <memory>
#include <unordered_map>

// The most straightforward implementation possible of an L3 order book (dynamic memory, suboptimal algorithms...)
// Basically the implementation I could code the fastest
/*
 * An L3 order book must provide O(1) best bid/best ask
 * Keep track of individual orders at each price level
 * Each price level order queue follows FIFO
 * Orders are identified by a unique ID
 */

/*
 * Methods:
 * A: Add Order
 * F: Add Order with MPID
 * E: Execute order (qty)
 * C: Execute order with price (qty, price)
 * X: Cancel Order
 * D: Delete Order
 * U: Replace (Delete + Add)
*/



template <typename T>
concept OrderConcept = requires(T t) {
    t.uuid;
    t.qty;
    t.original_qty;
    t.side;
};


template <
    OrderConcept OrderType,
typename OrderID = uint32_t,
typename Quantity = uint32_t,
typename Price = uint32_t,
bool is_bid = true
>
class NaiveOrderbookL3Side {
    public:
    NaiveOrderbookL3Side() = default;

    // A
    void add(OrderID uuid, Quantity qty, Price price) {
        auto order = std::make_shared<OrderType>(uuid, qty, qty);
        book[price].push_back(order);
        order_map[uuid] = {price, order};
    }

    // E
    void execute(OrderID uuid, Quantity qty) {
        auto it = order_map.find(uuid);
        if (it == order_map.end()) {
            return;
        }
        auto order = it->second.second.lock();
        if (!order) {
            return;
        }
        order->qty -= qty;
        auto price = it->second.first;
        if (order->qty <= 0) {
            book[price].erase(std::remove(book[price].begin(), book[price].end(), order), book[price].end());
            order_map.erase(it);
            if (book[price].empty()) {
                book.erase(price);
            }
        }
    }

    // C
    void cancel(OrderID uuid) {
        auto it = order_map.find(uuid);
        if (it == order_map.end()) {
            return;
        }
        auto order = it->second.second.lock();
        if (!order) {
            return;
        }
        auto price = it->second.first;

        book[price].erase(std::remove(book[price].begin(), book[price].end(), order), book[price].end());
        order_map.erase(it);
        if (book[price].empty()) {
            book.erase(price);
        }
    }

    Price best_price() const noexcept{
        if (book.empty()) {
            return 0;
        }

        if constexpr (is_bid) {
            return book.rbegin()->first;
        } else {
            return book.begin()->first;
        }
    }

    // Test-Only Methods
    bool empty() const noexcept {
        return book.empty();
    }

    bool contains(OrderID uuid) const {
        return order_map.contains(uuid);
    }

    size_t order_count() const noexcept {
        size_t count = 0;

        for (const auto& [price, orders] : book) {
            count += orders.size();
        }

        return count;
    }

    size_t level_count() const noexcept {
        return book.size();
    }

    size_t orders_at(Price price) const {
        auto it = book.find(price);

        return it == book.end() ? 0 : it->second.size();
    }

    Quantity quantity_at(Price price) const {
        auto it = book.find(price);

        if (it == book.end()) {
            return 0;
        }

        Quantity total = 0;

        for (const auto& order : it->second) {
            total += order->qty;
        }

        return total;
    }

    std::optional<Quantity> remaining_qty(OrderID uuid) const {
        auto it = order_map.find(uuid);

        if (it == order_map.end()) {
            return std::nullopt;
        }

        auto order = it->second.second.lock();

        if (!order) {
            return std::nullopt;
        }

        return order->qty;
    }

    std::optional<Price> order_price(OrderID uuid) const {
        auto it = order_map.find(uuid);

        if (it == order_map.end()) {
            return std::nullopt;
        }

        return it->second.first;
    }


private:

    std::map<Price, std::deque<std::shared_ptr<OrderType>>> book;
    std::unordered_map<OrderID, std::pair<Price, std::weak_ptr<OrderType>>> order_map;

};