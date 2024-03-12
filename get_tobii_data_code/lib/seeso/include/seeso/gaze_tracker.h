//
// Created by cosge on 2021-03-24.
//
#ifndef SEESO_GAZE_TRACKER_H_
#define SEESO_GAZE_TRACKER_H_

#include <vector>
#include <string>

#include "seeso/error_reporter.h"
#include "seeso/callback/icalibration_callback.h"
#include "seeso/callback/igaze_callback.h"
#include "seeso/callback/iuser_status_callback.h"
#include "seeso/framework/c_def.h"
#include "seeso/framework/core_callback.h"
#include "seeso/util/point.h"
#include "seeso/util/coord_converter_v2.h"

namespace seeso {

// This function must be called before any function or EyeTracker object is created.
// Only requires on Windows.
void global_init(const char* file = "seeso_core.dll");

std::string getVersionStr();

int32_t getVersionInt();

class GazeTracker {
 public:
  using converter_type = CoordConverterV2<float>;

  /**
   * Create GazeTracker
   * CoordConverter must be given, which converts [millimeters in camera-coordinate] to [pixels in display coordinate]
   *
   * You can use makeCameraToDisplayConverter() or makeDefaultCameraToDisplayConverter().
   * If you want to use raw-coordinate[millimeters in camera-coordinate], use makeNoOpConverter()
   *
   * You can change them later using GazeTracker::converter()
   *
   * @param coord_converter
   */
  explicit GazeTracker(const converter_type& coord_converter = makeNoOpConverter<float>());

  ~GazeTracker();

  // init SeeSo object
  // WARNING! ErrorReporter is experimental! It will not report all errors.
  // See framework/c_api.h for more information
  int initialize(const std::string& license_key,
                 const SeeSoStatusModuleOptions& status_option = {kSeeSoFalse, kSeeSoFalse, kSeeSoFalse},
                 ErrorReporter* error_reporter = DefaultErrorReporter());

  // destroy SeeSo object
  void deinitialize();
  bool isTrackerInitialized() const;

  // set the distance between face and camera
  void setFaceDistance(int cm);
  int getFaceDistance() const; // return cm

  // set listener
  void setGazeCallback(seeso::IGazeCallback* listener);
  void setCalibrationCallback(seeso::ICalibrationCallback* listener);
  void setUserStatusCallback(seeso::IUserStatusCallback* listener);

  // remove listener
  void removeGazeCallback();
  void removeCalibrationCallback();
  void removeUserStatusCallback();

  // set maximum fps
  void setTrackingFps(int fps);

  // Image
  /**
   *
   * @param time_stamp      frame timestamp(passed out to callback)
   * @param buffer          frame data
   * @param width           frame width
   * @param height          frame height
   * @return
   *    frame may be omitted if there's no idle thread.
   *    Then this returns false. Else true
   */
  bool addFrame(int64_t time_stamp, uint8_t* buffer, int width, int height);

  // set target region bound
  void setTargetBoundRegion(float left, float top, float right, float bottom);

  // Calibration
  void startCalibration(SeeSoCalibrationPointNum num, SeeSoCalibrationAccuracy criteria,
                        float left, float top, float right, float bottom);
  void enhanceCalibration(SeeSoCalibrationPointNum num, SeeSoCalibrationAccuracy criteria,
                          float left, float top, float right, float bottom);
  void startCollectSamples();
  void stopCalibration();
  void setCalibrationData(const std::vector<float>& serialData);

  // User Status Setting
  void setAttentionRegion(float left, float top, float right, float bottom);
  std::vector<float> getAttentionRegion() const;
  void removeAttentionRegion();

  void setAttentionInterval(int interval);
  float getAttentionScore();

  /**
   * converter getter
   * @return
   */
        converter_type& converter()       { return coord_converter_; }
  const converter_type& converter() const { return coord_converter_; }

 private:
  CoreCallback callback;
  converter_type coord_converter_;
  int face_distance_mm = 600; // mm

  // Use void pointer to hide C API when including this header
  void* seeso_object = nullptr;
};

} // namespace seeso

#endif // SEESO_GAZE_TRACKER_H_
