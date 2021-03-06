#include "base-body.h"

using namespace MPCWalkgen;
using namespace Zebulon;
using namespace Eigen;


BaseBody::BaseBody(const MPCData *generalData,
		   const RobotData *robotData,
		   const Interpolation *interpolation)
  :RigidBody(generalData, robotData, interpolation)
{}

BaseBody::~BaseBody(){}

void BaseBody::interpolate(GlobalSolution &solution, double /*currentTime*/, const Reference &/*velRef*/) {
  interpolation_->computeInterpolationByJerk(solution.mpcSolution.state_vec[0].baseTrajX_, solution.mpcSolution.state_vec[0].baseTrajY_, state_,
                                             dynamics(interpolationPosInt), solution.qpSolution(2*generalData_->nbSamplesQP),
                                             solution.qpSolution(3*generalData_->nbSamplesQP));

  interpolation_->computeInterpolationByJerk(solution.mpcSolution.state_vec[1].baseTrajX_, solution.mpcSolution.state_vec[1].baseTrajY_, state_,
                                             dynamics(interpolationPos), solution.qpSolution(2*generalData_->nbSamplesQP),
                                             solution.qpSolution(3*generalData_->nbSamplesQP));

  interpolation_->computeInterpolationByJerk(solution.mpcSolution.state_vec[2].baseTrajX_, solution.mpcSolution.state_vec[2].baseTrajY_, state_,
                                             dynamics(interpolationVel), solution.qpSolution(2*generalData_->nbSamplesQP),
                                             solution.qpSolution(3*generalData_->nbSamplesQP));

  interpolation_->computeInterpolationByJerk(solution.mpcSolution.state_vec[3].baseTrajX_, solution.mpcSolution.state_vec[3].baseTrajY_, state_,
                                             dynamics(interpolationAcc), solution.qpSolution(2*generalData_->nbSamplesQP),
                                             solution.qpSolution(3*generalData_->nbSamplesQP));


}

void BaseBody::computeDynamicsMatrices(LinearDynamics & dyn,
                                       double S, double T, int N, DynamicMatrixType type){
  dyn.S.setZero(N,5);
  dyn.U.setZero(N,N);
  dyn.UT.setZero(N,N);
  dyn.UInv.setZero(N,N);
  dyn.UInvT.setZero(N,N);


  switch (type){
    case posIntDynamic:
      for (int i=0; i<N; ++i) {
          dyn.S(i,0) = 1;
          dyn.S(i,1) =i*T + S;
          dyn.S(i,2) = S*S/2 + i*T*S + i*i*T*T/2;
          dyn.S(i,3) = S*S*S/6 + i*T*S*S/2 + i*i*T*T*S/2 + i*i*i*T*T*T/6;

          dyn.U(i,0) = dyn.UT(0,i) = S*S*S*S/24 + i*S*S*S*T/6 + i*i*S*S*T*T/4 + i*i*i*S*T*T*T/6;
          for (int j=1; j<N; j++) {
              if (j <= i) {
                  dyn.U(i,j) = dyn.UT(j,i) = T*T*T*T/24 + (i-j)*T*T*T*T/6 + (i-j)*(i-j)*T*T*T*T/4 + (i-j)*(i-j)*(i-j)*T*T*T*T/6;
                }
            }
        }
      break;

    case posDynamic:
      for (int i=0; i<N; ++i) {
          dyn.S(i,1) = 1;
          dyn.S(i,2) =i*T + S;
          dyn.S(i,3) = S*S/2 + i*T*S + i*i*T*T/2;

          dyn.U(i,0) = dyn.UT(0,i) = S*S*S/6 + i*T*S*S/2 + S*(i*i*T*T/2 );
          for (int j=1; j<N; j++) {
              if (j <= i) {
                  dyn.U(i,j) = dyn.UT(j,i) = T*T*T/6 + (i-j)*T*T*T/2 + (i-j)*(i-j)*T*T*T/2;
                }
            }
        }
      break;

    case velDynamic:
      for (int i=0;i<N;i++) {
          dyn.S(i,2) = 1;
          dyn.S(i,3) = i*T + S;

          dyn.U(i,0) = dyn.UT(0,i) = S*S/2 + i*T*S;
          for (int j=1; j<N; j++) {
              if (j<=i){
                  dyn.U(i,j) = dyn.UT(j,i) = T*T/2 + (i-j)*T*T;
                }
            }
        }
      break;

    case accDynamic:
      for (int i=0; i<N; i++) {
          dyn.S(i,3) = 1;

          dyn.U(i,0) = dyn.UT(0,i) = S;
          for (int j=1; j<N; j++) {
              if (j<=i){
                  dyn.U(i,j) = dyn.UT(j,i) = T;
                }
            }
        }
      break;

    default:
      dyn.U.setIdentity();
      dyn.UT.setIdentity();
      break;
    }
}



