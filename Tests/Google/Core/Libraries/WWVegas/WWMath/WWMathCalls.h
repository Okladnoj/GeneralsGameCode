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

#include <map>
#include <string>

#include "WWMathTestCase.h"

static const std::map<std::string, WWMathCall> wwmathCalls = {
	{ "Sin(0.5f)",                [] { return bits(WWMath::Sin(runtimeValue(0.5f))); } },
	{ "Sin(2.7f)",                [] { return bits(WWMath::Sin(runtimeValue(2.7f))); } },
	{ "Sin(187.66f)",             [] { return bits(WWMath::Sin(runtimeValue(187.66f))); } },
	{ "Sin(0.5)",                 [] { return bits(WWMath::Sin(runtimeValue(0.5))); } },
	{ "Sin(2.7)",                 [] { return bits(WWMath::Sin(runtimeValue(2.7))); } },
	{ "Sin(187.66)",              [] { return bits(WWMath::Sin(runtimeValue(187.66))); } },
	{ "Sinf(0.5f)",               [] { return bits(WWMath::Sinf(runtimeValue(0.5f))); } },
	{ "Sinf(2.7f)",               [] { return bits(WWMath::Sinf(runtimeValue(2.7f))); } },
	{ "Sinf(187.66f)",            [] { return bits(WWMath::Sinf(runtimeValue(187.66f))); } },
	{ "Sinf_Legacy(0.5f)",        [] { return bits(WWMath::Sinf_Legacy(runtimeValue(0.5f))); } },
	{ "Sinf_Legacy(2.7f)",        [] { return bits(WWMath::Sinf_Legacy(runtimeValue(2.7f))); } },
	{ "Sinf_Legacy(187.66f)",     [] { return bits(WWMath::Sinf_Legacy(runtimeValue(187.66f))); } },

	{ "Cos(0.5f)",                [] { return bits(WWMath::Cos(runtimeValue(0.5f))); } },
	{ "Cos(2.7f)",                [] { return bits(WWMath::Cos(runtimeValue(2.7f))); } },
	{ "Cos(187.66f)",             [] { return bits(WWMath::Cos(runtimeValue(187.66f))); } },
	{ "Cos(0.5)",                 [] { return bits(WWMath::Cos(runtimeValue(0.5))); } },
	{ "Cos(2.7)",                 [] { return bits(WWMath::Cos(runtimeValue(2.7))); } },
	{ "Cos(187.66)",              [] { return bits(WWMath::Cos(runtimeValue(187.66))); } },
	{ "Cosf(0.5f)",               [] { return bits(WWMath::Cosf(runtimeValue(0.5f))); } },
	{ "Cosf(2.7f)",               [] { return bits(WWMath::Cosf(runtimeValue(2.7f))); } },
	{ "Cosf(187.66f)",            [] { return bits(WWMath::Cosf(runtimeValue(187.66f))); } },
	{ "Cosf_Legacy(0.5f)",        [] { return bits(WWMath::Cosf_Legacy(runtimeValue(0.5f))); } },
	{ "Cosf_Legacy(2.7f)",        [] { return bits(WWMath::Cosf_Legacy(runtimeValue(2.7f))); } },
	{ "Cosf_Legacy(187.66f)",     [] { return bits(WWMath::Cosf_Legacy(runtimeValue(187.66f))); } },

	{ "Tan(0.5f)",                [] { return bits(WWMath::Tan(runtimeValue(0.5f))); } },
	{ "Tan(2.7f)",                [] { return bits(WWMath::Tan(runtimeValue(2.7f))); } },
	{ "Tan(187.66f)",             [] { return bits(WWMath::Tan(runtimeValue(187.66f))); } },
	{ "Tan(0.5)",                 [] { return bits(WWMath::Tan(runtimeValue(0.5))); } },
	{ "Tan(2.7)",                 [] { return bits(WWMath::Tan(runtimeValue(2.7))); } },
	{ "Tan(187.66)",              [] { return bits(WWMath::Tan(runtimeValue(187.66))); } },
	{ "Tanf(0.5f)",               [] { return bits(WWMath::Tanf(runtimeValue(0.5f))); } },
	{ "Tanf(2.7f)",               [] { return bits(WWMath::Tanf(runtimeValue(2.7f))); } },
	{ "Tanf(187.66f)",            [] { return bits(WWMath::Tanf(runtimeValue(187.66f))); } },

	{ "Asin(-0.5f)",              [] { return bits(WWMath::Asin(runtimeValue(-0.5f))); } },
	{ "Asin(0.25f)",              [] { return bits(WWMath::Asin(runtimeValue(0.25f))); } },
	{ "Asin(0.9f)",               [] { return bits(WWMath::Asin(runtimeValue(0.9f))); } },
	{ "Asin(-0.5)",               [] { return bits(WWMath::Asin(runtimeValue(-0.5))); } },
	{ "Asin(0.25)",               [] { return bits(WWMath::Asin(runtimeValue(0.25))); } },
	{ "Asin(0.9)",                [] { return bits(WWMath::Asin(runtimeValue(0.9))); } },
	{ "Asinf(-0.5f)",             [] { return bits(WWMath::Asinf(runtimeValue(-0.5f))); } },
	{ "Asinf(0.25f)",             [] { return bits(WWMath::Asinf(runtimeValue(0.25f))); } },
	{ "Asinf(0.9f)",              [] { return bits(WWMath::Asinf(runtimeValue(0.9f))); } },

	{ "Acos(-0.5f)",              [] { return bits(WWMath::Acos(runtimeValue(-0.5f))); } },
	{ "Acos(0.25f)",              [] { return bits(WWMath::Acos(runtimeValue(0.25f))); } },
	{ "Acos(0.9f)",               [] { return bits(WWMath::Acos(runtimeValue(0.9f))); } },
	{ "Acos(-0.5)",               [] { return bits(WWMath::Acos(runtimeValue(-0.5))); } },
	{ "Acos(0.25)",               [] { return bits(WWMath::Acos(runtimeValue(0.25))); } },
	{ "Acos(0.9)",                [] { return bits(WWMath::Acos(runtimeValue(0.9))); } },
	{ "Acosf(-0.5f)",             [] { return bits(WWMath::Acosf(runtimeValue(-0.5f))); } },
	{ "Acosf(0.25f)",             [] { return bits(WWMath::Acosf(runtimeValue(0.25f))); } },
	{ "Acosf(0.9f)",              [] { return bits(WWMath::Acosf(runtimeValue(0.9f))); } },

	{ "Atan(0.5f)",               [] { return bits(WWMath::Atan(runtimeValue(0.5f))); } },
	{ "Atan(2.7f)",               [] { return bits(WWMath::Atan(runtimeValue(2.7f))); } },
	{ "Atan(187.66f)",            [] { return bits(WWMath::Atan(runtimeValue(187.66f))); } },
	{ "Atan(0.5)",                [] { return bits(WWMath::Atan(runtimeValue(0.5))); } },
	{ "Atan(2.7)",                [] { return bits(WWMath::Atan(runtimeValue(2.7))); } },
	{ "Atan(187.66)",             [] { return bits(WWMath::Atan(runtimeValue(187.66))); } },
	{ "Atanf(0.5f)",              [] { return bits(WWMath::Atanf(runtimeValue(0.5f))); } },
	{ "Atanf(2.7f)",              [] { return bits(WWMath::Atanf(runtimeValue(2.7f))); } },
	{ "Atanf(187.66f)",           [] { return bits(WWMath::Atanf(runtimeValue(187.66f))); } },

	{ "Atan2(0.4f, 1.3f)",        [] { return bits(WWMath::Atan2(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ "Atan2(2.7f, 11.9f)",       [] { return bits(WWMath::Atan2(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ "Atan2(-3.5f, 2.0f)",       [] { return bits(WWMath::Atan2(runtimeValue(-3.5f), runtimeValue(2.0f))); } },
	{ "Atan2(0.4, 1.3)",          [] { return bits(WWMath::Atan2(runtimeValue(0.4), runtimeValue(1.3))); } },
	{ "Atan2(2.7, 11.9)",         [] { return bits(WWMath::Atan2(runtimeValue(2.7), runtimeValue(11.9))); } },
	{ "Atan2(-3.5, 2.0)",         [] { return bits(WWMath::Atan2(runtimeValue(-3.5), runtimeValue(2.0))); } },
	{ "Atan2f(0.4f, 1.3f)",       [] { return bits(WWMath::Atan2f(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ "Atan2f(2.7f, 11.9f)",      [] { return bits(WWMath::Atan2f(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ "Atan2f(-3.5f, 2.0f)",      [] { return bits(WWMath::Atan2f(runtimeValue(-3.5f), runtimeValue(2.0f))); } },

	{ "Sinh(0.5)",                [] { return bits(WWMath::Sinh(runtimeValue(0.5))); } },
	{ "Sinh(-0.5)",               [] { return bits(WWMath::Sinh(runtimeValue(-0.5))); } },
	{ "Sinh(2.7)",                [] { return bits(WWMath::Sinh(runtimeValue(2.7))); } },
	{ "Sinhf(0.5f)",              [] { return bits(WWMath::Sinhf(runtimeValue(0.5f))); } },
	{ "Sinhf(-0.5f)",             [] { return bits(WWMath::Sinhf(runtimeValue(-0.5f))); } },
	{ "Sinhf(2.7f)",              [] { return bits(WWMath::Sinhf(runtimeValue(2.7f))); } },

	{ "Cosh(0.5)",                [] { return bits(WWMath::Cosh(runtimeValue(0.5))); } },
	{ "Cosh(-0.5)",               [] { return bits(WWMath::Cosh(runtimeValue(-0.5))); } },
	{ "Cosh(2.7)",                [] { return bits(WWMath::Cosh(runtimeValue(2.7))); } },
	{ "Coshf(0.5f)",              [] { return bits(WWMath::Coshf(runtimeValue(0.5f))); } },
	{ "Coshf(-0.5f)",             [] { return bits(WWMath::Coshf(runtimeValue(-0.5f))); } },
	{ "Coshf(2.7f)",              [] { return bits(WWMath::Coshf(runtimeValue(2.7f))); } },

	{ "Tanh(0.5)",                [] { return bits(WWMath::Tanh(runtimeValue(0.5))); } },
	{ "Tanh(-0.5)",               [] { return bits(WWMath::Tanh(runtimeValue(-0.5))); } },
	{ "Tanh(2.7)",                [] { return bits(WWMath::Tanh(runtimeValue(2.7))); } },
	{ "Tanhf(0.5f)",              [] { return bits(WWMath::Tanhf(runtimeValue(0.5f))); } },
	{ "Tanhf(-0.5f)",             [] { return bits(WWMath::Tanhf(runtimeValue(-0.5f))); } },
	{ "Tanhf(2.7f)",              [] { return bits(WWMath::Tanhf(runtimeValue(2.7f))); } },

	{ "Exp(0.5)",                 [] { return bits(WWMath::Exp(runtimeValue(0.5))); } },
	{ "Exp(-0.5)",                [] { return bits(WWMath::Exp(runtimeValue(-0.5))); } },
	{ "Exp(2.7)",                 [] { return bits(WWMath::Exp(runtimeValue(2.7))); } },
	{ "Expf(0.5f)",               [] { return bits(WWMath::Expf(runtimeValue(0.5f))); } },
	{ "Expf(-0.5f)",              [] { return bits(WWMath::Expf(runtimeValue(-0.5f))); } },
	{ "Expf(2.7f)",               [] { return bits(WWMath::Expf(runtimeValue(2.7f))); } },

	{ "Log(0.5)",                 [] { return bits(WWMath::Log(runtimeValue(0.5))); } },
	{ "Log(2.7)",                 [] { return bits(WWMath::Log(runtimeValue(2.7))); } },
	{ "Log(187.66)",              [] { return bits(WWMath::Log(runtimeValue(187.66))); } },
	{ "Logf(0.5f)",               [] { return bits(WWMath::Logf(runtimeValue(0.5f))); } },
	{ "Logf(2.7f)",               [] { return bits(WWMath::Logf(runtimeValue(2.7f))); } },
	{ "Logf(187.66f)",            [] { return bits(WWMath::Logf(runtimeValue(187.66f))); } },

	{ "Log10(0.5)",               [] { return bits(WWMath::Log10(runtimeValue(0.5))); } },
	{ "Log10(2.7)",               [] { return bits(WWMath::Log10(runtimeValue(2.7))); } },
	{ "Log10(187.66)",            [] { return bits(WWMath::Log10(runtimeValue(187.66))); } },
	{ "Log10f(0.5f)",             [] { return bits(WWMath::Log10f(runtimeValue(0.5f))); } },
	{ "Log10f(2.7f)",             [] { return bits(WWMath::Log10f(runtimeValue(2.7f))); } },
	{ "Log10f(187.66f)",          [] { return bits(WWMath::Log10f(runtimeValue(187.66f))); } },

	{ "Pow(0.4, 1.3)",            [] { return bits(WWMath::Pow(runtimeValue(0.4), runtimeValue(1.3))); } },
	{ "Pow(2.7, 11.9)",           [] { return bits(WWMath::Pow(runtimeValue(2.7), runtimeValue(11.9))); } },
	{ "Pow(-3.5, 2.0)",           [] { return bits(WWMath::Pow(runtimeValue(-3.5), runtimeValue(2.0))); } },
	{ "Powf(0.4f, 1.3f)",         [] { return bits(WWMath::Powf(runtimeValue(0.4f), runtimeValue(1.3f))); } },
	{ "Powf(2.7f, 11.9f)",        [] { return bits(WWMath::Powf(runtimeValue(2.7f), runtimeValue(11.9f))); } },
	{ "Powf(-3.5f, 2.0f)",        [] { return bits(WWMath::Powf(runtimeValue(-3.5f), runtimeValue(2.0f))); } },

	{ "Sqrt(0.5f)",               [] { return bits(WWMath::Sqrt(runtimeValue(0.5f))); } },
	{ "Sqrt(2.7f)",               [] { return bits(WWMath::Sqrt(runtimeValue(2.7f))); } },
	{ "Sqrt(187.66f)",            [] { return bits(WWMath::Sqrt(runtimeValue(187.66f))); } },
	{ "Sqrt(0.5)",                [] { return bits(WWMath::Sqrt(runtimeValue(0.5))); } },
	{ "Sqrt(2.7)",                [] { return bits(WWMath::Sqrt(runtimeValue(2.7))); } },
	{ "Sqrt(187.66)",             [] { return bits(WWMath::Sqrt(runtimeValue(187.66))); } },
	{ "Sqrtf(0.5f)",              [] { return bits(WWMath::Sqrtf(runtimeValue(0.5f))); } },
	{ "Sqrtf(2.7f)",              [] { return bits(WWMath::Sqrtf(runtimeValue(2.7f))); } },
	{ "Sqrtf(187.66f)",           [] { return bits(WWMath::Sqrtf(runtimeValue(187.66f))); } },
	{ "Inv_Sqrt_Legacy(0.5f)",    [] { return bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(0.5f))); } },
	{ "Inv_Sqrt_Legacy(2.7f)",    [] { return bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(2.7f))); } },
	{ "Inv_Sqrt_Legacy(187.66f)", [] { return bits(WWMath::Inv_Sqrt_Legacy(runtimeValue(187.66f))); } },

	{ "Fabs(-0.5f)",              [] { return bits(WWMath::Fabs(runtimeValue(-0.5f))); } },
	{ "Fabs(2.7f)",               [] { return bits(WWMath::Fabs(runtimeValue(2.7f))); } },
	{ "Fabs(100.5f)",             [] { return bits(WWMath::Fabs(runtimeValue(100.5f))); } },
	{ "Fabs(-0.5)",               [] { return bits(WWMath::Fabs(runtimeValue(-0.5))); } },
	{ "Fabs(2.7)",                [] { return bits(WWMath::Fabs(runtimeValue(2.7))); } },
	{ "Fabs(100.5)",              [] { return bits(WWMath::Fabs(runtimeValue(100.5))); } },
	{ "Fabsf(-0.5f)",             [] { return bits(WWMath::Fabsf(runtimeValue(-0.5f))); } },
	{ "Fabsf(2.7f)",              [] { return bits(WWMath::Fabsf(runtimeValue(2.7f))); } },
	{ "Fabsf(100.5f)",            [] { return bits(WWMath::Fabsf(runtimeValue(100.5f))); } },
	{ "Fabsf_Legacy(-0.5f)",      [] { return bits(WWMath::Fabsf_Legacy(runtimeValue(-0.5f))); } },
	{ "Fabsf_Legacy(2.7f)",       [] { return bits(WWMath::Fabsf_Legacy(runtimeValue(2.7f))); } },
	{ "Fabsf_Legacy(100.5f)",     [] { return bits(WWMath::Fabsf_Legacy(runtimeValue(100.5f))); } },

	{ "Ceil(-0.5)",               [] { return bits(WWMath::Ceil(runtimeValue(-0.5))); } },
	{ "Ceil(2.7)",                [] { return bits(WWMath::Ceil(runtimeValue(2.7))); } },
	{ "Ceil(100.5)",              [] { return bits(WWMath::Ceil(runtimeValue(100.5))); } },
	{ "Ceilf(-0.5f)",             [] { return bits(WWMath::Ceilf(runtimeValue(-0.5f))); } },
	{ "Ceilf(2.7f)",              [] { return bits(WWMath::Ceilf(runtimeValue(2.7f))); } },
	{ "Ceilf(100.5f)",            [] { return bits(WWMath::Ceilf(runtimeValue(100.5f))); } },

	{ "Floor(-0.5)",              [] { return bits(WWMath::Floor(runtimeValue(-0.5))); } },
	{ "Floor(2.7)",               [] { return bits(WWMath::Floor(runtimeValue(2.7))); } },
	{ "Floor(100.5)",             [] { return bits(WWMath::Floor(runtimeValue(100.5))); } },
	{ "Floorf(-0.5f)",            [] { return bits(WWMath::Floorf(runtimeValue(-0.5f))); } },
	{ "Floorf(2.7f)",             [] { return bits(WWMath::Floorf(runtimeValue(2.7f))); } },
	{ "Floorf(100.5f)",           [] { return bits(WWMath::Floorf(runtimeValue(100.5f))); } },

	{ "Float_To_Long(-0.5f)",     [] { return bits(WWMath::Float_To_Long(runtimeValue(-0.5f))); } },
	{ "Float_To_Long(2.7f)",      [] { return bits(WWMath::Float_To_Long(runtimeValue(2.7f))); } },
	{ "Float_To_Long(100.5f)",    [] { return bits(WWMath::Float_To_Long(runtimeValue(100.5f))); } },
	{ "Float_To_Long(-0.5)",      [] { return bits(WWMath::Float_To_Long(runtimeValue(-0.5))); } },
	{ "Float_To_Long(2.7)",       [] { return bits(WWMath::Float_To_Long(runtimeValue(2.7))); } },
	{ "Float_To_Long(100.5)",     [] { return bits(WWMath::Float_To_Long(runtimeValue(100.5))); } },

	{ "Div_Safe(1.0f, 3.0f)",     [] { return bits(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(3.0f))); } },
	{ "Div_Safe(1.0f, 0.0f)",     [] { return bits(WWMath::Div_Safe(runtimeValue(1.0f), runtimeValue(0.0f))); } },
	{ "Div_Safe(1.0, 3.0)",       [] { return bits(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(3.0))); } },
	{ "Div_Safe(1.0, 0.0)",       [] { return bits(WWMath::Div_Safe(runtimeValue(1.0), runtimeValue(0.0))); } },
};
