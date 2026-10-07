#pragma once
#include <memory>

template <typename T, typename Alloc = std::allocator<T>>
class Vector
{
    using Traits = std::allocator_traits<Alloc>;
    using pointer = Traits::pointer;
public:
    Vector() = default;
    explicit Vector(std::size_t size) : data_(Traits::allocate(allocator_, size)), size_(size), capacity_(size) {
        std::uninitialized_fill_n(data_, size_, {0, 0, 0});
    }
    explicit Vector(std::size_t size, const T& default_value) : data_(Traits::allocate(allocator_, size)), size_(size), capacity_(size) {
        std::uninitialized_fill_n(data_, size_, default_value);
    }
    explicit Vector(std::initializer_list<T> list) : data_(Traits::allocate(allocator_, list.size())), size_(list.size()), capacity_(list.size()) {
        std::uninitialized_copy_n(list.begin(), list.size(), data_);
    }

    // Move and Copy constructors
    Vector(const Vector& other) {

        pointer new_data = Traits::allocate(allocator_, other.size_);
        std::uninitialized_copy_n(other.data_, other.size_, new_data);

        clear();
        Traits::deallocate(allocator_, data_, capacity_);

        data_ = new_data;
        size_ = other.size_;
        capacity_ = other.size_;
        allocator_ = other.allocator_;
    }

    Vector(Vector&& other) noexcept
     : allocator_(std::move(other.allocator_)),
       data_(other.data_),
       size_(other.size_),
       capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    Vector& operator=(const Vector& other) {
        if (this == &other) { return *this; }


        pointer new_data = Traits::allocate(allocator_, other.size_);
        std::uninitialized_copy_n(other.data_, other.size_, new_data);

        clear();
        Traits::deallocate(allocator_, data_, capacity_);

        data_ = new_data;
        size_ = other.size_;
        capacity_ = other.size_;


        allocator_ = other.allocator_;
        return *this;
    }

    Vector& operator=(Vector&& other) noexcept {
        if (this == &other) { return *this; }

        clear();
        Traits::deallocate(allocator_, data_, capacity_);

        data_ = other.data_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        allocator_ = other.allocator_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
        other.allocator_ = std::allocator<T>();

        return *this;
    }

    // Destructor
    ~Vector() {
        Traits::deallocate(allocator_, data_, capacity_);
        data_ = nullptr;
        capacity_ = 0;
        size_ = 0;
    }

    [[nodiscard]] T& operator[](std::size_t index) {
        return data_[index];
    }
    [[nodiscard]] const T& operator[](std::size_t index) const {
        return data_[index];
    }
    [[nodiscard]] T& at(std::size_t index) {
        if (index >= size_) throw std::out_of_range("");
        return data_[index];
    }
    [[nodiscard]] const T& at(std::size_t index) const {
        if (index >= size_) throw std::out_of_range("");
        return data_[index];
    }


    [[nodiscard]] std::size_t size() const { return size_; }
    [[nodiscard]] std::size_t capacity() const { return capacity_; }

    [[nodiscard]] pointer data() { return data_; }
    [[nodiscard]] const pointer data() const { return data_; }

    [[nodiscard]] pointer begin() { return data_; }
    [[nodiscard]] const pointer cbegin() const { return data_; }
    [[nodiscard]] pointer end() { return data_ + size_; }
    [[nodiscard]] const pointer cend() const { return data_ + size_; }

    [[nodiscard]] std::size_t empty() const { return size_ == 0; }
    [[nodiscard]] Alloc get_allocator() const { return allocator_; }

    [[nodiscard]] T& front() { return data_[0]; }
    [[nodiscard]] const T& front() const { return data_[0]; }
    [[nodiscard]] T& back() { return data_[size_ - 1]; }
    [[nodiscard]] const T& back() const { return data_[size_ - 1]; }

    void reserve(std::size_t new_capacity) {
        if (new_capacity <= capacity_) return;
        pointer new_data = Traits::allocate(allocator_, new_capacity);

        // 2. Safely move/copy elements from old memory to new_data
        try {
            for (std::size_t i = 0; i < size_; ++i) {
                Traits::construct(allocator_, new_data + i, std::move(data_[i]));
            }
        } catch (...) {
            // Clean up if construction fails midway
            Traits::deallocate(allocator_, new_data, new_capacity);
            throw; // Rethrow to let the user know insertion failed
        }

        // 3. Clean up the old active memory
        for (std::size_t i = 0; i < size_; ++i) {
            Traits::destroy(allocator_, data_ + i);
        }
        Traits::deallocate(allocator_, data_, capacity_);

        // 4. Commit the change to the class state only AFTER everything succeeds
        data_ = new_data;
        capacity_ = new_capacity;
    }

    // Copy into vector
    void push_back(const T& value) {
        resize_if_needed();
        Traits::construct(allocator_, data_ + size_, value);
        ++size_;
    }

    // Move into vector
    void push_back(T&& value) {
        resize_if_needed();
        Traits::construct(allocator_, data_ + size_, std::move(value));
        ++size_;
    }

    template< class... Args >
    void emplace_back( Args&&... args ) {
        resize_if_needed();
        Traits::construct(allocator_, data_ + size_, std::forward<Args>(args)...);
        ++size_;
    }

    void pop_back() {
        if (size_ <= 0) throw std::out_of_range(""); 
        Traits::destroy(allocator_, data_ + size_ - 1);
        --size_;
    }

    void clear() {
        for (std::size_t i = 0; i < size_; ++i) {
            Traits::destroy(allocator_, data_ + i);
        }
        size_ = 0;
    }

    void shrink_to_fit() {
        reserve(size_);
    }


private:
    void resize_if_needed() {
        if (size_ == capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
    }



    Alloc allocator_;
    pointer data_ = nullptr;
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;

};
