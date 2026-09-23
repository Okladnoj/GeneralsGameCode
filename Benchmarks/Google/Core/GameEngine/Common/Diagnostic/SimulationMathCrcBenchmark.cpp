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

#include <benchmark/benchmark.h>
#include <float.h>
#include <math.h>

#include "Common/XferCRC.h"
#include "Common/Diagnostic/SimulationMathCrc.h"
#include "GameLogic/FPUControl.h"
#include "WWMath/matrix3d.h"

static void appendSimulationMathCrcWithSystemMath(XferCRC &xfer)
{
	Matrix3D matrix;
	Matrix3D factorsMatrix;

	matrix.Set(
		4.1f, 1.2f, 0.3f, 0.4f,
		0.5f, 3.6f, 0.7f, 0.8f,
		0.9f, 1.0f, 2.1f, 1.2f);

	factorsMatrix.Set(
		(float)(::sin(0.7) * ::log10(2.3)),
		(float)(::cos(1.1) * ::pow(1.1, 2.0)),
		(float)::tan(0.3),
		(float)::asin(0.967302263),
		(float)::acos(0.967302263),
		(float)(::atan(0.967302263) * ::pow(1.1, 2.0)),
		(float)::atan2(0.4, 1.3),
		(float)::sinh(0.2),
		(float)(::cosh(0.4) * ::tanh(0.5)),
		(float)::sqrt(55788.84375),
		(float)(::exp(0.1) * ::log10(2.3)),
		(float)::log(1.4));

	Matrix3D::Multiply(matrix, factorsMatrix, &matrix);
	matrix.Get_Inverse(matrix);

	xfer.xferMatrix3D(&matrix);
}

static UnsignedInt calculateSimulationMathCrcWithSystemMath()
{
	XferCRC xfer;
	xfer.open("SimulationMathCrc");

	setFPMode();

	appendSimulationMathCrcWithSystemMath(xfer);

	_fpreset();

	xfer.close();

	return xfer.getCRC();
}

static void BM_SimulationMathCrc(benchmark::State &state)
{
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(SimulationMathCrc::calculate());
	}
}
BENCHMARK(BM_SimulationMathCrc);

static void BM_SimulationMathCrcWithSystemMath(benchmark::State &state)
{
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(calculateSimulationMathCrcWithSystemMath());
	}
}
BENCHMARK(BM_SimulationMathCrcWithSystemMath);
