//
// Created by YongGyu Lee on 2021/11/17.
//

#ifndef SEESO_FRAMEWORK_GLOBAL_INIT_H_
#define SEESO_FRAMEWORK_GLOBAL_INIT_H_


// This file must only be included in gaze_tracker.cc

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <windows.h>

#include <iostream>
#include <exception>

#include "seeso/framework/dll_function.h"

#define SEESO_SEESO_SET_DLL_FUNCTION_IMPL(hinst, target, name)  \
do {                                                \
  target.setFuncPtr(GetProcAddress(hinst, name));   \
  if (target == nullptr) {                          \
    std::cerr                                       \
      << "Failed to find " name " from dll\n";      \
    throw std::runtime_error(                       \
      "Failed to find " name " from dll\n");        \
  }                                                 \
} while (false)

#define SEESO_SET_DLL_FUNCTION(hinst, name) \
  SEESO_SEESO_SET_DLL_FUNCTION_IMPL(hinst, name, #name)

#define SEESO_SET_DLL_FUNCTION_PREFIX(prefix, hinst, name) \
  SEESO_SEESO_SET_DLL_FUNCTION_IMPL(hinst, prefix##name, #prefix #name)

#define SEESO_DECLARE_DLL_FUNCTION_LINKER(name) \
  DLLFunction<decltype(::name)> name

namespace seeso {
namespace { // NOLINT(build/namespaces_headers)
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoVersionString);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoVersionInteger);

SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsCreate);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsDelete);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsSetCameraFOV);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsSetNumThreads);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsSetErrorReporter);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerOptionsEnableStatusModule);

SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerCreate);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerDelete);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerInit);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerDeInit);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetCameraFOV);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerInitialized);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetFPS);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetFaceDistance);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetFixationCount);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetFilterType);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetTargetBoundRegion);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerAddFrame);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetSensorRotation);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerGetSensorRotation);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerGetAuthorizationResult);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerStartCalibration);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerEnhanceCalibration);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerStartCollectSamples);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerStopCalibration);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetCalibrationData);

SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetGazeCallback);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetFaceCallback);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetStatusCallback);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetCalibrationCallback);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetCallbackUserData);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerRemoveCallbackInterface);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetAttentionRegion);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerGetAttentionRegion);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerRemoveAttentionRegion);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerSetAttentionInterval);
SEESO_DECLARE_DLL_FUNCTION_LINKER(SeeSoTrackerGetAttentionScore);


} // anonymous namespace

void global_init(const char* file) {
  auto procIDDLL = LoadLibrary(file);
  // It will be NULL even if a *file* is found, but when other dlls that this program needs are missing
  if (procIDDLL == NULL) {
    std::cerr
      << "Failed to load 'seeso_core.dll'\n"
      << "Check if 'seeso_core.dll' in in the same directory with an executable file.\n"
      << "Even if 'seeso_core.dll' exists, loading can fail if one or more of the following files are missing:\n"
      << "'libcrypto-1_1-x64.dll', 'libcurl.dll', 'libssl-1_1-x64.dll', 'opencv_world410.dll'\n";
    throw std::runtime_error("Failed to load a dll");
  }

  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoVersionString);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoVersionInteger);

  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsCreate);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsDelete);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsSetCameraFOV);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsSetNumThreads);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsSetErrorReporter);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerOptionsEnableStatusModule);

  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerCreate);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerDelete);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerInit);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerDeInit);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetCameraFOV);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerInitialized);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetFPS);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetFaceDistance);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetFixationCount);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetFilterType);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetTargetBoundRegion);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerAddFrame);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetSensorRotation);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerGetSensorRotation);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerGetAuthorizationResult);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerStartCalibration);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerEnhanceCalibration);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerStartCollectSamples);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerStopCalibration);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetCalibrationData);

  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetGazeCallback);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetFaceCallback);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetStatusCallback);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetCalibrationCallback);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetCallbackUserData);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerRemoveCallbackInterface);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetAttentionRegion);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerGetAttentionRegion);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerRemoveAttentionRegion);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerSetAttentionInterval);
  SEESO_SET_DLL_FUNCTION(procIDDLL, SeeSoTrackerGetAttentionScore);
}

} // namespace seeso

#else

namespace seeso {

void global_init(const char*) {}

} // namespace seeso
#endif

#endif // SEESO_FRAMEWORK_GLOBAL_INIT_H_
