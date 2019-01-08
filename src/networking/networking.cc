#include "networking.h"
#include <enet/enet.h>
#include <utils/console.h>

namespace lambda
{
  namespace networking
  {
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void initializeNetworking()
    {
      if (enet_initialize() != 0)
      {
        LMB_LOG_ERR("[ENET] An error occurred while initializing.");
      }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    void deinitializeNetworking()
    {
      enet_deinitialize();
    }
  }
}