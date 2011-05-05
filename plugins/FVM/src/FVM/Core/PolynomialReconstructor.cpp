// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/CreateComponent.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementType.hpp"

#include "Math/MathChecks.hpp"
#include "Math/MathConsts.hpp"

#include "FVM/Core/PolynomialReconstructor.hpp"
#include "Mesh/CStencilComputerRings.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Math::MathConsts;
namespace CF {
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < PolynomialReconstructor, Component, LibCore > PolynomialReconstructor_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
PolynomialReconstructor::PolynomialReconstructor ( const std::string& name ) : 
  Component(name),
  m_order(0),
  m_dim(0)
{
  // options
  m_properties.add_option(OptionT<Uint>::create("order","Order","PolynomialReconstructor order",1));
  m_properties.add_option(OptionT<Uint>::create("dimension","Dimension","Dimension of PolynomialReconstructor",2));
  
  m_stencil_computer = create_static_component_ptr<CStencilComputerRings>("stencil_computer");
}

////////////////////////////////////////////////////////////////////////////////

void PolynomialReconstructor::set_cell(const Uint unified_elem_idx)
{
  m_idx = unified_elem_idx;
  if (m_stencils.size() == 0) m_stencils.resize(m_stencil_computer->unified_elements().size());
  std::vector<Uint>& stencil = m_stencils[m_idx];
  if (stencil.size() == 0)    m_stencil_computer->compute_stencil(unified_elem_idx,stencil);

   // DUDx_ave.Vacuum();
   // DUDy_ave.Vacuum();
   // DUDz_ave.Vacuum();
   // D1.Vacuum();
   // D2.Vacuum();
   // D3.Vacuum();
   // DxDx_ave = ZERO;
   // DxDy_ave = ZERO;
   // DxDz_ave = ZERO;
   // DyDy_ave = ZERO;
   // DyDz_ave = ZERO;
   // DzDz_ave = ZERO;
   // D = ZERO;
   
   Component::Ptr elements;
   Uint elem_idx;
   
   boost::tie(elements,elem_idx) = m_stencil_computer->unified_elements().location(m_idx);
   RealMatrix coordinates = elements->as_type<CElements>().get_coordinates(elem_idx);
   RealVector X0(coordinates.cols());
   elements->as_type<CElements>().element_type().compute_centroid(coordinates,X0);

   RealVector X(X0.size());
   RealVector dX(X0.size());
   boost_foreach(const Uint neighbor_idx, stencil)
   {
     if (neighbor_idx != m_idx)
     {
       boost::tie(elements,elem_idx) = m_stencil_computer->unified_elements().location(neighbor_idx);
       X = elements->as_type<CElements>().get_coordinates(elem_idx);
       dX = X-X0;
     }
   }
      // 
      // for ( n2 = 0 ; n2 <= n_pts-1 ; ++n2 ) {
      //    dX =  Grid.Cell[ i_index[n2] ][ j_index[n2] ][ k_index[n2] ].Xc - 
      //       Grid.Cell[i][j][k].Xc;
      //    DU =  W[ i_index[n2] ][ j_index[n2] ][ k_index[n2] ] -  W[i][j][k];
      //    
      //    DUDx_ave += DU*dX.x;
      //    DUDy_ave += DU*dX.y;
      //    DUDz_ave += DU*dX.z;
      //    DxDx_ave += dX.x*dX.x;
      //    DxDy_ave += dX.x*dX.y;
      //    DxDz_ave += dX.x*dX.z;
      //    DyDy_ave += dX.y*dX.y;
      //    DyDz_ave += dX.y*dX.z;
      //    DzDz_ave += dX.z*dX.z;
      //    
      // } /* endfor */
      // 
      // DUDx_ave = DUDx_ave/double(n_pts);
      // DUDy_ave = DUDy_ave/double(n_pts);
      // DUDz_ave = DUDz_ave/double(n_pts);
      // DxDx_ave = DxDx_ave/double(n_pts);
      // DxDy_ave = DxDy_ave/double(n_pts);
      // DxDz_ave = DxDz_ave/double(n_pts);
      // DyDy_ave = DyDy_ave/double(n_pts);
      // DyDz_ave = DyDz_ave/double(n_pts);
      // DzDz_ave = DzDz_ave/double(n_pts);
      // 
      // // (1) Either write a linear solver for 3x3 linear system
      // // (2) Or simplely use cramer's rule for this simple system
      // 
      // D = DxDx_ave*(DyDy_ave* DzDz_ave - DyDz_ave*DyDz_ave) + 
      //     DxDy_ave*(DxDz_ave*DyDz_ave - DxDy_ave*DzDz_ave)+
      //     DxDz_ave*(DxDy_ave*DyDz_ave - DxDz_ave*DyDy_ave);
      // 
      // D1 = DUDx_ave*(DyDy_ave* DzDz_ave - DyDz_ave*DyDz_ave) + 
      //      DUDy_ave*(DxDz_ave*DyDz_ave - DxDy_ave*DzDz_ave)+
      //      DUDz_ave*(DxDy_ave*DyDz_ave - DxDz_ave*DyDy_ave);
      // 
      // D2 =DxDx_ave*(DUDy_ave* DzDz_ave - DUDz_ave*DyDz_ave) + 
      //     DxDy_ave*(DxDz_ave*DUDz_ave - DUDx_ave*DzDz_ave)+
      //     DxDz_ave*(DUDx_ave*DyDz_ave - DxDz_ave*DUDy_ave);
      // 
      // D3 =DxDx_ave*(DyDy_ave* DUDz_ave - DyDz_ave*DUDy_ave) + 
      //     DxDy_ave*(DUDx_ave*DyDz_ave - DxDy_ave*DUDz_ave)+
      //     DxDz_ave*(DxDy_ave*DUDy_ave - DUDx_ave*DyDy_ave);
      // 
      // dWdx[i][j][k] = D1/D;
      // dWdy[i][j][k] = D2/D;
      // dWdz[i][j][k] = D3/D;
}


////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

////////////////////////////////////////////////////////////////////////////////////

