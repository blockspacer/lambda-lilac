/** @file wave.as */

/**
* @addtogroup Assets
* @{
**/

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
  /**
  * @class Asset::Wave
  * @brief A wrapper class for waves. Allows you to interact with engine waves.
  * @author Hilze Vonck
  **/
  class Wave
  {
    /**
    * @brief Constructor which will initialize this wave to invalid
    * @public
    **/
    Wave()
    {
      this.id = ToUint64(-1);
    }
    /**
    * @brief Constructor which will initialize this wave to a given id
    * @public
    **/
    Wave(const uint64&in id)
    {
      this.id = id;
    }
    /**
    * @brief Constructor which will initialize this wave to the same id as the other wave
    * @param other (Asset::Wave) The wave which needs to be copied
    * @public
    **/
    Wave(const Wave&in other)
    {
      id = other.id;
    }
    /**
    * @brief Get the engine id of the wave
    * @return (uint64) The engine id of the wave
    * @public
    **/
    uint64 GetId() const
    {
      return id;
    }

    private uint64 id;
  }
}

/**
* @}
**/
