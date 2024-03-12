#ifndef SEESO_UTIL_POINT_H_
#define SEESO_UTIL_POINT_H_

#include <tuple>
#include <ostream>
#include <sstream>

namespace seeso {

template<typename T>
struct Point {
  constexpr Point() = default;
  constexpr Point(T x, T y) : x(x), y(y) {}

  template<typename T2>
  constexpr Point(const Point<T2>& other)
    : x(static_cast<T>(other.x)),
      y(static_cast<T>(other.y)) {}

  template<typename Tuple>
  explicit Point(Tuple tp) : x(std::get<0>(tp)), y(std::get<1>(tp)) {
    static_assert(std::tuple_size<Tuple>::value == 2, "Tuple size must be 2");
  }

  T x = 0;
  T y = 0;
};


template<typename T>
std::ostream& operator<<(std::ostream& os, const Point<T>& pt) {
  std::stringstream ss;
  ss << "( " << pt.x << ", " << pt.y << " )";
  os << ss.str();
  return os;
}

} // namespace seeso

#endif // SEESO_UTIL_POINT_H_
