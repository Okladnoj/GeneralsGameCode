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

static const std::map<std::string, WWMathDump> wwmathDumps = {
	//                              DET                 win32 CRT
	{ "Sin(0.5f)",                { "3EF57744",         "3EF57744"         } },
	{ "Sin(2.7f)",                { "3EDAD188",         "3EDAD188"         } },
	{ "Sin(187.66f)",             { "BF3DDE0A",         "BF3DDE0A"         } },
	{ "Sin(0.5)",                 { "3FDEAEE8744B05F0", "3FDEAEE8744B05F0" } },
	{ "Sin(2.7)",                 { "3FDB5A312424A70C", "3FDB5A312424A70C" } },
	{ "Sin(187.66)",              { "BFE7BBC6596262AE", "BFE7BBC6596262AE" } },
	{ "Sinf(0.5f)",               { "3EF57744",         "3EF57744"         } },
	{ "Sinf(2.7f)",               { "3EDAD188",         "3EDAD188"         } },
	{ "Sinf(187.66f)",            { "BF3DDE0A",         "BF3DDE0A"         } },
	{ "Sinf_Legacy(0.5f)",        { "3EF57744",         "3EF57744"         } },
	{ "Sinf_Legacy(2.7f)",        { "3EDAD188",         "3EDAD188"         } },
	{ "Sinf_Legacy(187.66f)",     { "BF3DDE0A",         "BF3DDE0A"         } },

	{ "Cos(0.5f)",                { "3F60A940",         "3F60A940"         } },
	{ "Cos(2.7f)",                { "BF677146",         "BF677146"         } },
	{ "Cos(187.66f)",             { "3F2BB74E",         "3F2BB74E"         } },
	{ "Cos(0.5)",                 { "3FEC1528065B7D50", "3FEC1528065B7D50" } },
	{ "Cos(2.7)",                 { "BFECEE28B36603A6", "BFECEE28B36603A6" } },
	{ "Cos(187.66)",              { "3FE576E4096F1DE4", "3FE576E4096F1DE4" } },
	{ "Cosf(0.5f)",               { "3F60A940",         "3F60A940"         } },
	{ "Cosf(2.7f)",               { "BF677146",         "BF677146"         } },
	{ "Cosf(187.66f)",            { "3F2BB74E",         "3F2BB74E"         } },
	{ "Cosf_Legacy(0.5f)",        { "3F60A940",         "3F60A940"         } },
	{ "Cosf_Legacy(2.7f)",        { "BF677146",         "BF677146"         } },
	{ "Cosf_Legacy(187.66f)",     { "3F2BB74E",         "3F2BB74E"         } },

	{ "Tan(0.5f)",                { "3F0BDA7A",         "3F0BDA7B"         } },
	{ "Tan(2.7f)",                { "BEF20959",         "BEF20959"         } },
	{ "Tan(187.66f)",             { "BF8D87C1",         "BF8D87C1"         } },
	{ "Tan(0.5)",                 { "3FE17B4F5BF3474A", "3FE17B4F5BF3474A" } },
	{ "Tan(2.7)",                 { "BFDE412B62BA7143", "BFDE412B62BA7143" } },
	{ "Tan(187.66)",              { "BFF1B1009E8D90B0", "BFF1B1009E8D90B0" } },
	{ "Tanf(0.5f)",               { "3F0BDA7A",         "3F0BDA7B"         } },
	{ "Tanf(2.7f)",               { "BEF20959",         "BEF20959"         } },
	{ "Tanf(187.66f)",            { "BF8D87C1",         "BF8D87C1"         } },

	{ "Asin(-0.5f)",              { "BF060A94",         "BF060A92"         } },
	{ "Asin(0.25f)",              { "3E815F4E",         "3E815F4E"         } },
	{ "Asin(0.9f)",               { "3F8F549C",         "3F8F549B"         } },
	{ "Asin(-0.5)",               { "BFE0C152382D7366", "BFE0C152382D7366" } },
	{ "Asin(0.25)",               { "3FD02BE9CE0B87CD", "3FD02BE9CE0B87CD" } },
	{ "Asin(0.9)",                { "3FF1EA93705FA172", "3FF1EA93705FA172" } },
	{ "Asinf(-0.5f)",             { "BF060A94",         "BF060A92"         } },
	{ "Asinf(0.25f)",             { "3E815F4E",         "3E815F4E"         } },
	{ "Asinf(0.9f)",              { "3F8F549C",         "3F8F549B"         } },

	{ "Acos(-0.5f)",              { "40060A92",         "40060A92"         } },
	{ "Acos(0.25f)",              { "3FA8B807",         "3FA8B807"         } },
	{ "Acos(0.9f)",               { "3EE6ECFE",         "3EE6ECFE"         } },
	{ "Acos(-0.5)",               { "4000C152382D7366", "4000C152382D7366" } },
	{ "Acos(0.25)",               { "3FF51700E0C14B25", "3FF51700E0C14B25" } },
	{ "Acos(0.9)",                { "3FDCDD9F8F922E98", "3FDCDD9F8F922E98" } },
	{ "Acosf(-0.5f)",             { "40060A92",         "40060A92"         } },
	{ "Acosf(0.25f)",             { "3FA8B807",         "3FA8B807"         } },
	{ "Acosf(0.9f)",              { "3EE6ECFE",         "3EE6ECFE"         } },

	{ "Atan(0.5f)",               { "3EED6338",         "3EED6338"         } },
	{ "Atan(2.7f)",               { "3F9BA8DC",         "3F9BA8DC"         } },
	{ "Atan(187.66f)",            { "3FC8613E",         "3FC8613E"         } },
	{ "Atan(0.5)",                { "3FDDAC6700000000", "3FDDAC670561BB4F" } },
	{ "Atan(2.7)",                { "3FF3751B80000000", "3FF3751B7ED2AEC1" } },
	{ "Atan(187.66)",             { "3FF90C27C0000000", "3FF90C27BEADA1E0" } },
	{ "Atanf(0.5f)",              { "3EED6338",         "3EED6338"         } },
	{ "Atanf(2.7f)",              { "3F9BA8DC",         "3F9BA8DC"         } },
	{ "Atanf(187.66f)",           { "3FC8613E",         "3FC8613E"         } },

	{ "Atan2(0.4f, 1.3f)",        { "3E98D4DA",         "3E98D4DB"         } },
	{ "Atan2(2.7f, 11.9f)",       { "3E6477D5",         "3E6477D5"         } },
	{ "Atan2(-3.5f, 2.0f)",       { "BF869C79",         "BF869C79"         } },
	{ "Atan2(0.4, 1.3)",          { "3FD31A9B40000000", "3FD31A9B43436DE1" } },
	{ "Atan2(2.7, 11.9)",         { "3FCC8EFAA0000000", "3FCC8EFA8684064B" } },
	{ "Atan2(-3.5, 2.0)",         { "BFF0D38F20000000", "BFF0D38F2C5BA09F" } },
	{ "Atan2f(0.4f, 1.3f)",       { "3E98D4DA",         "3E98D4DB"         } },
	{ "Atan2f(2.7f, 11.9f)",      { "3E6477D5",         "3E6477D5"         } },
	{ "Atan2f(-3.5f, 2.0f)",      { "BF869C79",         "BF869C79"         } },

	{ "Sinh(0.5)",                { "3FE0ACD00FE63B97", "3FE0ACD00FE63B97" } },
	{ "Sinh(-0.5)",               { "BFE0ACD00FE63B97", "BFE0ACD00FE63B97" } },
	{ "Sinh(2.7)",                { "401DA0036F88823C", "401DA0036F88823C" } },
	{ "Sinhf(0.5f)",              { "3F056680",         "3F056680"         } },
	{ "Sinhf(-0.5f)",             { "BF056680",         "BF056680"         } },
	{ "Sinhf(2.7f)",              { "40ED001C",         "40ED001C"         } },

	{ "Cosh(0.5)",                { "3FF20AC1862AE8D0", "3FF20AC1862AE8D0" } },
	{ "Cosh(-0.5)",               { "3FF20AC1862AE8D0", "3FF20AC1862AE8D0" } },
	{ "Cosh(2.7)",                { "401DE4D4F525D240", "401DE4D4F525D23F" } },
	{ "Coshf(0.5f)",              { "3F90560C",         "3F90560C"         } },
	{ "Coshf(-0.5f)",             { "3F90560C",         "3F90560C"         } },
	{ "Coshf(2.7f)",              { "40EF26A8",         "40EF26A8"         } },

	{ "Tanh(0.5)",                { "3FDD9353D7568AF3", "3FDD9353D7568AF3" } },
	{ "Tanh(-0.5)",               { "BFDD9353D7568AF3", "BFDD9353D7568AF3" } },
	{ "Tanh(2.7)",                { "3FEFB655437478D5", "3FEFB655437478D5" } },
	{ "Tanhf(0.5f)",              { "3EEC9A9F",         "3EEC9A9F"         } },
	{ "Tanhf(-0.5f)",             { "BEEC9A9F",         "BEEC9A9F"         } },
	{ "Tanhf(2.7f)",              { "3F7DB2AA",         "3F7DB2AA"         } },

	{ "Exp(0.5)",                 { "3FFA61298E1E069C", "3FFA61298E1E069C" } },
	{ "Exp(-0.5)",                { "3FE368B2FC6F960A", "3FE368B2FC6F960A" } },
	{ "Exp(2.7)",                 { "402DC26C32572A3E", "402DC26C32572A3E" } },
	{ "Expf(0.5f)",               { "3FD3094C",         "3FD3094C"         } },
	{ "Expf(-0.5f)",              { "3F1B4598",         "3F1B4598"         } },
	{ "Expf(2.7f)",               { "416E1362",         "416E1362"         } },

	{ "Log(0.5)",                 { "BFE62E42FEFA39EF", "BFE62E42FEFA39EF" } },
	{ "Log(2.7)",                 { "3FEFC8B7F138BDEA", "3FEFC8B7F138BDEA" } },
	{ "Log(187.66)",              { "4014F043528DBB65", "4014F043528DBB65" } },
	{ "Logf(0.5f)",               { "BF317218",         "BF317218"         } },
	{ "Logf(2.7f)",               { "3F7E45C0",         "3F7E45C0"         } },
	{ "Logf(187.66f)",            { "40A7821B",         "40A7821B"         } },

	{ "Log10(0.5)",               { "BFD34413509F79FF", "BFD34413509F79FF" } },
	{ "Log10(2.7)",               { "3FDB9B76C2EF81F8", "3FDB9B76C2EF81F8" } },
	{ "Log10(187.66)",            { "40022FDD82152DEA", "40022FDD82152DEA" } },
	{ "Log10f(0.5f)",             { "BE9A209B",         "BE9A209B"         } },
	{ "Log10f(2.7f)",             { "3EDCDBB6",         "3EDCDBB6"         } },
	{ "Log10f(187.66f)",          { "40117EEC",         "40117EEC"         } },

	{ "Pow(0.4, 1.3)",            { "3FD3727E60000000", "3FD3727E49ADFAD4" } },
	{ "Pow(2.7, 11.9)",           { "410096F740000000", "410096F75FA58808" } },
	{ "Pow(-3.5, 2.0)",           { "4028800000000000", "4028800000000000" } },
	{ "Powf(0.4f, 1.3f)",         { "3E9B93F3",         "3E9B93F3"         } },
	{ "Powf(2.7f, 11.9f)",        { "4804B7BA",         "4804B7BA"         } },
	{ "Powf(-3.5f, 2.0f)",        { "41440000",         "41440000"         } },

	{ "Sqrt(0.5f)",               { "3F3504F3",         "3F3504F3"         } },
	{ "Sqrt(2.7f)",               { "3FD25352",         "3FD25352"         } },
	{ "Sqrt(187.66f)",            { "415B2EB7",         "415B2EB7"         } },
	{ "Sqrt(0.5)",                { "3FE6A09E667F3BCD", "3FE6A09E667F3BCD" } },
	{ "Sqrt(2.7)",                { "3FFA4A6A2F74C6AC", "3FFA4A6A2F74C6AC" } },
	{ "Sqrt(187.66)",             { "402B65D6E280ACC1", "402B65D6E280ACC1" } },
	{ "Sqrtf(0.5f)",              { "3F3504F3",         "3F3504F3"         } },
	{ "Sqrtf(2.7f)",              { "3FD25352",         "3FD25352"         } },
	{ "Sqrtf(187.66f)",           { "415B2EB7",         "415B2EB7"         } },
	{ "Inv_Sqrt_Legacy(0.5f)",    { "3FB504F3",         "3FB504F3"         } },
	{ "Inv_Sqrt_Legacy(2.7f)",    { "3F1BCBF0",         "3F1BCBF0"         } },
	{ "Inv_Sqrt_Legacy(187.66f)", { "3D958042",         "3D958041"         } },

	{ "Fabs(-0.5f)",              { "3F000000",         "3F000000"         } },
	{ "Fabs(2.7f)",               { "402CCCCD",         "402CCCCD"         } },
	{ "Fabs(100.5f)",             { "42C90000",         "42C90000"         } },
	{ "Fabs(-0.5)",               { "3FE0000000000000", "3FE0000000000000" } },
	{ "Fabs(2.7)",                { "400599999999999A", "400599999999999A" } },
	{ "Fabs(100.5)",              { "4059200000000000", "4059200000000000" } },
	{ "Fabsf(-0.5f)",             { "3F000000",         "3F000000"         } },
	{ "Fabsf(2.7f)",              { "402CCCCD",         "402CCCCD"         } },
	{ "Fabsf(100.5f)",            { "42C90000",         "42C90000"         } },
	{ "Fabsf_Legacy(-0.5f)",      { "3F000000",         "3F000000"         } },
	{ "Fabsf_Legacy(2.7f)",       { "402CCCCD",         "402CCCCD"         } },
	{ "Fabsf_Legacy(100.5f)",     { "42C90000",         "42C90000"         } },

	{ "Ceil(-0.5)",               { "8000000000000000", "8000000000000000" } },
	{ "Ceil(2.7)",                { "4008000000000000", "4008000000000000" } },
	{ "Ceil(100.5)",              { "4059400000000000", "4059400000000000" } },
	{ "Ceilf(-0.5f)",             { "80000000",         "80000000"         } },
	{ "Ceilf(2.7f)",              { "40400000",         "40400000"         } },
	{ "Ceilf(100.5f)",            { "42CA0000",         "42CA0000"         } },

	{ "Floor(-0.5)",              { "BFF0000000000000", "BFF0000000000000" } },
	{ "Floor(2.7)",               { "4000000000000000", "4000000000000000" } },
	{ "Floor(100.5)",             { "4059000000000000", "4059000000000000" } },
	{ "Floorf(-0.5f)",            { "BF800000",         "BF800000"         } },
	{ "Floorf(2.7f)",             { "40000000",         "40000000"         } },
	{ "Floorf(100.5f)",           { "42C80000",         "42C80000"         } },

	{ "Float_To_Long(-0.5f)",     { "0",                "0"                } },
	{ "Float_To_Long(2.7f)",      { "3",                "3"                } },
	{ "Float_To_Long(100.5f)",    { "100",              "100"              } },
	{ "Float_To_Long(-0.5)",      { "0",                "0"                } },
	{ "Float_To_Long(2.7)",       { "3",                "3"                } },
	{ "Float_To_Long(100.5)",     { "100",              "100"              } },

	{ "Div_Safe(1.0f, 3.0f)",     { "3EAAAAAB",         "3EAAAAAB"         } },
	{ "Div_Safe(1.0f, 0.0f)",     { "461C4000",         "7F800000"         } },
	{ "Div_Safe(1.0, 3.0)",       { "3FD5555555555555", "3FD5555555555555" } },
	{ "Div_Safe(1.0, 0.0)",       { "40C3880008472404", "7FF0000000000000" } },
};
