// Prints what division by zero, the NaN it produces and the float to int conversions give with this toolchain.
// Every input value is read from a file at run time, so the compiler cannot fold any of the operations.
// The conversion helpers, Div_Safe and setFPMode are copied from BaseType.h, wwmath.h and GameLogic.cpp as they are.
// The game expressions are copied from origin/main and from the PR with Div_Safe.
// Written for VC6 as well, so it keeps to C++98 and the old CRT.

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
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
typedef unsigned int UnsignedInt;
typedef float Real;

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

// ---- wwmath.h Div_Safe, USE_DETERMINISTIC_MATH branch, f716706ccb ----

inline float Div_Safe(float dividend, float divisor, float fallback)
{
	return (divisor == 0.0f) ? fallback : dividend / divisor;
}

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

// ---- inputs ----

struct NamedInput
{
	char name[64];
	double value;
};

static NamedInput g_namedInputs[64];
static int g_namedInputCount = 0;

static bool readInputs(const char *path)
{
	FILE *file = fopen(path, "r");
	if (file == NULL)
	{
		return false;
	}

	while (g_namedInputCount < 64 && fscanf(file, "%63s %lf", g_namedInputs[g_namedInputCount].name, &g_namedInputs[g_namedInputCount].value) == 2)
	{
		++g_namedInputCount;
	}

	fclose(file);
	return true;
}

static double inputValue(const char *name)
{
	int index;
	for (index = 0; index < g_namedInputCount; ++index)
	{
		if (strcmp(g_namedInputs[index].name, name) == 0)
		{
			return g_namedInputs[index].value;
		}
	}

	printf("missing input: %s\n", name);
	exit(2);
	return 0.0;
}

struct Inputs
{
	Real zero;
	Real one;
	Real minusOne;
	Real hundred;
	Real tiny;
	Real big;
	Real overInt;
	Real underInt;
	Real normal;
	Real half;
	double zeroDouble;
	double oneDouble;
	double minusOneDouble;
	Int overkillDamage;
	Int probabilityModifier;
	Real bonusPerOverkillPercent;
	Real maxHealth;
	Real health;
	Real tileDistance;
	Real spacingSmall;
	Real flightDistance;
	Real flightPathSpeed;
};

static void loadInputs(Inputs &inputs)
{
	inputs.zero = (Real)inputValue("zero");
	inputs.one = (Real)inputValue("one");
	inputs.minusOne = (Real)inputValue("minus_one");
	inputs.hundred = (Real)inputValue("hundred");
	inputs.tiny = (Real)inputValue("tiny");
	inputs.big = (Real)inputValue("big");
	inputs.overInt = (Real)inputValue("over_int");
	inputs.underInt = (Real)inputValue("under_int");
	inputs.normal = (Real)inputValue("normal");
	inputs.half = (Real)inputValue("half");
	inputs.zeroDouble = inputValue("zero");
	inputs.oneDouble = inputValue("one");
	inputs.minusOneDouble = inputValue("minus_one");
	inputs.overkillDamage = (Int)inputValue("overkill_damage");
	inputs.probabilityModifier = (Int)inputValue("probability_modifier");
	inputs.bonusPerOverkillPercent = (Real)inputValue("bonus_per_overkill_percent");
	inputs.maxHealth = (Real)inputValue("max_health");
	inputs.health = (Real)inputValue("health");
	inputs.tileDistance = (Real)inputValue("tile_distance");
	inputs.spacingSmall = (Real)inputValue("spacing_small");
	inputs.flightDistance = (Real)inputValue("flight_distance");
	inputs.flightPathSpeed = (Real)inputValue("flight_path_speed");
}

// ---- printing ----

