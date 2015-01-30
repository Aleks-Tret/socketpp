#pragma once

#include <memory>

template <typename T, typename D, D Deleter>
struct stateless_deleter
{
  typedef T pointer;

  void operator()(T x)
  {
    Deleter(x);
  }
};
