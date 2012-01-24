// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_Term_hpp
#define cf3_sdm_Term_hpp

#include "common/Table_fwd.hpp"
#include "common/BoostArray.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/Entities.hpp"
#include "solver/Action.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {

namespace physics        { class Variables; class Properties; }
namespace mesh   { class Field; class Dictionary; class Cells; class Space; class Entities; class Entity; class ElementType; class Face2Cell; }

namespace sdm {
class ShapeFunction;
class Term;
class SharedCaches;
/////////////////////////////////////////////////////////////////////////////////////

class sdm_API Term : public cf3::solver::Action {

public: // functions

  /// Contructor
  /// @param name of the component
  Term ( const std::string& name );

  /// Virtual destructor
  virtual ~Term();

  /// Get the class name
  static std::string type_name () { return "Term"; }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

  virtual void initialize() { link_fields(); create_term_field(); }
  void create_term_field();
  virtual void set_entities(const mesh::Entities& entities) { m_entities = entities.handle<mesh::Entities>(); }
  virtual void set_element(const Uint elem_idx) { m_elem_idx = elem_idx; }
  virtual void unset_element() { }
  void set_face(const Handle<mesh::Entities const>& entities, const Uint elem_idx, const Uint face_nb,
                Handle<mesh::Entities const>& neighbour_entities, Uint& neighbour_elem_idx, Uint& neighbour_face_nb,
                Handle<mesh::Entities const>& face_entities, Uint& face_idx, Uint& face_side);

  sdm::SharedCaches& shared_caches() { return *m_shared_caches; }
  Handle<mesh::Entities const> m_entities;
  Uint m_elem_idx;
  Uint m_face_nb;
  Handle<mesh::Field> m_term_field;
  Handle<mesh::Field> m_term_wave_speed_field;
  Handle<sdm::SharedCaches> m_shared_caches;

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @name ACCESSORS
  //@{

  mesh::Field& solution_field()                   { return *m_solution; }

  mesh::Field& residual_field()                   { return *m_residual; }

  mesh::Field& wave_speed_field()                 { return *m_wave_speed; }

  mesh::Field& jacob_det_field()                  { return *m_jacob_det; }

  mesh::Field& delta_field()                      { return *m_delta; }

  physics::Variables& solution_vars()             { return  *m_solution_vars; }

  //@} END ACCESSORS

protected: // function

  void link_fields();

private: // function

  void trigger_physical_model();

protected: // data

  Handle<mesh::Field> m_solution;     ///< access to the solution field

  Handle<mesh::Field> m_residual;     ///< access to the residual field

  Handle<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

  Handle<mesh::Field> m_jacob_det;    ///< access to the jacobian_determinant field

  Handle<mesh::Field> m_delta;        ///< access to the delta field (dx, dy, dz)

  Handle<physics::Variables> m_solution_vars; ///< access to the solution variables

  /// Compute wave speeds in flx_pts
  /// TRUE:  - computation in flx_pts
  ///        - more expensive
  /// FALSE: - computation in sol_pts
  ///        - less accurate (mostly for shocks)
  ///        - WARNING: Must be computed somewhere else!!!!!!!!!
  bool m_compute_wave_speed;
};

/////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_Term_hpp
