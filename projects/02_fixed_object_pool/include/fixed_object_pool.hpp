#pragma once


#include <cstddef>
#include <cstdlib>
#include <memory>

template <typename T, std::size_t Capacity>
class alignas(64) FixedObjectPool {
private:
    struct Node { Node* next; };

    union Slot { Node* next; alignas(alignof(T)) std::byte storage[sizeof(T)]; };

    Slot* m_first_free;

    Slot* m_slots;

public:
    constexpr FixedObjectPool() noexcept
    {
        m_slots = static_cast<Slot*>(std::aligned_alloc(alignof(Slot), Capacity * sizeof(Slot)));
        m_first_free = m_slots;

        for (Slot* slot = m_slots; slot < m_slots + Capacity - 1; ++slot)
        {
            slot->next = reinterpret_cast<Node*>(slot + 1);
        }
        (m_slots + Capacity - 1)->next = nullptr;
    }
    ~FixedObjectPool() noexcept
    {
        std::free(m_slots);
        m_slots = nullptr;
        m_first_free = nullptr;
    }

    FixedObjectPool(const FixedObjectPool&) = delete;
    FixedObjectPool& operator=(const FixedObjectPool&) = delete;
    FixedObjectPool(FixedObjectPool&&) = delete;
    FixedObjectPool& operator=(FixedObjectPool&&) = delete;

    template <typename... Args>
    [[nodiscard]] T* allocate(Args&&... args) noexcept
    {
        if (full()) return nullptr;
        Slot* current_free = m_first_free;
        m_first_free = reinterpret_cast<Slot*>(current_free->next);
        return std::construct_at(reinterpret_cast<T*>(&current_free->storage), std::forward<Args>(args)...);
    }

    void deallocate(T* ptr) noexcept
    {
        // Check if ptr holding T (not done, currently assuming address is valid)
        auto freed_node = reinterpret_cast<Slot*>(ptr);
        std::destroy_at(ptr);
        freed_node->next = reinterpret_cast<Node*>(m_first_free);
        m_first_free = freed_node;
    }

    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return Capacity; }
    [[nodiscard]] bool full() const noexcept { return m_first_free == nullptr; }
    // [[nodiscard]] std::size_t available() const noexcept;
};
