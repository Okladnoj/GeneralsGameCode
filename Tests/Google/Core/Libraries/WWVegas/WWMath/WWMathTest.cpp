/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

#include "WWMath/wwmath.h"

// The game simulation must compute the same bits from one build to the next.
// Deterministic math has one set of reference bits for every platform, the CRT math one per compiler.
#if USE_DETERMINISTIC_MATH
#define WWMATH_REFERENCE(deterministic, win32Crt) deterministic
#elif defined(_MSC_VER) && defined(_M_IX86)
#define WWMATH_REFERENCE(deterministic, win32Crt) win32Crt
#else
#define WWMATH_REFERENCE(deterministic, win32Crt) ""
#define WWMATH_HAS_NO_REFERENCE
#endif

template <typename T>
static T runtimeValue(T value)
{
	volatile T runtime = value;
	return runtime;
}

static std::string bits(float value)
{
	uint32_t raw;
	memcpy(&raw, &value, sizeof(raw));
	char text[16];
	snprintf(text, sizeof(text), "%08X", static_cast<unsigned int>(raw));
	return text;
}

static std::string bits(double value)
{
	uint64_t raw;
	memcpy(&raw, &value, sizeof(raw));
	char text[32];
	snprintf(text, sizeof(text), "%016llX", static_cast<unsigned long long>(raw));
	return text;
}

static std::string bits(long value)
{
	char text[32];
	snprintf(text, sizeof(text), "%ld", value);
	return text;
}

class WWMathBits : public ::testing::Test
{
protected:
	void SetUp() override
	{
#ifdef WWMATH_HAS_NO_REFERENCE
		GTEST_SKIP() << "No WWMath reference bits for this build configuration";
#endif
	}
};

TEST_F(WWMathBits, Sin)
{
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(0.5f))), WWMATH_REFERENCE("3EF57744", ""));
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(2.7f))), WWMATH_REFERENCE("3EDAD188", ""));
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(187.66f))), WWMATH_REFERENCE("BF3DDE0A", ""));
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(0.5))), WWMATH_REFERENCE("3FDEAEE8744B05F0", ""));
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(2.7))), WWMATH_REFERENCE("3FDB5A312424A70C", ""));
	EXPECT_EQ(bits(WWMath::Sin(runtimeValue(187.66))), WWMATH_REFERENCE("BFE7BBC6596262AE", ""));
	EXPECT_EQ(bits(WWMath::Sinf(runtimeValue(0.5f))), WWMATH_REFERENCE("3EF57744", ""));
	EXPECT_EQ(bits(WWMath::Sinf(runtimeValue(2.7f))), WWMATH_REFERENCE("3EDAD188", ""));
	EXPECT_EQ(bits(WWMath::Sinf(runtimeValue(187.66f))), WWMATH_REFERENCE("BF3DDE0A", ""));
	EXPECT_EQ(bits(WWMath::Sinf_Legacy(runtimeValue(0.5f))), WWMATH_REFERENCE("3EF57744", ""));
	EXPECT_EQ(bits(WWMath::Sinf_Legacy(runtimeValue(2.7f))), WWMATH_REFERENCE("3EDAD188", ""));
	EXPECT_EQ(bits(WWMath::Sinf_Legacy(runtimeValue(187.66f))), WWMATH_REFERENCE("BF3DDE0A", ""));
}

