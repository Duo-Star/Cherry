#include <iostream>
#include <cmath>
#include <limits>
#include <iomanip>

struct Vec2
{
    double x, y;

    // --- 常量定义 ---
    static constexpr double EPSILON = 1e-10;
    static const Vec2 ZERO;
    static const Vec2 INF;
    static const Vec2 NAN_VEC;
    static const Vec2 I; // (1, 0)
    static const Vec2 J; // (0, 1)
    static const Vec2 A; // (1, 1)

    // --- 构造函数 ---
    constexpr Vec2() : x(0.0), y(0.0) {}
    constexpr Vec2(double x, double y) : x(x), y(y) {}

    static inline Vec2 from_angle_length(double theta, double l)
    {
        return {std::cos(theta) * l, std::sin(theta) * l};
    }

    // --- 核心几何计算 ---

    // dot
    [[nodiscard]] inline double dot(const Vec2 &other) const
    {
        return x * other.x + y * other.y;
    }

    // cross
    [[nodiscard]] inline double cross(const Vec2 &other) const
    {
        return x * other.y - y * other.x;
    }

    // cross_len
    [[nodiscard]] inline double cross_len(const Vec2 &other) const
    {
        return std::abs(cross(other));
    }

    // pow2
    [[nodiscard]] inline double
    pow2() const
    {
        return x * x + y * y;
    }

    // len
    [[nodiscard]] inline double len() const
    {
        return std::sqrt(pow2());
    }

    // dis
    [[nodiscard]] inline double dis(const Vec2 &p) const
    {
        return (*this - p).len();
    }

    // dis_pow2
    [[nodiscard]] inline double dis_pow2(const Vec2 &p) const
    {
        return (*this - p).pow2();
    }

    // unit
    [[nodiscard]] inline Vec2 unit() const
    {
        double l = len();
        return (l > EPSILON) ? (*this / l) : ZERO;
    }

    // 投影：this 投影到 other 上
    [[nodiscard]] inline Vec2 project_vec(const Vec2 &other) const
    {
        Vec2 other_u = other.unit();
        return other_u * this->dot(other_u);
    }

    // 投影长度：this 在 other 方向上的长度（标量）
    [[nodiscard]] inline double project(const Vec2 &other) const
    {
        return this->dot(other) / other.len();
    }

    // RSV (Resolve Vector) - 分解向量
    // 返回 {lambda, mu} 使得 this = lambda * a + mu * b
    [[nodiscard]] inline std::pair<double, double> rsv(const Vec2 &a, const Vec2 &b) const
    {
        double det = a.cross(b);
        if (std::abs(det) < EPSILON)
        {
            return {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
        }
        double lam = this->cross(b) / det;
        double mu = a.cross(*this) / det;
        return {lam, mu};
    }

    // 判断垂直 / 平行
    [[nodiscard]] inline bool is_vertical(const Vec2 &other) const
    {
        return std::abs(dot(other)) < EPSILON;
    }

    [[nodiscard]] inline bool is_parallel(const Vec2 &other) const
    {
        return cross_len(other) < EPSILON;
    }

    // 角平分线向量
    [[nodiscard]] inline Vec2 angle_bisector(const Vec2 &other) const
    {
        return this->unit() + other.unit();
    }

    // 旋转 90 度 (逆时针)
    [[nodiscard]] inline Vec2 roll90() const
    {
        return {-y, x};
    }

    // 旋转任意角度（弧度）
    [[nodiscard]] inline Vec2 rotate(double theta) const
    {
        double cos_t = std::cos(theta);
        double sin_t = std::sin(theta);
        return {x * cos_t - y * sin_t, x * sin_t + y * cos_t};
    }
    // 计算与 other 的夹角余弦值（-1 ~ 1）
    [[nodiscard]] inline double cos_theta(const Vec2 &other) const
    {
        double den = len() * other.len();
        return (den < EPSILON) ? 0.0 : dot(other) / den;
    }

    // --- 运算符重载 ---

    inline Vec2 operator+(const Vec2 &rhs) const { return {x + rhs.x, y + rhs.y}; }
    inline Vec2 operator-(const Vec2 &rhs) const { return {x - rhs.x, y - rhs.y}; }
    inline Vec2 operator-() const { return {-x, -y}; }

    inline Vec2 operator*(double rhs) const { return {x * rhs, y * rhs}; }
    inline Vec2 operator/(double rhs) const { return {x / rhs, y / rhs}; }

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

    inline bool operator==(const Vec2 &rhs) const
    {
        return std::abs(x - rhs.x) < EPSILON && std::abs(y - rhs.y) < EPSILON;
    }

    // 友元函数：支持 double * Vec2
    friend inline Vec2 operator*(double lhs, const Vec2 &rhs)
    {
        return rhs * lhs;
    }

    // 格式化输出
    friend std::ostream &operator<<(std::ostream &os, const Vec2 &v)
    {
        double out_x = (std::abs(v.x) < EPSILON) ? 0.0 : v.x;
        double out_y = (std::abs(v.y) < EPSILON) ? 0.0 : v.y;
        os << "(" << std::fixed << std::setprecision(4) << out_x << ", " << out_y << ")";
        return os;
    }
};

// 静态常量初始化
inline constexpr Vec2 Vec2::ZERO = {0.0, 0.0};
inline const Vec2 Vec2::INF = {std::numeric_limits<double>::infinity(), std::numeric_limits<double>::infinity()};
inline const Vec2 Vec2::NAN_VEC = {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};
inline constexpr Vec2 Vec2::I = {1.0, 0.0};
inline constexpr Vec2 Vec2::J = {0.0, 1.0};
inline constexpr Vec2 Vec2::A = {1.0, 1.0};