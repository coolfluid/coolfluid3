// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <set>
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CreateComponent.hpp"

#include "Math/MathFunctions.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFaces.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/ElementType.hpp"

#include "FVM/BuildGhostStates.hpp"
#include "FVM/GhostCells.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::MathFunctions;

namespace CF {
namespace FVM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BuildGhostStates, CMeshTransformer, LibFVM > BuildGhostStates_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BuildGhostStates::BuildGhostStates ( const std::string& name ) : 
  CMeshTransformer(name)
{
  mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void BuildGhostStates::execute()
{
  CMesh& mesh = *m_mesh.lock();
  
  recursive_build_ghost_states(mesh.topology());
  
}

////////////////////////////////////////////////////////////////////////////////

void BuildGhostStates::recursive_build_ghost_states(Component& parent)
{
  CMesh& mesh = *m_mesh.lock();
  
  Uint dim = mesh.nodes().coordinates().row_size();
  std::string dim_str = to_str(dim);
  std::vector<Uint> dummy(1);
  boost_foreach(CRegion& region, find_components<CRegion>(parent) )
  {
    recursive_build_ghost_states(region);
    
    if ( find_components<CFaces>(region).size() != 0 )
    {
      // this region has faces to build ghost cells at

      CRegion::Ptr boundary_faces = region.create_component<CRegion>("boundary_faces");
      boost_foreach(CFaces& faces, find_components<CFaces>(region) )
        faces.move_to(boundary_faces);

      Uint nb_faces(0);
      boost_foreach(CFaces& faces, find_components<CFaces>(*boundary_faces) )
        nb_faces += faces.size();
        
      CRegion& ghost_states = *region.create_component<CRegion>("ghost_states");
      GhostCells& ghosts = *ghost_states.create_component<GhostCells>("Point");
      ghosts.initialize("CF.Mesh.SF.Point"+dim_str+"DLagrangeP1",mesh.nodes());
      CTable<Uint>& ghost_elem_connectivity = ghosts.connectivity_table();
      ghost_elem_connectivity.resize(nb_faces);
      ghost_elem_connectivity.set_row_size(1);
      CTable<Real>::Buffer nodes_buffer = mesh.nodes().coordinates().create_buffer();
      
      Uint conn_idx(0);
      
      boost_foreach(CFaces& faces, find_components<CFaces>(*boundary_faces) )
      {
        // CFinfo << faces.full_path().path() << CFendl;
        CFaceCellConnectivity::Ptr face2cell_ptr = find_component_ptr<CFaceCellConnectivity>(faces);
        if (is_not_null(face2cell_ptr))
        {
          CFaceCellConnectivity& face2cell = *face2cell_ptr;
          Uint unified_ghost_idx(find_component<CUnifiedData>(face2cell).size());
          //CFinfo << "size before = " << find_component<CUnifiedData<CCells> >(face2cell).size() << CFendl;
          boost_foreach(CCells& cells, find_components<CCells>(ghost_states))
            face2cell.cells().add(cells);
          //CFinfo << "size after = " << find_component<CUnifiedData<CCells> >(face2cell).size() << CFendl;
          CTable<Uint>& f2c_connectivity = find_component<CTable<Uint> >(face2cell);
          f2c_connectivity.set_row_size(2);
          std::vector<CCells::Ptr> cells;
          boost_foreach(Component::Ptr comp, face2cell.cells().components())
            cells.push_back(comp->as_ptr<CCells>());
          Uint comp_idx(0);
          Uint cell_idx(0);

          RealVector normal(dim);
          RealVector ghost_coord(dim);
          

          for (Uint face=0; face<face2cell.size(); ++face)
          {
            //CFinfo << "face " << faces.parent()->parent()->name() << "/" << faces.name() << "["<<face<<"]" << CFendl;
            Uint unified_elem_idx = face2cell.connectivity()[face][0]; // this is the inner cell of the boundary
            boost::tie(comp_idx,cell_idx) = face2cell.cells().location_idx(unified_elem_idx);
            std::string cell_type = cells[comp_idx]->element_type().element_type_name();

            RealMatrix cell_coordinates = cells[comp_idx]->get_coordinates(cell_idx);
            RealVector centroid(cell_coordinates.cols());
            cells[comp_idx]->element_type().compute_centroid(cell_coordinates,centroid);
            //CFLogVar(centroid.transpose());
            if (faces.element_type().dimensionality() == 0) // cannot compute normal from element_type
            {
              RealVector face_coord(dim); 
              for (Uint d=0; d<dim; ++d)
                face_coord[d] = mesh.nodes().coordinates()[faces.connectivity_table()[face][0]][d];
              normal = face_coord - centroid;
              normal.normalize();
              ghost_coord =  centroid + 2.0*(face_coord-centroid);
            }
            else
            {
              RealMatrix face_coordinates = faces.get_coordinates(face);
              faces.element_type().compute_normal(face_coordinates,normal);
              //CFinfo << "face_coordinates = \n" << face_coordinates << CFendl;
              // The equation of the plane containing the boundary face
              // and the given node (xp, yp, zp) (first node of the face),
              // with normal (a,b,c) is
              // a*x + b*y + c*z + k = 0
              // with k = -a*xp - b*yp - c*zp
              Real k = 0.0;
              // if (nodes.size() < 4) {
              k = - face_coordinates.row(0).dot(normal);

              // t is parameter for vectorial representation of a straight line
              // in space
              // t = (a*xM + b*yM + c*zM + k)/(a*a + b*b + c*c)

              const Real n2 = normal.dot(normal);
              const Real t = (centroid.dot(normal) + k)/n2;
               // The point N symmetric to M (position of the other state,
               // internal neighbor of the face) with respect to the
               // given plane is given by
               // (xN, yN, zN) = (xM, yM, zM) - 2*t*(a,b,c)
              ghost_coord = centroid - 2.*t*normal;
              // CFLogVar(ghost_coord.transpose());
            }

            dummy[0] = nodes_buffer.add_row(ghost_coord);
            ghost_elem_connectivity.set_row(conn_idx++,dummy);
            f2c_connectivity[face][1]=unified_ghost_idx++;
          }
        }
      }      
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