TEST_F(WWMathBits, Cos)
{
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(0.5f))), WWMATH_REFERENCE("3F60A940", ""));
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(2.7f))), WWMATH_REFERENCE("BF677146", ""));
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(187.66f))), WWMATH_REFERENCE("3F2BB74E", ""));
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(0.5))), WWMATH_REFERENCE("3FEC1528065B7D50", ""));
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(2.7))), WWMATH_REFERENCE("BFECEE28B36603A6", ""));
	EXPECT_EQ(bits(WWMath::Cos(runtimeValue(187.66))), WWMATH_REFERENCE("3FE576E4096F1DE4", ""));
	EXPECT_EQ(bits(WWMath::Cosf(runtimeValue(0.5f))), WWMATH_REFERENCE("3F60A940", ""));
	EXPECT_EQ(bits(WWMath::Cosf(runtimeValue(2.7f))), WWMATH_REFERENCE("BF677146", ""));
	EXPECT_EQ(bits(WWMath::Cosf(runtimeValue(187.66f))), WWMATH_REFERENCE("3F2BB74E", ""));
	EXPECT_EQ(bits(WWMath::Cosf_Legacy(runtimeValue(0.5f))), WWMATH_REFERENCE("3F60A940", ""));
	EXPECT_EQ(bits(WWMath::Cosf_Legacy(runtimeValue(2.7f))), WWMATH_REFERENCE("BF677146", ""));
	EXPECT_EQ(bits(WWMath::Cosf_Legacy(runtimeValue(187.66f))), WWMATH_REFERENCE("3F2BB74E", ""));
}

TEST_F(WWMathBits, Tan)
{
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(0.5f))), WWMATH_REFERENCE("3F0BDA7A", ""));
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(2.7f))), WWMATH_REFERENCE("BEF20959", ""));
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(187.66f))), WWMATH_REFERENCE("BF8D87C1", ""));
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(0.5))), WWMATH_REFERENCE("3FE17B4F5BF3474A", ""));
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(2.7))), WWMATH_REFERENCE("BFDE412B62BA7143", ""));
	EXPECT_EQ(bits(WWMath::Tan(runtimeValue(187.66))), WWMATH_REFERENCE("BFF1B1009E8D90B0", ""));
	EXPECT_EQ(bits(WWMath::Tanf(runtimeValue(0.5f))), WWMATH_REFERENCE("3F0BDA7A", ""));
	EXPECT_EQ(bits(WWMath::Tanf(runtimeValue(2.7f))), WWMATH_REFERENCE("BEF20959", ""));
	EXPECT_EQ(bits(WWMath::Tanf(runtimeValue(187.66f))), WWMATH_REFERENCE("BF8D87C1", ""));
}

TEST_F(WWMathBits, Asin)
{
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(-0.5f))), WWMATH_REFERENCE("BF060A94", ""));
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(0.25f))), WWMATH_REFERENCE("3E815F4E", ""));
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(0.9f))), WWMATH_REFERENCE("3F8F549C", ""));
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(-0.5))), WWMATH_REFERENCE("BFE0C152382D7366", ""));
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(0.25))), WWMATH_REFERENCE("3FD02BE9CE0B87CD", ""));
	EXPECT_EQ(bits(WWMath::Asin(runtimeValue(0.9))), WWMATH_REFERENCE("3FF1EA93705FA172", ""));
	EXPECT_EQ(bits(WWMath::Asinf(runtimeValue(-0.5f))), WWMATH_REFERENCE("BF060A94", ""));
	EXPECT_EQ(bits(WWMath::Asinf(runtimeValue(0.25f))), WWMATH_REFERENCE("3E815F4E", ""));
	EXPECT_EQ(bits(WWMath::Asinf(runtimeValue(0.9f))), WWMATH_REFERENCE("3F8F549C", ""));
}

TEST_F(WWMathBits, Acos)
{
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(-0.5f))), WWMATH_REFERENCE("40060A92", ""));
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(0.25f))), WWMATH_REFERENCE("3FA8B807", ""));
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(0.9f))), WWMATH_REFERENCE("3EE6ECFE", ""));
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(-0.5))), WWMATH_REFERENCE("4000C152382D7366", ""));
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(0.25))), WWMATH_REFERENCE("3FF51700E0C14B25", ""));
	EXPECT_EQ(bits(WWMath::Acos(runtimeValue(0.9))), WWMATH_REFERENCE("3FDCDD9F8F922E98", ""));
	EXPECT_EQ(bits(WWMath::Acosf(runtimeValue(-0.5f))), WWMATH_REFERENCE("40060A92", ""));
	EXPECT_EQ(bits(WWMath::Acosf(runtimeValue(0.25f))), WWMATH_REFERENCE("3FA8B807", ""));
	EXPECT_EQ(bits(WWMath::Acosf(runtimeValue(0.9f))), WWMATH_REFERENCE("3EE6ECFE", ""));
}

