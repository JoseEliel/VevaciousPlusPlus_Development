/*
 * FixedScaleOneLoopPotential.cpp
 *
 *  Created on: Mar 13, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "../../include/VevaciousPlusPlus.hpp"

namespace VevaciousPlusPlus
{

  FixedScaleOneLoopPotential::FixedScaleOneLoopPotential(
                                              std::string const& modelFilename,
                           RunningParameterManager& runningParameterManager ) :
    PotentialFromPolynomialAndMasses( modelFilename,
                                      runningParameterManager ),
    inverseRenormalizationScaleSquared( NAN )
  {
    // This constructor is just an initialization list.
  }

  FixedScaleOneLoopPotential::FixedScaleOneLoopPotential(
         PotentialFromPolynomialAndMasses& potentialFromPolynomialAndMasses ) :
    PotentialFromPolynomialAndMasses( potentialFromPolynomialAndMasses ),
    inverseRenormalizationScaleSquared( NAN )
  {
    // placeholder:
    /**/std::cout << std::endl
    << "Placeholder: "
    << "FixedScaleOneLoopPotential::FixedScaleOneLoopPotential( [copy] )";
    std::cout << std::endl;/**/
  }

  FixedScaleOneLoopPotential::~FixedScaleOneLoopPotential()
  {
    // placeholder:
    /**/std::cout << std::endl
    << "Placeholder: "
    << "FixedScaleOneLoopPotential::~FixedScaleOneLoopPotential()";
    std::cout << std::endl;/**/
  }


  // This returns the energy density in GeV^4 of the potential for a state
  // strongly peaked around expectation values (in GeV) for the fields given
  // by the values of fieldConfiguration and temperature in GeV given by
  // temperatureValue.
  double FixedScaleOneLoopPotential::operator()(
                               std::vector< double > const& fieldConfiguration,
                                          double const temperatureValue ) const
  {
    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << "FixedScaleOneLoopPotential::operator()( {";
    for( std::vector< double >::const_iterator
         fieldValue( fieldConfiguration.begin() );
         fieldValue < fieldConfiguration.end();
         ++fieldValue )
    {
      std::cout << "  " << *fieldValue;
    }
    std::cout << "  }, " << temperatureValue
    << " ) called. treeLevelPotential( fieldConfiguration ) = "
    << treeLevelPotential( fieldConfiguration );
    std::cout << std::endl;*/

    std::vector< DoubleVectorWithDouble > scalarMassesSquaredWithFactors;

    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << " scalarSquareMasses";
    std::cout << std::endl;*/
    AddMassesSquaredWithMultiplicity( fieldConfiguration,
                                      scalarSquareMasses,
                                      scalarMassesSquaredWithFactors );
    std::vector< DoubleVectorWithDouble > fermionMassesSquaredWithFactors;

    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << "FixedScaleOneLoopPotential::operator():"
    << " fermionMasses";
    std::cout << std::endl;*/
    AddMassesSquaredWithMultiplicity( fieldConfiguration,
                                      fermionSquareMasses,
                                      fermionMassesSquaredWithFactors );

    // debugging:
    /*std::cout << std::endl << "debugging:"
    << std::endl
    << "FixedScaleOneLoopPotential::operator():"
    << " vectorSquareMasses";
    std::cout << std::endl;*/
    std::vector< DoubleVectorWithDouble > vectorMassesSquaredWithFactors;
    AddMassesSquaredWithMultiplicity( fieldConfiguration,
                                      vectorSquareMasses,
                                      vectorMassesSquaredWithFactors );
    return ( treeLevelPotential( fieldConfiguration )
             + polynomialLoopCorrections( fieldConfiguration )
             + LoopAndThermalCorrections( fieldConfiguration,
                                          scalarMassesSquaredWithFactors,
                                          fermionMassesSquaredWithFactors,
                                          vectorMassesSquaredWithFactors,
                                          inverseRenormalizationScaleSquared,
                                          temperatureValue ) );
  }

  // This sets dsbFieldValueInputs based on the SLHA file just read in.
  void FixedScaleOneLoopPotential::EvaluateDsbInputAndSetScale()
  {
    currentMaximumRenormalizationScale = runningParameters.HighestBlockScale();
    currentMinimumRenormalizationScale = runningParameters.LowestBlockScale();
    if( currentMinimumRenormalizationScale < modelMinimumRenormalizationScale )
    {
      currentMinimumRenormalizationScale = modelMinimumRenormalizationScale;
    }
    squareOfMinimumRenormalizationScale = ( currentMinimumRenormalizationScale
                                        * currentMinimumRenormalizationScale );
    inverseRenormalizationScaleSquared
    = ( 1.0 / squareOfMinimumRenormalizationScale );
    runningParameters.UpdateRunningParameters(
                                          currentMinimumRenormalizationScale );
    std::vector< double > fieldOrigin( numberOfFields,
                                       0.0 );
    for( unsigned int fieldIndex( 0 );
         fieldIndex < numberOfFields;
         ++fieldIndex )
    {
      dsbFieldValueInputs[ fieldIndex ]
      = dsbFieldValuePolynomials[ fieldIndex ]( fieldOrigin );
    }

    // debugging:
    /**/std::cout << std::endl << "debugging:"
    << std::endl
    << "FixedScaleOneLoopPotential::EvaluateDsbInputAndSetScale() set"
    << " currentMaximumRenormalizationScale to "
    << currentMaximumRenormalizationScale
    << ", currentMinimumRenormalizationScale to "
    << currentMinimumRenormalizationScale
    << ", squareOfMinimumRenormalizationScale to "
    << squareOfMinimumRenormalizationScale
    << ", inverseRenormalizationScaleSquared to "
    << inverseRenormalizationScaleSquared
    << ", and DSB field values to:" << std::endl;
    for( unsigned int fieldIndex( 0 );
         fieldIndex < numberOfFields;
         ++fieldIndex )
    {
      std::cout << fieldNames[ fieldIndex ] << " -> "
      << dsbFieldValueInputs[ fieldIndex ] << std::endl;
    }
    std::cout << std::endl;/**/
  }

  // This should prepare homotopyContinuationStartSystem and
  // startPolynomialHessian appropriately.
  void FixedScaleOneLoopPotential::PrepareHomotopyContinuationStartSystem()
  {
    // placeholder:
    /**/std::cout << std::endl
    << "Placeholder: FixedScaleOneLoopPotential::"
    << "PrepareHomotopyContinuationStartSystem()";
    std::cout << std::endl;/**/
  }

  // This should prepare homotopyContinuationStartValues to be all the
  // solutions of homotopyContinuationStartSystem.
  void FixedScaleOneLoopPotential::PrepareHomotopyContinuationStartValues()
  {
    // placeholder:
    /**/std::cout << std::endl
    << "Placeholder: FixedScaleOneLoopPotential::"
    << "PrepareHomotopyContinuationStartValues()";
    std::cout << std::endl;/**/
  }

} /* namespace VevaciousPlusPlus */