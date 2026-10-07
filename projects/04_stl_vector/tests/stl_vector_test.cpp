#include <gtest/gtest.h>
#include <stl_vector.hpp>
#include <print>
#include <cstddef>



struct Vector3
{
    float x_, y_, z_;
    Vector3() : x_(0), y_(0), z_(0) {}
    Vector3(float x, float y, float z) : x_(x), y_(y), z_(z) {}
    Vector3(const Vector3& other) : x_(other.x_), y_(other.y_), z_(other.z_)
    {
        std::cout << "Copy constructor" << std::endl;
    }
    Vector3& operator=(const Vector3& other)
    {
        if (this == &other) return *this;
        std::cout << "Copy assignment" << std::endl;
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        return *this;
    }

    Vector3(Vector3&& other) noexcept: x_(other.x_), y_(other.y_), z_(other.z_)
    {
        std::cout << "Move constructor" << std::endl;
        other.x_ = 0;
        other.y_ = 0;
        other.z_ = 0;
    }

    Vector3& operator=(Vector3&& other) noexcept
    {
        if (this == &other) return *this;
        std::cout << "Move assignment" << std::endl;
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;

        other.x_ = 0;
        other.y_ = 0;
        other.z_ = 0;

        return *this;
    }

    // Vector3(std::initializer_list<float> list) : x_(list.begin), y_(list.begin()), z_(list.begin());



};

static bool operator==(const Vector3& lhs, const Vector3& rhs)
{
    return lhs.x_ == rhs.x_ && lhs.y_ == rhs.y_ && lhs.z_ == rhs.z_;
}

// static std::ostream& operator<<(std::ostream& os, const Vector3& vector)
// {
//     os << vector.x_ << " " << vector.y_ << " " << vector.z_;
//     return os;
// }


TEST(STLVectorTest, DefaultConstructor) {

    Vector<Vector3> vector;
    EXPECT_EQ(vector.size(), 0);
}

TEST(STLVectorTest, SizedConstruction) {
    Vector<Vector3> vector(5, {1, 2, 3});
    EXPECT_EQ(vector.size(), 5);
    EXPECT_EQ(vector.capacity(), 5);
    EXPECT_EQ(vector[0], Vector3(1, 2, 3));
}

TEST(STLVectorTest, InitializerListConstruction) {
    Vector<Vector3> vector{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    EXPECT_EQ(vector.size(), 3);
    EXPECT_EQ(vector.capacity(), 3);
    EXPECT_EQ(vector[0], Vector3(1, 2, 3));
    EXPECT_EQ(vector[1], Vector3(4, 5, 6));
    EXPECT_EQ(vector[2], Vector3(7, 8, 9));
}

TEST(STLVectorTest, Semantics) {
    Vector<Vector3> vector;
    vector.reserve(5);
    vector.emplace_back(1, 2, 3);
    EXPECT_EQ(vector.size(), 1);
    EXPECT_EQ(vector.capacity(), 5);
    auto vector2(vector);
    EXPECT_EQ(vector2.size(), 1);
    EXPECT_EQ(vector2.capacity(), 1);


    auto vector3(std::move(vector2));
    EXPECT_EQ(vector3.size(), 1);
    EXPECT_EQ(vector3.capacity(), 1);
}

TEST(STLVectorTest, Assignment) {
    Vector<Vector3> vector;
    vector.reserve(5);
    vector.emplace_back(1, 2, 3);
    EXPECT_EQ(vector.size(), 1);
    EXPECT_EQ(vector.capacity(), 5);
    EXPECT_EQ(vector[0], Vector3(1, 2, 3));

    auto vector2 = std::move(vector);
    EXPECT_EQ(vector2.size(), 1);
    EXPECT_EQ(vector2.capacity(), 5);
}

TEST(STLVectorTest, Resize) {
    Vector<Vector3> vector;
    vector.reserve(5);
    vector.emplace_back(1, 2, 3);
    EXPECT_EQ(vector.size(), 1);
    EXPECT_EQ(vector.capacity(), 5);
    EXPECT_EQ(vector[0], Vector3(1, 2, 3));
    vector.shrink_to_fit();
    EXPECT_EQ(vector.size(), 1);

}