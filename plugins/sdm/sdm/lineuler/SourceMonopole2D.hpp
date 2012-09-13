// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceMonopole2D_hpp
#define cf3_sdm_lineuler_SourceMonopole2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"

#include "solver/Time.hpp"

#include "sdm/SourceTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

struct SourceMonopole2DPhysData2D : PhysDataBase<4,2u>
{
  Real rho0;
  Real p0;
};


////////////////////////////////////////////////////////////////////////////////

/// @brief Monopole source term in 2D
/// @author Willem Deconinck
///
/// @f[ f(x,y,t) = \sin(\omega\ t) \  \varepsilon\ \exp(-\alpha\ ( (x-x0)^2 + y-y0)^2 ) ) @f]
/// @f[ S = \[ \begin{array}[c]
///     f(x,y,t) \\
///     0        \\
///     0        \\
///     c_0^2 \ f(x,y,t)
/// \end{array} \] @f]
///
/// Heat capacity ratio gamma and mean flow is necessary to compute the sound speed c0
class sdm_lineuler_API SourceMonopole2D : public SourceTerm< SourceMonopole2DPhysData2D >
{
public:
  static std::string type_name() { return "SourceMonopole2D"; }
  SourceMonopole2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("time",m_time)
      .description("Time component")
      .link_to(&m_time)
      .mark_basic();

    m_freq = 1./30.;
    options().add("freq",m_freq)
        .description("Frequency")
        .link_to(&m_freq)
        .mark_basic();

    m_source_loc << 0.,0.;
    std::vector<Real> source_loc_opt(NDIM);
    source_loc_opt[XX]=0.;
    source_loc_opt[YY]=0.;
    options().add("source_location",source_loc_opt)
        .description("Source location")
        .attach_trigger( boost::bind( &SourceMonopole2D::config_source_loc, this) )
        .mark_basic();

    m_alpha = 2.0;
    options().add("alpha",m_alpha)
        .description("Source width")
        .link_to(&m_alpha)
        .mark_basic();

    m_eps = 0.5;
    options().add("epsilon",m_eps)
        .description("Source amplitude")
        .link_to(&m_eps)
        .mark_basic();

    m_gamma = 1.4;
    options().add("gamma",m_gamma)
        .mark_basic()
        .description("Heat capacity ration")
        .link_to(&m_gamma);

    options().add("mean_flow",m_mean_flow)
        .mark_basic()
        .description("Mean flow")
        .link_to(&m_mean_flow);
  }

  void config_source_loc()
  {
    std::vector<Real> source_loc_opt = options().option("source_location")
        .value<std::vector<Real> >();
    for (Uint d=0; d<NDIM; ++d)
      m_source_loc[d] = source_loc_opt[d];
  }


  virtual void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    SourceTerm<SourceMonopole2DPhysData2D>::compute_sol_pt_phys_data(elem,sol_pt,phys_data);
    mesh::Field::View mean_flow = m_mean_flow->view( elem.space->connectivity()[elem.idx] );
    phys_data.rho0 = mean_flow[sol_pt][0];
    phys_data.p0 = mean_flow[sol_pt][3];
  }

  virtual ~SourceMonopole2D() {}

  // Check at setting of entities, if time is set properly
  virtual void set_entities(const mesh::Entities& entities)
  {
    if (is_null(m_time))
      throw common::SetupError(FromHere(),"Option \"time\" was not set and is required");

    if (is_null(m_mean_flow))
      throw common::SetupError(FromHere(),"Option \"mean_flow\" was not set and is required");

    SourceTerm<PhysData>::set_entities(entities);
  }

  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    using namespace std;
    using namespace math::Consts;

    // square of sound speed
    const Real c02 = m_gamma*data.p0/data.rho0;

    // angular frequency
    const Real omega = 2.*pi()*m_freq;

    // time
    const Real& t = m_time->current_time();

    m_source = f(data.coord) * sin(omega*t);

    source[0] = m_source;
    source[1] = 0.;
    source[2] = 0.;
    source[3] = c02 * m_source;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  /// spatial part of the source term
  Real f(const RealVectorNDIM& coord)
  {
    return m_eps * exp( -log(2.)/m_alpha*(
      (coord[XX]-m_source_loc[XX])*(coord[XX]-m_source_loc[XX]) +
      (coord[YY]-m_source_loc[YY])*(coord[YY]-m_source_loc[YY])  ) );
  }

  Real m_source;                      ///< dummy variable

  Handle<solver::Time const> m_time;  ///< Component to store time
  Real m_alpha;                       ///< Width of monopole
  Real m_eps;                         ///< Amplitude of monopole
  Real m_freq;                        ///< Frequency of monopole
  RealVectorNDIM m_source_loc;        ///< Location of monopole

  Handle< mesh::Field > m_mean_flow;  ///< mean flow
  Real m_gamma;                       ///< Heat capacity ratio
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceMonopole2D_hpp
