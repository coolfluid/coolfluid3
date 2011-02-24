// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_RDM_CBlended_hpp
#define CF_RDM_CBlended_hpp

#include <boost/mpl/for_each.hpp>

#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CRegion.hpp"

#include "RDM/LibRDM.hpp"
#include "RDM/CAction.hpp"
#include "RDM/SupportedTypes.hpp"

#include "RDM/CSchemeB.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace RDM {

/////////////////////////////////////////////////////////////////////////////////////

template < typename PHYS >
class RDM_API CBlended : public RDM::Action
{

  /// Looper defines a functor taking the type that boost::mpl::for_each
  /// passes. It is the core of the looping mechanism.
  struct ElementLoop
  {
    /// region to loop on
    Mesh::CRegion& region;
    /// component containing the element loop
    CBlended& comp;

    public: // functions

      /// Constructor
      ElementLoop( CBlended& comp_in, Mesh::CRegion& region_in ) : comp(comp_in), region(region_in) {}

      /// Operator needed for the loop over element types, identified by shape functions (SF)
      template < typename SF >
      void operator() ( SF& T );


  }; // ElementLoop

public: // typedefs

  typedef boost::shared_ptr< CBlended > Ptr;
  typedef boost::shared_ptr< CBlended const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CBlended ( const std::string& name );

  /// Virtual destructor
  virtual ~CBlended();

  /// Get the class name
  static std::string type_name () { return "CBlended<" + PHYS::type_name() + ">"; }

  /// Execute the loop for all elements
  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CBlended_hpp
