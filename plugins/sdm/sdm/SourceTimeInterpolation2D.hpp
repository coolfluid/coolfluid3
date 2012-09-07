// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_SourceTimeInterpolation2D_hpp
#define cf3_sdm_SourceTimeInterpolation2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/OptionComponent.hpp"

#include "math/Consts.hpp"
#include "math/MatrixTypesConversion.hpp"

#include "solver/Time.hpp"

#include "sdm/SourceTerm.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////

struct SourceTimeInterpolationPhysData2D : PhysDataBase<4u,2u>
{
  RealVectorNEQS source;
};

////////////////////////////////////////////////////////////////////////////////

/// @brief Source term, using interpolation between time steps
/// @author Willem Deconinck
class sdm_API SourceTimeInterpolation2D : public SourceTerm< SourceTimeInterpolationPhysData2D >
{
public:
  static std::string type_name() { return "SourceTimeInterpolation2D"; }
  SourceTimeInterpolation2D(const std::string& name) : SourceTerm< PhysData >(name)
  {
    options().add("source_past",m_source_past)
        .mark_basic()
        .link_to(&m_source_past)
        .description("Vector field containing lighthill source for momentum equations");


    options().add("source_future",m_source_future)
        .description("Vector field containing lighthill source for momentum equations")
        .link_to(&m_source_future)
        .mark_basic();

    options().add("time",m_time).link_to(&m_time).mark_basic();
  }

  virtual ~SourceTimeInterpolation2D() {}

  virtual void set_entities(const mesh::Entities& entities)
  {
    SourceTerm<SourceTimeInterpolationPhysData2D>::set_entities(entities);

    cf3_always_assert(m_source_past);
    cf3_always_assert(m_source_future);
    cf3_always_assert(m_time);

    m_time_now    = m_time->current_time();
    m_time_past   = m_source_past->properties().value<Real>("time");
    m_time_future = m_source_future->properties().value<Real>("time");

    cf3_always_assert(m_time->current_time() >= m_time_past);
    cf3_always_assert(m_time->current_time() <= m_time_future);
  }

  void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PhysData& phys_data )
  {
    mesh::Field::View past   = m_source_past->view(elem.space->connectivity()[elem.idx]);
    mesh::Field::View future = m_source_future->view(elem.space->connectivity()[elem.idx]);

    for (Uint d=0; d<NEQS; ++d)
    {
      phys_data.source[d] = past[sol_pt][d] + (future[sol_pt][d]-past[sol_pt][d])/(m_time_future-m_time_past) * (m_time_now-m_time_past);
    }
  }


  virtual void compute_source(PhysData& data,
                              RealVectorNEQS& source)
  {
    source[0] = data.source[0];
    source[1] = data.source[1];
    source[2] = data.source[2];
    source[3] = data.source[3];
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  Handle<mesh::Field>  m_source_past;    ///< Source past
  Handle<mesh::Field>  m_source_future;  ///< Source future
  Handle<solver::Time> m_time;


  Real m_time_now;
  Real m_time_past;
  Real m_time_future;

};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_SourceTimeInterpolation2D_hpp
