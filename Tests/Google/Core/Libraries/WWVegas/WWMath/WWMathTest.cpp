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
#include <cctype>
#include <string>
#include <vector>

#include "WWMathCalls.h"
#include "WWMathDumps.h"
#include "WWMathTestCase.h"

static const char *findDump(const std::string &name)
{
	if (wwmathColumn == WWMATH_COLUMN_NONE)
	{
		return nullptr;
	}

	const auto dump = wwmathDumps.find(name);
	if (dump == wwmathDumps.end())
	{
		return nullptr;
	}

	return dump->second[wwmathColumn];
}

static std::vector<WWMathCase> mergeCallsWithDumps()
{
	std::vector<WWMathCase> cases;
	for (const auto &call : wwmathCalls)
	{
		cases.push_back({ call.first, call.second, findDump(call.first) });
	}

	for (const auto &dump : wwmathDumps)
	{
		if (wwmathCalls.count(dump.first) == 0)
		{
			cases.push_back({ dump.first, nullptr, findDump(dump.first) });
		}
	}

	return cases;
}

static std::string testNameOf(const ::testing::TestParamInfo<WWMathCase> &info)
{
	std::string testName;
	for (char c : info.param.name)
	{
		if (isalnum(static_cast<unsigned char>(c)))
		{
			testName += c;
		}
		else if (c == '-')
		{
			testName += "Minus";
		}
		else if (!testName.empty() && testName.back() != '_')
		{
			testName += '_';
		}
	}

	while (!testName.empty() && testName.back() == '_')
	{
		testName.pop_back();
	}

	return testName;
}

class WWMathBits : public ::testing::TestWithParam<WWMathCase>
{
protected:
	void SetUp() override
	{
		if (wwmathColumn == WWMATH_COLUMN_NONE)
		{
			GTEST_SKIP() << "No WWMath dump column for this build configuration";
		}
	}
};

TEST_P(WWMathBits, MatchesDump)
{
	const WWMathCase &mathCase = GetParam();
	ASSERT_NE(mathCase.call, nullptr) << "Dump without a call: " << mathCase.name;

	const std::string computed = mathCase.call();
	ASSERT_NE(mathCase.dump, nullptr) << "Call without a dump: { \"" << mathCase.name << "\", { \"" << computed << "\" } },";

	EXPECT_EQ(computed, mathCase.dump);
}

INSTANTIATE_TEST_SUITE_P(WWMath, WWMathBits, ::testing::ValuesIn(mergeCallsWithDumps()), testNameOf);
