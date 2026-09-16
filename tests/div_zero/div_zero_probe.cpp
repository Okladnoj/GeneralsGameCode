// Prints what division by zero and the float to int conversions give with this toolchain.
// The conversion helpers and setFPMode are copied from BaseType.h and GameLogic.cpp as they are.
// Written for VC6 as well, so it keeps to C++98 and the old CRT.

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#if !defined(_MSC_VER)
#include <fenv.h>

// ---- compat.h, f716706ccb ----

#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif
#endif

typedef int Int;
typedef float Real;

volatile Real g_zero = 0.0f;
volatile Real g_one = 1.0f;
volatile double g_zeroDouble = 0.0;
volatile double g_oneDouble = 1.0;

// ---- BaseType.h, f716706ccb ----

__forceinline long fast_float2long_round(float f)
{
	long i;

#if defined(_MSC_VER) && _MSC_VER < 1300
	__asm {
		fld [f]
		fistp [i]
	}
#else
	i = lroundf(f);
#endif

	return i;
}

__forceinline float fast_float_trunc(float f)
{
#if defined(_MSC_VER) && _MSC_VER < 1300
  _asm
  {
    mov ecx,[f]
    shr ecx,23
    mov eax,0xff800000
    xor ebx,ebx
    sub cl,127
    cmovc eax,ebx
    sar eax,cl
    and [f],eax
  }
  return f;
#else
  unsigned x = *(unsigned *)&f;
  unsigned char exp = x >> 23;
  int mask = exp < 127 ? 0 : 0xff800000;
  exp -= 127;
  mask >>= exp & 31;
  x &= mask;
  return *(float *)&x;
#endif
}

__forceinline float fast_float_floor(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if (*(unsigned *)&f &0x80000000)
    f-=*(float *)&almost1;
  return fast_float_trunc(f);
}

__forceinline float fast_float_ceil(float f)
{
  static unsigned almost1=(126<<23)|0x7fffff;
  if ( (*(unsigned *)&f &0x80000000)==0)
    f+=*(float *)&almost1;
  return fast_float_trunc(f);
}

#define REAL_TO_INT_CEIL(x)				(fast_float2long_round(fast_float_ceil(x)))
#define REAL_TO_INT_FLOOR(x)			(fast_float2long_round(fast_float_floor(x)))

// ---- GameLogic.cpp setFPMode, f716706ccb ----

#if defined(_M_IX86)
void setFPMode()
{
	_fpreset();

	unsigned int curVal = _statusfp();
	unsigned int newVal = curVal;
	newVal = (newVal & ~_MCW_RC) | (_RC_NEAR & _MCW_RC);
	newVal = (newVal & ~_MCW_PC) | (_PC_24   & _MCW_PC);

	_controlfp(newVal, _MCW_PC | _MCW_RC);
}
#endif

// ---- probe ----

static unsigned floatBits(float value)
{
	unsigned bits;
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static void printDoubleBits(const char *name, double value)
{
	unsigned words[2];
	memcpy(words, &value, sizeof(words));
	printf("  %-6s double bits=%08X%08X\n", name, words[1], words[0]);
}

static void printToolchain()
{
#if defined(_MSC_VER)
	printf("_MSC_VER=%d\n", _MSC_VER);
#else
	printf("compiler=%s\n", __VERSION__);
#endif
#if defined(_M_X64)
	printf("arch=x64\n");
#elif defined(_M_IX86)
	printf("arch=x86\n");
#elif defined(__aarch64__)
	printf("arch=arm64\n");
#elif defined(__x86_64__)
	printf("arch=x86_64\n");
#endif
#if defined(_M_IX86_FP)
	printf("_M_IX86_FP=%d\n", _M_IX86_FP);
#else
	printf("_M_IX86_FP=undefined\n");
#endif
	printf("sizeof(long)=%d\n", (int)sizeof(long));
}

#if defined(_MSC_VER)
static void printControlWord()
{
	unsigned int control = _controlfp(0, 0);
	printf("  control=%08X  exception masks (_MCW_EM)=%08X of %08X\n", control, control & _MCW_EM, (unsigned int)_MCW_EM);
}
#elif defined(__APPLE__) && defined(__aarch64__)
static void printControlWord()
{
	fenv_t environment;
	fegetenv(&environment);
	unsigned long long trapEnableBits = environment.__fpcr & 0x9F00ULL;
	printf("  fpcr=%016llX  trap enable bits (0x9F00)=%04llX\n", (unsigned long long)environment.__fpcr, trapEnableBits);
}
#else
static void printControlWord()
{
	printf("  control word: not printed on this platform\n");
}
#endif

static void probeValue(const char *name, Real value)
{
	long rounded = fast_float2long_round(value);
	Int ceilInt = REAL_TO_INT_CEIL(value);
	Int floorInt = REAL_TO_INT_FLOOR(value);
	Int castFloat = (Int)value;
	Int castDouble = (Int)(double)value;

	printf("  %-6s float bits=%08X  (Int)float=%d  (Int)double=%d  fast_float2long_round=%ld  REAL_TO_INT_CEIL=%d  REAL_TO_INT_FLOOR=%d  fast_float_trunc bits=%08X\n",
		name, floatBits(value), castFloat, castDouble, rounded, ceilInt, floorInt, floatBits(fast_float_trunc(value)));
}

static void runProbe(const char *mode)
{
	printf("\n[%s]\n", mode);
	printControlWord();

	probeValue("1/0", g_one / g_zero);
	probeValue("-1/0", -g_one / g_zero);
	probeValue("0/0", g_zero / g_zero);
	probeValue("1e20", g_one * 1e20f);
	probeValue("3e9", g_one * 3e9f);
	probeValue("-3e9", g_one * -3e9f);
	probeValue("1e6", g_one * 1e6f);

	printDoubleBits("1/0", g_oneDouble / g_zeroDouble);
	printDoubleBits("0/0", g_zeroDouble / g_zeroDouble);

	printControlWord();
}

int main()
{
	setvbuf(stdout, NULL, _IONBF, 0);

	printToolchain();
	runProbe("default");

#if defined(_M_IX86)
	setFPMode();
	runProbe("game setFPMode");
#else
	printf("\n[game setFPMode] skipped: _MCW_PC exists only on x86\n");
#endif

	return 0;
}
