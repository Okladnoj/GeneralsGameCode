/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

#include "regexpr.h"
#include "always.h"
#include "wwstring.h"
#include <assert.h>
#include <regex>

/*
** RegularExpressionClass::DataStruct
*/
struct RegularExpressionClass::DataStruct {
  DataStruct() : IsValid(false) {}
  std::regex CompiledExpr;
  StringClass ExprString;
  bool IsValid;
};

/*
** RegularExpressionClass Implementation
*/

RegularExpressionClass::RegularExpressionClass(const char *expression)
    : Data(new DataStruct) {
  if (expression != 0) {
    Compile(expression);
  }
}

RegularExpressionClass::RegularExpressionClass(
    const RegularExpressionClass &copy)
    : Data(new DataStruct) {
  if (copy.Is_Valid()) {
    Compile(copy.Data->ExprString);
  }
}

RegularExpressionClass::~RegularExpressionClass(void) { delete Data; }

bool RegularExpressionClass::Compile(const char *expression) {
  assert(Data);

  if (expression == 0) {
    Data->IsValid = false;
    return false;
  }

  try {
    Data->CompiledExpr = std::regex(expression, std::regex::extended);
    Data->ExprString = expression;
    Data->IsValid = true;
    return true;
  } catch (...) {
    Data->IsValid = false;
    return false;
  }
}

bool RegularExpressionClass::Is_Valid(void) const {
  assert(Data);
  return Data->IsValid;
}

bool RegularExpressionClass::Match(const char *string) const {
  assert(Data);

  if (!Data->IsValid || string == 0) {
    return false;
  }

  std::cmatch results;
  try {
    if (std::regex_search(string, results, Data->CompiledExpr)) {
      if (results.position() == 0) {
        return true;
      }
    }
  } catch (...) {
  }

  return false;
}

/*
** Operators
*/

RegularExpressionClass &
RegularExpressionClass::operator=(const RegularExpressionClass &rhs) {
  if (this == &rhs) {
    return *this;
  }

  assert(rhs.Data);
  Compile(rhs.Data->ExprString);

  return *this;
}

bool RegularExpressionClass::operator==(
    const RegularExpressionClass &rhs) const {
  if (Is_Valid() != rhs.Is_Valid()) {
    return false;
  }

  if (Is_Valid()) {
    if (Data->ExprString != rhs.Data->ExprString) {
      return false;
    }
  }
  return true;
}

bool RegularExpressionClass::operator!=(
    const RegularExpressionClass &rhs) const {
  return !(*this == rhs);
}
