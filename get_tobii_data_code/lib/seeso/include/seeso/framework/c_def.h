//
// Created by YongGyu Lee on 2021/10/07.
//

#ifndef SEESO_CORE_C_DEF_H_ // NOLINT(build/header_guard)
#define SEESO_CORE_C_DEF_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum SeeSoBoolean {
  kSeeSoFalse = 0,
  kSeeSoTrue = 1,
} SeeSoBoolean;

// Status module options
// WARNING: This interface is likely to be changed in the future
typedef struct SeeSoStatusModuleOptions {
  SeeSoBoolean use_attention;
  SeeSoBoolean use_blink;
  SeeSoBoolean use_drowsiness;
} SeeSoStatusModuleOptions;

// Available calibration point nums
typedef enum SeeSoCalibrationPointNum {
  kSeeSoCalibrationPointOne = 1,
  kSeeSoCalibrationPointTwo = 2,
  kSeeSoCalibrationPointFive = 5,
  kSeeSoCalibrationPointSix = 6,
} SeeSoCalibrationPointNum;


// Calibration accuracy
typedef enum SeeSoCalibrationAccuracy {
  kSeeSoCalibrationAccuracyDefault = 0,
  kSeeSoCalibrationAccuracyLow,
  kSeeSoCalibrationAccuracyHigh,
} SeeSoCalibrationAccuracy;


// Calibration options
// WARNING: This interface is likely to be changed in the future
typedef struct SeeSoCalibrationOptions {
  SeeSoCalibrationPointNum points;
  SeeSoCalibrationAccuracy accuracy;

  float left;
  float top;
  float right;
  float bottom;
} SeeSoCalibrationOptions;


// Output data of callbacks

typedef enum SeeSoTrackingState {
  kSeeSoTrackingSuccess = 0,
  kSeeSoTrackingLowConfidence,
  kSeeSoTrackingUnsupported,
  kSeeSoTrackingFaceMissing,
} SeeSoTrackingState;


typedef enum SeeSoEyeMovementState {
  kSeeSoEyeMovementFixation = 0,
  kSeeSoEyeMovementSaccade = 2,
  kSeeSoEyeMovementUnknown = 3,
} SeeSoEyeMovementState;

typedef struct SeeSoGazeData {
  uint64_t timestamp;

  float gaze_x;
  float gaze_y;

  float fixation_x;
  float fixation_y;

  int32_t movement_duration;

  SeeSoTrackingState tracking_state;
  SeeSoEyeMovementState eye_movement_state;
} SeeSoGazeData;


typedef struct SeeSoFaceData {
  uint64_t timestamp;
  const float* data;
  uint32_t data_size;
} SeeSoFaceData;


typedef struct SeeSoAttentionData {
  uint64_t timestamp_begin;
  uint64_t timestamp_end;

  float average_score;
} SeeSoAttentionData;

typedef struct SeeSoStatusData {
  uint64_t timestamp;

  float movement_score;

  SeeSoBoolean is_fixation;

  float normalized_EAR;
  float normalized_EAR_left;
  float normalized_EAR_right;

  float ECR;

  SeeSoBoolean is_drowsy;

  SeeSoBoolean is_blink;
  SeeSoBoolean is_blink_left;
  SeeSoBoolean is_blink_right;
} SeeSoStatusData;

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // SEESO_CORE_C_DEF_H_