TEST_F(WWMathBits, Atan)
{
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(0.5f))), WWMATH_REFERENCE("3EED6338", ""));
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(2.7f))), WWMATH_REFERENCE("3F9BA8DC", ""));
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(187.66f))), WWMATH_REFERENCE("3FC8613E", ""));
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(0.5))), WWMATH_REFERENCE("3FDDAC6700000000", ""));
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(2.7))), WWMATH_REFERENCE("3FF3751B80000000", ""));
	EXPECT_EQ(bits(WWMath::Atan(runtimeValue(187.66))), WWMATH_REFERENCE("3FF90C27C0000000", ""));
	EXPECT_EQ(bits(WWMath::Atanf(runtimeValue(0.5f))), WWMATH_REFERENCE("3EED6338", ""));
	EXPECT_EQ(bits(WWMath::Atanf(runtimeValue(2.7f))), WWMATH_REFERENCE("3F9BA8DC", ""));
	EXPECT_EQ(bits(WWMath::Atanf(runtimeValue(187.66f))), WWMATH_REFERENCE("3FC8613E", ""));
}

TEST_F(WWMathBits, Atan2)
{
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(0.4f), runtimeValue(1.3f))), WWMATH_REFERENCE("3E98D4DA", ""));
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(2.7f), runtimeValue(11.9f))), WWMATH_REFERENCE("3E6477D5", ""));
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(-3.5f), runtimeValue(2.0f))), WWMATH_REFERENCE("BF869C79", ""));
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(0.4), runtimeValue(1.3))), WWMATH_REFERENCE("3FD31A9B40000000", ""));
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(2.7), runtimeValue(11.9))), WWMATH_REFERENCE("3FCC8EFAA0000000", ""));
	EXPECT_EQ(bits(WWMath::Atan2(runtimeValue(-3.5), runtimeValue(2.0))), WWMATH_REFERENCE("BFF0D38F20000000", ""));
	EXPECT_EQ(bits(WWMath::Atan2f(runtimeValue(0.4f), runtimeValue(1.3f))), WWMATH_REFERENCE("3E98D4DA", ""));
	EXPECT_EQ(bits(WWMath::Atan2f(runtimeValue(2.7f), runtimeValue(11.9f))), WWMATH_REFERENCE("3E6477D5", ""));
	EXPECT_EQ(bits(WWMath::Atan2f(runtimeValue(-3.5f), runtimeValue(2.0f))), WWMATH_REFERENCE("BF869C79", ""));
}

TEST_F(WWMathBits, Sinh)
{
	EXPECT_EQ(bits(WWMath::Sinh(runtimeValue(0.5))), WWMATH_REFERENCE("3FE0ACD00FE63B97", ""));
	EXPECT_EQ(bits(WWMath::Sinh(runtimeValue(-0.5))), WWMATH_REFERENCE("BFE0ACD00FE63B97", ""));
	EXPECT_EQ(bits(WWMath::Sinh(runtimeValue(2.7))), WWMATH_REFERENCE("401DA0036F88823C", ""));
	EXPECT_EQ(bits(WWMath::Sinhf(runtimeValue(0.5f))), WWMATH_REFERENCE("3F056680", ""));
	EXPECT_EQ(bits(WWMath::Sinhf(runtimeValue(-0.5f))), WWMATH_REFERENCE("BF056680", ""));
	EXPECT_EQ(bits(WWMath::Sinhf(runtimeValue(2.7f))), WWMATH_REFERENCE("40ED001C", ""));
}

