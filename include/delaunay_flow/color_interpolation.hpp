#ifndef DELAUNAY_FLOW_COLOR_INTERPOLATION_HPP
#define DELAUNAY_FLOW_COLOR_INTERPOLATION_HPP

#include "types.hpp"
#include <vector>

namespace delaunay_flow {

/** Initialize interpolation based on number of colors (0 = none, 1 = single, 2+ = gradient). */
void initInterpolation(const std::vector<Color>& colors);

/** Interpolate color at t in [0, 1]. Set by initInterpolation(). */
Color interpolate(float t);

}  // namespace delaunay_flow

#endif  // DELAUNAY_FLOW_COLOR_INTERPOLATION_HPP
