// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_LagrangeP2B_LibLagrangeP2B_hpp
#define CF_Mesh_LagrangeP2B_LibLagrangeP2B_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"
#include "Mesh/LagrangeP2B/API.hpp"

namespace CF {
namespace Mesh {

/// @brief namespace holding LagrangeP2B shape functions and elements
/// @author Willem Deconinck
namespace LagrangeP2B {

////////////////////////////////////////////////////////////////////////////////

/// Shape functions module for LagrangeP2B
/// @author Tiago Quintino, Willem Deconinck
class Mesh_LagrangeP2B_API LibLagrangeP2B : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibLagrangeP2B> Ptr;
  typedef boost::shared_ptr<LibLagrangeP2B const> ConstPtr;

  /// Constructor
  LibLagrangeP2B ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.Mesh.LagrangeP2B"; }


  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "LagrangeP2B"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the shape functions.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibLagrangeP2B"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end LibLagrangeP2B

////////////////////////////////////////////////////////////////////////////////

} // LagrangeP2B
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_LagrangeP2B_LibLagrangeP2B_hpp
