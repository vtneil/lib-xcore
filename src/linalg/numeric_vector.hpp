#ifndef LIB_XCORE_LINALG_NUMERIC_VECTOR_HPP
#define LIB_XCORE_LINALG_NUMERIC_VECTOR_HPP

#include "core/basic_iterator.hpp"
// #include "container/array.hpp"
//
//
// namespace math {
//   namespace detail {
//     template<typename T, size_t Size, size_t, size_t I>
//     void helper_assign_to(const T (&array)[Size], T &var) { var = array[I]; }
//
//     template<typename T, size_t Size, size_t IMax, size_t I, typename... Ts>
//     void helper_assign_to(const T (&array)[Size], T &var, Ts &...vars) {
//       helper_assign_to<T, Size, IMax, I>(array, var);
//       if (I < IMax) helper_assign_to<T, Size, IMax, I + 1>(array, vars...);
//     }
//
//     template<typename T, size_t Size, size_t, size_t I>
//     void helper_assign_from(T (&array)[Size], const T &var) { array[I] = var; }
//
//     template<typename T, size_t Size, size_t IMax, size_t I, typename... Ts>
//     void helper_assign_from(T (&array)[Size], const T &var, const Ts &...vars) {
//       helper_assign_from<T, Size, IMax, I>(array, var);
//       if (I < IMax) helper_assign_from<T, Size, IMax, I + 1>(array, vars...);
//     }
//   }  // namespace detail
//
//   namespace impl {
//     template<typename Tp, size_t Row, size_t Col>
//     class numeric_matrix_static_t;
//
//     template<typename Tp, size_t Size, template<typename, size_t> class Container = container::array_t>
//     struct basic_vector_t {
//     protected:
//       template<typename U, size_t V, size_t W>
//       friend class numeric_matrix_static_t;
//
//       using ArrayT = Container<Tp, Size>;
//
//       ArrayT arr_  = {};
//       size_t size_ = Size;
//
//     public:
//       constexpr basic_vector_t()                           = default;
//       constexpr basic_vector_t(const basic_vector_t &)     = default;
//       constexpr basic_vector_t(basic_vector_t &&) noexcept = default;
//
//       // todo: constructor in derived class instead
//
//       basic_vector_t &operator=(const basic_vector_t &other)     = default;
//       basic_vector_t &operator=(basic_vector_t &&other) noexcept = default;
//
//       //
//
//       FORCE_INLINE Tp                 &operator[](const size_t index) { return *(arr_ + index); }
//       FORCE_INLINE constexpr const Tp &operator[](const size_t index) const { return *(arr_ + index); }
//
//       FORCE_INLINE Tp                 &at(const size_t index) { return operator[](index); };
//       FORCE_INLINE constexpr const Tp &at(const size_t index) const { return operator[](index); };
//
//       FORCE_INLINE Tp                 &operator()(const size_t index) { return at(index); }
//       FORCE_INLINE constexpr const Tp &operator()(const size_t index) const { return at(index); }
//
//       //
//
//       basic_vector_t &operator+=(const basic_vector_t &other) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] += other.arr_[i];
//         return *this;
//       }
//
//       basic_vector_t &operator+=(const Tp (&array)[Size]) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] += array[i];
//         return *this;
//       }
//
//       basic_vector_t operator+(const basic_vector_t &other) const {
//         basic_vector_t tmp(*this);
//         tmp.operator+=(other);
//         return tmp;
//       }
//
//       basic_vector_t operator+(const Tp (&array)[Size]) const {
//         basic_vector_t tmp(*this);
//         tmp.operator+=(array);
//         return tmp;
//       }
//
//       basic_vector_t &operator-=(const basic_vector_t &other) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] -= other.arr_[i];
//         return *this;
//       }
//
//       basic_vector_t &operator-=(const Tp (&array)[Size]) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] -= array[i];
//         return *this;
//       }
//
//       basic_vector_t operator-(const basic_vector_t &other) const {
//         basic_vector_t tmp(*this);
//         tmp.operator-=(other);
//         return tmp;
//       }
//
//       basic_vector_t operator-(const Tp (&array)[Size]) const {
//         basic_vector_t tmp(*this);
//         tmp.operator-=(array);
//         return tmp;
//       }
//
//       basic_vector_t &operator*=(Tp rhs) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] *= rhs;
//         return *this;
//       }
//
//       basic_vector_t operator*(Tp rhs) const {
//         basic_vector_t tmp(*this);
//         tmp.operator*=(rhs);
//         return tmp;
//       }
//
//       basic_vector_t &operator/=(Tp rhs) {
//         for (size_t i = 0; i < Size; ++i) arr_[i] /= rhs;
//         return *this;
//       }
//
//       basic_vector_t operator/(Tp rhs) const {
//         basic_vector_t tmp(*this);
//         tmp.operator/=(rhs);
//         return tmp;
//       }
//
//       /**
//         * Calculates inner product of this vector (LHS transposed) and the other vector of the same dimension (RHS).
//         *
//         * @param other Other vector
//         * @return Inner product
//         */
//       Tp dot(const basic_vector_t &other) const {
//         Tp acc = 0;
//         for (size_t i = 0; i < Size; ++i) acc += arr_[i] * other.arr_[i];
//         return acc;
//       }
//
//       /**
//         * Calculates inner product of this vector (LHS transposed) and the other vector of the same dimension (RHS).
//         *
//         * @param array Other vector as array
//         * @return Inner product
//         */
//       Tp dot(const Tp (&array)[Size]) const {
//         Tp acc = 0;
//         for (size_t i = 0; i < Size; ++i) acc += arr_[i] * array[i];
//         return acc;
//       }
//
//       /**
//         * Calculates inner product of this vector (LHS transposed) and the other vector of the same dimension (RHS).
//         *
//         * @param other Other vector
//         * @return Inner product
//         */
//       constexpr Tp inner(const basic_vector_t &other) const { return dot(other); }
//
//       /**
//         * Calculates inner product of this vector (LHS transposed) and the other vector of the same dimension (RHS).
//         *
//         * @param array Other vector as array
//         * @return Inner product
//         */
//       constexpr Tp inner(const Tp (&array)[Size]) const { return dot(array); }
//
//       /**
//         * Calculates outer product of this vector (LHS) and the other vector (RHS transposed).
//         *
//         * @tparam OSize Other vector's dimension
//         * @param other Other vector
//         * @return Outer product
//         */
//       template<size_t OSize>
//       numeric_matrix_static_t<Tp, Size, OSize> outer(const basic_vector_t<Tp, OSize> &other) const {
//         numeric_matrix_static_t<Tp, Size, OSize> result;
//         for (size_t i = 0; i < Size; ++i)
//           for (size_t j = 0; j < OSize; ++j)
//             result[i][j] = arr_[i] * other.arr_[j];
//         return result;
//       }
//
//       /**
//         * Calculates outer product of this vector (LHS) and the other vector as array (RHS transposed).
//         *
//         * @tparam OSize Other vector's dimension
//         * @param array Other vector as array
//         * @return Outer product
//         */
//       template<size_t OSize>
//       numeric_matrix_static_t<Tp, Size, OSize> outer(const Tp (&array)[OSize]) const {
//         numeric_matrix_static_t<Tp, Size, OSize> result;
//         for (size_t i = 0; i < Size; ++i)
//           for (size_t j = 0; j < OSize; ++j)
//             result[i][j] = arr_[i] * array[j];
//         return result;
//       }
//
//       /**
//         * Finds a sum of all entries.
//         *
//         * @return A sum of all entries
//         */
//       Tp sum() const {
//         Tp acc = 0;
//         for (size_t i = 0; i < Size; ++i) acc += arr_[i];
//         return acc;
//       }
//
//       /**
//         * Finds a norm of this vector.
//         *
//         * @return A norm of this vector
//         */
//       constexpr Tp norm() const { return pow(dot(*this), 0.5); }
//
//       /**
//         * Returns a normalized vector of this vector.
//         *
//         * @return A normalized vector of this vector
//         */
//       constexpr basic_vector_t normalize() const { return basic_vector_t(*this) / norm(); }
//
//       /**
//         * Checks equality of this vector and the other vector.
//         *
//         * @tparam OSize Other vector's dimension
//         * @param other Other vector to compare
//         * @return
//         */
//       template<size_t OSize>
//       bool operator==(const basic_vector_t<Tp, OSize> &other) const {
//         if (this == &other) return true;
//         if (Size != OSize) return false;
//         for (size_t i = 0; i < Size; ++i)
//           if (arr_[i] != other.arr_[i]) return false;
//         return true;
//       }
//
//       /**
//         * Checks equality of this vector and the other array.
//         *
//         * @tparam OSize Other array's dimension
//         * @param array Array to compare
//         * @return
//         */
//       template<size_t OSize>
//       bool operator==(const Tp (&array)[OSize]) const {
//         if (Size != OSize) return false;
//         for (size_t i = 0; i < Size; ++i)
//           if (arr_[i] != array[i]) return false;
//         return true;
//       }
//
//       /**
//         * Checks inequality of this vector and the other vector.
//         *
//         * @tparam OSize Other vector's dimension
//         * @param other Other vector to compare
//         * @return
//         */
//       template<size_t OSize>
//       constexpr bool operator!=(const basic_vector_t<Tp, OSize> &other) const {
//         return !operator==(other);
//       }
//
//       /**
//         * Checks inequality of this vector and the other array.
//         *
//         * @tparam OSize Other array's dimension
//         * @param array Array to compare
//         * @return
//         */
//       template<size_t OSize>
//       constexpr bool operator!=(const Tp (&array)[OSize]) const { return !operator==(array); }
//
//       /**
//         * Checks equality of this vector and the other vector.
//         *
//         * @tparam OSize Other vector's dimension
//         * @param other Other vector to compare
//         * @return
//         */
//       template<size_t OSize>
//       constexpr bool equals(const basic_vector_t<Tp, OSize> &other) const { return operator==(other); }
//
//       /**
//         * Checks equality of this vector and the other vector with float/double threshold using
//         * equation abs(x_ - y) < threshold for equality.
//         *
//         * @tparam OSize Other vector's dimension
//         * @param other Other vector to compare
//         * @param threshold Equality threshold
//         * @return
//         */
//       template<size_t OSize>
//       bool float_equals(const basic_vector_t<Tp, OSize> &other, Tp threshold = 1e-10) const {
//         if (this == &other) return true;
//         if (Size != OSize) return false;
//         for (size_t i = 0; i < Size; ++i)
//           if (abs(arr_[i] - other.arr_[i]) > threshold) return false;
//         return true;
//       }
//
//       /**
//         * Checks equality of this vector and the other array.
//         *
//         * @tparam OSize Other array's dimension
//         * @param array Array to compare
//         * @return
//         */
//       template<size_t OSize>
//       constexpr bool equals(const Tp (&array)[OSize]) const { return operator==(array); }
//
//       /**
//         * Insert the vector into the vector.
//         *
//         * @tparam pos Position to insert
//         * @tparam OSize
//         * @param v Vector to insert
//         * @return Reference to this vector
//         */
//       template<size_t pos = 0, size_t OSize>
//       basic_vector_t &insert(const basic_vector_t<Tp, OSize> &v) {
//         static_assert(pos < Size, "Insertion failed! Position must be within range.");
//         static_assert(pos + OSize <= Size, "Insertion failed! Vector out of range.");
//         for (size_t i = 0; i < OSize; ++i) arr_[pos + i] = v[i];
//         return *this;
//       }
//
//       /**
//         * Insert the vector into the vector.
//         *
//         * @tparam pos Position to insert
//         * @tparam OSize
//         * @param array Vector to insert
//         * @return Reference to this vector
//         */
//       template<size_t pos = 0, size_t OSize>
//       basic_vector_t &insert(const Tp (&array)[OSize]) {
//         static_assert(pos < Size, "Insertion failed! Position must be within range.");
//         static_assert(pos + OSize <= Size, "Insertion failed! Vector out of range.");
//         for (size_t i = 0; i < OSize; ++i) arr_[pos + i] = array[i];
//         return *this;
//       }
//
//       /**
//         * Gets a slice of this vector.
//         *
//         * @tparam from From index
//         * @tparam to To index
//         * @return Sliced vector
//         */
//       template<size_t from, size_t to>
//       basic_vector_t<Tp, to - from> slice() {
//         static_assert(from < to, "from must be less than to.");
//         static_assert(to <= Size, "Slice range is out of range.");
//         basic_vector_t<Tp, to - from> result;
//         for (size_t i = 0; i < to - from; ++i) result[i] = arr_[from + i];
//         return result;
//       }
//
//       /**
//         * First N elements.
//         *
//         * @tparam N
//         * @return
//         */
//       template<size_t N>
//       basic_vector_t<Tp, N> head() {
//         static_assert(N <= Size, "N must be in range of dimension.");
//         return slice<0, N>();
//       }
//
//       /**
//         * Last N elements.
//         *
//         * @tparam N
//         * @return
//         */
//       template<size_t N>
//       basic_vector_t<Tp, N> tail() {
//         static_assert(N <= Size, "N must be in range of dimension.");
//         return slice<Size - N, Size>();
//       }
//
//       /**
//         * Converts this vector to a matrix representation of column vector
//         *
//         * @return Column vector as matrix
//         */
//       numeric_matrix_static_t<Tp, Size, 1> as_matrix_col() {
//         numeric_matrix_static_t<Tp, Size, 1> result;
//         for (size_t i = 0; i < Size; ++i) result[i][0] = arr_[i];
//         return result;
//       }
//
//       /**
//         * Converts this vector to a matrix representation of row vector
//         *
//         * @return Row vector as matrix
//         */
//       numeric_matrix_static_t<Tp, 1, Size> as_matrix_row() {
//         numeric_matrix_static_t<Tp, 1, Size> result;
//         for (size_t i = 0; i < Size; ++i) result[0][i] = arr_[i];
//         return result;
//       }
//
//       [[nodiscard]] FORCE_INLINE constexpr Tp       *begin() { return arr_.begin(); }
//       [[nodiscard]] FORCE_INLINE constexpr const Tp *begin() const { return arr_.begin(); }
//       [[nodiscard]] FORCE_INLINE constexpr Tp       *cbegin() { return arr_.cbegin(); }
//       [[nodiscard]] FORCE_INLINE constexpr const Tp *cbegin() const { return arr_.cbegin(); }
//       [[nodiscard]] FORCE_INLINE constexpr Tp       *end() { return arr_.end(); }
//       [[nodiscard]] FORCE_INLINE constexpr const Tp *end() const { return arr_.end(); }
//       [[nodiscard]] FORCE_INLINE constexpr Tp       *cend() { return arr_.cend(); }
//       [[nodiscard]] FORCE_INLINE constexpr const Tp *cend() const { return arr_.cend(); }
//
//       // Size/dimension
//
//       constexpr size_t size() const { return Size; }
//       constexpr size_t dim() const { return Size; }
//       constexpr size_t dimension() const { return Size; }
//
//       /**
//         * Swaps entries with the other vector.
//         *
//         * @param other Other vector
//         */
//       void swap(basic_vector_t &other) {
//         for (size_t i = 0; i < Size; ++i) xcore::swap(arr_[i], other.arr_[i]);
//       }
//
//       /**
//         * Creates a copy of this vector.
//         *
//         * @return A copy of this vector
//         */
//       constexpr basic_vector_t copy() const { return basic_vector_t(*this); }
//
//     private:
//       void allocate_zero() { allocate_fill(Tp()); }
//
//       void allocate_fill(const Tp &fill) { xcore::fill(arr_, arr_ + Size, fill); }
//
//       void allocate_from(const basic_vector_t &other) {
//         static_cast<void>(xcore::copy(other.arr_, other.arr_ + Size, arr_));
//       }
//
//       void allocate_from(const Tp (&array)[Size]) {
//         static_cast<void>(xcore::copy(array, array + Size, arr_));
//       }
//
//     public:
//       /**
//         * Creates a zero matrix.
//         *
//         * @return Zero matrix
//         */
//       static constexpr basic_vector_t zeros() { return basic_vector_t(); }
//
//       /**
//         * Creates a 1-filled matrix.
//         *
//         * @return 1-filled matrix
//         */
//       static constexpr basic_vector_t ones() { return basic_vector_t(1); }  // todo: what to do with this?
//     };
//
//     template<typename T, size_t Size>
//     basic_vector_t<T, Size> operator*(T lhs, const basic_vector_t<T, Size> &rhs) {
//       basic_vector_t<T, Size> tmp(rhs);
//       tmp.operator*=(lhs);
//       return tmp;
//     }
//
//     template<typename T, size_t Size>
//     basic_vector_t<T, Size> operator+(const T (&lhs)[Size], const basic_vector_t<T, Size> &rhs) {
//       basic_vector_t<T, Size> tmp(rhs);
//       tmp.operator+=(lhs);
//       return tmp;
//     }
//
//     template<typename T, size_t Size>
//     basic_vector_t<T, Size> operator-(const T (&lhs)[Size], const basic_vector_t<T, Size> &rhs) {
//       basic_vector_t<T, Size> tmp(rhs);
//       tmp.operator-=(lhs);
//       return tmp;
//     }
//   }  // namespace impl
// }  // namespace math

#endif  //LIB_XCORE_LINALG_NUMERIC_VECTOR_HPP
