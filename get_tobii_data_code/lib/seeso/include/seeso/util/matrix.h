//
// Created by YongGyu Lee on 2021/07/08.
//
// Matrix for CoordConverter
// Supports addition, substitution,
// scalar multiplication, matrix multiplication,
// inverse(limited), determinant(limited)

#ifndef SEESO_UTIL_MATRIX_H_
#define SEESO_UTIL_MATRIX_H_

#include <cstddef>

#include <ostream>
#include <type_traits>

namespace seeso {

template<typename ...T>
struct always_false { static constexpr bool value = false; };

template<typename T, int m, int n> class MatrixView;
template<typename T, int m, int n> class Matrix;
struct MatrixTagAll { int unused; };

template<typename M>
struct MatrixTraits {
  static_assert(always_false<M>::value, "Type is not a Matrix type");
};

template<typename M, int = MatrixTraits<M>::rows, int = MatrixTraits<M>::cols>
struct MatrixDeterminant {
  static_assert(always_false<M>::value, "Type must be square matrix");
};

template<typename M, int a = MatrixTraits<M>::rows, int b = MatrixTraits<M>::cols>
struct MatrixInvert {
  static_assert(a <= 2 && b <= 2, "Only matrix of under 2X2 is supported by now");
  static_assert(a == b, "Matrix must be square");
};



/**
 * Matrix
 * @tparam T
 * @tparam m
 * @tparam n
 */

template<typename T, int m, int n>
struct MatrixTraits<Matrix<T, m, n>> {
  enum { rows = m, cols = n, size = m * m };
  using value_type = T;
  using matrix_type = Matrix<T, m, n>;
};

template<typename T, int m, int n>
class Matrix {
 public:
  using matrix_type = Matrix<T, m, n>;
  using traits = MatrixTraits<matrix_type>;
  using value_type = typename traits::value_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  enum {
    rows = traits::rows,
    cols = traits::cols,
    size = traits::size
  };

  constexpr Matrix() : data_{} {}
  constexpr explicit Matrix(T v0) : data_{v0} {}
  constexpr Matrix(T v0, T v1) : data_{v0, v1} {}
  constexpr Matrix(T v0, T v1, T v2) : data_{v0, v1, v2} {}
  constexpr Matrix(T v0, T v1, T v2, T v3) : data_{v0, v1, v2, v3} {}
  Matrix(T val, MatrixTagAll) : data_{} {
    for(int i = 0; i < m*n; ++i) data_[i] = val;
  }

  template<typename T2, typename std::enable_if<!std::is_same<value_type, T2>::value, int>::type = 0>
  explicit Matrix(const Matrix<T2, m, n>& other) {
    for(int i = 0; i < m*n; ++i) data_[i] = static_cast<value_type>(other(i));
  }


  static matrix_type zeros() { return matrix_type{static_cast<T>(0), MatrixTagAll{}}; }
  static matrix_type ones()  { return matrix_type{static_cast<T>(1), MatrixTagAll{}}; }

  static matrix_type eye() {
    matrix_type result;
    for(int i = 0; i < rows; ++i)
      result(i, i) = 1;
    return result;
  }

  reference operator[](std::size_t i)       { return data_[i]; }
  const_reference operator[](std::size_t i) const { return data_[i]; }

  reference operator()(std::size_t i)       { return data_[i]; }
  const_reference operator()(std::size_t i) const { return data_[i]; }
  reference operator()(std::size_t i, std::size_t j)       { return data_[i*cols + j]; }
  const_reference operator()(std::size_t i, std::size_t j) const { return data_[i*cols + j]; }

  Matrix<T, n, m> transpose() const {
    Matrix<T, n, m> result;
    for(int i = 0; i < n; ++i) {
      for(int j = 0; j < m; ++j) {
        result(i, j) = this->operator()(j, i);
      }
    }
    return result;
  }

  matrix_type inv() const {
    return MatrixInvert<matrix_type>{}(*this);
  }

  value_type det() const {
    return MatrixDeterminant<matrix_type>{}(*this);
  }

  value_type data_[m*n]; // NOLINT(runtime/arrays)
};

/**
 * MatrixView
 * @tparam T
 * @tparam m
 * @tparam n
 */

template<typename T, int m, int n>
struct MatrixTraits<MatrixView<T, m, n>> {
  enum { rows = m, cols = n, size = m * m };
  using value_type = T;
  using matrix_type = MatrixView<T, m, n>;
};

template<typename T, int m, int n>
class MatrixView {
 public:
  using matrix_type = MatrixView<T, m, n>;
  using traits = MatrixTraits<matrix_type>;
  using value_type = typename traits::value_type;
  using reference = value_type&;
  using const_reference = const value_type&;
  enum {
    rows = traits::rows,
    cols = traits::cols,
    size = traits::size
  };

  explicit MatrixView(const Matrix<T, m, n>& matrix, int offset = 0, int step = 0)
    : data_(matrix.data_), offset_(offset), step_(step) {}

  const_reference operator[](std::size_t i) const {
    return data_[offset_ + cols * (i / step_) + (i % step_)];
  }
  const_reference operator()(std::size_t i) const {
    return data_[offset_ + cols * (i / step_) + (i % step_)];
  }
  const_reference operator()(std::size_t i, std::size_t j) const {
    return data_[offset_ + cols * j + i];
  }

