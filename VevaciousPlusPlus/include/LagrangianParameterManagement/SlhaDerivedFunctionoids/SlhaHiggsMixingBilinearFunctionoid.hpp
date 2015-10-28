/*
 * SlhaHiggsMixingBilinearFunctionoid.hpp
 *
 *  Created on: Oct 28, 2015
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#ifndef SLHAHIGGSMIXINGBILINEARFUNCTIONOID_HPP_
#define SLHAHIGGSMIXINGBILINEARFUNCTIONOID_HPP_

#include "CommonIncludes.hpp"
#include "LagrangianParameterManagement/SlhaDerivedFunctionoid.hpp"

namespace VevaciousPlusPlus
{

  class SlhaHiggsMixingBilinearFunctionoid : public SlhaDerivedFunctionoid
  {
  public:
    SlhaHiggsMixingBilinearFunctionoid(
       SlhaInterpolatedParameterFunctionoid const& treePseudoscalarMassSquared,
                                      SlhaTwoSourceFunctionoid const& tanBeta);
    virtual ~SlhaHiggsMixingBilinearFunctionoid();


    // This returns sin(beta) * cos(beta) * mA^2 evaluated at the scale given
    // through logarithmOfScale.
    virtual double operator()( double const logarithmOfScale ) const
    { return ( SinBetaCosBeta( tanBeta( logarithmOfScale ) )
               * treePseudoscalarMassSquared( logarithmOfScale ) ); }

    // This returns sin(beta) * cos(beta) * mA^2 evaluated from the given
    // parameters.
    virtual double operator()( double const logarithmOfScale,
                        std::vector< double > const& interpolatedValues ) const
    { return ( SinBetaCosBeta( interpolatedValues[ tanBetaIndex ] )
               * interpolatedValues[ treePseudoscalarMassSquaredIndex ] ); }

    // This is for creating a Python version of the potential.
    virtual std::string
    PythonParameterEvaluation( int const indentationSpaces ) const;


  protected:
    // This returns the value of sin(beta) * cos(beta) given the value of
    // tan(beta) without using possibly expensive trigonometric operations.
    static double SinBetaCosBeta( double const tanBeta )
    { return ( tanBeta / ( 1.0 + ( tanBeta * tanBeta ) ) ); }

    SlhaInterpolatedParameterFunctionoid const& treePseudoscalarMassSquared;
    size_t const treePseudoscalarMassSquaredIndex;
    SlhaTwoSourceFunctionoid const& tanBeta;
    size_t const tanBetaIndex;
  };





  // This is for creating a Python version of the potential.
  inline std::string
  SlhaHiggsMixingBilinearFunctionoid::PythonParameterEvaluation(
                                            int const indentationSpaces ) const
  {
    std::stringstream stringBuilder;
    stringBuilder << PythonIndent( indentationSpaces )
    << "parameterValues[ " << IndexInValuesVector()
    << " ] = ( ( parameterValues[ " << treePseudoscalarMassSquaredIndex
    << " ] * parameterValues[ " << tanBetaIndex
    << " ] ) / ( 1.0 + ( parameterValues[ \" << tanBetaIndex << \" ] )**2 ) )";
    return stringBuilder.str();
  }

} /* namespace VevaciousPlusPlus */

#endif /* SLHAHIGGSMIXINGBILINEARFUNCTIONOID_HPP_ */