#ifndef SEESO_FRAMEWORK_CORE_CALLBACK_H_
#define SEESO_FRAMEWORK_CORE_CALLBACK_H_

#include <cstdint>

#include <memory>
#include <vector>

#include "seeso/callback/icalibration_callback.h"
#include "seeso/callback/iface_callback.h"
#include "seeso/callback/igaze_callback.h"
#include "seeso/callback/iuser_status_callback.h"
#include "seeso/framework/callback_dispatcher.h"
#include "seeso/framework/c_def.h"
#include "seeso/util/coord_converter_v2.h"

namespace seeso {

class CoreCallback final : private internal::CallbackDispatcher<CoreCallback> {
 public:
  CoreCallback();

  void OnGaze(const SeeSoGazeData* data);
  void OnFace(const SeeSoFaceData* data);

  void OnAttention(const SeeSoAttentionData* data);
  void OnStatus(const SeeSoStatusData* data);

  void OnCalibrationProgress(float progress);
  void OnCalibrationNextPoint(float next_point_x, float next_point_y);
  void OnCalibrationFinished(std::vector<float> calib_data);

  /**
   * Set callbacks
   * Note: these functions are not thread-safe
   * @param callback
   */

  void setGazeCallback(IGazeCallback* callback);
  void setCalibrationCallback(ICalibrationCallback* callback);
  void setUserStatusCallback(IUserStatusCallback* callback);
  //  void setFaceCallback(seeso::IFaceCallback* callback);

  /**
   * Set camera <-> display coordinate converter
   * Note: this function is not thread-safe
   * @param cc
   */
  void setConverter(CoordConverterV2<float>* cc);

 private:
  IGazeCallback* gaze_callback;
  ICalibrationCallback* calibration_callback;
  IUserStatusCallback* user_status_callback;
  //  IFaceCallback* face_callback;
  const CoordConverterV2<float>* coord_converter_;
};

} // namespace seeso

#endif // SEESO_FRAMEWORK_CORE_CALLBACK_H_
