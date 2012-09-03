// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_SourceLighthill2D_hpp
#define cf3_sdm_lineuler_SourceLighthill2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"

#include "solver/Time.hpp"

#include "sdm/SourceTerm.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "Physics/LinEuler/Cons2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

struct SourceLighthillPhysData2D : PhysDataBase<4u,2u>
{
  RealVectorNDIM lighthill_source;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Source term, using the lighthill tensor
///
/// The Lighthill Tensor T is here defined as  ( u'_i u_'j )
///
/// The computed source:
///
/// short notation:   S = rho0*d( u'_i u'_j )/d( x_j ) - < rho0*d( u'_i u'_j )/d( x_j ) >
///      [                  0                 ]     [                    0                   ]
///  S = [ rho0 ( d(u'u')/dx  +  d(u'v')/dy ) ]  -  [ rho0 ( d(<u'u'>)/dx  +  d(<u'v'>)/dy ) ]
///      [ rho0 ( d(u'v')/dx  +  d(v'v')/dy ) ]     [ rho0 ( d(<v'u'>)/dx  +  d(<v'v'>)/dy ) ]
///      [                  0                 ]     [                    0                   ]
///
/// with u' = U[XX] - U0[XX]
/// with v' = U[YY] - U0[YY]
/// with < lolo > the time-averaged lolo
///
/// @author Willem Deconinck
class sdm_lineuler_API SourceLighthill2D : public SourceTerm< SourceLighthillPhysData2D >
{
public:
  static std::string type_name() { return "SourceLighthill2D"; }
  SourceLighthill2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("lighthill_source_field_past",m_lighthill_source_field_past)
        .mark_basic()
        .link_to(&m_lighthill_source_field_past)
        .attach_trigger( boost::bind( &SourceLighthill2D::config_past, this ) )
        .description("Vector field containing lighthill source for momentum equations");


    options().add("lighthill_source_field_future",m_lighthill_source_field_future)
        .description("Vector field containing lighthill source for momentum equations")
        .link_to(&m_lighthill_source_field_future)
        .mark_basic()
        .attach_trigger( boost::bind( &SourceLighthill2D::config_future, this ) );

    options().add("time",m_time).link_to(&m_time).mark_basic();

    m_rho0 = 1.;
    options().add("rho0",m_rho0)
        .description("Uniform mean density")
        .link_to(&m_rho0)
        .mark_basic();
  }

  virtual ~SourceLighthill2D() {}

  void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    cf3_always_assert(m_lighthill_source_field_past);
    cf3_always_assert(m_lighthill_source_field_future);
    cf3_always_assert(m_time);

    mesh::Field::View lighthill_source_field_past   = m_lighthill_source_field_past->view(elem.space->connectivity()[elem.idx]);
    mesh::Field::View lighthill_source_field_future = m_lighthill_source_field_future->view(elem.space->connectivity()[elem.idx]);

    m_time_past = m_lighthill_source_field_past->properties().value<Real>("time");
    m_time_future = m_lighthill_source_field_future->properties().value<Real>("time");

    cf3_always_assert(m_time->current_time() >= m_time_past);
    cf3_always_assert(m_time->current_time() <= m_time_future);

    for (Uint d=0; d<NDIM; ++d)
    {
      phys_data.lighthill_source[d] = lighthill_source_field_past[sol_pt][d] + (lighthill_source_field_future[sol_pt][d]-lighthill_source_field_past[sol_pt][d])/(m_time_future-m_time_past) * (m_time->current_time()-m_time_past);
    }
  }



  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    source[0] = 0.;
    source[1] = m_rho0 * data.lighthill_source[XX];
    source[2] = m_rho0 * data.lighthill_source[YY];
    source[3] = 0.;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  void config_past()
  {
    m_time_past = m_lighthill_source_field_past->properties().value<Real>("time");
  }

  void config_future()
  {
    m_time_future = m_lighthill_source_field_future->properties().value<Real>("time");
  }

private:

  Real m_rho0;                                   ///< Mean uniform density
  Handle<mesh::Field> m_lighthill_source_field_past;  ///< Source velocity field
  Handle<mesh::Field> m_lighthill_source_field_future;  ///< Source velocity field


  Handle<solver::Time> m_time;
  Real m_time_past;
  Real m_time_future;

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceLighthill2D_hpp
