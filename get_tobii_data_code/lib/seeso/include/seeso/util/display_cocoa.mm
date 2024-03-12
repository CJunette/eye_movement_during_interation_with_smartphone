#include "seeso/util/display.h"

#include <cstdint>

#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>

#include <CoreGraphics/CGDisplayConfiguration.h>

@interface WindowTracker : NSObject
+ (WindowTracker *)sharedInstance;
- (seeso::Rect)getWindowRectWithName:(NSString *) name;
- (NSWindow * _Nullable )findWindowWithName:(NSString *) name;
@property(nonatomic, assign)  NSApplication *application;
@end

@implementation WindowTracker

+ (WindowTracker *)sharedInstance {
  static WindowTracker *shared = nil;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    shared = [[WindowTracker alloc] init];
    shared.application = [NSApplication sharedApplication];
  });
  return shared;
}

- (NSWindow *)findWindowWithName:(NSString *) name{
  auto windows = [_application windows];
  for(NSWindow * window in windows) {
    if ([window.miniwindowTitle isEqualToString:(name)]) {
      return window;
    }
  }
  return nil;
}

- (seeso::Rect)getWindowRectWithName:(NSString *) name {
  auto window = [self findWindowWithName:name];
  if(window != nil){
    return { window.frame.origin.x, window.frame.origin.y, window.frame.size.width, window.frame.size.height };
  }
  return seeso::Rect();
}

@end

seeso::Rect getCurrentWindowRect(const std::string& name){
  auto convertString = [NSString stringWithUTF8String:name.c_str()];
  auto rect = [[WindowTracker sharedInstance] getWindowRectWithName: convertString];
  return rect;
}

namespace seeso {

static int global_display_height = -1;
static int global_display_id = 0;

static int convertYaxis(int y, int height, int display_id = 0) {
  if ((global_display_height < 0 || global_display_id != display_id) && display_id >= 0) {
    const auto displays = getDisplayLists();
    if (displays.size() >= display_id + 1)
      global_display_height = displays[display_id].heightPx;
  }

  return global_display_height - y - height;
}


std::vector<DisplayInfo> getDisplayLists() {
  static constexpr int max_display = 100;
  std::vector<CGDirectDisplayID> display_ids;
  std::vector<DisplayInfo> result;
  // get display IDs
  {
    uint32_t display_number;
    std::vector<CGDirectDisplayID> display_ids_(max_display);
    CGGetActiveDisplayList(max_display, display_ids_.data(), &display_number);
    display_ids.insert(display_ids.begin(), display_ids_.begin(), display_ids_.begin() + display_number);
  }

  for (auto id : display_ids) {
    auto px1 = CGDisplayPixelsWide(id);
    auto py1 = CGDisplayPixelsHigh(id);

    auto size_mm = CGDisplayScreenSize(id);
    result.push_back({
      "", // display_name
      "", // display_string
      0,  // display_state_flag
      "", // display_id
      id, // display_id_num
      "", // display_key
      static_cast<float>(size_mm.width),  // width_mm
      static_cast<float>(size_mm.height), // height_mm
      static_cast<int>(px1),  // width_px
      static_cast<int>(py1)   // height_px
    });
    CGDisplayRelease(id);
  }

  return result;
}

Rect getWindowRect(const std::string& name) {
  auto windowRect = getCurrentWindowRect(name);
  windowRect.y = convertYaxis(windowRect.y, windowRect.height);
  return windowRect;
}

Point<long> getWindowPosition(const std::string& windowName) {
  auto windowRect = getCurrentWindowRect(windowName);
  windowRect.y = convertYaxis(windowRect.y, windowRect.height);
  return {static_cast<long>(windowRect.x), static_cast<long>(windowRect.y)};
}

} // namespace seeso
