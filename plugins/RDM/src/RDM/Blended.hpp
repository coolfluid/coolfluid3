// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_Blended_hpp
#define CF_RDM_Blended_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CRegion.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/Action.hpp"
#include "RDM/SupportedTypes.hpp"

#include "RDM/CSchemeB.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

template < typename PHYS >
class RDM_API Blended : public RDM::Action
{

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct ElementLoop
  {
    /// region to loop on
    Mesh::CRegion& region;
    /// component containing the element loop
    Blended& comp;

    public: // functions

      /// Constructor
      ElementLoop( Blended& comp_in, Mesh::CRegion& region_in ) : comp(comp_in), region(region_in) {}

      /// Operator needed for the loop over element types, identified by shape functions (SF)
      template < typename SF >
      void operator() ( SF& T );


  }; // ElementLoop

public: // typedefs

  typedef boost::shared_ptr< Blended > Ptr;
  typedef boost::shared_ptr< Blended const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  Blended ( const std::string& name );

  /// Virtual destructor
  virtual ~Blended();

  /// Get the class name
  static std::string type_name () { return "Blended<" + PHYS::type_name() + ">"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Blended_hpp
