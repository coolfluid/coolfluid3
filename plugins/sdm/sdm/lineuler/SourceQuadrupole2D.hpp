// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceQuadrupole2D_hpp
#define cf3_sdm_lineuler_SourceQuadrupole2D_hpp

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

////////////////////////////////////////////////////////////////////////////////

/// @brief Quadrupole source term in 2D
/// @author Willem Deconinck
///
/// @f[ S = \[ \begin{array}[c]
///     0 \\
///      eps*sin((pi/4*(x-x0)/alpha) * exp(-log(2)/alpha*(y-y0)^2) * sin(omega*t) \\
///     -eps*sin((pi/4*(y-y0)/alpha) * exp(-log(2)/alpha*(x-x0)^2) * sin(omega*t) \\
///     0 
/// \end{array} \] @f]
///
class sdm_lineuler_API SourceQuadrupole2D : public SourceTerm< PhysDataBase<4,2> >
{
public:
  static std::string type_name() { return "SourceQuadrupole2D"; }
  SourceQuadrupole2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("time",m_time)
      .description("Time component")
      .link_to(&m_time)
      .mark_basic();

    m_freq = 1./60.;
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
        .attach_trigger( boost::bind( &SourceQuadrupole2D::config_source_loc, this) )
        .mark_basic();

    m_alpha = 5.;
    options().add("alpha",m_alpha)
        .description("Source width")
        .link_to(&m_alpha)
        .mark_basic();

    m_eps = 0.01;
    options().add("epsilon",m_eps)
        .description("Source amplitude")
        .link_to(&m_eps)
        .mark_basic();
  }

  void config_source_loc()
  {
    std::vector<Real> source_loc_opt = options().option("source_location")
        .value<std::vector<Real> >();
    for (Uint d=0; d<NDIM; ++d)
      m_source_loc[d] = source_loc_opt[d];
  }

  virtual ~SourceQuadrupole2D() {}

  // Check at setting of entities, if time is set properly
  virtual void set_entities(const mesh::Entities& entities)
  {
    if (is_null(m_time))
      throw common::SetupError(FromHere(),"Option \"time\" was not set and is required");
    SourceTerm<PhysData>::set_entities(entities);
  }

  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    using namespace math::Consts;

    source[0] = 0.;
    source[1] =  fx(data.coord) * sin(2.*pi()*m_freq*m_time->current_time());
    source[2] = -fy(data.coord) * sin(2.*pi()*m_freq*m_time->current_time());
    source[3] = 0.;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  /// spatial part of the source term
  Real fx(const RealVectorNDIM& coord)
  {
    using namespace std;
    using namespace math::Consts;
    
    const Real dx = coord[XX]-m_source_loc[XX];
    const Real dy = coord[YY]-m_source_loc[YY];

    if (abs(dx/2.) > m_alpha)      
      return 0.;
    else
      return m_eps*sin( pi()/4 * dx/m_alpha ) * exp( -log(2)/m_alpha * pow(dy,2) );
  }

  /// spatial part of the source term
  Real fy(const RealVectorNDIM& coord)
  {
    using namespace std;
    using namespace math::Consts;
    
    const Real dx = coord[XX]-m_source_loc[XX];
    const Real dy = coord[YY]-m_source_loc[YY];

    if (abs(dy/2.) > m_alpha)      
      return 0.;
    else
      return m_eps*sin( pi()/4 * dy/m_alpha ) * exp( -log(2)/m_alpha * pow(dx,2) );
  }

  Handle<solver::Time const> m_time;  ///< Component to store time
  Real m_alpha;                       ///< Width of Quadrupole
  Real m_eps;                         ///< Amplitude of Quadrupole
  Real m_freq;                        ///< Frequency of Quadrupole
  RealVectorNDIM m_source_loc;        ///< Location of Quadrupole

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceQuadrupole2D_hpp
