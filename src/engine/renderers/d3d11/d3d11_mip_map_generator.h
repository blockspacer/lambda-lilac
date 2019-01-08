#pragma once
#include <iostream>

//#define LUMA_BASED
//#define SHARPEN

namespace lambda
{
  namespace utilities
  {
    ///////////////////////////////////////////////////////////////////////////
    void sharpeningFunc(
      unsigned char* tl,
      unsigned char* tr,
      unsigned char* bl,
      unsigned char* br,
      unsigned char* data
    )
    {
#ifdef LUMA_BASED
      auto luma_dot = [](unsigned char* rhs)
      {
        static const unsigned char luma[] = { 76, 150, 29 };
        return ((float)luma[0] / 255.0f) * ((float)rhs[0] / 255.0f) +
          ((float)luma[1] / 255.0f) * ((float)rhs[1] / 255.0f) +
          ((float)luma[2] / 255.0f) * ((float)rhs[2] / 255.0f) +
          ((float)luma[3] / 255.0f) * ((float)rhs[3] / 255.0f);
      };

      float lumaTL = luma_dot(tl);
      float lumaTR = luma_dot(tr);
      float lumaBL = luma_dot(bl);
      float lumaBR = luma_dot(br);

      // Possible error.
      float blur_x = -((lumaTL + lumaTR) - (lumaBL + lumaBR)); 
      float blur_y = ((lumaTL + lumaBL) - (lumaTR + lumaBR));

      unsigned char* s1 = nullptr;
      unsigned char* s2 = nullptr;
#ifdef SHARPEN
      if (std::fabsf(blur_x) > std::fabsf(blur_y))
#else
      if (std::fabsf(blur_x) < std::fabsf(blur_y))
#endif
      {
        if (blur_y < 0.0f)
        {
          s1 = tl;
          s2 = tr;
        }
        else
        {
          s1 = bl;
          s2 = br;
        }
      }
#ifdef SHARPEN
      else if (std::fabsf(blur_y) > std::fabsf(blur_x))
#else
      else if (std::fabsf(blur_y) < std::fabsf(blur_x))
#endif
      {
#ifdef SHARPEN
        if (blur_x < 0.0f)
#else
        if (blur_x > 0.0f)
#endif
        {
          s1 = tr;
          s2 = br;
        }
        else
        {
          s1 = tl;
          s2 = bl;
        }
      }
      else
      {
        data[0] = (tl[0] + tr[0] + bl[0] + br[0]) / 4;
        data[1] = (tl[1] + tr[1] + bl[1] + br[1]) / 4;
        data[2] = (tl[2] + tr[2] + bl[2] + br[2]) / 4;
        data[3] = (tl[3] + tr[3] + bl[3] + br[3]) / 4;
        return;
      }

      data[0] = (s1[0] + s2[0]) / 2;
      data[1] = (s1[1] + s2[1]) / 2;
      data[2] = (s1[2] + s2[2]) / 2;
      data[3] = (s1[3] + s2[3]) / 2;
#else
      data[0] = (tl[0] + tr[0] + bl[0] + br[0]) / 4;
      data[1] = (tl[1] + tr[1] + bl[1] + br[1]) / 4;
      data[2] = (tl[2] + tr[2] + bl[2] + br[2]) / 4;
      data[3] = (tl[3] + tr[3] + bl[3] + br[3]) / 4;
#endif
    }
  }
}