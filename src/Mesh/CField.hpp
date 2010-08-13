#ifndef CF_Mesh_CField_hpp
#define CF_Mesh_CField_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/MeshAPI.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"


namespace CF {
namespace Common 
{
  class CLink;
}
namespace Mesh {
  
  class CRegion;

////////////////////////////////////////////////////////////////////////////////

/// Field component class
/// This class stores fields which can be applied 
/// to fields (Cfield)
/// @author Willem Deconinck, Tiago Quintino
class Mesh_API CField : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CField> Ptr;
  typedef boost::shared_ptr<CField const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CField ( const CName& name );

  /// Virtual destructor
  virtual ~CField();

  /// Get the class name
  static std::string type_name () { return "CField"; }

  /// Configuration Options
  static void defineConfigOptions ( Common::OptionList& options ) {}

  // functions specific to the CField component
  
  /// create a Cfield component
  /// @param name of the field
  CField& create_field (const std::string& field_name, CRegion& support, const Uint dim, std::map<CArray*,CArray*>& data_for_coordinates);
  
  /// create a CElements component, initialized to take connectivity data for the given type
  /// @param name of the field
  /// @param element_type_name type of the elements
  CElements& create_elements (const std::string& element_type_name, CArray::Ptr data);
  
  /// create a coordinates component, initialized with the coordinate dimension
  /// @param name of the field
  /// @param element_type_name type of the elements  
  CArray& create_data(const Uint& dim);
  
  const CRegion& support() const;
  
  /// @return the number of elements stored in this field, including any subfields
  Uint recursive_elements_count() const
  {
    Uint elem_count = 0;
    BOOST_FOREACH(const CElements& elements, recursive_range_typed<CElements>(*this))
    {
      elem_count += elements.elements_count();
    }
    return elem_count;
  }
  
  /// @return the number of elements stored in this field, including any subfields
  template <typename Predicate>
  Uint recursive_filtered_elements_count(const Predicate& pred) const
  {
    Uint elem_count = 0;
    BOOST_FOREACH(const CElements& elements, recursive_filtered_range_typed<CElements>(*this,pred))
    {
      elem_count += elements.elements_count();
    }
    return elem_count;
  }
  
  std::string field_name() const { return m_field_name; }
  
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private:
  
  std::string m_field_name;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CField_hpp
