// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_EulerDNS_hpp
#define cf3_UFEM_EulerDNS_hpp

#include "../InitialConditions.hpp"

#include "LibUFEMLES.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"
#include "solver/actions/Proto/Expression.hpp"

namespace cf3 {
namespace UFEM {
namespace les {

namespace detail
{

inline void cell_sizes(const mesh::LagrangeP1::Hexa3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(3) - nodes.row(0)).norm();
  d3 = (nodes.row(4) - nodes.row(0)).norm();
}

inline void cell_sizes(const mesh::LagrangeP1::Prism3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(2) - nodes.row(0)).norm();
  d3 = (nodes.row(3) - nodes.row(0)).norm();
}

inline void cell_sizes(const mesh::LagrangeP1::Tetra3D::NodesT& nodes, Real& d1, Real& d2, Real& d3)
{
  d1 = (nodes.row(1) - nodes.row(0)).norm();
  d2 = (nodes.row(2) - nodes.row(0)).norm();
  d3 = (nodes.row(3) - nodes.row(0)).norm();
}

inline void aspect_ratios(const Real d1, const Real d2, const Real d3, Real& a1, Real& a2)
{
  if (d1 >= d2 && d1 >= d3)
  {
    a1 = d2/d1;
    a2 = d3/d1;
  }
  else if (d2 >= d1 && d2 >= d3)
  {
    a1 = d1/d2;
    a2 = d3/d2;
  }
  else
  {
    a1 = d1/d3;
    a2 = d2/d3;
  }
}
  
/// Proto functor to compute the turbulent viscosity
struct ComputeNuWALE
{
  typedef void result_type;
  
  ComputeNuWALE() :
    cw(0.5),
    use_anisotropic_correction(false)
  {
  }

  template<typename UT, typename NUT, typename ValenceT>
  void operator()(const UT& u, NUT& nu, const ValenceT& valence, const Real nu_visc) const
  {
    typedef typename UT::EtypeT ElementT;
    static const Uint dim = ElementT::dimension;
    
    typedef mesh::Integrators::GaussMappedCoords<1, ElementT::shape> GaussT;
    typedef Eigen::Matrix<Real, dim, dim> SMatT;
        
    const SMatT grad_u = u.nabla(GaussT::instance().coords.col(0))*u.value();
    const SMatT S = 0.5*(grad_u + grad_u.transpose());
    const Real S_norm2 = S.squaredNorm();
    const SMatT grad_u2 = grad_u*grad_u;

    SMatT Sd = 0.5*(grad_u2 + grad_u2.transpose());
    Sd.diagonal().array() -= grad_u2.trace()/3.;
    const Real Sd_norm2 = Sd.squaredNorm();

    // Compute the anisotropic cell size adjustment using the method of Scotti et al.
    Real f = 1.;
    if(use_anisotropic_correction)
    {
      Real d1, d2, d3, a1, a2;
      cell_sizes(u.support().nodes(), d1, d2, d3);
      aspect_ratios(d1, d2, d3, a1, a2);
      const Real log_a1 = ::log(a1);
      const Real log_a2 = ::log(a2);
      f = ::cosh(::sqrt(4./27.*(log_a1*log_a1 - log_a1*log_a2 + log_a2*log_a2)));
    }
    
    // Get the isotropic length scale
    const Real delta_iso = ::pow(u.support().volume(), 2./3.);

    Real nu_t = cw*cw*f*f*delta_iso * ::pow(Sd_norm2, 1.5) / (::pow(S_norm2, 2.5) + ::pow(Sd_norm2, 1.25));
    if(nu_t < 0. || !std::isfinite(nu_t))
      nu_t = 0.;
    
    const Eigen::Matrix<Real, ElementT::nb_nodes, 1> nodal_vals = (nu_t + nu_visc)*valence.value().array().inverse();
    nu.add_nodal_values(nodal_vals);
  }
  
  // Model constant
  Real cw;
  bool use_anisotropic_correction;
};

}
  
/// WALE LES model component
class WALE : public solver::actions::Proto::ProtoAction
{
public: // functions
  /// Contructor
  /// @param name of the component
  WALE( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "WALE"; }
  
  virtual void execute();

private:
  void trigger_set_expression();
  void trigger_initial_conditions();
  virtual void on_regions_set();
  Handle<InitialConditions> m_initial_conditions;
  Handle<common::Component> m_node_valence;
  Handle<solver::actions::Proto::ProtoAction> m_reset_viscosity;
  
  /// The data stored by the WALE op terminal
  solver::actions::Proto::MakeSFOp<detail::ComputeNuWALE>::stored_type m_wale_op;
  
  /// Terminal with a reference to the WALE op data
  solver::actions::Proto::MakeSFOp<detail::ComputeNuWALE>::reference_type wale;
};

} // les
} // UFEM
} // cf3


#endif // cf3_UFEM_EulerDNS_hpp
