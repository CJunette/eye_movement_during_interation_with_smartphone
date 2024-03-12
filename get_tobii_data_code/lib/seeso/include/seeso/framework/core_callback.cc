#include "seeso/framework/core_callback.h"

#include <vector>

#include "seeso/framework/c_def.h"

namespace seeso {

namespace {

struct : public IGazeCallback {
  void OnGaze(uint64_t timestamp, float x, float y,
              SeeSoTrackingState tracking_state,
              SeeSoEyeMovementState eye_movement_state) override {}
} NullGazeCallback;

struct : public ICalibrationCallback {
  void OnCalibrationProgress(float progress) override {}
  void OnCalibrationNextPoint(float x, float y) override {}
  void OnCalibrationFinish(const std::vector<float>& data) override {}
} NullCalibrationCallback;

struct : public IUserStatusCallback {
  void OnAttention(uint64_t timestampBegin, uint64_t timestampEnd, float score) override {}
  void OnDrowsiness(uint64_t timestamp, bool isDrowsiness) override {}
  void OnBlink(uint64_t timestamp, bool isBlinkLeft, bool isBlinkRight, bool isBlink, float eyeOpenness) override {}
} NullUserStatusCallback;

struct : public IFaceCallback {
  void OnLandmark() override {}
  void OnRegion() override {}
} NullFaceCallback;

const CoordConverterV2<float> default_coord_converter;

} // anonymous namespace


CoreCallback::CoreCallback()
: gaze_callback(&NullGazeCallback),
  calibration_callback(&NullCalibrationCallback),
  user_status_callback(&NullUserStatusCallback),
  coord_converter_(&default_coord_converter) {}

void CoreCallback::OnGaze(const SeeSoGazeData* data) {
  float x = data->gaze_x;
  float y = data->gaze_y;
  if (x != -1001.f) {
    const auto p = coord_converter_->convert({x, y});
    x = p[0];
    y = p[1];
  }

  gaze_callback->OnGaze(data->timestamp, x, y, data->tracking_state, data->eye_movement_state);
}

void CoreCallback::OnAttention(const SeeSoAttentionData *data) {
  user_status_callback->OnAttention(data->timestamp_begin, data->timestamp_end, data->average_score);
}

void CoreCallback::OnStatus(const SeeSoStatusData* data) {
  user_status_callback->OnDrowsiness(data->timestamp, data->is_drowsy == kSeeSoTrue);
  user_status_callback->OnBlink(
      data->timestamp,
      data->is_blink_left == kSeeSoTrue,
      data->is_blink_right == kSeeSoTrue,
      data->is_blink == kSeeSoTrue,
      data->normalized_EAR);
}

void CoreCallback::OnFace(const SeeSoFaceData *data) {
  // Experimental
}

void CoreCallback::OnCalibrationProgress(float progress) {
  calibration_callback->OnCalibrationProgress(progress);
}

void CoreCallback::OnCalibrationNextPoint(float next_point_x, float next_point_y) {
  const auto px = coord_converter_->convert({next_point_x, next_point_y});
  calibration_callback->OnCalibrationNextPoint(px[0], px[1]);
}

void CoreCallback::OnCalibrationFinished(std::vector<float> calib_data) {
  calibration_callback->OnCalibrationFinish(calib_data);
}

void CoreCallback::setGazeCallback(seeso::IGazeCallback* callback) {
  gaze_callback = callback == nullptr ? &NullGazeCallback : callback;
}

void CoreCallback::setCalibrationCallback(seeso::ICalibrationCallback* callback) {
  calibration_callback = callback == nullptr ? &NullCalibrationCallback : callback;
}

void CoreCallback::setUserStatusCallback(seeso::IUserStatusCallback* callback) {
  user_status_callback = callback == nullptr ? &NullUserStatusCallback : callback;
}
void CoreCallback::setConverter(CoordConverterV2<float> *cc) {
  coord_converter_ = cc == nullptr ? &default_coord_converter : cc;
}

} // namespace seeso
