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
#include "Mesh/Integrators/GaussImplementation.hpp"

#include "Solver/Actions/CLoop.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/SupportedTypes.hpp"

#include "RDM/CSchemeLDAT.hpp"
//#include "RDM/CSchemeN.hpp"

#include "RDM/RotationAdv2D.hpp"
#include "RDM/Burgers2D.hpp"

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
    /// region to loop on
    Mesh::CRegion& region;
    /// component containing the element loop
    CLDA& comp;

    public: // functions

      /// Constructor
      ElementLoop( CLDA& comp_in, Mesh::CRegion& region_in ) : comp(comp_in), region(region_in) {}

      /// Operator needed for the loop over element types, identified by shape functions (SF)
      template < typename SF >
      void operator() ( SF& T )
      {
//        CFinfo << " -- CLDA in [" << region.full_path().string() << "]" << CFendl;

        boost_foreach(Mesh::CElements& elements,
                      Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
        {
//          CFinfo << " --- elements " << elements.full_path().string() << CFendl;

          // create an LDA for this specific type

          const Uint order = 2;

          typedef Mesh::Integrators::GaussMappedCoords< order, SF::shape> QD;
          typedef CSchemeLDAT< SF, QD, Burgers2D > SchemeT;

          // get the scheme
          typename SchemeT::Ptr scheme = comp.get_child<SchemeT>( SchemeT::type_name() );
          if( is_null(scheme) )
            scheme = comp.create_component< SchemeT >( SchemeT::type_name() );

          // loop on elements of that type
          scheme->set_elements(elements);

          if (scheme->can_start_loop())
          {
            const Uint nb_elem = elements.size();
            for ( Uint elem = 0; elem != nb_elem; ++elem )
            {
              scheme->select_loop_idx(elem);
              scheme->execute();
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
  CLDA ( const std::string& name );

  /// Virtual destructor
  virtual ~CLDA();

  /// Get the class name
  static std::string type_name () { return "CLDA"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CLDA_hpp