TEST_F(WWMathBits, Cosh)
{
	EXPECT_EQ(bits(WWMath::Cosh(runtimeValue(0.5))), WWMATH_REFERENCE("3FF20AC1862AE8D0", ""));
	EXPECT_EQ(bits(WWMath::Cosh(runtimeValue(-0.5))), WWMATH_REFERENCE("3FF20AC1862AE8D0", ""));
	EXPECT_EQ(bits(WWMath::Cosh(runtimeValue(2.7))), WWMATH_REFERENCE("401DE4D4F525D240", ""));
	EXPECT_EQ(bits(WWMath::Coshf(runtimeValue(0.5f))), WWMATH_REFERENCE("3F90560C", ""));
	EXPECT_EQ(bits(WWMath::Coshf(runtimeValue(-0.5f))), WWMATH_REFERENCE("3F90560C", ""));
	EXPECT_EQ(bits(WWMath::Coshf(runtimeValue(2.7f))), WWMATH_REFERENCE("40EF26A8", ""));
}

TEST_F(WWMathBits, Tanh)
{
	EXPECT_EQ(bits(WWMath::Tanh(runtimeValue(0.5))), WWMATH_REFERENCE("3FDD9353D7568AF3", ""));
	EXPECT_EQ(bits(WWMath::Tanh(runtimeValue(-0.5))), WWMATH_REFERENCE("BFDD9353D7568AF3", ""));
	EXPECT_EQ(bits(WWMath::Tanh(runtimeValue(2.7))), WWMATH_REFERENCE("3FEFB655437478D5", ""));
	EXPECT_EQ(bits(WWMath::Tanhf(runtimeValue(0.5f))), WWMATH_REFERENCE("3EEC9A9F", ""));
	EXPECT_EQ(bits(WWMath::Tanhf(runtimeValue(-0.5f))), WWMATH_REFERENCE("BEEC9A9F", ""));
	EXPECT_EQ(bits(WWMath::Tanhf(runtimeValue(2.7f))), WWMATH_REFERENCE("3F7DB2AA", ""));
}

TEST_F(WWMathBits, Exp)
{
	EXPECT_EQ(bits(WWMath::Exp(runtimeValue(0.5))), WWMATH_REFERENCE("3FFA61298E1E069C", ""));
	EXPECT_EQ(bits(WWMath::Exp(runtimeValue(-0.5))), WWMATH_REFERENCE("3FE368B2FC6F960A", ""));
	EXPECT_EQ(bits(WWMath::Exp(runtimeValue(2.7))), WWMATH_REFERENCE("402DC26C32572A3E", ""));
	EXPECT_EQ(bits(WWMath::Expf(runtimeValue(0.5f))), WWMATH_REFERENCE("3FD3094C", ""));
	EXPECT_EQ(bits(WWMath::Expf(runtimeValue(-0.5f))), WWMATH_REFERENCE("3F1B4598", ""));
	EXPECT_EQ(bits(WWMath::Expf(runtimeValue(2.7f))), WWMATH_REFERENCE("416E1362", ""));
}

TEST_F(WWMathBits, Log)
{
	EXPECT_EQ(bits(WWMath::Log(runtimeValue(0.5))), WWMATH_REFERENCE("BFE62E42FEFA39EF", ""));
	EXPECT_EQ(bits(WWMath::Log(runtimeValue(2.7))), WWMATH_REFERENCE("3FEFC8B7F138BDEA", ""));
	EXPECT_EQ(bits(WWMath::Log(runtimeValue(187.66))), WWMATH_REFERENCE("4014F043528DBB65", ""));
	EXPECT_EQ(bits(WWMath::Logf(runtimeValue(0.5f))), WWMATH_REFERENCE("BF317218", ""));
	EXPECT_EQ(bits(WWMath::Logf(runtimeValue(2.7f))), WWMATH_REFERENCE("3F7E45C0", ""));
	EXPECT_EQ(bits(WWMath::Logf(runtimeValue(187.66f))), WWMATH_REFERENCE("40A7821B", ""));
}

