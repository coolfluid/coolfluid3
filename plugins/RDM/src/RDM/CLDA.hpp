// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CLDA_hpp
#define CF_RDM_CLDA_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/Actions/CLoop.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/SupportedTypes.hpp"

#include "RDM/CSchemeLDAT.hpp"
#include "RDM/CSchemeN.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

class RDM_API CLDA : public Solver::Actions::CLoop
{

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct ElementLoop
  {
    private: // data

      /// Region to loop on
      Mesh::CRegion& region;

    public: // functions

      /// Constructor
      ElementLoop( Mesh::CRegion& region_in ) : region(region_in) {}

      /// Operator needed for the loop over shape functions (SF)
      template < typename SF >
      void operator() ( SF& T )
      {
        boost_foreach(Mesh::CElements& elements,
                      Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
        {
          // create an LDA for this specific type

          const Uint order = 5;

          typedef Mesh::Integrators::GaussMappedCoords< order, SF::shape> QD;

          create_static_component< CSchemeLDAT< SF, QD, > >("cell_loop");

          // loop on elements of that type
          op.set_elements(elements);

          if (op.can_start_loop())
          {
            const Uint nb_elem = elements.size();
            for ( Uint elem = 0; elem != nb_elem; ++elem )
            {
              op.select_loop_idx(elem);
              op.template executeT<SF>();
            }
          }
        }
      }

  }; // ElementLoop

public: // typedefs

  typedef boost::shared_ptr< CLDA > Ptr;
  typedef boost::shared_ptr< CLDA const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CLDA ( const std::string& name ) :
    CLoop(name)
  {
    regist_typeinfo(this);
  }

  /// Virtual destructor
  virtual ~CLDA() {}

  /// Get the class name
  static std::string type_name () { return "CLDA"; }

  /// Execute the loop for all elements
  virtual void execute()
  {
    boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
    {
      CFinfo << region->full_path().string() << CFendl;
      
      CLDA::ElementLoop elem_loop(*m_action,*region);

      boost::mpl::for_each< RDM::CellTypes >(elem_loop);
    }
  }

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CLDA_hpp
