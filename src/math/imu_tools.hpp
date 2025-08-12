#ifndef LIB_XCORE_LINALG_IMU_TOOLS_HPP
#define LIB_XCORE_LINALG_IMU_TOOLS_HPP

#include "core/ported_std.hpp"
#include "./numeric_vector.hpp"

LIB_XCORE_BEGIN_NAMESPACE

typedef struct {
  real_t roll;
  real_t pitch;
  real_t yaw;
} euler_t;

typedef struct {
  real_t w;
  real_t x;
  real_t y;
  real_t z;
} quaternion_t;

euler_t quaternion_to_euler(const quaternion_t &quaternion, bool degrees = false);

numeric_vector<3> quaternion_to_euler(const numeric_vector<4> &quaternion, bool degrees = false);

quaternion_t euler_to_quaternion(const euler_t &euler, bool degrees = false);

numeric_vector<4> euler_to_quaternion(const numeric_vector<3> &euler, bool degrees = false);

LIB_XCORE_END_NAMESPACE

#endif  //LIB_XCORE_LINALG_IMU_TOOLS_HPP
