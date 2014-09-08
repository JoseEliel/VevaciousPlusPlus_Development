/*
 * MinimizingPotentialOnHemispheres.cpp
 *
 *  Created on: Sep 2, 2014
 *      Author: Ben O'Leary (benjamin.oleary@gmail.com)
 */

#include "BounceActionEvaluation/BounceActionPathFinding/MinimizingPotentialOnHemispheres.hpp"

namespace VevaciousPlusPlus
{

  MinimizingPotentialOnHemispheres::MinimizingPotentialOnHemispheres(
                                    PotentialFunction const& potentialFunction,
                                               MinuitBetweenPaths* pathRefiner,
                                             size_t const minimumNumberOfNodes,
                                              size_t const movesPerImprovement,
                                             unsigned int const minuitStrategy,
                                       double const minuitToleranceFraction ) :
    MinimizingPotentialOnHypersurfaces( potentialFunction,
                                        pathRefiner,
                                        movesPerImprovement,
                                        minuitStrategy,
                                        minuitToleranceFraction ),
    minimumNumberOfNodes( minimumNumberOfNodes ),
    stepSize( NAN ),
    currentAngleScaling( NAN )
  {
    // This constructor is just an initialization list.
  }

  MinimizingPotentialOnHemispheres::~MinimizingPotentialOnHemispheres()
  {
    // This does nothing.
  }


  // This sets up the nodes by minimizing the potential on a hypersector of a
  // hypersphere of radius equal to ( 0.5 * stepSize ) from the last set node
  // beginning with the false vacuum, facing the true vacuum, and then makes
  // a step of length stepSize through this point on the hemisphere from the
  // node to create a new node, which is then used as the center of the
  // "hemisphere" for the next node. This continues until the direct distance
  // to the true vacuum is less than stepSize, or until Minuit2 has exceeded
  // movesPerImprovement function calls in total with this call of
  // ImproveNodes(), and nodesConverged is set appropriately.
  void MinimizingPotentialOnHemispheres::ImproveNodes()
  {
    size_t numberOfMovesSoFar( 0 );
    while( numberOfMovesSoFar < movesPerImprovement )
    {
      pathNodes.push_back( pathNodes.back() );
      std::vector< double >& centerNode( pathNodes[ pathNodes.size() - 2 ] );
      std::vector< double >& currentNode( pathNodes[ pathNodes.size() - 1 ] );
      SetParallelVector( centerNode,
                         pathNodes.back() );
      SetUpHouseholderReflection();
      SetCurrentMinuitSteps();

      ROOT::Minuit2::MnMigrad mnMigrad( *this,
                                 std::vector< double >( ( numberOfFields - 1 ),
                                                        0.0 ),
                                        minuitInitialSteps,
                                        minuitStrategy );
      MinuitMinimum minuitResult( mnMigrad( movesPerImprovement,
                                            currentMinuitTolerance ) );

      Eigen::VectorXd const
      stepVector( StepVector( minuitResult.VariableValues() ) );
      for( size_t fieldIndex( 0 );
           fieldIndex < numberOfFields;
           ++fieldIndex )
      {
        currentNode[ fieldIndex ] = ( centerNode[ fieldIndex ]
                                      + stepVector( fieldIndex ) );
      }
      if( )
    }
  }

  // This takes the parameterization as the components of a vector
  // perpendicular to axis 0 which is then added to a unit vector along axis
  // 0. This then could be scaled to give a unit vector within the hemisphere
  // of positive field 0, but first the angle it makes with axis 0 is scaled
  // so that the next node will be closer to the true vacuum than the current
  // last node before the true vacuum. This vector is then scaled to be of
  // length stepSize.
  Eigen::VectorXd MinimizingPotentialOnHemispheres::StepVector(
                      std::vector< double > const& nodeParameterization ) const
  {
    double tangentSquared( 0.0 );
    for( std::vector< double >::const_iterator
         nodeParameter( nodeParameterization.begin() );
         nodeParameter < nodeParameterization.end();
         ++nodeParameter )
    {
      tangentSquared += ( (*nodeParameter) * (*nodeParameter) );
    }
    double const parameterizationRadius( sqrt( tangentSquared ) );
    double const
    scaledAngle( currentAngleScaling * atan( parameterizationRadius ) );
    double const radialScaling( ( stepSize * sin( scaledAngle ) )
                                / parameterizationRadius );
    Eigen::VectorXd untransformedNode( numberOfFields );
    untransformedNode( 0 ) = ( stepSize * cos( scaledAngle ) );
    for( size_t fieldIndex( 1 );
        fieldIndex < numberOfFields;
        ++fieldIndex )
    {
      untransformedNode( fieldIndex )
      = ( radialScaling * nodeParameterization[ fieldIndex - 1 ] );
    }
    return ( reflectionMatrix * untransformedNode );
  }

} /* namespace VevaciousPlusPlus */
