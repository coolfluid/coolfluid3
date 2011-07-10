// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/foreach.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/FindComponents.hpp"
#include "Common/Foreach.hpp"
#include "Common/StreamHelpers.hpp"

#include "Mesh/Actions/CBubbleRemove.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"

#include "Math/MathFunctions.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

  using namespace Common;
  using namespace Math::MathFunctions;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CBubbleRemove, CMeshTransformer, LibActions> CBubbleRemove_Builder;

//////////////////////////////////////////////////////////////////////////////

CBubbleRemove::CBubbleRemove( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Enriches a Lagrangian space with bubble functions in each element");
  properties()["description"] = std::string("  Usage: CBubbleRemove");
}

/////////////////////////////////////////////////////////////////////////////

std::string CBubbleRemove::brief_description() const
{
  return properties().value<std::string>("brief");
}

/////////////////////////////////////////////////////////////////////////////


std::string CBubbleRemove::help() const
{
  return "  " + properties().value<std::string>("brief") + "\n" +
      properties().value<std::string>("description");
}

/////////////////////////////////////////////////////////////////////////////

void CBubbleRemove::execute()
{
  CFinfo << "---------------------------------------------------" << CFendl;
  CFinfo << FromHere().str() << CFendl;

  /// @todo only supports simplexes

  CMesh& mesh = *m_mesh.lock();

  boost_foreach(CElements& elements, find_components_recursively<CCells>(mesh))
  {
    CFinfo << "---------------------------------------------------" << CFendl;

    CFinfo << elements.uri().string() << CFendl;

//    CFinfo << "elems size " << elements.size() << CFendl;
//    CFinfo << "conn size "  << elements.node_connectivity().size() << " x "
//                            << elements.node_connectivity().row_size() << CFendl;
//    CFinfo << "coords size "<< elements.nodes().coordinates().size() << " x "
//                            << elements.nodes().coordinates().row_size() << CFendl;

    // backup the connectivity table
    CTable<Uint>::Ptr backup = this->create_component_ptr< CTable<Uint> > ("backup");

    backup->set_row_size(elements.node_connectivity().row_size()); // size appropriately
    backup->resize(elements.node_connectivity().size());           // size appropriately

    *backup = elements.node_connectivity(); /* copy table */
    CTable<Uint>::ArrayT& backtable = backup->array();

    // compute new nb cols
    const Uint currnodes = elements.node_connectivity().row_size();
    const Uint newnbcols = currnodes - 1;
    const Uint lastcol = currnodes - 1;
    // resize and keep same nb of rows (nb elements)
    elements.node_connectivity().set_row_size(newnbcols);

//    CFinfo << "new conn size "  << elements.node_connectivity().size() << " x "
//                                << elements.node_connectivity().row_size() << CFendl;

    // get connectivity table
    CConnectivity::ArrayT& conntable = elements.node_connectivity().array();

    // get coordinates
    CTable<Real>& coords = elements.nodes().coordinates();

    const Uint nb_elem = elements.size();
    //const Uint dim = coords.row_size();

    // get a buffer to the coordinates
    CTable<Real>::Buffer buf = coords.create_buffer();

    // loop on the elements
    for ( Uint elem = 0; elem != nb_elem; ++elem )
    {

//      CFinfo << "elem " << elem << CFendl;

//      for ( Uint n = 0; n != currnodes; ++n )
//        CFinfo << " " << backtable[elem][n];
//      CFinfo << CFendl;

      // remove a node of the nodes structure
      buf.rm_row ( backtable[elem][lastcol] );

      // update the connectivity table without last column
      for ( Uint n = 0; n != currnodes-1; ++n )
        conntable[elem][n] = backtable[elem][n];
    }

    // flush the coordinate buffer
    buf.flush();

    // delete backup table
    this->remove_component("backup");

    // change the shape function
    elements.configure_option("element_type" , std::string("CF.Mesh.SF.Triag2DLagrangeP2") );

  }

}

//////////////////////////////////////////////////////////////////////////////

} // Actions
} // Mesh
} // CF
