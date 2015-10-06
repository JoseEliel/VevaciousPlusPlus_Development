/*
 * BounceAlongPathWithThreshold.cpp
 *
 *  Created on: Jul 1, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "TunnelingCalculation/BounceActionTunneling/BounceAlongPathWithThreshold.hpp"

namespace VevaciousPlusPlus
{

  BounceAlongPathWithThreshold::BounceAlongPathWithThreshold(
                                          PotentialFunction& potentialFunction,
                           std::vector< BouncePathFinder* > const& pathFinders,
                                BounceActionCalculator* const actionCalculator,
                TunnelingCalculator::TunnelingStrategy const tunnelingStrategy,
                                     double const survivalProbabilityThreshold,
                               unsigned int const thermalIntegrationResolution,
                                        unsigned int const temperatureAccuracy,
                                    unsigned int const pathPotentialResolution,
                                      double const vacuumSeparationFraction ) :
    BounceActionTunneler( potentialFunction,
                          tunnelingStrategy,
                          survivalProbabilityThreshold,
                          temperatureAccuracy,
                          vacuumSeparationFraction ),
    pathFinders( pathFinders ),
    actionCalculator( actionCalculator ),
    thermalIntegrationResolution( thermalIntegrationResolution ),
    pathPotentialResolution( pathPotentialResolution )
  {
    // This constructor is just an initialization list.
  }

  BounceAlongPathWithThreshold::~BounceAlongPathWithThreshold()
  {
    delete actionCalculator;

    for( size_t deletionIndex( 0 );
         deletionIndex < pathFinders.size();
         ++deletionIndex )
    {
      delete pathFinders[ deletionIndex ];
    }
  }


  // This sets thermalSurvivalProbability by numerically integrating up to the
  // critical temperature for tunneling to be possible from T = 0 unless the
  // integral already passes a threshold, and sets
  // dominantTemperatureInGigaElectronVolts to be the temperature with the
  // lowest survival probability.
  void BounceAlongPathWithThreshold::ContinueThermalTunneling(
                                           PotentialMinimum const& falseVacuum,
                                           PotentialMinimum const& trueVacuum,
                              double const potentialAtOriginAtZeroTemperature )
  {
    // First we set up the (square of the) threshold distance that we demand
    // between the vacua at every temperature to trust the tunneling
    // calculation.
    double const thresholdSeparationSquared( vacuumSeparationFractionSquared
                                * falseVacuum.SquareDistanceTo( trueVacuum ) );

    // Now we start at almost the critical temperature, and sum up for
    // decreasing temperatures:
    double partialDecayWidth( 0.0 );
    // The partial decay width scaled by the volume of the observable Universe
    // is recorded in partialDecayWidth so that the bounce action threshold for
    // each temperature can be calculated taking into account the contributions
    // from higher temperatures.
    double const temperatureStep( rangeOfMaxTemperatureForOriginToTrue.first
                 / static_cast< double >( thermalIntegrationResolution + 1 ) );
    double currentTemperature( temperatureStep );
    thermalPotentialMinimizer.SetTemperature( currentTemperature );
    PotentialMinimum thermalFalseVacuum( thermalPotentialMinimizer(
                                          falseVacuum.FieldConfiguration() ) );
    // We have to check to see if the field origin should be the thermal false
    // vacuum. The result of thermalPotentialMinimizer already has the value of
    // the potential at the field origin subtracted, so we just compare with
    // zero.
    if( thermalFalseVacuum.PotentialValue() > 0.0 )
    {
      thermalFalseVacuum
      = PotentialMinimum( potentialFunction.FieldValuesOrigin(),
                          0.0 );
    }
    PotentialMinimum thermalTrueVacuum( thermalPotentialMinimizer(
                                           trueVacuum.FieldConfiguration() ) );
    double const thresholdDecayWidth( -log( survivalProbabilityThreshold )
                 / ( temperatureStep * exp( lnOfThermalIntegrationFactor ) ) );
    double actionThreshold( -currentTemperature
      * log( currentTemperature * currentTemperature * thresholdDecayWidth ) );
    double bounceOverTemperature( BoundedBounceAction( thermalFalseVacuum,
                                                       thermalTrueVacuum,
                                                       currentTemperature,
                                                       actionThreshold,
                                                   thresholdSeparationSquared )
                                  / currentTemperature );
    if( bounceOverTemperature < maximumPowerOfNaturalExponent )
    {
      partialDecayWidth += ( exp( -bounceOverTemperature )
                             / ( currentTemperature * currentTemperature ) );
    }

    double smallestExponent( bounceOverTemperature );
    dominantTemperatureInGigaElectronVolts = 0.0;

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "currentTemperature = " << currentTemperature
    << ", bounceOverTemperature = " << bounceOverTemperature
    << ", partialDecayWidth = " << partialDecayWidth
    << ", thresholdPartialWidth = " << thresholdDecayWidth;
    std::cout << std::endl;/**/

    for( size_t whichStep( 1 );
         whichStep < thermalIntegrationResolution;
         ++whichStep )
    {
      if( partialDecayWidth > thresholdDecayWidth )
      {
        // We don't bother calculating the rest of the contributions to the
        // integral of the decay width if it is already large enough that the
        // survival probability is below the threshold.
        break;
      }
      currentTemperature += temperatureStep;
      thermalPotentialMinimizer.SetTemperature( currentTemperature );
      // We update the positions of the thermal vacua based on their positions
      // at the last temperature step.
      thermalFalseVacuum
      = thermalPotentialMinimizer( thermalFalseVacuum.FieldConfiguration() );
      // We have to keep checking to see if the field origin should be the
      // thermal false vacuum. The result of thermalPotentialMinimizer already
      // has the value of the potential at the field origin subtracted, so we
      // just compare with zero.
      if( thermalFalseVacuum.PotentialValue() > 0.0 )
      {
        thermalFalseVacuum
        = PotentialMinimum( potentialFunction.FieldValuesOrigin(),
                            0.0 );
      }
      thermalTrueVacuum
      = thermalPotentialMinimizer( thermalTrueVacuum.FieldConfiguration() );

      if( thermalTrueVacuum.SquareDistanceTo( thermalFalseVacuum )
          < thresholdSeparationSquared )
      {
        // If the thermal vacua have gotten so close that a tunneling
        // calculation is suspect, we break and take only the contributions
        // from lower temperatures.
        break;
      }

      // If the bounce action from the next step is sufficiently small, then
      // the contribution to survivalExponent could make survivalExponent >
      // thresholdDecayWidth, which would mean that the survival probability is
      // definitely lower than survivalProbabilityThreshold.
      actionThreshold = ( -currentTemperature
                          * log( currentTemperature * currentTemperature
                             * ( thresholdDecayWidth - partialDecayWidth ) ) );
      bounceOverTemperature = ( BoundedBounceAction( thermalFalseVacuum,
                                                     thermalTrueVacuum,
                                                     currentTemperature,
                                                     actionThreshold,
                                                   thresholdSeparationSquared )
                                / currentTemperature );

      if( bounceOverTemperature < maximumPowerOfNaturalExponent )
      {
        partialDecayWidth += ( exp( -bounceOverTemperature )
                               / ( currentTemperature * currentTemperature ) );
      }
      if( bounceOverTemperature < smallestExponent )
      {
        smallestExponent = bounceOverTemperature;
        dominantTemperatureInGigaElectronVolts = currentTemperature;
      }
    }
    if( partialDecayWidth > 0.0 )
    {
      logOfMinusLogOfThermalProbability = ( lnOfThermalIntegrationFactor
                                + log( partialDecayWidth * temperatureStep ) );
    }
    else
    {
      logOfMinusLogOfThermalProbability
      = -exp( maximumPowerOfNaturalExponent );
      std::cout
      << std::endl
      << "Warning! The calculated integrated thermal decay width was so close"
      << " to 0 that taking its logarithm would be problematic, so setting the"
      << " logarithm of the negative of the logarithm of the thermal survival"
      << " probability to " << logOfMinusLogOfThermalProbability << ".";
      std::cout << std::endl;
    }
    SetThermalSurvivalProbability();
  }

  // This returns either the dimensionless bounce action integrated over four
  // dimensions (for zero temperature) or the dimensionful bounce action
  // integrated over three dimensions (for non-zero temperature) for tunneling
  // from falseVacuum to trueVacuum at temperature tunnelingTemperature, or an
  // upper bound if the upper bound drops below actionThreshold during the
  // course of the calculation. The vacua are assumed to already be the minima
  // at tunnelingTemperature.
  double BounceAlongPathWithThreshold::BoundedBounceAction(
                                           PotentialMinimum const& falseVacuum,
                                            PotentialMinimum const& trueVacuum,
                                             double const tunnelingTemperature,
                                                  double const actionThreshold,
                                 double const requiredVacuumSeparationSquared )
  {
    std::vector< std::vector< double > > straightPath( 2,
                                            falseVacuum.FieldConfiguration() );
    straightPath.back() = trueVacuum.FieldConfiguration();
    TunnelPath const* bestPath( new LinearSplineThroughNodes( straightPath,
                                                    std::vector< double >( 0 ),
                                                      tunnelingTemperature ) );

    actionCalculator->ResetVacua( falseVacuum,
                                  trueVacuum,
                                  tunnelingTemperature );


    SplinePotential pathPotential( potentialFunction,
                                   *bestPath,
                                   pathPotentialResolution,
                                   requiredVacuumSeparationSquared );


    BubbleProfile const* bestBubble( (*actionCalculator)( *bestPath,
                                                          pathPotential ) );

    std::cout << std::endl
    << "Initial path bounce action = " << bestBubble->bounceAction;
    if( bestPath->NonZeroTemperature() )
    {
      std::cout << " GeV";
    }
    std::cout << ", threshold is " << actionThreshold;
    if( bestPath->NonZeroTemperature() )
    {
      std::cout << " GeV";
    }
    std::cout << ".";
    std::cout << std::endl;

    // debugging:
    /**/std::string straightPathPicture( "StraightBubbleProfile.eps" );
    std::cout << std::endl << "debugging:"
    << std::endl
    << "Initial straight path being plotted in " << straightPathPicture << ".";
    std::cout << std::endl;
    std::vector< std::string > fieldColors;
    fieldColors.push_back( "red" );
    fieldColors.push_back( "brown" );
    fieldColors.push_back( "blue" );
    fieldColors.push_back( "purple" );
    fieldColors.push_back( "green" );
    fieldColors.push_back( "cyan" );
    actionCalculator->PlotBounceConfiguration( *bestPath,
                                               *bestBubble,
                                               fieldColors,
                                               straightPathPicture,
                                               pathPotentialResolution );/**/

    for( std::vector< BouncePathFinder* >::iterator
         pathFinder( pathFinders.begin() );
         pathFinder < pathFinders.end();
         ++pathFinder )
    {
      std::cout << std::endl
      << "Passing best path so far to next path finder.";
      std::cout << std::endl;

      (*pathFinder)->SetVacuaAndTemperature( falseVacuum,
                                             trueVacuum,
                                             tunnelingTemperature );
      TunnelPath const* currentPath( bestPath );
      BubbleProfile const* currentBubble( bestBubble );

      // The paths produced in sequence by pathFinder are kept separate from
      // bestPath to give more freedom to pathFinder internally (though I
      // cannot right now think of any way in which it would actually be
      // useful, apart from maybe some crazy MCMC which might try to get out of
      // a local minimum, which wouldn't work if it was sent back to the local
      // minimum at each step).

      // Keeping track of a best path and bubble separately from the last-used
      // path and bubble without copying any instances requires a bit of
      // book-keeping. Each iteration of the loop below will produce new
      // instances of a path and a bubble, and either the new path and bubble
      // need to be deleted or the previous best need to be deleted. These two
      // pointers keep track of which is to be deleted. They start as NULL,
      // allowing the first iteration to delete them before any path has been
      // marked for deletion.
      TunnelPath const* pathDeleter( NULL );
      BubbleProfile const* bubbleDeleter( NULL );

      // This loop will get a path from pathFinder and then repeat if
      // pathFinder decides that the path can be improved once the bubble
      // profile is obtained, as long as the bounce action has not dropped
      // below the threshold.
      do
      {
        // The nextPath and nextBubble pointers are not strictly necessary,
        // but they make the logic of the code clearer and will probably be
        // optimized away by the compiler anyway.
        TunnelPath const*
        nextPath( (*pathFinder)->TryToImprovePath( *currentPath,
                                                   *currentBubble ) );

        // debugging:
        /**/std::cout << std::endl << "debugging:"
        << std::endl
        << "nextPath:" << std::endl
        << nextPath->AsDebuggingString() << std::endl;
        SplinePotential potentialApproximation( potentialFunction,
                                                *nextPath,
                                                pathPotentialResolution,
                                             requiredVacuumSeparationSquared );
        std::cout << "potentialApproximation:" << std::endl
        << potentialApproximation.AsDebuggingString();
        std::cout << std::endl;/**/

        BubbleProfile const* nextBubble( (*actionCalculator)( *nextPath,
                                                    potentialApproximation ) );

        // On the first iteration of the loop, these pointers are NULL, so
        // it's no problem to delete them. On subsequent iterations, they point
        // at the higher-action path and bubble from the last iterations
        // comparison between its nextPath and bestPath.
        delete bubbleDeleter;
        delete pathDeleter;

        if( nextBubble->bounceAction < bestBubble->bounceAction )
        {
          // If nextBubble was an improvement on bestBubble, what bestPath
          // currently points at gets marked for deletion either on the next
          // iteration of this loop or just after the loop, and then bestPath
          // is set to point at nextPath, with the corresponding operations for
          // the bubble pointers.
          bubbleDeleter = bestBubble;
          bestBubble = nextBubble;
          pathDeleter = bestPath;
          bestPath = nextPath;
        }
        else
        {
          // If nextBubble wasn't an improvement on bestBubble, it and nextPath
          // will be deleted after being used to generate the nextPath and
          // nextBubble of the next iteration of the loop (or after the loop if
          // this ends up being the last iteration) through these pointers.
          bubbleDeleter = nextBubble;
          pathDeleter = nextPath;
        }
        currentBubble = nextBubble;
        currentPath = nextPath;

        std::cout << std::endl
        << "Improved path bounce action = " << currentBubble->bounceAction;
        if( currentPath->NonZeroTemperature() )
        {
          std::cout << " GeV";
        }
        std::cout << ", lowest bounce action so far = "
        << bestBubble->bounceAction;
        if( currentPath->NonZeroTemperature() )
        {
          std::cout << " GeV";
        }
        std::cout << ", threshold is " << actionThreshold;
        if( currentPath->NonZeroTemperature() )
        {
          std::cout << " GeV";
        }
        std::cout << ".";
        std::cout << std::endl;

        // debugging:
        /**/std::string nextPathPicture( "NextBubbleProfile.eps" );
        std::cout << std::endl << "debugging:"
        << std::endl
        << "nextPath being plotted in " << nextPathPicture << ".";
        std::cout << std::endl;
        std::vector< std::string > fieldColors;
        fieldColors.push_back( "red" );
        fieldColors.push_back( "brown" );
        fieldColors.push_back( "blue" );
        fieldColors.push_back( "purple" );
        fieldColors.push_back( "green" );
        fieldColors.push_back( "cyan" );
        actionCalculator->PlotBounceConfiguration( *nextPath,
                                                   *nextBubble,
                                                   fieldColors,
                                                   nextPathPicture,
                                                   pathPotentialResolution );
        std::cout << std::endl;
        std::cout << "Dummy input for pause.";
        std::cout << std::endl;
        std::cin >> nextPathPicture;
        std::cout << std::endl;
        std::cout << "Resuming.";
        std::cout << std::endl;/**/

      } while( ( bestBubble->bounceAction > actionThreshold )
               &&
               (*pathFinder)->PathCanBeImproved( *currentBubble ) );
      // At the end of the loop, these point at the last tried path and bubble
      // which did not end up as the best ones, so deleting them is no problem.
      delete bubbleDeleter;
      delete pathDeleter;

      // We don't bother with the rest of the path finders if the action has
      // already dropped below the threshold.
      if( bestBubble->bounceAction < actionThreshold )
      {
        std::cout
        << std::endl
        << "Bounce action dropped below threshold, breaking off from looking"
        << " for further path improvements.";
        std::cout << std::endl;

        break;
      }
    }
    // debugging:
    /**/std::string finalPathPicture( "FinalBubbleProfile.eps" );
    std::cout << std::endl << "debugging:"
    << std::endl
    << "Final deformed path being plotted in " << finalPathPicture << ".";
    std::cout << std::endl;
    actionCalculator->PlotBounceConfiguration( *bestPath,
                                               *bestBubble,
                                               fieldColors,
                                               finalPathPicture,
                                               pathPotentialResolution );/**/

    std::cout << std::endl
    << "Lowest path bounce action at " << tunnelingTemperature << " GeV was "
    << bestBubble->bounceAction;
    if( bestPath->NonZeroTemperature() )
    {
      std::cout << " GeV";
    }
    std::cout << ", threshold is " << actionThreshold;
    if( bestPath->NonZeroTemperature() )
    {
      std::cout << " GeV";
    }
    std::cout << ".";
    std::cout << std::endl;

    double const bounceAction( bestBubble->bounceAction );
    delete bestBubble;
    delete bestPath;
    return bounceAction;
  }

} /* namespace VevaciousPlusPlus */