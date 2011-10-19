// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_CCells_hpp
#define cf3_Mesh_CCells_hpp

////////////////////////////////////////////////////////////////////////////////


#include "Mesh/CElements.hpp"

namespace cf3 {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// CCells component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CCells : public CElements {

public: // typedefs

  typedef boost::shared_ptr<CCells> Ptr;
  typedef boost::shared_ptr<CCells const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CCells ( const std::string& name );
  
  /// Initialize the CCells using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& coordinates);

  /// Initialize the CCells using the given type
  virtual void initialize(const std::string& element_type_name, Geometry& geometry);
    
  /// Virtual destructor
  virtual ~CCells();

  /// Get the class name
  static std::string type_name () { return "CCells"; }

};

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_CCells_hpp
