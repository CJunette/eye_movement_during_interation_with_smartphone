//
// Created by cosge on 2021-03-26.
//

#ifndef SEESO_FRAMEWORK_CALLBACK_DISPATCHER_H_
#define SEESO_FRAMEWORK_CALLBACK_DISPATCHER_H_

#include <cstdint>
#include <vector>

#include "seeso/framework/c_def.h"

namespace seeso {
namespace internal {

/** Callback Object Dispatcher
 * @brief callback method dispatcher
 *
 * instead of CallbackInterface's virtual method, this class' static methods are passed to internal SeeSo object.
 * (since member function pointer cannot be expressed to C-style function pointers, and to avoid different vtable ABI)
 * Dispatcher function is called from internal SeeSo object,
 * and then the dispatcher functions calls CallbackInterface object's methods.
 *
 * @tparam Derived      Interface type
 */
template<typename Derived>
class CallbackDispatcher {
  static constexpr Derived* derived(void* ptr) { return static_cast<Derived*>(ptr); }
  static constexpr const Derived* derived(const void* ptr) { return static_cast<const Derived*>(ptr); }

 protected:
  ~CallbackDispatcher() = default;

 public:
  using derived_type = Derived;

  static void dispatchOnGaze(void* obj, const SeeSoGazeData* data) {
    static auto OnGaze = &derived_type::OnGaze;
    (derived(obj)->*OnGaze)(data);
  }

  static void dispatchOnFace(void* obj, const SeeSoFaceData* data) {
    static auto OnFace = &derived_type::OnFace;
    (derived(obj)->*OnFace)(data);
  }

  static void dispatchOnAttention(void* obj, const SeeSoAttentionData* data) {
    static auto OnAttention = &derived_type::OnAttention;
    (derived(obj)->*OnAttention)(data);
  }

  static void dispatchOnStatus(void* obj, const SeeSoStatusData* data) {
    static auto OnStatus = &derived_type::OnStatus;
    (derived(obj)->OnStatus)(data);
  }

  static void dispatchOnCalibrationProgress(void* obj, float progress) {
    static auto OnCalibrationProgress = &derived_type::OnCalibrationProgress;
    (derived(obj)->*OnCalibrationProgress)(progress);
  }

  static void dispatchOnCalibrationNextPoint(void* obj, float next_point_x, float next_point_y) {
    static auto OnCalibrationNextPoint = &derived_type::OnCalibrationNextPoint;
    (derived(obj)->*OnCalibrationNextPoint)(next_point_x, next_point_y);
  }

  static void dispatchOnCalibrationFinished(void* obj, const float* data, uint32_t data_size) {
    static auto OnCalibrationFinished = &derived_type::OnCalibrationFinished;
    (derived(obj)->*OnCalibrationFinished)({data, data + data_size});
  }
};


} // namespace internal
} // namespace seeso

#endif // SEESO_FRAMEWORK_CALLBACK_DISPATCHER_H_
