#ifndef MATH_FOREST_VEC2_HPP
#define MATH_FOREST_VEC2_HPP

#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>
#include <utility>

namespace mf
{

    struct Vec2
    {
        double x, y;

        // --- 常量 ---
        static constexpr double EPSILON = 1e-10;

        // --- 构造函数 ---
        constexpr Vec2() : x(0.0), y(0.0) {}
        constexpr Vec2(double x, double y) : x(x), y(y) {}

        static inline Vec2 from_angle_length(double theta, double l)
        {
            return {std::cos(theta) * l, std::sin(theta) * l};
        }

        // --- 核心几何计算 ---
        [[nodiscard]] inline double dot(const Vec2 &other) const { return x * other.x + y * other.y; }
        // 计算叉积（标量值，表示平行四边形的有向面积）
        [[nodiscard]] inline double cross(const Vec2 &other) const { return x * other.y - y * other.x; }
        // 计算叉积的绝对值（即平行四边形面积）
        [[nodiscard]] inline double cross_len(const Vec2 &other) const { return std::abs(cross(other)); }
        // 计算长度的平方（避免不必要的开方运算）
        [[nodiscard]] inline double pow2() const { return x * x + y * y; }
        // 计算长度
        [[nodiscard]] inline double len() const { return std::sqrt(pow2()); }
        // 计算与另一个点的距离
        [[nodiscard]] inline double dis(const Vec2 &p) const { return (*this - p).len(); }

        // 返回单位向量
        [[nodiscard]] inline Vec2 unit() const
        {
            double l = len();
            return (l > EPSILON) ? (*this / l) : Vec2{0, 0};
        }

        // 分解向量 lambda * a + mu * b
        [[nodiscard]] inline std::pair<double, double> rsv(const Vec2 &a, const Vec2 &b) const
        {
            double det = a.cross(b);
            if (std::abs(det) < EPSILON)
            {
                return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
            }
            return {this->cross(b) / det, a.cross(*this) / det};
        }

        // --- 运算符重载 (Inline 以保证性能) ---
        inline Vec2 operator+(const Vec2 &rhs) const { return {x + rhs.x, y + rhs.y}; }
        inline Vec2 operator-(const Vec2 &rhs) const { return {x - rhs.x, y - rhs.y}; }
        inline Vec2 operator-() const { return {-x, -y}; }
        inline Vec2 operator*(double rhs) const { return {x * rhs, y * rhs}; }
        inline Vec2 operator/(double rhs) const { return {x / rhs, y / rhs}; }
        // 复合赋值运算符
        inline Vec2 &operator+=(const Vec2 &rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
        inline Vec2 &operator-=(const Vec2 &rhs)
        {
            x -= rhs.x;
            y -= rhs.y;
            return *this;
        }
        inline Vec2 &operator*=(double rhs)
        {
            x *= rhs;
            y *= rhs;
            return *this;
        }
        inline Vec2 &operator/=(double rhs)
        {
            x /= rhs;
            y /= rhs;
            return *this;
        }
        // 近似相等比较（考虑浮点误差）
        inline bool operator==(const Vec2 &rhs) const
        {
            return std::abs(x - rhs.x) < EPSILON && std::abs(y - rhs.y) < EPSILON;
        }
    };

    // 常用常量定义 (C++17 inline variables 允许在头文件中直接初始化静态成员)
    inline constexpr Vec2 VEC2_ZERO = {0.0, 0.0};
    inline constexpr Vec2 VEC2_I = {1.0, 0.0};
    inline constexpr Vec2 VEC2_J = {0.0, 1.0};

    // 友元运算符：支持 double * Vec2
    inline Vec2 operator*(double lhs, const Vec2 &rhs) { return rhs * lhs; }

    // 格式化输出
    inline std::ostream &operator<<(std::ostream &os, const Vec2 &v)
    {
        double out_x = (std::abs(v.x) < Vec2::EPSILON) ? 0.0 : v.x;
        double out_y = (std::abs(v.y) < Vec2::EPSILON) ? 0.0 : v.y;
        return os << "(" << std::fixed << std::setprecision(4) << out_x << ", " << out_y << ")";
    }

} // namespace geometry

#endif // MATH_FOREST_VEC2_HPP
