//
// Created by YongGyu Lee on 2021-03-24.
//

#include "seeso/gaze_tracker.h"

#include <functional>
#include <stdexcept>
#include <string>
#include <vector>

#include "seeso/framework/c_api.h"
#include "seeso/framework/global_init.h"

static inline constexpr SeeSoTracker* cast_tracker(void * ptr) {
  return static_cast<SeeSoTracker*>(ptr);
}

static inline constexpr const SeeSoTracker* cast_tracker(const void * ptr) {
  return static_cast<const SeeSoTracker *>(ptr);
}

namespace seeso {

std::string getVersionStr() {
  return SeeSoVersionString();
}

int32_t getVersionInt() {
  return SeeSoVersionInteger();
}

GazeTracker::~GazeTracker() {
  SeeSoTrackerDelete(cast_tracker(seeso_object));
  seeso_object = nullptr;
}

GazeTracker::GazeTracker(const CoordConverterV2<float>& coord_converter) {
  callback.setConverter(&coord_converter_);
  coord_converter_ = coord_converter;
}

int GazeTracker::initialize(const std::string& license_key,
                            const SeeSoStatusModuleOptions& status_option,
                            ErrorReporter* error_reporter) {
  if (seeso_object == nullptr) {
    seeso_object = SeeSoTrackerCreate(license_key.c_str(), static_cast<uint32_t>(license_key.size()));
  }

  SeeSoTrackerOptions* options = SeeSoTrackerOptionsCreate();
  SeeSoTrackerOptionsEnableStatusModule(options, std::addressof(status_option));

  if (error_reporter) {
    SeeSoTrackerOptionsSetErrorReporter(
        options,
        [](void* reporter, const char* msg) -> int {
          return static_cast<ErrorReporter*>(reporter)->report(msg);
        },
        error_reporter);
  } else {
    // Pass nullptr to use default internal error reporter.
    SeeSoTrackerOptionsSetErrorReporter(options, nullptr, nullptr);
  }

  auto internal_code = SeeSoTrackerGetAuthorizationResult(cast_tracker(seeso_object));
  if (internal_code != 0) {
    return internal_code + 2;
  }

  SeeSoTrackerInit(cast_tracker(seeso_object), options);


  using dispatcher = internal::CallbackDispatcher<CoreCallback>;

  SeeSoTrackerSetGazeCallback(cast_tracker(seeso_object), &dispatcher::dispatchOnGaze);
  SeeSoTrackerSetFaceCallback(cast_tracker(seeso_object), &dispatcher::dispatchOnFace);
  SeeSoTrackerSetStatusCallback(cast_tracker(seeso_object),
                                &dispatcher::dispatchOnAttention, &dispatcher::dispatchOnStatus);
  SeeSoTrackerSetCalibrationCallback(cast_tracker(seeso_object),
                                     &dispatcher::dispatchOnCalibrationNextPoint,
                                     &dispatcher::dispatchOnCalibrationProgress,
                                     &dispatcher::dispatchOnCalibrationFinished);
  SeeSoTrackerSetCallbackUserData(cast_tracker(seeso_object), std::addressof(callback));
  return 0;
}

void GazeTracker::deinitialize() {
  SeeSoTrackerRemoveCallbackInterface(cast_tracker(seeso_object));
  SeeSoTrackerDeInit(cast_tracker(seeso_object));
}

bool GazeTracker::isTrackerInitialized() const {
  return SeeSoTrackerInitialized(cast_tracker(seeso_object)) == kSeeSoTrue;
}

void GazeTracker::setTrackingFps(int fps) {
  SeeSoTrackerSetFPS(cast_tracker(seeso_object), fps);
}

void GazeTracker::setFaceDistance(int cm) {
  face_distance_mm = cm * 10;
  SeeSoTrackerSetFaceDistance(cast_tracker(seeso_object), face_distance_mm);
}

int GazeTracker::getFaceDistance() const {
  return face_distance_mm / 10;
}

void GazeTracker::setTargetBoundRegion(float left, float top, float right, float bottom) {
  auto tl = coord_converter_.revert({left, top});
  auto br = coord_converter_.revert({right, bottom});
  SeeSoTrackerSetTargetBoundRegion(cast_tracker(seeso_object), tl[0], tl[1], br[0], br[1]);
}

void GazeTracker::startCalibration(SeeSoCalibrationPointNum num, SeeSoCalibrationAccuracy criteria,
                                   float left, float top, float right, float bottom) {
  auto tl = coord_converter_.revert({left, top});
  auto br = coord_converter_.revert({right, bottom});

  SeeSoCalibrationOptions options{};
  options.points = num;
  options.accuracy = criteria;
  options.left = tl[0];
  options.top = tl[1];
  options.right = br[0];
  options.bottom = br[1];

  SeeSoTrackerStartCalibration(cast_tracker(seeso_object), std::addressof(options));
}


void GazeTracker::enhanceCalibration(SeeSoCalibrationPointNum num, SeeSoCalibrationAccuracy criteria,
                                     float left, float top, float right, float bottom) {
  if (num != kSeeSoCalibrationPointOne && num != kSeeSoCalibrationPointTwo) {
    // enhance calibration only supports one or two points calibration
    return;
  }
  auto tl = coord_converter_.revert({left, top});
  auto br = coord_converter_.revert({right, bottom});

  SeeSoCalibrationOptions options{};
  options.points = num;
  options.accuracy = criteria;
  options.left = tl[0];
  options.top = tl[1];
  options.right = br[0];
  options.bottom = br[1];

  SeeSoTrackerEnhanceCalibration(cast_tracker(seeso_object), std::addressof(options));
}


void GazeTracker::startCollectSamples() {
  SeeSoTrackerStartCollectSamples(cast_tracker(seeso_object));
}

void GazeTracker::stopCalibration() {
  SeeSoTrackerStopCalibration(cast_tracker(seeso_object));
}

void GazeTracker::setCalibrationData(const std::vector<float>& serialData) {
  SeeSoTrackerSetCalibrationData(cast_tracker(seeso_object),
                                 serialData.data(),
                                 static_cast<uint32_t>(serialData.size()));
}

bool GazeTracker::addFrame(int64_t time_stamp, uint8_t* buffer, int width, int height) {
  return SeeSoTrackerAddFrame(cast_tracker(seeso_object), time_stamp, buffer, width, height) == kSeeSoTrue;
}

void GazeTracker::setGazeCallback(seeso::IGazeCallback* listener) {
  callback.setGazeCallback(listener);
}

void GazeTracker::setCalibrationCallback(seeso::ICalibrationCallback* listener) {
  callback.setCalibrationCallback(listener);
}

void GazeTracker::setUserStatusCallback(seeso::IUserStatusCallback* listener) {
  callback.setUserStatusCallback(listener);
}

void GazeTracker::removeGazeCallback() {
  callback.setGazeCallback(nullptr);
}

void GazeTracker::removeCalibrationCallback() {
  callback.setCalibrationCallback(nullptr);
}

void GazeTracker::removeUserStatusCallback() {
  callback.setUserStatusCallback(nullptr);
}

void GazeTracker::setAttentionRegion(float left, float top, float right, float bottom) {
  const auto tl = coord_converter_.revert({left, top});
  const auto br = coord_converter_.revert({right, bottom});
  SeeSoTrackerSetAttentionRegion(cast_tracker(seeso_object), tl[0], tl[1], br[0], br[1]);
}

std::vector<float> GazeTracker::getAttentionRegion() const {
  float roi[4];
  const auto success = SeeSoTrackerGetAttentionRegion(cast_tracker(seeso_object), roi);
  if (!success)
    return {};

  const auto tl = coord_converter_.convert({roi[0], roi[1]});
  const auto br = coord_converter_.convert({roi[2], roi[3]});
  return {tl[0], tl[1], br[0], br[1]};
}

void GazeTracker::removeAttentionRegion() {
  SeeSoTrackerRemoveAttentionRegion(cast_tracker(seeso_object));
}

void GazeTracker::setAttentionInterval(int interval) {
  SeeSoTrackerSetAttentionInterval(cast_tracker(seeso_object), interval);
}

float GazeTracker::getAttentionScore() {
  return SeeSoTrackerGetAttentionScore(cast_tracker(seeso_object));
}

} // namespace seeso
