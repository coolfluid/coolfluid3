// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_BCDirichlet_hpp
#define CF_FVM_BCDirichlet_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitter.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CTable.hpp"
#include "Common/Foreach.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldView;
}
namespace FVM {

class FVM_API ConnectedFieldView
{
public:
  
  void initialize(Mesh::CField2::Ptr field, Mesh::CFaces::Ptr faces)
  {
    set_field(field);
    set_elements(faces);
  }
  
  void set_field(Mesh::CField2::Ptr field) { return set_field(*field); }
  void set_field(Mesh::CField2& field) { m_field = field.as_type<Mesh::CField2>(); }

  bool set_elements(Mesh::CFaces::Ptr faces)
  {
    cf_assert(is_not_null(faces));
    return set_elements(*faces);
  }
  
  bool set_elements(Mesh::CFaces& faces)
  {
    if (is_null(m_field.lock()))
      throw Common::SetupError(FromHere(),"set_field(field) must be called first");

    m_faces = faces.as_type<Mesh::CFaces>();
    Mesh::CFaceCellConnectivity::Ptr f2c = faces.get_child<Mesh::CFaceCellConnectivity>("cell_connectivity");
    m_views.resize(f2c->cells_components().size());
    index_foreach(i,Mesh::CCells::Ptr cells, f2c->cells_components())
    {
      cf_assert(is_not_null(cells));
      m_views[i] = Common::allocate_component<Mesh::CFieldView>("view");
      m_views[i]->initialize(*m_field.lock(), cells);
    }
    return true;
  }

  Mesh::CTable<Real>::Row operator[](const Uint face_idx)
  {
    cf_assert(m_views.size());
    Mesh::CFaces& faces = *m_faces.lock();
    Mesh::CFaceCellConnectivity& f2c = *faces.get_child<Mesh::CFaceCellConnectivity>("cell_connectivity");
    const Uint cell = f2c.elements(face_idx)[0];
    boost::tie(cells_comp_idx,cell_idx) = f2c.element_loc_idx(cell);
    cf_assert( cells_comp_idx < m_views.size());
    cf_assert( is_not_null(m_views[cells_comp_idx]) );
    cf_assert( cell_idx < m_views[cells_comp_idx]->size() );
    return (*m_views[cells_comp_idx])[cell_idx];
  }
  
  Mesh::CField2& field() { return *m_field.lock(); }
  
private:

  boost::weak_ptr<Mesh::CFaces> m_faces;
  std::vector<Mesh::CFieldView::Ptr> m_views;
  
  boost::weak_ptr<Mesh::CField2> m_field;

  Uint cells_comp_idx;
  Uint cell_idx;

};

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API BCDirichlet : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<BCDirichlet> Ptr;
  typedef boost::shared_ptr<BCDirichlet const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  BCDirichlet ( const std::string& name );

  /// Virtual destructor
  virtual ~BCDirichlet() {};

  /// Get the class name
  static std::string type_name () { return "BCDirichlet"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();

  void trigger_elements();
  
private: // data
  
  ConnectedFieldView m_connected_solution;
  
  Real m_rho;
  Real m_u;
  Real m_p;
  Real m_gm1;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_BCDirichlet_hpp
