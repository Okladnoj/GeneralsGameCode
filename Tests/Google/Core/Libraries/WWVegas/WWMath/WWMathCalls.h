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

#pragma once

#include "WWMathKeys.h"
#include "WWMathTestCase.h"

static constexpr WWMathCallEntry wwmathCalls[] = {
	{ WWMathKey::Sin_0_5f,                bitsOf<float, WWMath::Sin, 0.5f> },
	{ WWMathKey::Sin_2_7f,                bitsOf<float, WWMath::Sin, 2.7f> },
	{ WWMathKey::Sin_187_66f,             bitsOf<float, WWMath::Sin, 187.66f> },
	{ WWMathKey::Sin_0_5,                 bitsOf<double, WWMath::Sin, 0.5> },
	{ WWMathKey::Sin_2_7,                 bitsOf<double, WWMath::Sin, 2.7> },
	{ WWMathKey::Sin_187_66,              bitsOf<double, WWMath::Sin, 187.66> },
	{ WWMathKey::Sinf_0_5f,               bitsOf<float, WWMath::Sinf, 0.5f> },
	{ WWMathKey::Sinf_2_7f,               bitsOf<float, WWMath::Sinf, 2.7f> },
	{ WWMathKey::Sinf_187_66f,            bitsOf<float, WWMath::Sinf, 187.66f> },
	{ WWMathKey::Sinf_Legacy_0_5f,        bitsOf<float, WWMath::Sinf_Legacy, 0.5f> },
	{ WWMathKey::Sinf_Legacy_2_7f,        bitsOf<float, WWMath::Sinf_Legacy, 2.7f> },
	{ WWMathKey::Sinf_Legacy_187_66f,     bitsOf<float, WWMath::Sinf_Legacy, 187.66f> },

	{ WWMathKey::Cos_0_5f,                bitsOf<float, WWMath::Cos, 0.5f> },
	{ WWMathKey::Cos_2_7f,                bitsOf<float, WWMath::Cos, 2.7f> },
	{ WWMathKey::Cos_187_66f,             bitsOf<float, WWMath::Cos, 187.66f> },
	{ WWMathKey::Cos_0_5,                 bitsOf<double, WWMath::Cos, 0.5> },
	{ WWMathKey::Cos_2_7,                 bitsOf<double, WWMath::Cos, 2.7> },
	{ WWMathKey::Cos_187_66,              bitsOf<double, WWMath::Cos, 187.66> },
	{ WWMathKey::Cosf_0_5f,               bitsOf<float, WWMath::Cosf, 0.5f> },
	{ WWMathKey::Cosf_2_7f,               bitsOf<float, WWMath::Cosf, 2.7f> },
	{ WWMathKey::Cosf_187_66f,            bitsOf<float, WWMath::Cosf, 187.66f> },
	{ WWMathKey::Cosf_Legacy_0_5f,        bitsOf<float, WWMath::Cosf_Legacy, 0.5f> },
	{ WWMathKey::Cosf_Legacy_2_7f,        bitsOf<float, WWMath::Cosf_Legacy, 2.7f> },
	{ WWMathKey::Cosf_Legacy_187_66f,     bitsOf<float, WWMath::Cosf_Legacy, 187.66f> },

	{ WWMathKey::Tan_0_5f,                bitsOf<float, WWMath::Tan, 0.5f> },
	{ WWMathKey::Tan_2_7f,                bitsOf<float, WWMath::Tan, 2.7f> },
	{ WWMathKey::Tan_187_66f,             bitsOf<float, WWMath::Tan, 187.66f> },
	{ WWMathKey::Tan_0_5,                 bitsOf<double, WWMath::Tan, 0.5> },
	{ WWMathKey::Tan_2_7,                 bitsOf<double, WWMath::Tan, 2.7> },
	{ WWMathKey::Tan_187_66,              bitsOf<double, WWMath::Tan, 187.66> },
	{ WWMathKey::Tanf_0_5f,               bitsOf<float, WWMath::Tanf, 0.5f> },
	{ WWMathKey::Tanf_2_7f,               bitsOf<float, WWMath::Tanf, 2.7f> },
	{ WWMathKey::Tanf_187_66f,            bitsOf<float, WWMath::Tanf, 187.66f> },

	{ WWMathKey::Asin_Minus0_5f,          bitsOf<float, WWMath::Asin, -0.5f> },
	{ WWMathKey::Asin_0_25f,              bitsOf<float, WWMath::Asin, 0.25f> },
	{ WWMathKey::Asin_0_9f,               bitsOf<float, WWMath::Asin, 0.9f> },
	{ WWMathKey::Asin_Minus0_5,           bitsOf<double, WWMath::Asin, -0.5> },
	{ WWMathKey::Asin_0_25,               bitsOf<double, WWMath::Asin, 0.25> },
	{ WWMathKey::Asin_0_9,                bitsOf<double, WWMath::Asin, 0.9> },
	{ WWMathKey::Asinf_Minus0_5f,         bitsOf<float, WWMath::Asinf, -0.5f> },
	{ WWMathKey::Asinf_0_25f,             bitsOf<float, WWMath::Asinf, 0.25f> },
	{ WWMathKey::Asinf_0_9f,              bitsOf<float, WWMath::Asinf, 0.9f> },

	{ WWMathKey::Acos_Minus0_5f,          bitsOf<float, WWMath::Acos, -0.5f> },
	{ WWMathKey::Acos_0_25f,              bitsOf<float, WWMath::Acos, 0.25f> },
	{ WWMathKey::Acos_0_9f,               bitsOf<float, WWMath::Acos, 0.9f> },
	{ WWMathKey::Acos_Minus0_5,           bitsOf<double, WWMath::Acos, -0.5> },
	{ WWMathKey::Acos_0_25,               bitsOf<double, WWMath::Acos, 0.25> },
	{ WWMathKey::Acos_0_9,                bitsOf<double, WWMath::Acos, 0.9> },
	{ WWMathKey::Acosf_Minus0_5f,         bitsOf<float, WWMath::Acosf, -0.5f> },
	{ WWMathKey::Acosf_0_25f,             bitsOf<float, WWMath::Acosf, 0.25f> },
	{ WWMathKey::Acosf_0_9f,              bitsOf<float, WWMath::Acosf, 0.9f> },

	{ WWMathKey::Atan_0_5f,               bitsOf<float, WWMath::Atan, 0.5f> },
	{ WWMathKey::Atan_2_7f,               bitsOf<float, WWMath::Atan, 2.7f> },
	{ WWMathKey::Atan_187_66f,            bitsOf<float, WWMath::Atan, 187.66f> },
	{ WWMathKey::Atan_0_5,                bitsOf<double, WWMath::Atan, 0.5> },
	{ WWMathKey::Atan_2_7,                bitsOf<double, WWMath::Atan, 2.7> },
	{ WWMathKey::Atan_187_66,             bitsOf<double, WWMath::Atan, 187.66> },
	{ WWMathKey::Atanf_0_5f,              bitsOf<float, WWMath::Atanf, 0.5f> },
	{ WWMathKey::Atanf_2_7f,              bitsOf<float, WWMath::Atanf, 2.7f> },
	{ WWMathKey::Atanf_187_66f,           bitsOf<float, WWMath::Atanf, 187.66f> },

	{ WWMathKey::Atan2_0_4f_1_3f,         bitsOf<float, WWMath::Atan2, 0.4f, 1.3f> },
	{ WWMathKey::Atan2_2_7f_11_9f,        bitsOf<float, WWMath::Atan2, 2.7f, 11.9f> },
	{ WWMathKey::Atan2_Minus3_5f_2_0f,    bitsOf<float, WWMath::Atan2, -3.5f, 2.0f> },
	{ WWMathKey::Atan2_0_4_1_3,           bitsOf<double, WWMath::Atan2, 0.4, 1.3> },
	{ WWMathKey::Atan2_2_7_11_9,          bitsOf<double, WWMath::Atan2, 2.7, 11.9> },
	{ WWMathKey::Atan2_Minus3_5_2_0,      bitsOf<double, WWMath::Atan2, -3.5, 2.0> },
	{ WWMathKey::Atan2f_0_4f_1_3f,        bitsOf<float, WWMath::Atan2f, 0.4f, 1.3f> },
	{ WWMathKey::Atan2f_2_7f_11_9f,       bitsOf<float, WWMath::Atan2f, 2.7f, 11.9f> },
	{ WWMathKey::Atan2f_Minus3_5f_2_0f,   bitsOf<float, WWMath::Atan2f, -3.5f, 2.0f> },

	{ WWMathKey::Sinh_0_5,                bitsOf<double, WWMath::Sinh, 0.5> },
	{ WWMathKey::Sinh_Minus0_5,           bitsOf<double, WWMath::Sinh, -0.5> },
	{ WWMathKey::Sinh_2_7,                bitsOf<double, WWMath::Sinh, 2.7> },
	{ WWMathKey::Sinhf_0_5f,              bitsOf<float, WWMath::Sinhf, 0.5f> },
	{ WWMathKey::Sinhf_Minus0_5f,         bitsOf<float, WWMath::Sinhf, -0.5f> },
	{ WWMathKey::Sinhf_2_7f,              bitsOf<float, WWMath::Sinhf, 2.7f> },

	{ WWMathKey::Cosh_0_5,                bitsOf<double, WWMath::Cosh, 0.5> },
	{ WWMathKey::Cosh_Minus0_5,           bitsOf<double, WWMath::Cosh, -0.5> },
	{ WWMathKey::Cosh_2_7,                bitsOf<double, WWMath::Cosh, 2.7> },
	{ WWMathKey::Coshf_0_5f,              bitsOf<float, WWMath::Coshf, 0.5f> },
	{ WWMathKey::Coshf_Minus0_5f,         bitsOf<float, WWMath::Coshf, -0.5f> },
	{ WWMathKey::Coshf_2_7f,              bitsOf<float, WWMath::Coshf, 2.7f> },

	{ WWMathKey::Tanh_0_5,                bitsOf<double, WWMath::Tanh, 0.5> },
	{ WWMathKey::Tanh_Minus0_5,           bitsOf<double, WWMath::Tanh, -0.5> },
	{ WWMathKey::Tanh_2_7,                bitsOf<double, WWMath::Tanh, 2.7> },
	{ WWMathKey::Tanhf_0_5f,              bitsOf<float, WWMath::Tanhf, 0.5f> },
	{ WWMathKey::Tanhf_Minus0_5f,         bitsOf<float, WWMath::Tanhf, -0.5f> },
	{ WWMathKey::Tanhf_2_7f,              bitsOf<float, WWMath::Tanhf, 2.7f> },

	{ WWMathKey::Exp_0_5,                 bitsOf<double, WWMath::Exp, 0.5> },
	{ WWMathKey::Exp_Minus0_5,            bitsOf<double, WWMath::Exp, -0.5> },
	{ WWMathKey::Exp_2_7,                 bitsOf<double, WWMath::Exp, 2.7> },
	{ WWMathKey::Expf_0_5f,               bitsOf<float, WWMath::Expf, 0.5f> },
	{ WWMathKey::Expf_Minus0_5f,          bitsOf<float, WWMath::Expf, -0.5f> },
	{ WWMathKey::Expf_2_7f,               bitsOf<float, WWMath::Expf, 2.7f> },

	{ WWMathKey::Log_0_5,                 bitsOf<double, WWMath::Log, 0.5> },
	{ WWMathKey::Log_2_7,                 bitsOf<double, WWMath::Log, 2.7> },
	{ WWMathKey::Log_187_66,              bitsOf<double, WWMath::Log, 187.66> },
	{ WWMathKey::Logf_0_5f,               bitsOf<float, WWMath::Logf, 0.5f> },
	{ WWMathKey::Logf_2_7f,               bitsOf<float, WWMath::Logf, 2.7f> },
	{ WWMathKey::Logf_187_66f,            bitsOf<float, WWMath::Logf, 187.66f> },

	{ WWMathKey::Log10_0_5,               bitsOf<double, WWMath::Log10, 0.5> },
	{ WWMathKey::Log10_2_7,               bitsOf<double, WWMath::Log10, 2.7> },
	{ WWMathKey::Log10_187_66,            bitsOf<double, WWMath::Log10, 187.66> },
	{ WWMathKey::Log10f_0_5f,             bitsOf<float, WWMath::Log10f, 0.5f> },
	{ WWMathKey::Log10f_2_7f,             bitsOf<float, WWMath::Log10f, 2.7f> },
	{ WWMathKey::Log10f_187_66f,          bitsOf<float, WWMath::Log10f, 187.66f> },

	{ WWMathKey::Pow_0_4_1_3,             bitsOf<double, WWMath::Pow, 0.4, 1.3> },
	{ WWMathKey::Pow_2_7_11_9,            bitsOf<double, WWMath::Pow, 2.7, 11.9> },
	{ WWMathKey::Pow_Minus3_5_2_0,        bitsOf<double, WWMath::Pow, -3.5, 2.0> },
	{ WWMathKey::Powf_0_4f_1_3f,          bitsOf<float, WWMath::Powf, 0.4f, 1.3f> },
	{ WWMathKey::Powf_2_7f_11_9f,         bitsOf<float, WWMath::Powf, 2.7f, 11.9f> },
	{ WWMathKey::Powf_Minus3_5f_2_0f,     bitsOf<float, WWMath::Powf, -3.5f, 2.0f> },

	{ WWMathKey::Sqrt_0_5f,               bitsOf<float, WWMath::Sqrt, 0.5f> },
	{ WWMathKey::Sqrt_2_7f,               bitsOf<float, WWMath::Sqrt, 2.7f> },
	{ WWMathKey::Sqrt_187_66f,            bitsOf<float, WWMath::Sqrt, 187.66f> },
	{ WWMathKey::Sqrt_0_5,                bitsOf<double, WWMath::Sqrt, 0.5> },
	{ WWMathKey::Sqrt_2_7,                bitsOf<double, WWMath::Sqrt, 2.7> },
	{ WWMathKey::Sqrt_187_66,             bitsOf<double, WWMath::Sqrt, 187.66> },
	{ WWMathKey::Sqrtf_0_5f,              bitsOf<float, WWMath::Sqrtf, 0.5f> },
	{ WWMathKey::Sqrtf_2_7f,              bitsOf<float, WWMath::Sqrtf, 2.7f> },
	{ WWMathKey::Sqrtf_187_66f,           bitsOf<float, WWMath::Sqrtf, 187.66f> },
	{ WWMathKey::Inv_Sqrt_Legacy_0_5f,    bitsOf<float, WWMath::Inv_Sqrt_Legacy, 0.5f> },
	{ WWMathKey::Inv_Sqrt_Legacy_2_7f,    bitsOf<float, WWMath::Inv_Sqrt_Legacy, 2.7f> },
	{ WWMathKey::Inv_Sqrt_Legacy_187_66f, bitsOf<float, WWMath::Inv_Sqrt_Legacy, 187.66f> },

	{ WWMathKey::Fabs_Minus0_5f,          bitsOf<float, WWMath::Fabs, -0.5f> },
	{ WWMathKey::Fabs_2_7f,               bitsOf<float, WWMath::Fabs, 2.7f> },
	{ WWMathKey::Fabs_100_5f,             bitsOf<float, WWMath::Fabs, 100.5f> },
	{ WWMathKey::Fabs_Minus0_5,           bitsOf<double, WWMath::Fabs, -0.5> },
	{ WWMathKey::Fabs_2_7,                bitsOf<double, WWMath::Fabs, 2.7> },
	{ WWMathKey::Fabs_100_5,              bitsOf<double, WWMath::Fabs, 100.5> },
	{ WWMathKey::Fabsf_Minus0_5f,         bitsOf<float, WWMath::Fabsf, -0.5f> },
	{ WWMathKey::Fabsf_2_7f,              bitsOf<float, WWMath::Fabsf, 2.7f> },
	{ WWMathKey::Fabsf_100_5f,            bitsOf<float, WWMath::Fabsf, 100.5f> },
	{ WWMathKey::Fabsf_Legacy_Minus0_5f,  bitsOf<float, WWMath::Fabsf_Legacy, -0.5f> },
	{ WWMathKey::Fabsf_Legacy_2_7f,       bitsOf<float, WWMath::Fabsf_Legacy, 2.7f> },
	{ WWMathKey::Fabsf_Legacy_100_5f,     bitsOf<float, WWMath::Fabsf_Legacy, 100.5f> },

	{ WWMathKey::Ceil_Minus0_5,           bitsOf<double, WWMath::Ceil, -0.5> },
	{ WWMathKey::Ceil_2_7,                bitsOf<double, WWMath::Ceil, 2.7> },
	{ WWMathKey::Ceil_100_5,              bitsOf<double, WWMath::Ceil, 100.5> },
	{ WWMathKey::Ceilf_Minus0_5f,         bitsOf<float, WWMath::Ceilf, -0.5f> },
	{ WWMathKey::Ceilf_2_7f,              bitsOf<float, WWMath::Ceilf, 2.7f> },
	{ WWMathKey::Ceilf_100_5f,            bitsOf<float, WWMath::Ceilf, 100.5f> },

	{ WWMathKey::Floor_Minus0_5,          bitsOf<double, WWMath::Floor, -0.5> },
	{ WWMathKey::Floor_2_7,               bitsOf<double, WWMath::Floor, 2.7> },
	{ WWMathKey::Floor_100_5,             bitsOf<double, WWMath::Floor, 100.5> },
	{ WWMathKey::Floorf_Minus0_5f,        bitsOf<float, WWMath::Floorf, -0.5f> },
	{ WWMathKey::Floorf_2_7f,             bitsOf<float, WWMath::Floorf, 2.7f> },
	{ WWMathKey::Floorf_100_5f,           bitsOf<float, WWMath::Floorf, 100.5f> },

	{ WWMathKey::Float_To_Long_Minus0_5f, bitsOf<float, WWMath::Float_To_Long, -0.5f> },
	{ WWMathKey::Float_To_Long_2_7f,      bitsOf<float, WWMath::Float_To_Long, 2.7f> },
	{ WWMathKey::Float_To_Long_100_5f,    bitsOf<float, WWMath::Float_To_Long, 100.5f> },
	{ WWMathKey::Float_To_Long_Minus0_5,  bitsOf<double, WWMath::Float_To_Long, -0.5> },
	{ WWMathKey::Float_To_Long_2_7,       bitsOf<double, WWMath::Float_To_Long, 2.7> },
	{ WWMathKey::Float_To_Long_100_5,     bitsOf<double, WWMath::Float_To_Long, 100.5> },

	{ WWMathKey::Div_Safe_1_0f_3_0f,      bitsOf<float, WWMath::Div_Safe, 1.0f, 3.0f> },
	{ WWMathKey::Div_Safe_1_0f_0_0f,      bitsOf<float, WWMath::Div_Safe, 1.0f, 0.0f> },
	{ WWMathKey::Div_Safe_1_0_3_0,        bitsOf<double, WWMath::Div_Safe, 1.0, 3.0> },
	{ WWMathKey::Div_Safe_1_0_0_0,        bitsOf<double, WWMath::Div_Safe, 1.0, 0.0> },
};

static_assert(hasEveryKeyInOrder(wwmathCalls), "wwmathCalls must have one row per WWMathKey, in WWMathKey order");
