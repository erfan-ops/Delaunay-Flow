#include <delaunay_flow/color_interpolation.hpp>
#include <delaunay_flow/types.hpp>
#include <immintrin.h>
#include <vector>

namespace delaunay_flow {

namespace {

std::vector<Color> colors_global;
Color colors_first;
float colors_size_1;

[[nodiscard]] Color interpolateNone(float) noexcept {
    return {0, 0, 0, 1};
}

[[nodiscard]] Color interpolateSingle(float) noexcept {
    return colors_first;
}

[[nodiscard]] Color interpolateMulti(float t) noexcept {
    if (t <= 0.0f) return colors_global.front();
    if (t >= 1.0f) return colors_global.back();

    const float scaled = t * colors_size_1;
    const int index = static_cast<int>(scaled);
    const float local_t = scaled - index;

    const Color& c1 = colors_global[static_cast<size_t>(index)];
    const Color& c2 = colors_global[static_cast<size_t>(index) + 1];

    __m128 v1 = _mm_loadu_ps(c1.data());
    __m128 v2 = _mm_loadu_ps(c2.data());
    __m128 tvec = _mm_set1_ps(local_t);

    __m128 result = _mm_fmadd_ps(_mm_sub_ps(v2, v1), tvec, v1);

    Color out;
    _mm_storeu_ps(out.data(), result);
    return out;
}

static Color (*interpolateFn)(float) = interpolateNone;

}  // namespace

void initInterpolation(const std::vector<Color>& colors) {
    colors_global = colors;
    if (colors.empty()) {
        interpolateFn = interpolateNone;
    } else if (colors.size() == 1) {
        colors_first = colors.front();
        interpolateFn = interpolateSingle;
    } else {
        colors_size_1 = static_cast<float>(colors.size() - 1);
        interpolateFn = interpolateMulti;
    }
}

Color interpolate(float t) {
    return interpolateFn(t);
}

}  // namespace delaunay_flow