static unsigned floatBits(float value)
{
	unsigned bits;
	memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static void printFloat(const char *name, float value)
{
	printf("  %-34s %08X\n", name, floatBits(value));
}

static void printDouble(const char *name, double value)
{
	unsigned words[2];
	memcpy(words, &value, sizeof(words));
	printf("  %-34s %08X%08X\n", name, words[1], words[0]);
}

static void printInt(const char *name, Int value)
{
	printf("  %-34s %d\n", name, value);
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

static void printInputs(const Inputs &inputs)
{
	printf("\n[inputs]\n");
	printFloat("zero", inputs.zero);
	printFloat("one", inputs.one);
	printFloat("minus_one", inputs.minusOne);
	printFloat("hundred", inputs.hundred);
	printFloat("tiny", inputs.tiny);
	printFloat("big", inputs.big);
	printFloat("over_int", inputs.overInt);
	printFloat("under_int", inputs.underInt);
	printFloat("normal", inputs.normal);
	printFloat("half", inputs.half);
	printInt("overkill_damage", inputs.overkillDamage);
	printInt("probability_modifier", inputs.probabilityModifier);
	printFloat("bonus_per_overkill_percent", inputs.bonusPerOverkillPercent);
	printFloat("max_health", inputs.maxHealth);
	printFloat("health", inputs.health);
	printFloat("tile_distance", inputs.tileDistance);
	printFloat("spacing_small", inputs.spacingSmall);
	printFloat("flight_distance", inputs.flightDistance);
	printFloat("flight_path_speed", inputs.flightPathSpeed);
}

// ---- probes ----

static void probeFloatOperations(const Inputs &in)
{
	Real positiveInfinity = in.one / in.zero;
	Real notANumber = in.zero / in.zero;

	printf("\n  float operations\n");
	printFloat("one / zero", positiveInfinity);
	printFloat("minus_one / zero", in.minusOne / in.zero);
	printFloat("zero / zero", notANumber);
	printFloat("(zero * minus_one) / zero", (in.zero * in.minusOne) / in.zero);
	printFloat("(one / zero) * zero", positiveInfinity * in.zero);
	printFloat("(one / zero) - (one / zero)", positiveInfinity - positiveInfinity);
	printFloat("(zero / zero) * hundred", notANumber * in.hundred);
	printFloat("(zero / zero) + one", notANumber + in.one);
	printFloat("-(zero / zero)", -notANumber);
	printFloat("one / tiny", in.one / in.tiny);
}

static void probeDoubleOperations(const Inputs &in)
{
	double positiveInfinity = in.oneDouble / in.zeroDouble;
	double notANumber = in.zeroDouble / in.zeroDouble;

	printf("\n  double operations\n");
	printDouble("one / zero", positiveInfinity);
	printDouble("minus_one / zero", in.minusOneDouble / in.zeroDouble);
	printDouble("zero / zero", notANumber);
	printDouble("(one / zero) - (one / zero)", positiveInfinity - positiveInfinity);
	printDouble("-(zero / zero)", -notANumber);
	printDouble("sqrt(minus_one)", sqrt(in.minusOneDouble));
	printDouble("fabs(zero / zero)", fabs(notANumber));
}

static void probeFloatConversion(const char *name, Real value)
{
	printf("  %-22s bits=%08X  (Int)=%d  (UnsignedInt)=%u  (Int)(double)=%d  fast_float2long_round=%ld  REAL_TO_INT_CEIL=%d  REAL_TO_INT_FLOOR=%d  fast_float_trunc=%08X\n",
		name, floatBits(value), (Int)value, (UnsignedInt)value, (Int)(double)value,
		fast_float2long_round(value), (Int)REAL_TO_INT_CEIL(value), (Int)REAL_TO_INT_FLOOR(value), floatBits(fast_float_trunc(value)));
}

static void probeDoubleConversion(const char *name, double value)
{
	unsigned words[2];
	memcpy(words, &value, sizeof(words));
	printf("  %-22s bits=%08X%08X  (Int)=%d  (UnsignedInt)=%u\n", name, words[1], words[0], (Int)value, (UnsignedInt)value);
}

static void probeConversions(const Inputs &in)
{
	printf("\n  float to int\n");
	probeFloatConversion("one / zero", in.one / in.zero);
	probeFloatConversion("minus_one / zero", in.minusOne / in.zero);
	probeFloatConversion("zero / zero", in.zero / in.zero);
	probeFloatConversion("-(zero / zero)", -(in.zero / in.zero));
	probeFloatConversion("big", in.big);
	probeFloatConversion("over_int", in.overInt);
	probeFloatConversion("under_int", in.underInt);
	probeFloatConversion("normal", in.normal);
	probeFloatConversion("half", in.half);
	probeFloatConversion("-half", -in.half);

	printf("\n  double to int\n");
	probeDoubleConversion("one / zero", in.oneDouble / in.zeroDouble);
	probeDoubleConversion("zero / zero", in.zeroDouble / in.zeroDouble);
	probeDoubleConversion("big", (double)in.big);
}

static Int maxInt(Int left, Int right)
{
	return left > right ? left : right;
}

static void probeSlowDeathBehavior(const Inputs &in)
{
	Int overkillDamage = in.overkillDamage;

	Real overkillPercent = (float)overkillDamage / (float)in.maxHealth;
	Int overkillModifier = overkillPercent * in.bonusPerOverkillPercent;
	Int result = maxInt(in.probabilityModifier + overkillModifier, 1);

	Real overkillPercentSafe = Div_Safe((float)overkillDamage, in.maxHealth, 0.0f);
	Int overkillModifierSafe = overkillPercentSafe * in.bonusPerOverkillPercent;
	Int resultSafe = maxInt(in.probabilityModifier + overkillModifierSafe, 1);

	printf("\n  SlowDeathBehavior::getProbabilityModifier, overkill_damage / max_health\n");
	printFloat("upstream overkillPercent", overkillPercent);
	printInt("upstream overkillModifier", overkillModifier);
	printInt("upstream result", result);
	printFloat("Div_Safe overkillPercent", overkillPercentSafe);
	printInt("Div_Safe overkillModifier", overkillModifierSafe);
	printInt("Div_Safe result", resultSafe);
}

static void probeSlavedUpdate(const Inputs &in)
{
	printf("\n  SlavedUpdate::update, health / max_health * 100\n");
	printInt("upstream health=health", (Int)(in.health / in.maxHealth * 100.0f));
	printInt("upstream health=one", (Int)(in.one / in.maxHealth * 100.0f));
	printInt("Div_Safe health=health", (Int)(Div_Safe(in.health, in.maxHealth, 0.0f) * 100.0f));
	printInt("Div_Safe health=one", (Int)(Div_Safe(in.one, in.maxHealth, 0.0f) * 100.0f));
}

static void probeBridgeBehavior(const Inputs &in)
{
	printf("\n  BridgeBehavior::createScaffolding, REAL_TO_INT_CEIL(tile_distance / spacing) + 1\n");
	printInt("upstream spacing=zero", REAL_TO_INT_CEIL(in.tileDistance / in.zero) + 1);
	printInt("upstream spacing=spacing_small", REAL_TO_INT_CEIL(in.tileDistance / in.spacingSmall) + 1);
	printInt("upstream spacing=tiny", REAL_TO_INT_CEIL(in.tileDistance / in.tiny) + 1);
	printInt("Div_Safe spacing=zero", REAL_TO_INT_CEIL(Div_Safe(in.tileDistance, in.zero, 0.0f)) + 1);
	printInt("Div_Safe spacing=spacing_small", REAL_TO_INT_CEIL(Div_Safe(in.tileDistance, in.spacingSmall, 0.0f)) + 1);
	printInt("Div_Safe spacing=tiny", REAL_TO_INT_CEIL(Div_Safe(in.tileDistance, in.tiny, 0.0f)) + 1);
}

static void probeDumbProjectileBehavior(const Inputs &in)
{
	Int segments = ceil(in.flightDistance / in.flightPathSpeed);
	Int segmentsSafe = (Int)ceil(Div_Safe(in.flightDistance, in.flightPathSpeed, 1.0f));

	printf("\n  DumbProjectileBehavior, ceil(flight_distance / flight_path_speed)\n");
	printInt("upstream m_flightPathSegments", segments);
	printInt("Div_Safe m_flightPathSegments", segmentsSafe);
}

static void runProbe(const char *mode, const Inputs &inputs)
{
	printf("\n[%s]\n", mode);
	printControlWord();

	probeFloatOperations(inputs);
	probeDoubleOperations(inputs);
	probeConversions(inputs);
	probeSlowDeathBehavior(inputs);
	probeSlavedUpdate(inputs);
	probeBridgeBehavior(inputs);
	probeDumbProjectileBehavior(inputs);

	printControlWord();
}

int main(int argc, char **argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);

	if (argc < 2 || !readInputs(argv[1]))
	{
		printf("usage: div_zero_probe <inputs file>\n");
		return 2;
	}

	Inputs inputs;
	loadInputs(inputs);

	printToolchain();
	printInputs(inputs);
	runProbe("default", inputs);

#if defined(_M_IX86)
	setFPMode();
	runProbe("game setFPMode", inputs);
#else
	printf("\n[game setFPMode] skipped: _MCW_PC exists only on x86\n");
#endif

	return 0;
}