 private:
  int offset_ = 0;
  int step_ = 0;
  T* data_ = nullptr;
};


/**
 * MatrixDeterminant
 * @tparam M
 */
template<typename M>
struct MatrixDeterminant<M, 1, 1> {
  typename M::value_type operator()(const M& m) const noexcept { return m(0); }
};

template<typename M>
struct MatrixDeterminant<M, 2, 2> {
  typename M::value_type operator()(const M& m) const noexcept { return m(0)*m(3) - m(1)*m(2); }
};

template<typename M, int n>
struct MatrixDeterminant<M, n, n> {
  static_assert(always_false<M>::value, "Only matrix of under 2X2 is supported by now");
  using value_type = typename M::value_type;
  value_type operator()(const M& m_) const {
    int flag = 1;
    value_type sum = 0;
    MatrixDeterminant<MatrixView<value_type, n-1, n-1>> det(m_);
    for(int i = 0; i < n; ++i) {
//      sum += flag * det(); not yet implemented
      flag = -flag;
    }
    return sum;
  }
};

/**
 * MatrixInvert
 * @tparam M
 */
template<typename M>
struct MatrixInvert<M, 1, 1> {
  using matrix_type = Matrix<typename M::value_type, 1, 1>;
  matrix_type operator()(const M& m) const noexcept { return m; }
};

template<typename M>
struct MatrixInvert<M, 2, 2> {
  using value_type = typename M::value_type;
  using matrix_type = Matrix<value_type, 2, 2>;
  using common_type = typename std::common_type<value_type, double>::type;
  static constexpr MatrixDeterminant<matrix_type> det {};

  matrix_type operator()(const M& m) const noexcept {
    const auto det_inv = static_cast<common_type>(1.) / static_cast<common_type>(MatrixDeterminant<matrix_type>{}(m));
    return {
      static_cast<value_type>(det_inv * static_cast<common_type>(m(3))),
      static_cast<value_type>(-det_inv * static_cast<common_type>(m(1))),
      static_cast<value_type>(-det_inv * static_cast<common_type>(m(2))),
      static_cast<value_type>(det_inv * static_cast<common_type>(m(0)))
    };
  }
};

/**
 * equal comparison
 * @param lhs
 * @param rhs
 * @return value
 */
template<typename T, int m, int n>
bool operator == (const Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) {
  for(int i = 0; i < m*n; ++i)
    if (lhs(i) != rhs(i)) return false;
  return true;
}

template<typename T, int m, int n>
bool operator != (const Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) {
  for(int i = 0; i < m*n; ++i)
    if (lhs(i) == rhs(i)) return false;
  return true;
}

/**
 * arithmetic operations
 */
template<typename T, int m, int n>
Matrix<T, m, n> operator + (const Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) {
  Matrix<T, m, n> result;
  for(int i = 0; i < m*n; ++i) {
    result(i) = lhs(i) + rhs(i);
  }
  return result;
}

template<typename T, int m, int n>
Matrix<T, m, n>& operator += (Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) { // NOLINT(runtime/references)
  for(int i = 0; i < m*n; ++i) {
    lhs(i) += rhs(i);
  }
  return lhs;
}


template<typename T, int m, int n>
Matrix<T, m, n> operator - (const Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) {
  Matrix<T, m, n> result;
  for(int i = 0; i < m*n; ++i) {
    result(i) = lhs(i) - rhs(i);
  }
  return result;
}

template<typename T, int m, int n>
Matrix<T, m, n>& operator -= (Matrix<T, m, n>& lhs, const Matrix<T, m, n>& rhs) { // NOLINT(runtime/references)
  for(int i = 0; i < m*n; ++i) {
    lhs(i) -= rhs(i);
  }
  return lhs;
}

template<typename T, int m, int n, int l>
Matrix<T, m, n> operator * (const Matrix<T, m, l>& lhs, const Matrix<T, l, n>& rhs) {
  Matrix<T, m, n> result;
  for( int i = 0; i < m; i++ ) {
    for (int j = 0; j < n; j++) {
      T sum = 0;
      for (int k = 0; k < l; k++)
        sum += lhs(i, k) * rhs(k, j);
      result(i, j) = sum;
    }
  }
  return result;
}

template<typename T, int m, int n, typename U>
typename std::enable_if<std::is_arithmetic<U>::value, Matrix<T, m, n>>::type
operator * (const Matrix<T, m, n>& lhs, U num) {
  using common_type = typename std::common_type<T, U>::type;
  Matrix<T, m, n> result;
  for(int i = 0; i < m*n; ++i)
    result(i) = static_cast<T>(static_cast<common_type>(lhs(i)) * static_cast<common_type>(num));
  return result;
}

template<typename T, int m, int n, int l>
Matrix<T, m, n> operator *= (Matrix<T, m, l>& lhs, const Matrix<T, l, n>& rhs) { // NOLINT(runtime/references)
  Matrix<T, m, n> temp(lhs);
  for( int i = 0; i < m; i++ ) {
    for (int j = 0; j < n; j++) {
      T sum = 0;
      for (int k = 0; k < l; k++)
        sum += temp(i, k) * rhs(k, j);
      lhs(i, j) = sum;
    }
  }
}

template<typename T, int m, int n, typename U>
typename std::enable_if<std::is_arithmetic<U>::value, Matrix<T, m, n>>::type
operator *= (Matrix<T, m, n>& lhs, U num) { // NOLINT(runtime/references)
  using common_type = typename std::common_type<T, U>::type;
  for(int i = 0; i < m*n; ++i)
    lhs(i) = static_cast<T>(static_cast<common_type>(lhs(i)) * static_cast<common_type>(num));
  return lhs;
}

template<typename T, int m, int n>
std::ostream& operator<<(std::ostream& os, const Matrix<T, m, n>& matrix) {
  os << "[";
  for(int i = 0; i < matrix.rows; ++i) {
    if (matrix.cols > 0)
      os << matrix(i, 0);
    for(int j = 1; j < matrix.cols; ++j) {
      os << ", " << matrix(i, j);
    }
    os << ";";
  }
  os << "]";
  return os;
}

} // namespace seeso

#endif // SEESO_UTIL_MATRIX_H_
