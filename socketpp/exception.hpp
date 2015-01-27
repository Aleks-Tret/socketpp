#pragma once

#include <exception>

namespace socketpp {
  class SocketException: public std::exception {
  };
}
