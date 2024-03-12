//
// Created by cosge on 2021-04-27.
//

#ifndef SEESO_CALLBACK_IUSER_STATUS_CALLBACK_H_
#define SEESO_CALLBACK_IUSER_STATUS_CALLBACK_H_

#include <cstdint>

namespace seeso {

class IUserStatusCallback {
 protected:
  IUserStatusCallback(const IUserStatusCallback&) = default;
  IUserStatusCallback& operator=(const IUserStatusCallback&) = default;

 public:
  IUserStatusCallback() = default;
  virtual ~IUserStatusCallback() = default;

  /** Attention Status Callback
   *
   * @param timestampBegin     Beginning timestamp (passed by EyeTracker::AddFrame())
   * @param timestampEnd       End timestamp (passed by EyeTracker::AddFrame())
   * @param score              Attention rate score (0.0f ~ 1.0f)
   */
  virtual void OnAttention(uint64_t timestampBegin, uint64_t timestampEnd, float score) = 0;

  /** Drowsiness Status Callback
   *
   * @param timestamp           timestamp (passed by EyeTracker::AddFrame())
   * @param isDrowsiness        Drowsiness flag (True/False)
   */
  virtual void OnDrowsiness(uint64_t timestamp, bool isDrowsiness) = 0;

  /** Blink Statis Callback
   *
   * @param timestamp           timestamp (passed by EyeTracker::AddFrame())
   * @param isBlinkLeft         Left Eye Blink flag (True/False)
   * @param isBlinkRight        Right Eye Blink flag (True/False)
   * @param isBlink             Eye Blink flag (True/False)
   * @param eyeOpenness         Eye Openness rate (0.0f ~ 1.0f)
   */
  virtual void OnBlink(uint64_t timestamp, bool isBlinkLeft, bool isBlinkRight, bool isBlink, float eyeOpenness) = 0;
};

} // namespace seeso

#endif // SEESO_CALLBACK_IUSER_STATUS_CALLBACK_H_
