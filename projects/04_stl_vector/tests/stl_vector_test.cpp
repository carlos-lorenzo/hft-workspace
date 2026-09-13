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
        std::cout << "Copied";
    }
    Vector3& operator=(const Vector3& other)
    {
        if (this == &other) return *this;
        std::cout << "Copied" << std::endl;
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        return *this;
    }

    Vector3(Vector3&& other) noexcept: x_(other.x_), y_(other.y_), z_(other.z_)
    {
        std::cout << "Moved" << std::endl;
        other.x_ = 0;
        other.y_ = 0;
        other.z_ = 0;
    }

    Vector3& operator=(Vector3&& other) noexcept
    {
        if (this == &other) return *this;
        std::cout << "Moved" << std::endl;
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

static std::ostream& operator<<(std::ostream& os, const Vector3& vector)
{
    os << vector.x_ << " " << vector.y_ << " " << vector.z_;
    return os;
}


TEST(STLVectorTest, Constructors) {

    Vector<Vector3> my_vector1;
    EXPECT_EQ(my_vector1.size(), 0);
    EXPECT_EQ(my_vector1.capacity(), 0);

    Vector<Vector3> my_vector2(1);
    EXPECT_EQ(my_vector2.size(), 1);
    EXPECT_EQ(my_vector2.capacity(), 1);

    Vector<Vector3> my_vector3(5, {5, 5, 5});
    EXPECT_EQ(my_vector3.size(), 5);
    EXPECT_EQ(my_vector3.capacity(), 5);
    Vector3 test_vector3 {5, 5, 5};
    EXPECT_TRUE(my_vector3[0] == test_vector3);

    std::cout << my_vector3[0] << std::endl;
}

TEST(STLVectorTest, Reallocation) {
    Vector<float> my_vector1(5, 1);
    my_vector1.push_back(1);
    Vector<float> my_vector2{std::move(my_vector1)};
    std::cout << my_vector2[0] << std::endl;

}

TEST(STLVectorTest, Deletion) {
}