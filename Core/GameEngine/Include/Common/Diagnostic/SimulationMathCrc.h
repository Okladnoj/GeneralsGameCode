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

// Flags for diagnostic math benchmarks
#define RUN_MATH_BENCHMARK_REPLAY400_FLAG (0)

class SimulationMathCrc
{
public:
	static UnsignedInt calculate();
	static UnsignedInt calculateDouble();
	static UnsignedInt calculateDoubleNoDownCast();
	static void writeParityLog(const char *path);
	// Walks the complete 2^32 float domain of every unary _Legacy wrapper (Atan2 by axis sweeps
	// plus a fixed-seed sample), under both x87 precision modes. Runs for a long time; it is a
	// certification pass, not a smoke test.
	static void certifyLegacyExhaustive(const char *path);
	static void runBenchmark(int iterations = 10000);
};
