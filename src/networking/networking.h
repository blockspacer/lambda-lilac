#pragma once
#include "server.h"
#include "client.h"

namespace lambda
{
  namespace networking
  {
    extern void initializeNetworking();
    extern void deinitializeNetworking();
  }
}