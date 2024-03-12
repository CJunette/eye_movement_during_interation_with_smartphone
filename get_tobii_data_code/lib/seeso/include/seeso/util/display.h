#ifndef SEESO_UTIL_DISPLAY_H_
#define SEESO_UTIL_DISPLAY_H_

#include <cstdint>

#include <string>
#include <vector>
#include <tuple>

#include "seeso/util/point.h"

namespace seeso {

struct DisplayInfo {
  std::string displayName;
  std::string displayString;
  int displayStateFlag;
  std::string displayId;
  uint32_t displayId_num;
  std::string displayKey;
  float widthMm;
  float heightMm;
  int widthPx;
  int heightPx;
};

std::vector<DisplayInfo> getDisplayLists();

Point<long> getWindowPosition(const std::string& windowName);

struct Rect {
  double x = 0, y = 0;
  double width = 0, height = 0;
};

Rect getWindowRect(const std::string& name);


} // namespace seeso

#endif // SEESO_UTIL_DISPLAY_H_
