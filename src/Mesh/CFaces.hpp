// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFaces_hpp
#define CF_Mesh_CFaces_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CFaces component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CFaces : public CElements {

public: // typedefs

  typedef boost::shared_ptr<CFaces> Ptr;
  typedef boost::shared_ptr<CFaces const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CFaces ( const std::string& name );
  
  /// Initialize the CFaces using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& coordinates);

  /// Initialize the CFaces using the given type
  virtual void initialize(const std::string& element_type_name, CNodes& nodes);
    
  /// Virtual destructor
  virtual ~CFaces();

  /// Get the class name
  static std::string type_name () { return "CFaces"; }

};

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFaces_hpp
