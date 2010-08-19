#ifndef CF_Mesh_CFieldElements_hpp
#define CF_Mesh_CFieldElements_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {

  class CArray;

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
  CFieldElements ( const CName& name );
  
  /// Initialize the CFieldElements using the given type
  //void initialize(const std::string& element_type_name, CArray& data);
    
  void add_element_based_storage();
  void add_node_based_storage(CArray& nodal_data);
  
  void initialize(CElements& element_in);

  /// Virtual destructor
  virtual ~CFieldElements();

  /// Get the class name
  static std::string type_name () { return "CFieldElements"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}
  
  /// Mutable access to the nodal data (e.g. node coordinates);
  CArray& nodal_data();
  
  /// Const access to the nodal data (e.g. node coordinates)
  const CArray& nodal_data() const;

  /// Mutable access to the elemental data (e.g. centroid)
  CArray& elemental_data();
  
  /// Const access to the elemental data (e.g. centroid)
  const CArray& elemental_data() const;
  
  /// Mutable access to the coordinates
  virtual CArray& coordinates() { return get_geometry_elements().coordinates(); }
  
  /// Const access to the coordinates
  virtual const CArray& coordinates() const { return get_geometry_elements().coordinates(); }
    
  CElements& get_geometry_elements();
  const CElements& get_geometry_elements() const;
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

protected: // data
    
  std::string m_nodal_data_name;
  std::string m_elemental_data_name;
  
};
  
////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFieldElements_hpp
