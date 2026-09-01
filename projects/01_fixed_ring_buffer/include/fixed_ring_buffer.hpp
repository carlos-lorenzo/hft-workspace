#pragma once

#include <concepts>
#include <cstddef>
#include <type_traits>

template <typename T>
concept TradingElement = std::is_nothrow_move_constructible_v<T> && std::is_trivially_destructible_v<T>;

template <TradingElement T, std::size_t Capacity>
class alignas(64) FixedRingBuffer {

private:
    // Stored as m_head (padding) | m_data (padding) | m_tail (padding)
    alignas(64) size_t m_head;
    alignas(64) std::array<T, Capacity> m_data;
    alignas(64) size_t m_tail;

public:
    constexpr FixedRingBuffer() noexcept
    {
        static_assert(std::has_single_bit(Capacity) && Capacity > 0, "Capacity must be a multiple of 2 and non-zero");
        m_head = 0;
        m_tail = 0;
    }
    // Destructor, Swap for move and copy constructors, and operator=
    ~FixedRingBuffer() noexcept = default;

    // Copying is a big nono
    FixedRingBuffer(const FixedRingBuffer& other) noexcept = delete;
    FixedRingBuffer operator= (const FixedRingBuffer& other) noexcept = delete;

    // Moving is a big nono
    FixedRingBuffer(FixedRingBuffer&& other) noexcept = delete;
    FixedRingBuffer& operator=(FixedRingBuffer&& other) noexcept = delete;


    [[nodiscard]] bool try_push(T&& item) noexcept
    {
        if (full()) return false;
        m_data[m_head & (Capacity - 1)] = std::move(item);
        ++m_head;
        return true;
    }
    [[nodiscard]] bool try_pop(T& value) noexcept
    {
        if (empty()) return false;
        value = std::move(m_data[m_tail & (Capacity - 1)]);
        ++m_tail;
        return true;
    }

    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }
    [[nodiscard]] constexpr bool empty() const noexcept { return m_head == m_tail; }
    [[nodiscard]] constexpr bool full() const noexcept { return m_head - m_tail == Capacity; }
    [[nodiscard]] constexpr size_t size() const noexcept { return m_head - m_tail; }


};
