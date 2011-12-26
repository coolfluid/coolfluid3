// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_Term_hpp
#define cf3_SFDM_Term_hpp

#include "common/Table_fwd.hpp"
#include "common/BoostArray.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/Entities.hpp"
#include "solver/Action.hpp"

#include "SFDM/LibSFDM.hpp"

namespace cf3 {

namespace RiemannSolvers { class RiemannSolver; }
namespace physics        { class Variables; class Properties; }
namespace mesh   { class Field; class SpaceFields; class Cells; class Space; class Entities; class Entity; class ElementType; class Face2Cell; }

namespace SFDM {
class ShapeFunction;
class Term;
class SharedCaches;
/////////////////////////////////////////////////////////////////////////////////////

class Flyweight
{
public:

  /// Constructors
  Flyweight(const mesh::Entities& entities_comp, const Uint element_idx, Term& this_term);
  Flyweight(const Flyweight& flyweight);

  /// Assignment operator
  Flyweight& operator= (const Flyweight& flyweight);

  /// Destructor
  ~Flyweight () {}

  /// Extrinsic state
  struct Element
  {
    Element(const Flyweight& flyweight, const Uint element_idx);
    Uint idx;
    common::TableConstRow<Uint>::type field_idx;
    typedef common::TableArray<Real>::type::array_view<2>::type FieldView;
    FieldView solution;
    FieldView residual;
    FieldView wave_speed;
    FieldView jacob_det;
  };
  boost::shared_ptr<Element> element;

  /// Intrinsic state
  SFDM::Term& term;
  const mesh::Entities& entities;
  const mesh::ElementType& geometry;
  const mesh::Space& space;
  const SFDM::ShapeFunction& sf;

  /// Set extrinsic state
  Element& set_element(const Uint element_idx);

  /// Convection computations
  void reconstruct_solution_in_flx_pt(const Uint flx_pt, RealVector& sol_in_flx_pt);
  void add_flx_pt_gradient_contribution_to_residual(const Uint flx_pt, const RealVector& flx_in_flx_pt, bool outward=true);
  void add_flx_pt_contribution_to_wave_speed(const Uint flx_pt, const Real& ws_in_flx_pt);
  void compute_analytical_flux(const Uint flx_pt, const RealVector& sol_in_flx_pt, RealVector& flx_in_flx_pt, Real& ws_in_flx_pt);
  void compute_numerical_flux(const Uint flx_pt, const RealVector& sol_left, const RealVector& sol_right, RealVector& flx_in_flx_pt, Real& ws_in_flx_pt);

  struct Cache
  {
    std::auto_ptr<physics::Properties> phys_props;
    RealMatrix phys_flux;
    RealMatrix dummy_grads;
    RealVector phys_ev;
    RealVector phys_coords;
    RealVector plane_jacobian_normal;
    RealVector unit_normal;
    Real plane_jacobian_det;
    Real coeff;
    RealMatrix geometry_nodes;
  };
};

class SFDM_API Term : public cf3::solver::Action {

  friend class Flyweight;

public: // typedefs

  /// provider
  
  

public: // functions

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name );

  /// Virtual destructor
  virtual ~Term();

  /// Get the class name
  static std::string type_name () { return "Term"; }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

  virtual void initialize() { }
  void create_term_field();
  virtual void set_entities(const mesh::Entities& entities) { m_entities = entities.handle<mesh::Entities>(); }
  virtual void set_element(const Uint elem_idx) { m_elem_idx = elem_idx; }
  virtual void unset_element() { }
  void set_neighbour(const Handle<mesh::Entities const>& entities, const Uint elem_idx, const Uint face_nb,
                     Handle<mesh::Entities const>& neighbour_entities, Uint& neighbour_elem_idx, Uint& neighbour_face_nb,
                     Handle<mesh::Entities const>& face_entities, Uint& face_idx);

  SFDM::SharedCaches& shared_caches() { return *m_shared_caches; }
  Uint m_elem_idx;
  Handle<mesh::Entities const> m_entities;
  Handle<mesh::Field> m_term_field;
  Handle<mesh::Field> m_term_wave_speed_field;
  Handle<SFDM::SharedCaches> m_shared_caches;

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @name ACCESSORS
  //@{

  mesh::Field& solution_field()                   { return *m_solution; }

  mesh::Field& residual_field()                   { return *m_residual; }

  mesh::Field& wave_speed_field()                 { return *m_wave_speed; }

  mesh::Field& jacob_det_field()                  { return *m_jacob_det; }

  mesh::Field& delta_field()                      { return *m_delta; }

  RiemannSolvers::RiemannSolver& riemann_solver() { return *m_riemann_solver; }

  physics::Variables& solution_vars()             { return  *m_solution_vars; }

  //@} END ACCESSORS

protected: // function

  void link_fields();

private: // function

  void trigger_physical_model();

  void allocate_cache();

protected:

  Flyweight create_flyweight(const mesh::Entities& entities, const Uint element_idx=0);
  Flyweight create_flyweight(const mesh::Entity& entity);
  std::vector< boost::shared_ptr<Flyweight> > create_flyweight(const mesh::Face2Cell& face);

protected: // data

  Handle<mesh::Field> m_solution;     ///< access to the solution field

  Handle<mesh::Field> m_residual;     ///< access to the residual field

  Handle<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

  Handle<mesh::Field> m_jacob_det;    ///< access to the jacobian_determinant field

  Handle<mesh::Field> m_delta;        ///< access to the delta field (dx, dy, dz)

  Handle<physics::Variables> m_solution_vars; ///< access to the solution variables

  Handle<RiemannSolvers::RiemannSolver> m_riemann_solver; ///< access to the riemann solver

  /// Compute wave speeds in flx_pts
  /// TRUE:  - computation in flx_pts
  ///        - more expensive
  /// FALSE: - computation in sol_pts
  ///        - less accurate (mostly for shocks)
  ///        - WARNING: Must be computed somewhere else!!!!!!!!!
  bool m_compute_wave_speed;

  Flyweight::Cache cache;

};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

#endif // cf3_SFDM_Term_hpp
