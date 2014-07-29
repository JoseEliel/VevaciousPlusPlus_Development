/*
 * GradientFromStartingPoints.hpp
 *
 *  Created on: Jun 30, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef GRADIENTFROMSTARTINGPOINTS_HPP_
#define GRADIENTFROMSTARTINGPOINTS_HPP_

#include "PotentialMinimizer.hpp"
#include "StartingPointFinder.hpp"
#include "GradientMinimizer.hpp"
#include "PotentialEvaluation/PotentialFunction.hpp"
#include "SlhaManagement/SlhaManager.hpp"
#include "HomotopyContinuation/Hom4ps2Runner.hpp"
#include "GradientBasedMinimization/MinuitPotentialMinimizer.hpp"

namespace VevaciousPlusPlus
{
  // This class takes memory management ownership of the components given to
  // the constructor as pointers! It'd be nice to use std::unique_ptrs, but we
  // are stubbornly sticking to allowing non-C++11-compliant compilers.
  class GradientFromStartingPoints : public PotentialMinimizer
  {
  public:
    GradientFromStartingPoints( PotentialFunction const& potentialFunction,
                                StartingPointFinder* const startingPointFinder,
                                GradientMinimizer* const gradientMinimizer,
                              double const extremumSeparationThresholdFraction,
                                double const nonDsbRollingToDsbScalingFactor );
    virtual ~GradientFromStartingPoints();


    // This uses startingPointFinder to find the starting points for
    // gradientMinimizer to minimize potentialFunction, evaluated at a
    // temperature given by minimizationTemperature, and records the found
    // minima in foundMinima. It also sets dsbVacuum (using
    // polynomialPotential.DsbFieldValues() as a starting point for
    // gradientMinimizer), and records the minima lower than dsbVacuum in
    // panicVacua, and of those, it sets panicVacuum to be the minimum in
    // panicVacua closest to dsbVacuum.
    virtual void FindMinima( double const minimizationTemperature = 0.0 );

    // This uses gradientMinimizer to find the minimum at temperature
    // minimizationTemperature nearest to minimumToAdjust (which is assumed to
    // be a minimum of the potential at a different temperature).
    virtual PotentialMinimum
    AdjustMinimumForTemperature( PotentialMinimum const& minimumToAdjust,
                                 double const minimizationTemperature );


  protected:
    StartingPointFinder* startingPointFinder;
    GradientMinimizer* gradientMinimizer;
    std::vector< std::vector< double > > startingPoints;
    double extremumSeparationThresholdFraction;
    double nonDsbRollingToDsbScalingFactor;
  };




  // This uses gradientMinimizer to find the minimum at temperature
  // minimizationTemperature nearest to minimumToAdjust (which is assumed to
  // be a minimum of the potential at a different temperature).
  inline PotentialMinimum
  GradientFromStartingPoints::AdjustMinimumForTemperature(
                                       PotentialMinimum const& minimumToAdjust,
                                         double const minimizationTemperature )
  {
    gradientMinimizer->SetTemperature( minimizationTemperature );
    return (*gradientMinimizer)( minimumToAdjust.FieldConfiguration() );
  }

} /* namespace VevaciousPlusPlus */
#endif /* GRADIENTFROMSTARTINGPOINTS_HPP_ */
