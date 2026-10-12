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
#include <set>
#include <string>
#include <vector>

#include "WWMathCalls.h"
#include "WWMathDumps.h"
#include "WWMathTestCase.h"

template <typename Table>
static std::set<std::string> findDuplicateKeys(const Table &table)
{
	std::set<std::string> seenKeys;
	std::set<std::string> duplicateKeys;
	for (const auto &entry : table)
	{
		if (!seenKeys.insert(entry.key).second)
		{
			duplicateKeys.insert(entry.key);
		}
	}

	return duplicateKeys;
}

static const char *findDump(const std::string &key)
{
	if (wwmathColumn == WWMATH_COLUMN_NONE)
	{
		return nullptr;
	}

	for (const WWMathDumpEntry &entry : wwmathDumps)
	{
		if (entry.key == key)
		{
			return entry.dump[wwmathColumn];
		}
	}

	return nullptr;
}

static std::vector<WWMathCase> mergeCallsWithDumps()
{
	std::vector<WWMathCase> cases;
	std::set<std::string> mergedKeys;
	for (const WWMathCallEntry &entry : wwmathCalls)
	{
		if (mergedKeys.insert(entry.key).second)
		{
			cases.push_back({ entry.key, entry.call, findDump(entry.key) });
		}
	}

	for (const WWMathDumpEntry &entry : wwmathDumps)
	{
		if (mergedKeys.insert(entry.key).second)
		{
			cases.push_back({ entry.key, nullptr, findDump(entry.key) });
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

TEST(WWMathTables, KeysAreUnique)
{
	for (const std::string &key : findDuplicateKeys(wwmathCalls))
	{
		ADD_FAILURE() << "Duplicate key in wwmathCalls: " << key;
	}

	for (const std::string &key : findDuplicateKeys(wwmathDumps))
	{
		ADD_FAILURE() << "Duplicate key in wwmathDumps: " << key;
	}
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
