/** @file colour.as */
#include "rgb.as"
#include "hsv.as"
#include "colour_conversion.as"

/**
* @class Colour
* @brief A universal colour class that will make using and managing different colour spaces easier
* @author Hilze Vonck
**/
class Colour
{
    /**
    * @brief Constructor which initializes the colour to an RGB value 
    * @param rgb (const RGB&) The rgb value that this colour needs to be initialized to 
    * @public
    **/  
    Colour(const RGB&in rgb)
    {
        global_format = rgb;
    }
    /**
    * @brief Constructor which initializes the colour to an hsv value 
    * @param hsv (const HSV&) The hsv value that this colour needs to be initialized to 
    * @public
    **/  
    Colour(const HSV&in hsv)
    {
        global_format = HSVToRGB(hsv);
    }
    /**
    * @brief Returns this colour value as an RGB value 
    * @return (RGB) This colours value represented in RGB 
    * @public
    **/  
    RGB AsRGB() const
    {
        return global_format;
    }
    /**
    * @brief Returns this colour value as an HSV value 
    * @return (HSV) This colours value represented in HSV 
    * @public
    **/  
    HSV AsHSV() const
    {
        return RGBToHSV(global_format);
    }
    /**
    * @brief Set this colour to a different RGB value
    * @param rgb (const RGB&) The new RGB colour that this colour should store 
    * @public
    **/  
    void SetColour(const RGB&in rgb)
    {
        global_format = rgb;
    }
    /**
    * @brief Set this colour to a different HSV value
    * @param hsv (const HSV&) The new HSV colour that this colour should store 
    * @public
    **/  
    void SetColour(const HSV&in hsv)
    {
        global_format = HSVToRGB(hsv);
    }

    private RGB global_format; //!< The stored colour. Uses RGB since that is the most commonly used colour space
}