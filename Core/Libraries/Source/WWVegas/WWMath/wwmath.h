#pragma once

#include "always.h"
#include <assert.h>
#include <float.h>
#include <math.h>

#define WWMATH_EPSILON 0.0001f
#define WWMATH_EPSILON2 (WWMATH_EPSILON * WWMATH_EPSILON)
#define WWMATH_PI 3.141592654f
#define WWMATH_TWO_PI 6.283185308f
#define WWMATH_FLOAT_MAX (FLT_MAX)
#define WWMATH_FLOAT_MIN (FLT_MIN)
#define WWMATH_SQRT2 1.414213562f
#define WWMATH_SQRT3 1.732050808f
#define WWMATH_OOSQRT2 0.707106781f
#define WWMATH_OOSQRT3 0.577350269f

#ifndef RAD_TO_DEG
#define RAD_TO_DEG(x) (((double)x) * 180.0 / WWMATH_PI)
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD(x) (((double)x) * WWMATH_PI / 180.0)
#endif

#ifndef RAD_TO_DEGF
#define RAD_TO_DEGF(x) (((float)x) * 180.0f / WWMATH_PI)
#endif

#ifndef DEG_TO_RADF
#define DEG_TO_RADF(x) (((float)x) * WWMATH_PI / 180.0f)
#endif

// Forward declarations of the tables
extern float _FastAcosTable[1024];
extern float _FastAsinTable[1024];
extern float _FastSinTable[1024];
extern float _FastInvSinTable[1024];

class WWMath {
public:
  enum { ARC_TABLE_SIZE = 1024, SIN_TABLE_SIZE = 1024 };

  static void Init(void);
  static void Shutdown(void);

  static WWINLINE float Fabs(float val) {
    int value = *(int *)&val;
    value &= 0x7fffffff;
    return *(float *)&value;
  }

  static WWINLINE int Float_To_Int_Chop(const float &f) { return (int)f; }
  static WWINLINE int Float_To_Int_Floor(const float &f) {
    return (int)floorf(f);
  }

  static WWINLINE float Cos(float val) { return cosf(val); }
  static WWINLINE float Sin(float val) { return sinf(val); }
  static WWINLINE float Sqrt(float val) { return sqrtf(val); }
  static WWINLINE float Inv_Sqrt(float val) { return 1.0f / sqrtf(val); }
  static WWINLINE long Float_To_Long(float f) { return (long)f; }
  static WWINLINE long Float_To_Long(double f) { return (long)f; }

  static WWINLINE float Fast_Sin(float val) {
    val *= float(SIN_TABLE_SIZE) / (2.0f * WWMATH_PI);
    int idx0 = (int)floorf(val);
    float frac = val - (float)idx0;
    idx0 = ((unsigned)idx0) & (SIN_TABLE_SIZE - 1);
    int idx1 = (idx0 + 1) & (SIN_TABLE_SIZE - 1);
    return (1.0f - frac) * _FastSinTable[idx0] + frac * _FastSinTable[idx1];
  }

  static WWINLINE float Fast_Inv_Sin(float val) { return 1.0f / Fast_Sin(val); }
  static WWINLINE float Fast_Cos(float val) {
    return Fast_Sin(val + (WWMATH_PI * 0.5f));
  }
  static WWINLINE float Fast_Inv_Cos(float val) { return 1.0f / Fast_Cos(val); }

  static WWINLINE float Fast_Acos(float val) {
    if (fabsf(val) > 0.975f)
      return acosf(val);
    val *= float(ARC_TABLE_SIZE / 2);
    int idx0 = (int)floorf(val);
    float frac = val - (float)idx0;
    idx0 += ARC_TABLE_SIZE / 2;
    int idx1 = idx0 + 1;
    return (1.0f - frac) * _FastAcosTable[idx0] + frac * _FastAcosTable[idx1];
  }

  static WWINLINE float Acos(float val) { return acosf(val); }

