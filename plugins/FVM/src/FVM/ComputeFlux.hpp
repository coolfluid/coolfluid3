// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_ComputeFlux_hpp
#define CF_FVM_ComputeFlux_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitter.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CTable.hpp"
#include "Common/Foreach.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldView;
}
namespace FVM {

class FVM_API ConnectedCellFaceFieldView
{
public:
  
  void initialize(Mesh::CField2::Ptr field, Mesh::CCellFaces::Ptr faces)
  {
    set_field(field);
    set_elements(faces);
  }
  
  void set_field(Mesh::CField2::Ptr field) { return set_field(*field); }
  void set_field(Mesh::CField2& field) { m_field = field.as_type<Mesh::CField2>(); }

  void set_elements(Mesh::CCellFaces::Ptr faces)
  {
    cf_assert(is_not_null(faces));
    set_elements(*faces);
  }
  
  void set_elements(Mesh::CCellFaces& faces)
  {
    if (is_null(m_field.lock()))
      throw Common::SetupError(FromHere(),"set_field(field) must be called first");
      
    m_faces = faces.as_type<Mesh::CCellFaces>();
    m_views.resize(faces.cell_connectivity().cells_components().size());
    index_foreach(i,Mesh::CCells::Ptr cells, faces.cell_connectivity().cells_components())
    {
      cf_assert(is_not_null(cells));
      m_views[i] = Common::allocate_component<Mesh::CFieldView>("view");
      m_views[i]->initialize(*m_field.lock(), cells);
    }
  }

  std::vector<Mesh::CTable<Real>::Row> operator[](const Uint face_idx)
  {
    cf_assert(m_views.size());
    std::vector<Mesh::CTable<Real>::Row> vec;
    Mesh::CCellFaces& faces = *m_faces.lock();
    boost_foreach(const Uint cell, faces.cell_connectivity().elements(face_idx))
    {
      boost::tie(cells_comp_idx,cell_idx) = faces.cell_connectivity().element_loc_idx(cell);
      cf_assert(cells_comp_idx < m_views.size());
      cf_assert( is_not_null(m_views[cells_comp_idx]) );
      cf_assert(cell_idx < m_views[cells_comp_idx]->size());
      vec.push_back((*m_views[cells_comp_idx])[cell_idx]);
    }
    return vec;
  }
  
  Mesh::CField2& field() { return *m_field.lock(); }
  
private:

  boost::weak_ptr<Mesh::CCellFaces> m_faces;
  std::vector<Mesh::CFieldView::Ptr> m_views;
  
  boost::weak_ptr<Mesh::CField2> m_field;

  Uint cells_comp_idx;
  Uint cell_idx;

};

///////////////////////////////////////////////////////////////////////////////////////

class FVM_API ComputeFlux : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<ComputeFlux> Ptr;
  typedef boost::shared_ptr<ComputeFlux const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeFlux ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeFlux() {};

  /// Get the class name
  static std::string type_name () { return "ComputeFlux"; }

  /// execute the action
  virtual void execute ();

private: // helper functions

  void config_solution();
  void config_residual();
  void config_advection();
  void config_area();
  void config_normal();

  void trigger_elements();
  
private: // data
  
  ConnectedCellFaceFieldView m_connected_residual;
  ConnectedCellFaceFieldView m_connected_solution;
  ConnectedCellFaceFieldView m_connected_advection;
  
  Mesh::CScalarFieldView::Ptr m_face_area;
  Mesh::CFieldView::Ptr m_face_normal;
  
  RealVector m_flux;
  Real m_wave_speed_left;
  Real m_wave_speed_right;
  RealVector m_normal;
  RealVector m_state_L;
  RealVector m_state_R;
  
  enum {LEFT=0,RIGHT=1};
  
  boost::shared_ptr<RoeFluxSplitter> m_fluxsplitter;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_ComputeFlux_hpp
