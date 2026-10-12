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

#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <ostream>
#include <string>

#include "WWMath/wwmath.h"

// The game simulation must compute the same bits from one build to the next.
// Every call has a dump of its bits, with one column per build configuration.
enum WWMathColumn
{
	WWMATH_COLUMN_NONE = -1,
	WWMATH_COLUMN_DETERMINISTIC,
	WWMATH_COLUMN_WIN32_CRT,
	WWMATH_COLUMN_COUNT
};

#if USE_DETERMINISTIC_MATH
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_DETERMINISTIC;
#elif defined(_MSC_VER) && defined(_M_IX86)
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_WIN32_CRT;
#else
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_NONE;
#endif

typedef std::string (*WWMathCall)();
typedef std::array<const char *, WWMATH_COLUMN_COUNT> WWMathDump;

struct WWMathCallEntry
{
	std::string key;
	WWMathCall call;
};

struct WWMathDumpEntry
{
	std::string key;
	WWMathDump dump;
};

struct WWMathCase
{
	std::string name;
	WWMathCall call;
	const char *dump;
};

inline void PrintTo(const WWMathCase &mathCase, std::ostream *stream)
{
	*stream << mathCase.name;
}

template <typename T>
inline T runtimeValue(T value)
{
	volatile T runtime = value;
	return runtime;
}

inline std::string toBitsString(float value)
{
	return std::format("{:08X}", std::bit_cast<uint32_t>(value));
}

inline std::string toBitsString(double value)
{
	return std::format("{:016X}", std::bit_cast<uint64_t>(value));
}

inline std::string toBitsString(long value)
{
	return std::to_string(value);
}