  static WWINLINE float Fast_Asin(float val) {
    if (fabsf(val) > 0.975f)
      return asinf(val);
    val *= float(ARC_TABLE_SIZE / 2);
    int idx0 = (int)floorf(val);
    float frac = val - (float)idx0;
    idx0 += ARC_TABLE_SIZE / 2;
    int idx1 = idx0 + 1;
    return (1.0f - frac) * _FastAsinTable[idx0] + frac * _FastAsinTable[idx1];
  }

  static WWINLINE float Asin(float val) { return asinf(val); }

  static WWINLINE float Atan(float x) { return static_cast<float>(atan(x)); }
  static WWINLINE float Atan2(float y, float x) {
    return static_cast<float>(atan2(y, x));
  }

  static WWINLINE float Sign(float val) {
    if (val > 0.0f)
      return +1.0f;
    if (val < 0.0f)
      return -1.0f;
    return 0.0f;
  }

  static WWINLINE float Ceil(float val) { return ceilf(val); }
  static WWINLINE float Floor(float val) { return floorf(val); }
  static WWINLINE float Round(float val) { return floorf(val + 0.5f); }

  static WWINLINE bool Fast_Is_Float_Positive(const float &val) {
    return !((*(int *)(&val)) & 0x80000000);
  }

  static WWINLINE bool Is_Power_Of_2(const unsigned int val) {
    return val && !(val & (val - 1));
  }

  static float Random_Float(void);

  static WWINLINE float Random_Float(float min, float max) {
    return Random_Float() * (max - min) + min;
  }

  static WWINLINE float Clamp(float val, float min = 0.0f, float max = 1.0f) {
    if (val < min)
      return min;
    if (val > max)
      return max;
    return val;
  }

  static WWINLINE double Clamp(double val, double min = 0.0f,
                               double max = 1.0f) {
    if (val < min)
      return min;
    if (val > max)
      return max;
    return val;
  }

  static WWINLINE int Clamp_Int(int val, int min_val, int max_val) {
    if (val < min_val)
      return min_val;
    if (val > max_val)
      return max_val;
    return val;
  }

  static WWINLINE float Wrap(float val, float min = 0.0f, float max = 1.0f) {
    float range = max - min;
    if (val >= max)
      val -= range;
    if (val < min)
      val += range;
    if (val < min)
      val = min;
    if (val > max)
      val = max;
    return val;
  }

  static WWINLINE double Wrap(double val, double min = 0.0f,
                              double max = 1.0f) {
    double range = max - min;
    if (val >= max)
      val -= range;
    if (val < min)
      val += range;
    if (val < min)
      val = min;
    if (val > max)
      val = max;
    return val;
  }

  static WWINLINE float Min(float a, float b) { return (a < b) ? a : b; }
  static WWINLINE float Max(float a, float b) { return (a > b) ? a : b; }

  static WWINLINE int Float_As_Int(const float f) { return *((int *)&f); }

  static WWINLINE float Lerp(float a, float b, float lerp) {
    return (a + (b - a) * lerp);
  }
  static WWINLINE double Lerp(double a, double b, float lerp) {
    return (a + (b - a) * lerp);
  }

  static WWINLINE unsigned char Unit_Float_To_Byte(float f) {
    return (unsigned char)(f * 255.0f);
  }
  static WWINLINE float Byte_To_Unit_Float(unsigned char byte) {
    return ((float)byte) / 255.0f;
  }

  static WWINLINE bool Is_Valid_Float(float x) {
    unsigned int *p = (unsigned int *)&x;
    return ((*p) & 0x7f800000) != 0x7f800000;
  }

  static WWINLINE bool Is_Valid_Double(double x) {
    unsigned long long *p = (unsigned long long *)&x;
    return ((*p) & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
  }

  static WWINLINE float Normalize_Angle(float angle) {
    return angle -
           (WWMATH_TWO_PI * floorf((angle + WWMATH_PI) / WWMATH_TWO_PI));
  }
};

template <class T> WWINLINE void Swap(T &a, T &b) {
  T temp = a;
  a = b;
  b = temp;
}
