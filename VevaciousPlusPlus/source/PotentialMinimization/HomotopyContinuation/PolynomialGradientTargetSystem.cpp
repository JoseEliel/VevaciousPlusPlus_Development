/*
 * PolynomialGradientTargetSystem.cpp
 *
 *  Created on: Apr 4, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "PotentialMinimization/HomotopyContinuation/PolynomialGradientTargetSystem.hpp"

namespace VevaciousPlusPlus
{

  PolynomialGradientTargetSystem::PolynomialGradientTargetSystem(
                                      PolynomialSum const& potentialPolynomial,
                                                   size_t const numberOfFields,
                                      ParameterUpdatePropagator& previousPropagator,
                            std::vector< size_t > const& fieldsAssumedPositive,
                            std::vector< size_t > const& fieldsAssumedNegative,
                                bool const treeLevelMinimaOnlyAsValidSolutions,
                            double const assumedPositiveOrNegativeTolerance ) :
    HomotopyContinuationTargetSystem( previousPropagator ),
    potentialPolynomial( potentialPolynomial ),
    numberOfVariables( numberOfFields ),
    numberOfFields( numberOfFields ),
    targetSystem(),
    highestDegreeOfTargetSystem( 0 ),
    minimumScale( -2.0 ),
    maximumScale( -2.0 ),
    startSystem(),
    startValues(),
    validSolutions(),
    targetHessian(),
    startHessian(),
    fieldsAssumedPositive( fieldsAssumedPositive ),
    fieldsAssumedNegative( fieldsAssumedNegative ),
    treeLevelMinimaOnlyAsValidSolutions( treeLevelMinimaOnlyAsValidSolutions ),
    assumedPositiveOrNegativeTolerance( assumedPositiveOrNegativeTolerance )
  {
    // This constructor is just an initialization list.
  }

  PolynomialGradientTargetSystem::~PolynomialGradientTargetSystem()
  {
    // This does nothing.
  }


  // This fills targetHessian from targetSystem.
  void PolynomialGradientTargetSystem::PreparePolynomialHessian()
  {
    targetHessian.resize( numberOfVariables,
                          std::vector< PolynomialSum >( numberOfVariables ) );
    for( size_t gradientIndex( 0 );
         gradientIndex < numberOfVariables;
         ++gradientIndex )
    {
      std::vector< PolynomialTerm > const& gradientVector(
                             targetSystem[ gradientIndex ].PolynomialTerms() );
      for( size_t fieldIndex( 0 );
           fieldIndex < numberOfVariables;
           ++fieldIndex )
      {
        for( std::vector< PolynomialTerm >::const_iterator
             whichTerm( gradientVector.begin() );
             whichTerm < gradientVector.end();
             ++whichTerm )
        {
          if( whichTerm->NonZeroDerivative( fieldIndex ) )
          {
            targetHessian[ gradientIndex ][ fieldIndex ].PolynomialTerms(
                     ).push_back( whichTerm->PartialDerivative( fieldIndex ) );
          }
        }
      }
    }
  }

  // This sets startSystem[ variableIndex ] to be a polynomial of degree
  // highestDegreeOfTargetSystem with solutions given by
  // startValues[ variableIndex ] which is also set.
  // After that, startHessian[ variableIndex ] is also set accordingly.
  // It sets startSystem[ variableIndex ] to be of the form
  // x * ( x^2 - y_0^2 ) * ( x^2 - y_1^2 ) * ...
  // where x is the variable with index variableIndex and y_n is
  // startValues[ variableIndex ][ n ].
  // If highestDegreeOfTargetSystem is even,
  // startValues[ variableIndex ][ n-1 ] is set to n * q_n
  // for n going from 1 to d, where
  // d = highestDegreeOfTargetSystem / 2, and q_n is
  // lowerEndOfStartValues if d = 1 or
  // (lowerEndOfStartValues^([d-n]/[d-1])
  //  * upperEndOfStartValues^([n-1]/[d-1])) if d > 1.
  // If highestDegreeOfTargetSystem is odd, startValues[ variableIndex ][ n ]
  // is set by the above formula and startValues[ variableIndex ][ 0 ] is set
  // to 0.
  // So, if the highest degree polynomial of targetSystem is 3, d = 1, so the
  // starting values are set to
  // { 0, lowerEndOfStartValues }.
  // If highestDegreeOfTargetSystem is 4, d = 2, so:
  // { lowerEndOfStartValues, upperEndOfStartValues }.
  // If highestDegreeOfTargetSystem is 5, d = 2, so:
  // { 0, lowerEndOfStartValues, upperEndOfStartValues }.
  // If highestDegreeOfTargetSystem is 7, d = 3, so:
  // { 0, lowerEndOfStartValues,
  // ( lowerEndOfStartValues^(1/2) * upperEndOfStartValues^(1/2) ),
  // upperEndOfStartValues }.
  // If highestDegreeOfTargetSystem is 9, d = 4, so:
  // { 0, lowerEndOfStartValues,
  // ( lowerEndOfStartValues^(2/3) * upperEndOfStartValues^(1/3) ),
  // ( lowerEndOfStartValues^(1/3) * upperEndOfStartValues^(2/3) ),
  // upperEndOfStartValues }.
  void
  PolynomialGradientTargetSystem::SetStartSystem( size_t const variableIndex,
                                            double const lowerEndOfStartValues,
                                           double const upperEndOfStartValues )
  {
    startValues[ variableIndex ].clear();
    startSystem[ variableIndex ] = ProductOfPolynomialSums();
    size_t numberOfNonZeroStartValues( highestDegreeOfTargetSystem / 2 );
    std::complex< double > startValue( 0.0,
                                       0.0 );
    if( ( highestDegreeOfTargetSystem % 2 ) == 1 )
    {
      startValues[ variableIndex ].push_back( startValue );
      PolynomialTerm fieldTerm;
      fieldTerm.RaiseFieldPower( variableIndex,
                                 1 );
      PolynomialSum constructedSum;
      constructedSum.PolynomialTerms().push_back( fieldTerm );
      startSystem[ variableIndex ].MultiplyBy( constructedSum );
    }
    if( numberOfNonZeroStartValues == 1 )
    {
      startValues[ variableIndex ].push_back( lowerEndOfStartValues );
      startSystem[ variableIndex ].MultiplyBy( variableIndex,
                                               2,
                                               lowerEndOfStartValues );
    }
    else if( numberOfNonZeroStartValues > 1 )
    {
      double upperPower( 0.0 );
      double const powerDivisor( 1.0
                   / static_cast< double >( numberOfNonZeroStartValues - 1 ) );
      for( size_t valueIndex( 0 );
           valueIndex < numberOfNonZeroStartValues;
           ++valueIndex )
      {
        upperPower = ( valueIndex * powerDivisor );
        startValue.real() = ( pow( lowerEndOfStartValues,
                                   ( 1.0 - upperPower ) )
                              * pow( upperEndOfStartValues,
                                     upperPower ) );
        startValues[ variableIndex ].push_back( startValue );
        startSystem[ variableIndex ].MultiplyBy( variableIndex,
                                                 2,
                                                 startValue.real() );
      }
    }
    SetStartHessian( variableIndex );
  }

  // This vetoes a homotopy continuation solution if any of the fields with
  // index in fieldsAssumedPositive are negative (allowing for a small amount
  // of numerical jitter given by equationTolerance) or if any of the fields
  // with index in fieldsAssumedNegitive are positive (also allowing for a
  // small amount of numerical jitter), or if
  // treeLevelMinimaOnlyAsValidSolutions is true and the solution does not
  // correspond to a minimum (rather than just an extremum) of
  // potentialPolynomial.
  bool PolynomialGradientTargetSystem::AllowedSolution(
                     std::vector< double > const& solutionConfiguration ) const
  {
    for( std::vector< size_t >::const_iterator
         positiveIndex( fieldsAssumedPositive.begin() );
         positiveIndex < fieldsAssumedPositive.end();
         ++positiveIndex )
    {
      if( solutionConfiguration[ *positiveIndex ]
          < -assumedPositiveOrNegativeTolerance )
      {
        return false;
      }
    }
    for( std::vector< size_t >::const_iterator
         negativeIndex( fieldsAssumedNegative.begin() );
         negativeIndex < fieldsAssumedNegative.end();
         ++negativeIndex )
    {
      if( solutionConfiguration[ *negativeIndex ]
          > assumedPositiveOrNegativeTolerance )
      {
        return false;
      }
    }

    // We only check for a negative eigenvalue if skipping was requested.
    if( treeLevelMinimaOnlyAsValidSolutions )
    {
      // We need to check to see if targetHessian has any negative
      // eigenvalues for solutionConfiguration.
      Eigen::SelfAdjointEigenSolver< Eigen::MatrixXd >
      eigenvalueFinder( FieldHessianOfPotential( solutionConfiguration ),
                        Eigen::EigenvaluesOnly );
      // The eigenvalues are sorted in ascending order according to the Eigen
      // documentation, so it suffices to only check the 1st value.
      if( eigenvalueFinder.eigenvalues()( 0 ) < 0.0 )
      {
        return false;
      }
    }
    // If we get here, either we were not skipping in the 1st place, or none of
    // the eigenvalues were negative.
    return true;
  }

  // This fills startHessian[ variableIndex ] from
  // startValues[ variableIndex ].
  void
  PolynomialGradientTargetSystem::SetStartHessian( size_t const variableIndex )
  {
    startHessian[ variableIndex ]
    = std::vector< SumOfProductOfPolynomialSums >( numberOfVariables );
    size_t const indexOfFirstNonZero( highestDegreeOfTargetSystem % 2 );
    size_t const
    numberOfNonZeroStartValues( startValues[ variableIndex ].size() );
    PolynomialTerm fieldTerm;
    fieldTerm.RaiseFieldPower( variableIndex,
                               1 );
    if( indexOfFirstNonZero == 1 )
    {
      fieldTerm.RaiseFieldPower( variableIndex,
                                 1 );
      ProductOfPolynomialSums constructedProductOfSums;
      for( size_t valueIndex( indexOfFirstNonZero );
           valueIndex < numberOfNonZeroStartValues;
           ++valueIndex )
      {
        constructedProductOfSums.MultiplyBy( variableIndex,
                                             2,
                           startValues[ variableIndex ][ valueIndex ].real() );
      }
      // The Hessian of the start system is diagonal!
      startHessian[ variableIndex ][ variableIndex ].AddToSum(
                                                    constructedProductOfSums );
    }
    fieldTerm.MultiplyBy( 2.0 );
    // Now we multiply ( x^2 - y_0^2 ) * ( x^2 - y_1^2 ) * ... by 2x^2 or 2x^1
    // appropriately.
    PolynomialSum zeroConstantQuadratic;
    zeroConstantQuadratic.PolynomialTerms().push_back( fieldTerm );
    for( size_t differentiateIndex( indexOfFirstNonZero );
         differentiateIndex < numberOfNonZeroStartValues;
         ++differentiateIndex )
    {
      ProductOfPolynomialSums constructedProductOfSums;
      constructedProductOfSums.MultiplyBy( zeroConstantQuadratic );
      for( size_t valueIndex( indexOfFirstNonZero );
           valueIndex < numberOfNonZeroStartValues;
           ++valueIndex )
      {
        if( valueIndex != differentiateIndex )
        {
          constructedProductOfSums.MultiplyBy( variableIndex,
                                               2,
                           startValues[ variableIndex ][ valueIndex ].real() );
        }
      }
      // The Hessian of the start system is diagonal!
      startHessian[ variableIndex ][ variableIndex ].AddToSum(
                                                    constructedProductOfSums );
    }
  }

} /* namespace VevaciousPlusPlus */
