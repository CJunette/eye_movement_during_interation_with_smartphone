//
// Created by YongGyu Lee on 2021/10/12.
//

#include "seeso/error_reporter.h"

#include <cstdio>

#include <string>

namespace seeso {

int ErrorReporter::report(const std::string& message) {
  return report(message.c_str());
}

class DefaultErrorReporterImpl : public ErrorReporter {
 public:
  int report(const char* message) override {
    std::fprintf(stderr, "SeeSo Error: %s\n", message);
    return 0;
  }
};

ErrorReporter* DefaultErrorReporter() {
  static ErrorReporter* reporter = new DefaultErrorReporterImpl();
  return reporter;
}

} // namespace seeso
