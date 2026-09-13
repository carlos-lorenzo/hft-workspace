#pragma once
#include <expected>

template <typename T>
class Vector
{
public:
    Vector() noexcept : size_(0), capacity_(0), data_(nullptr)
    {

    }

    explicit Vector(const size_t size) : size_(size), capacity_(size), data_(new T[capacity_])
    {

    }
    explicit Vector(const size_t size, const T& default_value) : size_(size), capacity_(size), data_(new T[capacity_])
    {
        std::fill(data_, data_ + size_, default_value); // Already default constructed as new was called
    }
    explicit Vector(std::initializer_list<T> elements) : size_(elements.size()), capacity_(elements.size()), data_(new T[elements.size()])
    {
        std::copy_n(elements.begin(), size_, data_);
    }

    // Move, copy constructors and destructor
    Vector (const Vector& other) : size_(other.size_), capacity_(other.size_), data_(new T[capacity_])
    {
        std::copy_n(other.data_, size_, data_);
    }
    Vector (Vector&& other) noexcept : size_(other.size_), capacity_(other.size_), data_(other.data_)
    {
        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }
    Vector& operator=(const Vector& other)
    {
        if (this == &other) return *this;

        if (capacity_ < other.size_)
        {
            delete[] data_;
            capacity_ = other.size_;
            data_ = new T[capacity_];
        }

        size_ = other.size_;
        std::copy_n(other.data_, other.size_, data_);
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept
    {
        if (this == &other) return *this;

        delete[] data_;
        capacity_ = other.size_;
        data_ = other.data_;
        size_ = other.size_;

        other.data_ = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;

        return *this;
    }

    ~Vector() noexcept
    {
        delete[] data_;
        capacity_ = 0;
        size_ = 0;
    }

    [[nodiscard]] size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] T* begin() const noexcept { return data_; }
    [[nodiscard]] T* end() const noexcept { return data_ + size_; }
    [[nodiscard]] T& operator[](size_t index) const noexcept { return data_[index]; }
    [[nodiscard]] T& at(size_t index)
    {
        if (index >= size_) return nullptr;
        return data_[index];
    }
    void pop_back() noexcept
    {
        std::destroy_at(std::prev(end()));
        --size_;
    }

    void push_back(const T& element)
    {
        if (size_ == capacity_) {
            if (!resize()) throw std::bad_alloc();
        }
        data_[size_++] = element;
    }

    void push_back(T&& element)
    {
        if (size_ == capacity_) {
            if (!resize()) throw std::bad_alloc();
        }
        data_[size_++] = std::move(element);
    }

    template< class... Args >
    void emplace_back( Args&&... args )
    {
        if (size_ == capacity_) {
            if (!resize()) throw std::bad_alloc();
        }
        std::construct_at(data_[size_], std::forward<Args>(args)...);
        ++size_;
    }

    void reserve(const size_t capacity)
    {
        auto new_capacity = capacity;
        auto new_data = new T[new_capacity];
        std::move_iterator<T>(data_, size_, new_data);
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }


private:
    size_t size_;
    size_t capacity_;
    T* data_;

    bool resize()
    {
        auto new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
        auto new_data = new (std::nothrow) T[new_capacity];
        if (new_data == nullptr) return false;
        std::copy_n(data_, size_, new_data);
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
        return true;
    }

};