TEST_F(WWMathBits, Log10)
{
	EXPECT_EQ(bits(WWMath::Log10(runtimeValue(0.5))), WWMATH_REFERENCE("BFD34413509F79FF", ""));
	EXPECT_EQ(bits(WWMath::Log10(runtimeValue(2.7))), WWMATH_REFERENCE("3FDB9B76C2EF81F8", ""));
	EXPECT_EQ(bits(WWMath::Log10(runtimeValue(187.66))), WWMATH_REFERENCE("40022FDD82152DEA", ""));
	EXPECT_EQ(bits(WWMath::Log10f(runtimeValue(0.5f))), WWMATH_REFERENCE("BE9A209B", ""));
	EXPECT_EQ(bits(WWMath::Log10f(runtimeValue(2.7f))), WWMATH_REFERENCE("3EDCDBB6", ""));
	EXPECT_EQ(bits(WWMath::Log10f(runtimeValue(187.66f))), WWMATH_REFERENCE("40117EEC", ""));
}

TEST_F(WWMathBits, Pow)
{
	EXPECT_EQ(bits(WWMath::Pow(runtimeValue(0.4), runtimeValue(1.3))), WWMATH_REFERENCE("3FD3727E60000000", ""));
	EXPECT_EQ(bits(WWMath::Pow(runtimeValue(2.7), runtimeValue(11.9))), WWMATH_REFERENCE("410096F740000000", ""));
	EXPECT_EQ(bits(WWMath::Pow(runtimeValue(-3.5), runtimeValue(2.0))), WWMATH_REFERENCE("4028800000000000", ""));
	EXPECT_EQ(bits(WWMath::Powf(runtimeValue(0.4f), runtimeValue(1.3f))), WWMATH_REFERENCE("3E9B93F3", ""));
	EXPECT_EQ(bits(WWMath::Powf(runtimeValue(2.7f), runtimeValue(11.9f))), WWMATH_REFERENCE("4804B7BA", ""));
	EXPECT_EQ(bits(WWMath::Powf(runtimeValue(-3.5f), runtimeValue(2.0f))), WWMATH_REFERENCE("41440000", ""));
}

TEST_F(WWMathBits, Sqrt)
{
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(0.5f))), WWMATH_REFERENCE("3F3504F3", ""));
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(2.7f))), WWMATH_REFERENCE("3FD25352", ""));
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(187.66f))), WWMATH_REFERENCE("415B2EB7", ""));
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(0.5))), WWMATH_REFERENCE("3FE6A09E667F3BCD", ""));
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(2.7))), WWMATH_REFERENCE("3FFA4A6A2F74C6AC", ""));
	EXPECT_EQ(bits(WWMath::Sqrt(runtimeValue(187.66))), WWMATH_REFERENCE("402B65D6E280ACC1", ""));
	EXPECT_EQ(bits(WWMath::Sqrtf(runtimeValue(0.5f))), WWMATH_REFERENCE("3F3504F3", ""));
	EXPECT_EQ(bits(WWMath::Sqrtf(runtimeValue(2.7f))), WWMATH_REFERENCE("3FD25352", ""));
	EXPECT_EQ(bits(WWMath::Sqrtf(runtimeValue(187.66f))), WWMATH_REFERENCE("415B2EB7", ""));
	EXPECT_EQ(bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(0.5f))), WWMATH_REFERENCE("3FB504F3", ""));
	EXPECT_EQ(bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(2.7f))), WWMATH_REFERENCE("3F1BCBF0", ""));
	EXPECT_EQ(bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(187.66f))), WWMATH_REFERENCE("3D958042", ""));
}

