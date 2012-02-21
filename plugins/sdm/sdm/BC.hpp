// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the BCs of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_BC_hpp
#define cf3_sdm_BC_hpp

#include "common/Table_fwd.hpp"
#include "common/BoostArray.hpp"

#include "math/MatrixTypes.hpp"
#include "mesh/Entities.hpp"
#include "solver/Action.hpp"

#include "sdm/LibSDM.hpp"

namespace cf3 {

namespace mesh   { class Field; class Dictionary; class Cells; class Space; class Entities; class Entity; class ElementType; class Face2Cell; }

namespace sdm {

class SharedCaches;

/////////////////////////////////////////////////////////////////////////////////////

class sdm_API BC : public cf3::solver::Action {

public: // typedefs

  /// provider
  
public: // functions

  /// Contructor
  /// @param name of the component
  BC ( const std::string& name );

  /// Virtual destructor
  virtual ~BC();

  /// Get the class name
  static std::string type_name () { return "BC"; }

  //////////////////////////////////////////////////////////////////////////////////////////////////////

  virtual void execute() = 0;
  virtual void initialize() { link_fields(); }
  virtual void set_face_entities(const mesh::Entities& face_entities) { m_face_entities = face_entities.handle<mesh::Entities>(); }
  virtual void set_face_element(const Uint face_elem_idx) { m_face_elem_idx = face_elem_idx; }
  virtual void unset_face_element() {}
  void find_inner_cell(const Handle<mesh::Entities const>& face_entities, const Uint face_idx,
                     Handle<mesh::Entities const>& cell_entities, Uint& cell_idx, Uint& cell_face_nb);

  sdm::SharedCaches& shared_caches() { return *m_shared_caches; }
  Uint m_face_elem_idx;
  Handle<mesh::Entities const> m_face_entities;
  Handle<sdm::SharedCaches> m_shared_caches;

  //////////////////////////////////////////////////////////////////////////////////////////////////////
  /// @name ACCESSORS
  //@{

  mesh::Field& solution_field()                   { return *m_solution; }

  mesh::Field& residual_field()                   { return *m_residual; }

  mesh::Field& wave_speed_field()                 { return *m_wave_speed; }

  mesh::Field& jacob_det_field()                  { return *m_jacob_det; }

  mesh::Field& delta_field()                      { return *m_delta; }

  //@} END ACCESSORS

protected: // function

  void link_fields();

protected: // data

  Handle<mesh::Field> m_solution;     ///< access to the solution field

  Handle<mesh::Field> m_residual;     ///< access to the residual field

  Handle<mesh::Field> m_wave_speed;   ///< access to the wave_speed field

  Handle<mesh::Field> m_jacob_det;    ///< access to the jacobian_deBCinant field

  Handle<mesh::Field> m_delta;        ///< access to the delta field (dx, dy, dz)
};

/////////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_BC_hpp
