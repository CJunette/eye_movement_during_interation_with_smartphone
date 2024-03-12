#include "seeso/util/display.h"

#include <windows.h>
#include <wtypes.h>
#include <tchar.h>

#include <cstdint>

#include <utility>
#include <vector>
#include <string>
#include <type_traits>

#pragma comment(lib, "user32.lib")
#pragma warning(disable:4996)

#define SEP '\\'
#define SEP_STR "\\"


namespace seeso {

static bool GetMonitorSizeFromEDID(DISPLAY_DEVICE& ddMon, DWORD& Width, DWORD& Height); // NOLINT(runtime/references) TODO(?)

std::vector<DisplayInfo> getDisplayLists() {
  std::vector<DisplayInfo> displays;

  DISPLAY_DEVICE dd;
  ZeroMemory(&dd, sizeof(dd));
  dd.cb = sizeof(DISPLAY_DEVICE);

  DWORD deviceNum = 0;
  while (EnumDisplayDevices(NULL, deviceNum, &dd, 0)) {
    DISPLAY_DEVICE newdd;
    ZeroMemory(&newdd, sizeof(newdd));
    newdd.cb = sizeof(DISPLAY_DEVICE);

    DWORD monitorNum = 0;
    while (EnumDisplayDevices(dd.DeviceName, monitorNum, &newdd, 0)) {
      DWORD mWidth = 0;
      DWORD mHeight = 0;
      if (GetMonitorSizeFromEDID(newdd, mWidth, mHeight)) {
        // obtain width, height
        DisplayInfo display;
        display.displayName = std::string(newdd.DeviceName);
        display.displayString = std::string(newdd.DeviceString);
        display.displayStateFlag = static_cast<int>(newdd.StateFlags);
        display.displayId = std::string(newdd.DeviceID);
        display.displayId_num = 0;
        display.displayKey = std::string(newdd.DeviceKey + 42);
        // cm -> mm
        display.widthMm = static_cast<float>(static_cast<int>(mWidth) * 10);
        display.heightMm = static_cast<float>(static_cast<int>(mHeight) * 10);

        // primary monitor's screen resolution
        DWORD dwWidth = GetSystemMetrics(SM_CXSCREEN);
        DWORD dwHeight = GetSystemMetrics(SM_CYSCREEN);
        display.widthPx = static_cast<int>(dwWidth);
        display.heightPx = static_cast<int>(dwHeight);

        displays.emplace_back(std::move(display));
      }
      monitorNum++;
    }
    deviceNum++;
  }

  return displays;
}

Point<long> getWindowPosition(const std::string& windowName) {
  HWND handle = FindWindowA(NULL, windowName.c_str());
  RECT rect;
  if (GetWindowRect(handle, &rect))
    return {rect.left, rect.top};
  return {0, 0};
}

Rect getWindowRect(const std::string& windowName) {
  HWND handle = FindWindowA(NULL, windowName.c_str());
  RECT rect;
  seeso::Rect result{};

  if (GetWindowRect(handle, &rect)) {
    result.x = static_cast<double>(rect.left);
    result.y = static_cast<double>(rect.top);
    result.width = static_cast<double>(rect.right - rect.left);
    result.height = static_cast<double>(rect.bottom - rect.top);
    return result;
  }
  return Rect{};
}

bool GetMonitorSizeFromEDID(DISPLAY_DEVICE& ddMon, DWORD& Width, DWORD& Height) { // NOLINT(runtime/references) TODO(?)
  // read edid
  bool result = false;
  Width = 0;
  Height = 0;
  TCHAR model[8];
  TCHAR* s = _tcschr(ddMon.DeviceID, SEP) + 1;
  size_t len = _tcschr(s, SEP) - s;
  if (len >= _countof(model))
    len = _countof(model) - 1;
  _tcsncpy_s(model, s, len);

  TCHAR* path = _tcschr(ddMon.DeviceID, SEP) + 1;
  TCHAR str[MAX_PATH] = _T("SYSTEM\\CurrentControlSet\\Enum\\DISPLAY\\");

//  _tcsncat_s(str, path, _tcschr(path, SEP) - path);
  _tcsncat_s(str, 1024, path, _tcschr(path, SEP) - path);

  path = _tcschr(path, SEP) + 1;
  HKEY hKey;
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, str, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
    DWORD i = 0;
    DWORD size = MAX_PATH;
    FILETIME ft;
    while (RegEnumKeyEx(hKey, i, str, &size, NULL, NULL, NULL, &ft) == ERROR_SUCCESS) {
      HKEY hKey2;
      if (RegOpenKeyEx(hKey, str, 0, KEY_READ, &hKey2) == ERROR_SUCCESS) {
        size = MAX_PATH;
        if (RegQueryValueEx(hKey2, _T("Driver"), NULL, NULL, (LPBYTE) &str, &size) == ERROR_SUCCESS) {
          if (_tcscmp(str, path) == 0) {
            HKEY hKey3;
            if (RegOpenKeyEx(hKey2, _T("Device Parameters"), 0, KEY_READ, &hKey3) == ERROR_SUCCESS) {
              BYTE EDID[256];
              size = 256;
              if (RegQueryValueEx(hKey3, _T("EDID"), NULL, NULL, (LPBYTE) &EDID, &size) == ERROR_SUCCESS) {
                DWORD p = 8;
                TCHAR model2[9];

                char byte1 = EDID[p];
                char byte2 = EDID[p + 1];
                model2[0] = ((byte1 & 0x7C) >> 2) + 64;
                model2[1] = ((byte1 & 3) << 3) + ((byte2 & 0xE0) >> 5) + 64;
                model2[2] = (byte2 & 0x1F) + 64;
                _stprintf(model2 + 3, _T("%X%X%X%X"), (EDID[p + 3] & 0xf0) >> 4, EDID[p + 3] & 0xf,
                          (EDID[p + 2] & 0xf0) >> 4, EDID[p + 2] & 0x0f);
                if (_tcscmp(model, model2) == 0) {
                  Width = EDID[21];
                  Height = EDID[22];
                  result = true;
                } else {
                  // EDID incorrect
                }
              }
              RegCloseKey(hKey3);
            }
          }
        }
        RegCloseKey(hKey2);
      }
      i++;
    }
    RegCloseKey(hKey);
  }
  return result;
}

} // namespace seeso
