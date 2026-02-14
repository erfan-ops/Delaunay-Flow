#pragma once

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <utility>
#include <vector>

namespace delaunator {

//@see https://stackoverflow.com/questions/33333363/built-in-mod-vs-custom-mod-function-improve-the-performance-of-modulus-op/33333636#33333636
inline size_t fast_mod(const size_t i, const size_t c);

// Kahan and Babuska summation, Neumaier variant; accumulates less FP error
inline double sum(const std::vector<double>& x);

inline double dist(
    const double ax,
    const double ay,
    const double bx,
    const double by
);

inline double circumradius(
    const double ax,
    const double ay,
    const double bx,
    const double by,
    const double cx,
    const double cy
);

inline bool orient(
    const double px,
    const double py,
    const double qx,
    const double qy,
    const double rx,
    const double ry
);

inline std::pair<double, double> circumcenter(
    const double ax,
    const double ay,
    const double bx,
    const double by,
    const double cx,
    const double cy
);

struct compare {
    std::vector<double> const& coords;
    double cx;
    double cy;

    bool operator()(std::size_t i, std::size_t j);
};

constexpr double EPSILON = std::numeric_limits<double>::epsilon();

inline bool in_circle(
    const double ax,
    const double ay,
    const double bx,
    const double by,
    const double cx,
    const double cy,
    const double px,
    const double py
);

constexpr std::size_t INVALID_INDEX = std::numeric_limits<std::size_t>::max();

inline bool check_pts_equal(double x1, double y1, double x2, double y2);

// monotonically increases with real angle, but doesn't need expensive trigonometry
inline double pseudo_angle(const double dx, const double dy);

struct DelaunatorPoint {
    std::size_t i;
    double x;
    double y;
    std::size_t t;
    std::size_t prev;
    std::size_t next;
    bool removed;
};

class Delaunator {
public:
    std::vector<double> const& coords;
    std::vector<std::size_t> triangles;
    std::vector<std::size_t> halfedges;
    std::vector<std::size_t> hull_prev;
    std::vector<std::size_t> hull_next;
    std::vector<std::size_t> hull_tri;
    std::size_t hull_start;

    Delaunator(std::vector<double> const& in_coords);
    ~Delaunator() = default;

    // Prevent copying (due to reference member)
    Delaunator(const Delaunator&) = delete;
    Delaunator& operator=(const Delaunator&) = delete;

    // Allow moving
    Delaunator(Delaunator&&) = default;
    Delaunator& operator=(Delaunator&&) = default;

    double get_hull_area();

private:
    std::vector<std::size_t> m_hash;
    double m_center_x;
    double m_center_y;
    std::size_t m_hash_size;
    std::vector<std::size_t> m_edge_stack;

    std::size_t legalize(std::size_t a);
    std::size_t hash_key(double x, double y) const;
    std::size_t add_triangle(
        std::size_t i0,
        std::size_t i1,
        std::size_t i2,
        std::size_t a,
        std::size_t b,
        std::size_t c);
    void link(std::size_t a, std::size_t b);
};

} // namespace delaunator
