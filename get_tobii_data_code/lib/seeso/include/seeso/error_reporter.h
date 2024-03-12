//
// Created by YongGyu Lee on 2021/10/12.
//

#ifndef SEESO_ERROR_REPORTER_H_
#define SEESO_ERROR_REPORTER_H_

#include <string>

namespace seeso {

class ErrorReporter {
 public:
  virtual ~ErrorReporter() = default;
  virtual int report(const char* message) = 0;

  int report(const std::string& message);
};

ErrorReporter* DefaultErrorReporter();

} // namespace seeso

#endif // SEESO_ERROR_REPORTER_H_
