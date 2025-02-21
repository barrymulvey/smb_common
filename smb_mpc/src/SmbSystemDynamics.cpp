//
// Created by johannes on 01.05.19.
//
#include <smb_mpc/SmbSystemDynamics.h>

using namespace ocs2;

namespace smb_mpc {
SmbSystemDynamics::SmbSystemDynamics(const std::string &modelName,
                                     const std::string &modelFolder,
                                     bool recompileLibraries, bool verbose) {
  this->initialize(SmbDefinitions::STATE_DIM, SmbDefinitions::INPUT_DIM,
                   modelName, modelFolder, recompileLibraries, verbose);
}
ad_vector_t SmbSystemDynamics::systemFlowMap(
    ocs2::ad_scalar_t time, const ocs2::ad_vector_t &state,
    const ocs2::ad_vector_t &input, const ocs2::ad_vector_t &parameters) const {

  using ad_quat_t = Eigen::Quaternion<ad_scalar_t>;
  using ad_vec3_t = Eigen::Matrix<ad_scalar_t, 3, 1>;

  ad_vec3_t positionDerivative = ad_vec3_t::Zero();
  ad_quat_t orientationDerivative;
  orientationDerivative.coeffs().setZero();

  ad_scalar_t v_x = SmbConversions::readLinVel(input);
  ad_scalar_t omega_z = SmbConversions::readAngVel(input);

  ad_vec3_t currentPosition = SmbConversions::readPosition(state);
  ad_quat_t currentRotation = SmbConversions::readRotation(state);

  /// Begin: Compute positionDerivative and orientationDerivative
  ad_vec3_t linearVelocity = ad_vec3_t::Zero();
  linearVelocity[0] = v_x;
  
  // transform linear velocity from base frame to world frame:
  positionDerivative = currentRotation * linearVelocity;
  
  ad_quat_t deltaRotation;
  deltaRotation.w() = 0;
  deltaRotation.x() = 0;
  deltaRotation.y() = 0;
  deltaRotation.z() = omega_z/2;
  
  orientationDerivative = (currentRotation * deltaRotation);
  /// End: Compute positionDerivative and orientationDerivative
  
  ad_vector_t stateDerivative = ad_vector_t::Zero(SmbDefinitions::STATE_DIM);
  stateDerivative << positionDerivative, orientationDerivative.coeffs();
  return stateDerivative;
}
} // namespace smb_mpc
