/** @file colour_conversion.as */
#include "rgb.as"
#include "hsv.as"

/**
* @brief Converts an HSV value into an RGB value
* @return hsv (const HSV&) The HSV value that needs to be converted
* @return (RGB) The converted RGB value
* @author Hilze Vonck
**/
RGB HSVToRGB(const HSV&in hsv)
{
    RGB rgb;

    if(hsv.s == 0.0f)
    {
        rgb.r = hsv.v;
        rgb.g = hsv.v; 
        rgb.b = hsv.v;
    }
    else
    {
        float h = hsv.h;
        if(h == 360.0f)
        {
            h = 0.0f;
        }
        else
        {
            h /= 60.0f;
        }

        uint8 i = ToUint8(Trunc(h));
        float f = h - i;

        float p = hsv.v * (1.0f - hsv.s);
        float q = hsv.v * (1.0f - (hsv.s * f));
        float t = hsv.v * (1.0f - (hsv.s * (1.0f - f)));

        switch(i)
        {
        case 0:
            rgb.r = hsv.v;
            rgb.g = t;
            rgb.b = p;
            break;
        case 1:
            rgb.r = q;
            rgb.g = hsv.v;
            rgb.b = p;
            break;
        case 2:
            rgb.r = p;
            rgb.g = hsv.v;
            rgb.b = t;
            break;
        case 3:
            rgb.r = p;
            rgb.g = q;
            rgb.b = hsv.v;
            break;
        case 4:
            rgb.r = t;
            rgb.g = p;
            rgb.b = hsv.v;
            break;
        default:
            rgb.r = hsv.v;
            rgb.g = p;
            rgb.b = q;
            break;
        }
    }

    return rgb;
}

/**
* @brief Converts an RGB value into an HSV value
* @return rgb (const RGB&) The RGB value that needs to be converted
* @return (HSV) The converted HSV value
* @author Hilze Vonck
**/
HSV RGBToHSV(const RGB&in rgb)
{
    float r = rgb.r * 255.0f;
    float g = rgb.g * 255.0f;
    float b = rgb.b * 255.0f;
    HSV hsv;

    float min = Min(Min(r, g), b);
    hsv.v = Max(Max(r, g), b);
    float delta = hsv.v - min;

    if(hsv.v == 0.0f)
    {
        hsv.s = 0.0f;
    }
    else
    {
        hsv.s = delta / hsv.v;
    }

    if(hsv.s == 0.0f)
    {
        hsv.h = 0.0f;
    }
    else
    {
        if(r == hsv.v)
        {
            hsv.h = (g - b) / delta;
        }
        else if (g == hsv.v)
        {
            hsv.h = 2.0f + (g - r) / delta;
        }
        else if(b == hsv.v)
        {
            hsv.h = 4.0f + (r - g) / delta;
        }

        hsv.h *= 60.0f;

        if(hsv.h < 0.0f)
        {
            hsv.h += 360.0f;
        }
    }

    hsv.v /= 255.0f;
    return hsv;
}