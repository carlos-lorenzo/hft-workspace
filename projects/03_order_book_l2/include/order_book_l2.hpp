#pragma once


#include <cstddef>
#include <bit>
#include <cstdint>
#include <array>
#include <print>

template <typename T>
concept OrderQuantity = std::is_integral_v<T> && std::is_unsigned_v<T>;

struct alignas(64) PriceLevel
{
    uint64_t price_ticks {0};
    uint32_t quantity {0};
    uint32_t orders {0};
};

inline std::ostream& operator<<(std::ostream& os, const PriceLevel& price_level) noexcept
{
    return os << price_level.price_ticks << ", " << price_level.quantity << ", " << price_level.orders << std::endl;
}

struct HierarchicalBitmap4096 { uint64_t summary{0ULL}; uint64_t leaves[64]{0}; };

constexpr size_t capacity = 4096;
constexpr size_t bitmask = capacity - 1;
constexpr size_t max_best_shift = 512;

template <OrderQuantity Qty, bool is_ask>
class alignas(64) OrderBookSideL2
{
private:
    uint64_t m_base_price_ticks;
    uint64_t m_max_price_ticks;
    uint64_t m_min_price_ticks;

    size_t m_best_index;
    std::array<PriceLevel, capacity> m_levels;
    HierarchicalBitmap4096 m_bitmap;

    void reset_window(const uint64_t new_price_ticks) noexcept
    {
        m_base_price_ticks = new_price_ticks - 2048;
        m_min_price_ticks = m_base_price_ticks;
        m_max_price_ticks = m_base_price_ticks + capacity;
        m_bitmap.summary = 0;
        std::ranges::fill(m_bitmap.leaves, 0);
    }

    void recenter_window() noexcept
    {

        auto central_price = m_levels[m_best_index].price_ticks;

        // The price at physical index 0
        m_base_price_ticks = std::min(std::max(central_price + (capacity - m_best_index), (capacity/2 - 1)), sizeof(uint64_t) - (capacity/2));

        // New maximum and minimum allowed prices - used for checking out of bounds
        auto new_min = central_price - ((capacity/2) - 1);
        auto new_max = central_price + (capacity/2);


        // Price rallied up
        if (new_max > m_max_price_ticks)
        {
            for (auto price_ticks = m_max_price_ticks + 1; price_ticks <= new_max; ++price_ticks)
            {
                 set_level(price_ticks, 0, 0);
            }
        } else // Price rallied down
        {
            for (auto price_ticks = m_min_price_ticks - 1; price_ticks >= new_min; --price_ticks)
            {
                set_level(price_ticks, 0, 0);
            }
        }
        m_max_price_ticks = new_max;
        m_min_price_ticks = new_min;
    }

    [[nodiscard]] size_t deviation_from_base() const noexcept
    {
        if (m_levels[m_best_index].price_ticks > m_base_price_ticks) return m_levels[m_best_index].price_ticks - m_base_price_ticks;
        return m_base_price_ticks - m_levels[m_best_index].price_ticks;
    }

    void set_level(const uint64_t price_ticks, const Qty qty, const uint32_t count) noexcept
    {
        const auto idx = (price_ticks - m_base_price_ticks) & (bitmask);

        m_levels[idx].price_ticks = price_ticks;
        m_levels[idx].quantity = qty;
        m_levels[idx].orders = count;


        // Update the bitmap based on qty
        const auto leaf_index = idx>>6;
        if (qty > 0)
        {
            m_bitmap.leaves[leaf_index] |= 1ULL << (idx & 63);
            m_bitmap.summary |= (1ULL) << leaf_index;
        }
        else m_bitmap.leaves[leaf_index] &= ~(1ULL << (idx & 63));

        if (m_bitmap.leaves[leaf_index] == 0) m_bitmap.summary &= ~(1ULL << leaf_index);
    }

public:
    explicit constexpr OrderBookSideL2(const uint64_t initial_base_price) noexcept :
        m_base_price_ticks(initial_base_price),
        m_max_price_ticks(initial_base_price + capacity),
        m_min_price_ticks(initial_base_price),
        m_best_index(capacity)
    {
    }

    void apply_update(const uint64_t price_ticks, const Qty qty, const uint32_t count) noexcept
    {
        if constexpr (is_ask)
        {
            if (price_ticks > m_max_price_ticks) return; // Ignore values far from BBO that don't create a new one
            if (price_ticks < m_min_price_ticks) [[unlikely]] reset_window(price_ticks);
        }
        else
        {
            if (price_ticks < m_min_price_ticks) return; // Ignore values far from BBO that don't create a new one
            if (price_ticks > m_max_price_ticks) [[unlikely]] reset_window(price_ticks);
        }


       set_level(price_ticks, qty, count);

        // Set m_best_index based on m_bitmap
        if (m_bitmap.summary == 0) m_best_index = capacity;

        else if constexpr (is_ask)
        {
            auto best_index_word = std::countr_zero(m_bitmap.summary);
            auto best_index_bit = std::countr_zero(m_bitmap.leaves[best_index_word]);
            m_best_index = best_index_word * 64 + best_index_bit;
        }
        else
        {
            auto best_index_word = 63 - std::countl_zero(m_bitmap.summary);
            auto best_index_bit = 63 - std::countl_zero(m_bitmap.leaves[best_index_word]);
            m_best_index = best_index_word * 64 + best_index_bit;
        }

        if (deviation_from_base() > max_best_shift) recenter_window();


    }

    [[nodiscard]] PriceLevel get_bbo() const noexcept
    {
        if (m_best_index >= capacity) return PriceLevel{};
        return m_levels[m_best_index];
    }

    auto best_index() noexcept { return m_best_index; }
};


template <OrderQuantity Qty>
class alignas(64) OrderBookL2
{
private:
    OrderBookSideL2<Qty, false> m_bid;
    OrderBookSideL2<Qty, true> m_ask;


public:

    void apply_update(bool is_bid, uint64_t price, Qty qty, uint32_t count) noexcept
    {
        if (is_bid) return m_bid.apply_update(price, qty, count);
        return m_ask.apply_update(price, qty, count);
    }

    [[nodiscard]] PriceLevel get_bbo(const bool is_ask) const noexcept
    {
        if (is_ask) return m_ask.get_bbo();
        return m_bid.get_bbo();
    }

};

