/** @file rgb.as */

/**
* @class RGB
* @brief An RGB representation of a colour
* @author Hilze Vonck
**/
class RGB
{
    /**
    * @brief Constructor which initializes this colour to white
    * @public
    **/
    RGB()
    {
        r = g = b = 1.0f;
    }
    /**
    * @brief Constructor which initializes this colour to a given r, g and b value
    * @param r (const float) The red value that this colour should have
    * @param g (const float) The green value that this colour should have
    * @param b (const float) The blue value that this colour should have
    * @public
    **/
    RGB(const float&in r, const float&in g, const float&in b)
    {
        this.r = r;
        this.g = g;
        this.b = b;
    }
    RGB(const Vec3&in v)
    {
        this.r = v.x;
        this.g = v.y;
        this.b = v.z;
    }
    Vec3 AsVec3() const
    {
      return Vec3(r, g, b);
    }
    void FromVec3(const Vec3&in v)
    {
      r = v.x;
      g = v.y;
      b = v.z;
    }

    float r; //!< The red value
    float g; //!< The green value
    float b; //!< The blue value
}
