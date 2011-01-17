// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFieldElements_hpp
#define CF_Mesh_CFieldElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CElements.hpp"

namespace CF {
namespace Common {
  class CLink;
}
namespace Mesh {

  template <typename T> class CTable;
  class CNodes;

////////////////////////////////////////////////////////////////////////////////

/// CFieldElements component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CFieldElements : public CElements {

public: // typedefs

  typedef boost::shared_ptr<CFieldElements> Ptr;
  typedef boost::shared_ptr<CFieldElements const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CFieldElements ( const std::string& name );
  
  /// Initialize the CFieldElements using the given type
  //void initialize(const std::string& element_type_name, CTable<Real>& data);
    
  void add_element_based_storage();
  void add_node_based_storage(CTable<Real>& nodal_data);
  
  /// Virtual destructor
  virtual ~CFieldElements();

  /// Get the class name
  static std::string type_name () { return "CFieldElements"; }

  /// Mutable access to the nodal data (e.g. node coordinates);
  CTable<Real>& data();
  
  /// Const access to the nodal data (e.g. node coordinates)
  const CTable<Real>& data() const;
  
  /// Mutable access to the coordinates
  virtual CNodes& nodes() { return get_geometry_elements().nodes(); }
  
  /// Const access to the coordinates
  virtual const CNodes& nodes() const { return get_geometry_elements().nodes(); }

  CElements& get_geometry_elements();
  const CElements& get_geometry_elements() const;

protected: // data

  std::string m_data_name;

private:
  
};
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFieldElements_hpp
