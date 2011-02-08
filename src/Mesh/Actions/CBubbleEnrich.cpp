// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/Actions/CBubbleEnrich.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {
  
  using namespace Common;
  using namespace Math::MathFunctions;
  
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBubbleEnrich, CMeshTransformer, LibActions> CBubbleEnrich_Builder;

//////////////////////////////////////////////////////////////////////////////

CBubbleEnrich::CBubbleEnrich( const std::string& name )
: CMeshTransformer(name)
{
   
  properties()["brief"] = std::string("Enriches a Lagrangian space with bubble functions in each element");
  std::string desc = "  Usage: CBubbleEnrich \n\n";
	properties()["description"] = desc;
}

/////////////////////////////////////////////////////////////////////////////

std::string CBubbleEnrich::brief_description() const
{
  return properties()["brief"].value<std::string>();
}

/////////////////////////////////////////////////////////////////////////////

  
std::string CBubbleEnrich::help() const
{
  return "  " + properties()["brief"].value<std::string>() + "\n" + properties()["description"].value<std::string>();
}  
  
/////////////////////////////////////////////////////////////////////////////

void CBubbleEnrich::transform( const CMesh::Ptr& meshptr,
                               const std::vector<std::string>& args)
{

  CMesh& mesh = *meshptr;

  boost_foreach(CRegion& region, find_components_recursively<CRegion>(mesh) )
    boost_foreach(CElements& elements, find_components_recursively<CElements>(region))
  {
    // backup the connectivity table
    CTable<Uint>::Ptr backup = this->create_component< CTable<Uint> > ("backup");
    CTable<Uint>::ArrayT backtable = backup->array();
    backtable = elements.connectivity_table();

    // compute new nb cols
    const Uint currnodes = elements.connectivity_table().row_size();
    const Uint newnbcols = currnodes + 1;
    // resize and keep same nb of rows (nb elements)
    elements.connectivity_table().set_row_size(newnbcols);

    // get connectivity table
    CTable<Uint>::ArrayT conntable = elements.connectivity_table().array();

    // get coordinates
    CTable<Real>& coords = elements.nodes().coordinates();

    const Uint nb_elem = elements.size();
    const Uint dim = coords.row_size();

    // average coordinate
    RealVector centroid (dim);
    centroid.setZero();

    // get a buffer to the coordinates
    CTable<Real>::Buffer buf = coords.create_buffer();

    for ( Uint elem = 0; elem != nb_elem; ++elem )
    {
      // compute average of node coordinates
      for ( Uint n = 0; n != currnodes; ++n )
        for ( Uint d = 0; d != dim; ++d )
         centroid[d] += coords[elem][d];

      // add a node to the nodes structure
      Uint idx = buf.add_row( centroid );

      // modify this row
      CTable<Uint>::Row row = conntable[elem];
      // add the index of the node to the connectivity table
      for ( Uint n = 0; n != currnodes; ++n )
        row(n) = backtable(n);
      row(currnodes) = idx;
    }

    // delete backup table
    this->remove_component("backup");

    // remove the old shape function
    const ElementType& etype = elements.element_type();
    elements.remove_component(etype.name());

    // add the new shape function
    elements.set_element_type( "CF.Mesh.SF.Triag2DLagrangeP2B" );
  }
}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
