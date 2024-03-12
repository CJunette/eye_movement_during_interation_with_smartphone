//
// Created by YongGyu Lee on 2021/04/29.
//

#ifndef SEESO_CORE_C_API_H_ // NOLINT(build/header_guard)
#define SEESO_CORE_C_API_H_

// C API for SeeSo SDK
//
// C API is experimental, so the following interfaces
// are likely to be changed until the stable version is released.

#include <stdarg.h>
#include <stdint.h>

#include "c_def.h" // NOLINT(build/include_subdir)

#ifdef EMSCRIPTEN
#   include <emscripten.h>
#   define SEESO_EXPORT EMSCRIPTEN_KEEPALIVE
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#   ifdef SEESO_EXPORT_DLL
#     define SEESO_EXPORT __declspec(dllexport)
#   else
#     define SEESO_EXPORT __declspec(dllimport)
#   endif
#elif defined __GNUC__ && __GNUC__ >= 4 && defined(__APPLE__) && defined(__MACH__)
#   define SEESO_EXPORT __attribute__ ((visibility ("default")))
# else
#   define SEESO_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Global functions

// Get version as string.
// Same with SeeSoVersionInteger() + string suffix (suffix may be empty)
// Use SeeSoVersionInteger() to parse version more easily
SEESO_EXPORT const char* SeeSoVersionString();

// Get version as integer. 9-digit integer, 00 00 00000 (major, minor, patch)
SEESO_EXPORT int32_t SeeSoVersionInteger();

// SeeSoTrackerOptions
// Create SeeSoTracker with SeeSoTrackerOptions
typedef struct SeeSoTrackerOptions SeeSoTrackerOptions;

// Return a new options with default settings.
//
// FoV = pi / 4
// num_threads = 3
// status_module: Disabled
SEESO_EXPORT SeeSoTrackerOptions* SeeSoTrackerOptionsCreate();

SEESO_EXPORT void SeeSoTrackerOptionsDelete(SeeSoTrackerOptions* options);

SEESO_EXPORT void SeeSoTrackerOptionsSetCameraFOV(SeeSoTrackerOptions* options, float fov);

SEESO_EXPORT void SeeSoTrackerOptionsSetNumThreads(SeeSoTrackerOptions* options, int32_t num_threads);

// WARNING: Error reporting interface is experimental. Not all errors will be reported through reporter.
// Current reporting errors:
//   - Status region not set
SEESO_EXPORT void SeeSoTrackerOptionsSetErrorReporter(SeeSoTrackerOptions* options,
                                                      int (*reporter)(void* user_data, const char* msg),
                                                      void* user_data);

// WARNING: This interface is likely to be changed in the future
SEESO_EXPORT void SeeSoTrackerOptionsEnableStatusModule(SeeSoTrackerOptions* options,
                                                        const SeeSoStatusModuleOptions* status_options);


// SeeSoTracker
typedef struct SeeSoTracker SeeSoTracker;

// TODO(?): Merge Create and Init
SEESO_EXPORT SeeSoTracker* SeeSoTrackerCreate(const char* license_key, uint32_t key_length);

SEESO_EXPORT void SeeSoTrackerDelete(SeeSoTracker* obj);

// TODO(?): handle invalid settings
SEESO_EXPORT void SeeSoTrackerInit(SeeSoTracker* obj, const SeeSoTrackerOptions* options);

SEESO_EXPORT void SeeSoTrackerSetCameraFOV(SeeSoTracker* obj, float fov);

SEESO_EXPORT void SeeSoTrackerDeInit(SeeSoTracker* obj);

