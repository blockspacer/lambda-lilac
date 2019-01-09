/** @file hsv.as */

/**
* @class HSV
* @brief An HSV representation of a colour
* @author Hilze Vonck
**/
class HSV
{
    /**
    * @brief Constructor which initializes this colour to white
    * @public
    **/  
    HSV()
    {
        h = s = v = 1.0f;
    }
    /**
    * @brief Constructor which initializes this colour to a given h, s and v value
    * @param h (const float) The hue value that this colour should have
    * @param s (const float) The saturation value that this colour should have
    * @param v (const float) The value value that this colour should have
    * @public
    **/  
    HSV(const float&in h, const float&in s, const float&in v)
    {
        this.h = h;
        this.s = s;
        this.v = v;
    }

    float h; //!< The hue value
    float s; //!< The saturation value
    float v; //!< The value value
}
