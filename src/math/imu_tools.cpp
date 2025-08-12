#include "./imu_tools.hpp"
#include <cmath>
#include "./standard_constants.hpp"

LIB_XCORE_NAMESPACE::euler_t LIB_XCORE_NAMESPACE::quaternion_to_euler(const quaternion_t &quaternion, const bool degrees) {
  euler_t euler;

  const real_t sinr_cosp = 2. * (quaternion.w * quaternion.x + quaternion.y * quaternion.z);
  const real_t cosr_cosp = 1. - 2. * (quaternion.x * quaternion.x + quaternion.y * quaternion.y);
  euler.roll             = atan2(sinr_cosp, cosr_cosp);

  const real_t sinp = sqrt(1. + 2. * (quaternion.w * quaternion.y - quaternion.x * quaternion.z));
  const real_t cosp = sqrt(1. - 2. * (quaternion.w * quaternion.y - quaternion.x * quaternion.z));
  euler.pitch       = 2. * atan2(sinp, cosp) - M_PI / 2.;

  const real_t siny_cosp = 2. * (quaternion.w * quaternion.z + quaternion.x * quaternion.y);
  const real_t cosy_cosp = 1. - 2. * (quaternion.y * quaternion.y + quaternion.z * quaternion.z);
  euler.yaw              = atan2(siny_cosp, cosy_cosp);

  if (degrees) {
    euler.roll *= RAD_TO_DEG;
    euler.pitch *= RAD_TO_DEG;
    euler.yaw *= RAD_TO_DEG;
  }

  return euler;
}

LIB_XCORE_NAMESPACE::numeric_vector<3> LIB_XCORE_NAMESPACE::quaternion_to_euler(const numeric_vector<4> &quaternion, const bool degrees) {
  numeric_vector<3> euler;

  const real_t sinr_cosp = 2. * (quaternion[0] * quaternion[1] + quaternion[2] * quaternion[3]);
  const real_t cosr_cosp = 1. - 2. * (quaternion[1] * quaternion[1] + quaternion[2] * quaternion[2]);
  euler[0]               = atan2(sinr_cosp, cosr_cosp);

  const real_t sinp = sqrt(1. + 2. * (quaternion[0] * quaternion[2] - quaternion[1] * quaternion[3]));
  const real_t cosp = sqrt(1. - 2. * (quaternion[0] * quaternion[2] - quaternion[1] * quaternion[3]));
  euler[1]          = 2. * atan2(sinp, cosp) - M_PI / 2.;

  const real_t siny_cosp = 2. * (quaternion[0] * quaternion[3] + quaternion[1] * quaternion[2]);
  const real_t cosy_cosp = 1. - 2. * (quaternion[2] * quaternion[2] + quaternion[3] * quaternion[3]);
  euler[2]               = atan2(siny_cosp, cosy_cosp);

  if (degrees) euler *= RAD_TO_DEG;

  return euler;
}

LIB_XCORE_NAMESPACE::quaternion_t LIB_XCORE_NAMESPACE::euler_to_quaternion(const euler_t &euler, const bool degrees) {
  real_t yaw, pitch, roll;

  // if the angles are in degrees convert them to radians
  if (degrees) {
    yaw   = euler.yaw * DEG_TO_RAD;
    pitch = euler.pitch * DEG_TO_RAD;
    roll  = euler.roll * DEG_TO_RAD;
  } else {
    roll  = euler.roll;
    pitch = euler.pitch;
    yaw   = euler.yaw;
  }

  const real_t cr = cos(roll * 0.5);
  const real_t sr = sin(roll * 0.5);
  const real_t cp = cos(pitch * 0.5);
  const real_t sp = sin(pitch * 0.5);
  const real_t cy = cos(yaw * 0.5);
  const real_t sy = sin(yaw * 0.5);

  quaternion_t q;
  q.w = cr * cp * cy + sr * sp * sy;
  q.x = sr * cp * cy - cr * sp * sy;
  q.y = cr * sp * cy + sr * cp * sy;
  q.z = cr * cp * sy - sr * sp * cy;

  return q;
}

LIB_XCORE_NAMESPACE::numeric_vector<4> LIB_XCORE_NAMESPACE::euler_to_quaternion(const numeric_vector<3> &euler, const bool degrees) {
  real_t yaw, pitch, roll;

  if (degrees) {
    roll  = euler[0] * DEG_TO_RAD;
    pitch = euler[1] * DEG_TO_RAD;
    yaw   = euler[2] * DEG_TO_RAD;
  } else {
    roll  = euler[2];
    pitch = euler[1];
    yaw   = euler[0];
  }

  const real_t cr = cos(roll * 0.5);
  const real_t sr = sin(roll * 0.5);
  const real_t cp = cos(pitch * 0.5);
  const real_t sp = sin(pitch * 0.5);
  const real_t cy = cos(yaw * 0.5);
  const real_t sy = sin(yaw * 0.5);

  numeric_vector<4> q;
  q[0] = cr * cp * cy + sr * sp * sy;
  q[1] = sr * cp * cy - cr * sp * sy;
  q[2] = cr * sp * cy + sr * cp * sy;
  q[3] = cr * cp * sy - sr * sp * cy;

  return q;
}
