// -*- coding: utf-8 -*-
// Copyright (C) 2013 Cuong Pham <cuong.pham@normalesup.org>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#include "ParabolicPathSmooth/ParabolicRamp.h"
#include "ParabolicPathSmooth/DynamicPath.h"

namespace ParabolicRamp = ParabolicRampInternal;

namespace mergewaypoints
{

/** Iteratively merge all ramps that are shorter than minswitchtime. Determine the optimal time duration that allows to do so
    \param origramps input ramps
    \param ramps result ramps
    \param upperbound maximum time duration to try
    \param precision precision in the dichotomy search for the best timescaling coef
    \param iters max number of random iterations
 */
bool IterativeMergeRamps(const std::list<ParabolicRamp::ParabolicRampND>& origramps,std::list<ParabolicRamp::ParabolicRampND>& resramps, ConstraintTrajectoryTimingParametersPtr params, dReal upperbound, bool checkcontrollertime, SpaceSamplerBasePtr uniformsampler, ParabolicRamp::RampFeasibilityChecker& check, bool docheck);

/** Same as IterativeMergeRamps but run a straightforward line search on the trajectory duration instead of dichotomy search
**/
bool IterativeMergeRampsNoDichotomy(const std::list<ParabolicRamp::ParabolicRampND>& origramps,std::list<ParabolicRamp::ParabolicRampND>& resramps, ConstraintTrajectoryTimingParametersPtr params, dReal upperbound, dReal stepsize, bool checkcontrollertime, SpaceSamplerBasePtr uniformsampler, ParabolicRamp::RampFeasibilityChecker& check, bool docheck);

/** If the beginning or the end of the ramps are linear segments then treat them separately
**/
bool FixRampsEnds(std::list<ParabolicRamp::ParabolicRampND>& origramps,std::list<ParabolicRamp::ParabolicRampND>& resramps, ConstraintTrajectoryTimingParametersPtr params, ParabolicRamp::RampFeasibilityChecker& check);

/** Compute a straight ramp with good time duration
**/
bool ComputeStraightRamp(ParabolicRamp::ParabolicRampND& newramp,const ParabolicRamp::Vector x0, const ParabolicRamp::Vector x1, ConstraintTrajectoryTimingParametersPtr params,ParabolicRamp::RampFeasibilityChecker& check);


/** Timescale a ramp. Assume the ramp is unitary.
    \param origramps input ramp
    \param resramps result ramp
    \param coef timescaling coefficient
 */
void ScaleRampsTime(const std::list<ParabolicRamp::ParabolicRampND>& origramps,std::list<ParabolicRamp::ParabolicRampND>& ramps,dReal coef,bool trysmart, ConstraintTrajectoryTimingParametersPtr params);

/** Determine the minimum switchtime in a ramp
    \param rampnd input ramp
 */
dReal DetermineMinswitchtime(const ParabolicRamp::ParabolicRampND& rampnd);
dReal DetermineMinswitchtimeRamps(const std::list<ParabolicRamp::ParabolicRampND>& ramps);

/** Compute time duration of ramps
    \param rampnd input ramp
 */
dReal ComputeRampsDuration(const std::list<ParabolicRamp::ParabolicRampND>& ramps);


/** Count the number of pieces in a ramp
    \param rampnd input ramp
 */
size_t CountUnitaryRamps(const ParabolicRamp::ParabolicRampND& rampnd);

void PrintRamps(const std::list<ParabolicRamp::ParabolicRampND>& ramps,ConstraintTrajectoryTimingParametersPtr params,bool warning);


void BreakIntoUnitaryRamps(const std::list<ParabolicRamp::ParabolicRampND>& ramps,std::list<ParabolicRamp::ParabolicRampND>& resramps);
} // end namespace mergewaypoints
