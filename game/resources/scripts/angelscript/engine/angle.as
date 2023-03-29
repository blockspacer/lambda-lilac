/** @file angle.as */

namespace Utility //! Namespace containing all utility classes, enums and functions
{
    /**
    * @class Utility::Angle
    * @brief Helper class to make using angles easier.
    * @author Hilze Vonck
    **/
    class Angle
    {
        /**
        * @brief Constructor which will set the angle in radians
        * @param rad (const float) Angle in radians
        * @public
        **/
        Angle(const float&in rad)
        {
            this.rad = rad;
        }
        /**
        * @brief Constructor which sets the angle to zero
        * @public
        **/
        Angle()
        {
            rad = 0.0f;
        }
        /**
        * @brief Returns the angle in radians
        * @return (float) Angle in radians
        * @public
        **/
        float AsRad() const
        {
            return rad;
        }
        /**
        * @brief Returns the angle in degrees
        * @return (float) Angle in degrees
        * @public
        **/
        float AsDeg() const
        {
            return rad * RadToDeg;
        }
        /**
        * @brief Sets the angle to a radian value
        * @param rad (const float) New angle in radians
        **/
        void SetRad(const float&in rad)
        {
            this.rad = rad;
        }
        /**
        * @brief Sets the angle to a degree value
        * @param deg (const float) New angle in degrees
        * @public
        **/
        void SetDeg(const float&in deg)
        {
            rad = deg * DegToRad;
        }

        private float rad; //!< Angle is internally stored in radians
    }

    /**
    * @brief Creates a new angle for you from a radian value
    * @param rad (const float) Angle in radians
    * @return (Utility::Angle) Angle constructed from a radian value
    * @author Hilze Vonck
    **/
    Angle AngleFromRad(const float rad)
    {
        return Angle(rad);
    }
    /**
    * @brief Creates a new angle for you from a degree value
    * @param deg (const float) Angle in degrees
    * @return (Asset::Angle) Angle constructed from a degree value
    * @author Hilze Vonck
    **/
    Angle AngleFromDeg(const float deg)
    {
        return Angle(deg * DegToRad);
    }
}