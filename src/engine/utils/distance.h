#pragma once

namespace lambda
{
  namespace utilities
  {
    class Distance
    {
    public:
      Distance(float meter = 0.0f) : meter_(meter) {}

      float asMeter() const { return meter_; }
      float asCentimeter() const { return meter_ * meterToCentimeter(); }
      float asMillimeter() const { return meter_ * meterToMillimeter(); }

      void setMeter(const float& meter) { meter_ = meter; }
      void setCentimeter(const float& centimeter) { meter_ = centimeter * centimeterToMeter(); }
      void setMillimeter(const float& millimeter) { meter_ = millimeter * millimeterToMeter(); }

      static Distance fromMeter     (const float& meter)      { return Distance(meter); }
      static Distance fromCentimeter(const float& centimeter) { return Distance(centimeter * centimeterToMeter()); }
      static Distance fromMillimeter(const float& millimeter) { return Distance(millimeter * millimeterToMeter()); }

      static float meterToMillimeter()      { return 1000.0f; }
      static float meterToCentimeter()      { return 100.0f;  }
      static float centimeterToMeter()      { return 0.01f;   }
      static float centimeterToMillimeter() { return 10.0f;   }
      static float millimeterToMeter()      { return 0.001f;  }
      static float millimeterToCentimeter() { return 0.1f;    }

    private:
      float meter_;
    };
  }
}