SEESO_EXPORT int SeeSoTrackerInitialized(const SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerSetFPS(SeeSoTracker* obj, int32_t fps);

SEESO_EXPORT void SeeSoTrackerSetFaceDistance(SeeSoTracker* obj, int32_t distance_mm);

// WARNING: This is no-op
SEESO_EXPORT void SeeSoTrackerSetFixationCount(SeeSoTracker* obj, uint32_t count);

// WARNING: This is no-op
SEESO_EXPORT void SeeSoTrackerSetFilterType(SeeSoTracker* obj, int32_t type);

SEESO_EXPORT int SeeSoTrackerAddFrame(SeeSoTracker* obj, int64_t timestamp, uint8_t* buffer,
                                      int32_t width, int32_t height);

// WARNING: Experimental. Likely to be changed in the future.
SEESO_EXPORT void SeeSoTrackerSetSensorRotation(SeeSoTracker* obj, int32_t rotation);

// WARNING: Experimental. Likely to be changed in the future.
SEESO_EXPORT int32_t SeeSoTrackerGetSensorRotation(const SeeSoTracker* obj);

SEESO_EXPORT int SeeSoTrackerGetAuthorizationResult(const SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerSetTargetBoundRegion(SeeSoTracker* obj, float left, float top,
                                                   float right, float bottom);

SEESO_EXPORT void SeeSoTrackerStartCalibration(SeeSoTracker* obj, const SeeSoCalibrationOptions* options);

SEESO_EXPORT void SeeSoTrackerEnhanceCalibration(SeeSoTracker* obj, const SeeSoCalibrationOptions* options);

SEESO_EXPORT void SeeSoTrackerStartCollectSamples(SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerStopCalibration(SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerSetCalibrationData(SeeSoTracker* obj, const float* data, uint32_t size);

typedef void(*SeeSoOnGaze      )(void* user_data, const SeeSoGazeData* data);
typedef void(*SeeSoOnFace      )(void* user_data, const SeeSoFaceData* data);

typedef void(*SeeSoOnAttention )(void* user_data, const SeeSoAttentionData* data);
typedef void(*SeeSoOnStatus    )(void* user_data, const SeeSoStatusData* data);

typedef void(*SeeSoOnCalibrationNextPoint)(void* user_data, float x, float y);
typedef void(*SeeSoOnCalibrationProgress )(void* user_data, float progress);
typedef void(*SeeSoOnCalibrationFinish   )(void* user_data, const float* data, uint32_t data_size);

// Attach callbacks to the SeeSoTracker.
SEESO_EXPORT void SeeSoTrackerSetGazeCallback(SeeSoTracker* tracker, SeeSoOnGaze on_gaze);

SEESO_EXPORT void SeeSoTrackerSetFaceCallback(SeeSoTracker* tracker, SeeSoOnFace on_face);

SEESO_EXPORT void SeeSoTrackerSetStatusCallback(SeeSoTracker* tracker,
                                                SeeSoOnAttention attention_callback,
                                                SeeSoOnStatus status_callback);

SEESO_EXPORT void SeeSoTrackerSetCalibrationCallback(SeeSoTracker* tracker,
                                                     SeeSoOnCalibrationNextPoint on_calib_next_point,
                                                     SeeSoOnCalibrationProgress on_calib_progress,
                                                     SeeSoOnCalibrationFinish on_calib_finish);

// Set custom data for callback output
SEESO_EXPORT void SeeSoTrackerSetCallbackUserData(SeeSoTracker* tracker, void* user_data);

SEESO_EXPORT void SeeSoTrackerRemoveCallbackInterface(SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerSetAttentionRegion(SeeSoTracker* obj,
                                                 float left, float top, float right, float bottom);

SEESO_EXPORT int SeeSoTrackerGetAttentionRegion(const SeeSoTracker* obj, float* dst);

SEESO_EXPORT void SeeSoTrackerRemoveAttentionRegion(SeeSoTracker* obj);

SEESO_EXPORT void SeeSoTrackerSetAttentionInterval(SeeSoTracker* obj, int interval);

SEESO_EXPORT float SeeSoTrackerGetAttentionScore(const SeeSoTracker* obj);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif // SEESO_CORE_C_API_H_
