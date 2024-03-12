//
// Created by YongGyu Lee on 2021/07/08.
//
//
// CoordConverterV2 can be used for matrix transformation(Ax + B)
// In SeeSo, this is used for converting camera coordinate (in millimeters) to display coordinate (in pixels)
// Each coordinate is described below
//
// Camera coordinate
//           +y
//            ^
//            |
//            |
//            |
//        (camera) -------> +x
//      (0, 0)
//
//
// Display coordinate (pixels)
// (0, 0) ______________________  -> +x
//        |                    |
//        |      Display       |
//        |                    |
//        |____________________|
//        |
//        v
//       +y
//
//
// You can create a converter manually with a transformation matrix and a translation matrix.
// Or you can use a helper function, makeXXConverter, below the definition of CoordConverterV2<T>
//

#ifndef SEESO_UTIL_COORD_CONVERTER_V2_H_
#define SEESO_UTIL_COORD_CONVERTER_V2_H_

#include "seeso/util/matrix.h"
#include "seeso/util/point.h"

namespace seeso {

/**
 * Do coordinate conversion.
 * M = Am + B or it's reverse
 */
template<typename T = double>
class CoordConverterV2 {
 public:
  using value_type = T;
  using transform_type = Matrix<value_type, 2, 2>;
  using translate_type = Matrix<value_type, 2, 1>;
  using coordinate_type = Matrix<value_type, 2, 1>;

  /** default constructor: R=I, T=Zeros */
  constexpr CoordConverterV2() = default;

  CoordConverterV2(const transform_type& r, const translate_type& t)
    : transform_(r), translate_(t) {}

  /**
   * transformation matrix getter - setters
   */
  CoordConverterV2& transform(const transform_type& t) { // NOLINT(build/include_what_you_use)
    transform_ = t; return *this;
  }

  transform_type& transform()       { return transform_; }
  const transform_type& transform() const { return transform_; }

  /**
   * translation matrix getter - setters
   */
  CoordConverterV2& translate(const translate_type& t) { translate_ = t; return *this; }

  translate_type& translate()       { return translate_; }
  const translate_type& translate() const { return translate_; }

  /**
   * Convert a coordinate
   * @param m target
   * @return Rm + T
   */
  coordinate_type convert(const coordinate_type& m) const {
    return transform_ * m + translate_;
  }


  /**
   * Revert a coordinate
   * @param m target
   * @return R'(m - T), where R` = R^(-1)
   */
  coordinate_type revert(const coordinate_type& m) const {
    return transform_.inv() * (m - translate_);
  }

 private:
  transform_type transform_ = transform_type::eye();
  translate_type translate_ = translate_type::zeros();
};

/**
 * make [camera mm] <-> [display px] converter
 * @tparam T
 * @param display_tl        Relative position of the top-left corner of the display from the camera in millimeters.
 *                          If the camera is centered at the top-center of the display, this value is (-display_mm.x/2, 0).
 *                          In the above case, you can use makeDefaultCameraToDisplayConverter
 * @param display_pixels    Size of the display in pixels
 * @param display_mm        Size of the display in millimeters
 * @return converter
 */
template<typename T>
CoordConverterV2<T> makeCameraToDisplayConverter(
  const Point<float>& display_tl,
  const Point<float>& display_pixels,
  const Point<float>& display_mm) {
  using converter_type = CoordConverterV2<T>;
  using transform_type = typename converter_type::transform_type;
  using translate_type = typename converter_type::translate_type;

  transform_type mm_to_pixel(
    display_pixels.x / display_mm.x, 0,
    0, display_pixels.y / display_mm.y);
  transform_type flip_y(1, 0, 0, -1);

  converter_type converter;
  converter.transform(flip_y * mm_to_pixel);
  converter.translate(converter.transform() * translate_type(-display_tl.x, -display_tl.y));
  return converter;
}

/**
 * Make default [camera mm] <-> [display px] converter.
 * This assumes that the camera is located at the top-center of the display
 * @tparam T
 * @param display_pixels    Size of the display in pixels
 * @param display_mm        Size of the display in millimeters
 * @return converter
 */
template<typename T>
inline CoordConverterV2<T> makeDefaultCameraToDisplayConverter(
  const Point<float>& display_pixels, const Point<float>& display_mm) {
  return makeCameraToDisplayConverter<T>({-display_mm.x / 2, 0}, display_pixels, display_mm);
}

template<typename T>
inline CoordConverterV2<T> makeDefaultCameraToDisplayConverter(
    float px_width, float px_height, float mm_width, float mm_height) {
  return makeCameraToDisplayConverter<T>({-mm_width / 2, 0}, {px_width, px_height}, {mm_width, mm_height});
}

/**
 * Make no-op converter. Output of convert and revert is same with input
 * @tparam T
 * @return converter
 */
template<typename T>
inline CoordConverterV2<T> makeNoOpConverter() {
  return {};
}

} // namespace seeso

#endif // SEESO_UTIL_COORD_CONVERTER_V2_H_