TEST_F(WWMathBits, Fabs)
{
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(-0.5f))), WWMATH_REFERENCE("3F000000", ""));
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(2.7f))), WWMATH_REFERENCE("402CCCCD", ""));
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(100.5f))), WWMATH_REFERENCE("42C90000", ""));
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(-0.5))), WWMATH_REFERENCE("3FE0000000000000", ""));
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(2.7))), WWMATH_REFERENCE("400599999999999A", ""));
	EXPECT_EQ(bits(WWMath::Fabs(runtimeValue(100.5))), WWMATH_REFERENCE("4059200000000000", ""));
	EXPECT_EQ(bits(WWMath::Fabsf(runtimeValue(-0.5f))), WWMATH_REFERENCE("3F000000", ""));
	EXPECT_EQ(bits(WWMath::Fabsf(runtimeValue(2.7f))), WWMATH_REFERENCE("402CCCCD", ""));
	EXPECT_EQ(bits(WWMath::Fabsf(runtimeValue(100.5f))), WWMATH_REFERENCE("42C90000", ""));
	EXPECT_EQ(bits(WWMath::Fabsf_Legacy(runtimeValue(-0.5f))), WWMATH_REFERENCE("3F000000", ""));
	EXPECT_EQ(bits(WWMath::Fabsf_Legacy(runtimeValue(2.7f))), WWMATH_REFERENCE("402CCCCD", ""));
	EXPECT_EQ(bits(WWMath::Fabsf_Legacy(runtimeValue(100.5f))), WWMATH_REFERENCE("42C90000", ""));
}

TEST_F(WWMathBits, Ceil)
{
	EXPECT_EQ(bits(WWMath::Ceil(runtimeValue(-0.5))), WWMATH_REFERENCE("8000000000000000", ""));
	EXPECT_EQ(bits(WWMath::Ceil(runtimeValue(2.7))), WWMATH_REFERENCE("4008000000000000", ""));
	EXPECT_EQ(bits(WWMath::Ceil(runtimeValue(100.5))), WWMATH_REFERENCE("4059400000000000", ""));
	EXPECT_EQ(bits(WWMath::Ceilf(runtimeValue(-0.5f))), WWMATH_REFERENCE("80000000", ""));
	EXPECT_EQ(bits(WWMath::Ceilf(runtimeValue(2.7f))), WWMATH_REFERENCE("40400000", ""));
	EXPECT_EQ(bits(WWMath::Ceilf(runtimeValue(100.5f))), WWMATH_REFERENCE("42CA0000", ""));
}

TEST_F(WWMathBits, Floor)
{
	EXPECT_EQ(bits(WWMath::Floor(runtimeValue(-0.5))), WWMATH_REFERENCE("BFF0000000000000", ""));
	EXPECT_EQ(bits(WWMath::Floor(runtimeValue(2.7))), WWMATH_REFERENCE("4000000000000000", ""));
	EXPECT_EQ(bits(WWMath::Floor(runtimeValue(100.5))), WWMATH_REFERENCE("4059000000000000", ""));
	EXPECT_EQ(bits(WWMath::Floorf(runtimeValue(-0.5f))), WWMATH_REFERENCE("BF800000", ""));
	EXPECT_EQ(bits(WWMath::Floorf(runtimeValue(2.7f))), WWMATH_REFERENCE("40000000", ""));
	EXPECT_EQ(bits(WWMath::Floorf(runtimeValue(100.5f))), WWMATH_REFERENCE("42C80000", ""));
}

TEST_F(WWMathBits, Float_To_Long)
{
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(-0.5f))), WWMATH_REFERENCE("0", ""));
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(2.7f))), WWMATH_REFERENCE("3", ""));
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(100.5f))), WWMATH_REFERENCE("100", ""));
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(-0.5))), WWMATH_REFERENCE("0", ""));
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(2.7))), WWMATH_REFERENCE("3", ""));
	EXPECT_EQ(bits(WWMath::Float_To_Long(runtimeValue(100.5))), WWMATH_REFERENCE("100", ""));
}

TEST_F(WWMathBits, Div_Safe)
{
	EXPECT_EQ(bits(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(3.0f))), WWMATH_REFERENCE("3EAAAAAB", ""));
	EXPECT_EQ(bits(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(0.0f))), WWMATH_REFERENCE("461C4000", ""));
	EXPECT_EQ(bits(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(3.0))), WWMATH_REFERENCE("3FD5555555555555", ""));
	EXPECT_EQ(bits(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(0.0))), WWMATH_REFERENCE("40C3880008472404", ""));
}
