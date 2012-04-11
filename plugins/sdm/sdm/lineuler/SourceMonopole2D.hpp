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

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API SourceMonopole2D : public SourceTerm< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "SourceMonopole2D"; }
  SourceMonopole2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add_option("time",m_time)
      .description("Time component")
      .link_to(&m_time);
    
    m_omega = 2.*math::Consts::pi()/30.;
    options().add_option("omega",m_omega)
        .description("Angular frequency")
        .link_to(&m_omega);

    m_source_loc << 0.,0.;
    std::vector<Real> source_loc_opt(NDIM);
    source_loc_opt[0]=0.;
    source_loc_opt[1]=0.;
    options().add_option("source_location",source_loc_opt)
        .description("Source location")
        .attach_trigger( boost::bind( &SourceMonopole2D::config_source_loc, this) );

    m_alpha = 0.5*log(2.);
    options().add_option("alpha",m_alpha)
        .description("Source width")
        .link_to(&m_alpha);

    m_eps = 0.5;
    options().add_option("epsilon",m_eps)
        .description("Source amplitude")
        .link_to(&m_eps);
  }

  void config_source_loc()
  {
    std::vector<Real> source_loc_opt = options().option("source_location")
        .value<std::vector<Real> >();
    for (Uint d=0; d<NDIM; ++d)
      m_source_loc[d] = source_loc_opt[d];
  }

  virtual ~SourceMonopole2D() {}

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
    m_source = f(data.coord) * sin(m_omega*m_time->current_time());
    source[0] = m_source;
    source[1] = 0.;
    source[2] = 0.;
    source[3] = m_source;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
private:
  
  /// spatial part of the source term
  Real f(const RealVectorNDIM& coord)
  {
    return m_eps * exp( -m_alpha*( 
      (coord[XX]-m_source_loc[XX])*(coord[XX]-m_source_loc[XX]) + 
      (coord[YY]-m_source_loc[YY])*(coord[YY]-m_source_loc[YY])  ) );
  }
  
  Real m_source;                      ///< dummy variable
  
  Handle<solver::Time const> m_time;  ///< Component to store time
  Real m_alpha;                       ///< Width of monopole
  Real m_eps;                         ///< Amplitude of monopole
  Real m_omega;                       ///< Angular frequency of monopole
  RealVectorNDIM m_source_loc;        ///< Location of monopole

};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceMonopole2D_hpp
