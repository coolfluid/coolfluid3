// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp
#define cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp

////////////////////////////////////////////////////////////////////////////////

#include "sdm/BCWeak.hpp"
#include "sdm/lineuler/LibLinEuler.hpp"
#include "sdm/lineuler/NonReflectiveConvection2D.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {
namespace lineuler {

////////////////////////////////////////////////////////////////////////////////

class sdm_lineuler_API BCSubsonicOutlet2D : public BCWeak< PhysDataBase<4u,2u> >
{
public:
  static std::string type_name() { return "BCSubsonicOutlet2D"; }
  BCSubsonicOutlet2D(const std::string& name) : BCWeak< PhysData >(name)
  {
    correct_non_reflective_term = create_component("non_reflective_convection","cf3.sdm.lineuler.NonReflectiveConvection2D")->handle<Term>();
  }
  virtual ~BCSubsonicOutlet2D() {}

  virtual void execute();

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:

  virtual void initialize()
  {
    BCWeak< PhysData >::initialize();   
    correct_non_reflective_term->options().configure_option("solver",m_solver);
    correct_non_reflective_term->options().configure_option("mesh",m_mesh);
    correct_non_reflective_term->options().configure_option("physical_model",m_physical_model);
    correct_non_reflective_term->initialize();
  }

  virtual void set_inner_cell()
  {
    BCWeak< PhysData >::set_inner_cell();
    correct_non_reflective_term->set_entities(*cell_entities);
    correct_non_reflective_term->set_element(cell_idx);
  }
  virtual void unset_inner_cell()
  {
    BCWeak< PhysData >::unset_inner_cell();
    correct_non_reflective_term->unset_element();
  }

  virtual void compute_solution(const PhysData &inner_cell_data, const RealVectorNDIM &boundary_face_normal, RealVectorNEQS &boundary_face_solution){}

  Handle<CacheT<FluxPointPlaneJacobianNormal<NDIM> > > flx_pt_plane_jacobian_normal;
  RealVectorNDIM rhoU;
  Real rhoU_normal;

  Handle<Term> correct_non_reflective_term;
};

////////////////////////////////////////////////////////////////////////////////

} // lineuler
} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_lineuler_BCSubsonicOutlet2D_hpp
