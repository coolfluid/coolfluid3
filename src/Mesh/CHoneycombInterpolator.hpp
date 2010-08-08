#ifndef CF_Mesh_CHoneycombInterpolator_hpp
#define CF_Mesh_CHoneycombInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CInterpolator.hpp"
#include "Mesh/CElements.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Mesh_API CHoneycombInterpolator : public CInterpolator
{
public: // typedefs

  typedef boost::shared_ptr<CHoneycombInterpolator> Ptr;
  typedef boost::shared_ptr<CHoneycombInterpolator const> ConstPtr;

private: // typedefs

  typedef boost::multi_array<std::vector<std::pair<const CElements*,Uint> >,3> Honeycomb;
  
public: // functions  
  /// constructor
  CHoneycombInterpolator( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CHoneycombInterpolator"; }
  
  static void defineConfigOptions ( CF::Common::OptionList& options );

private: // functions

  virtual void construct_internal_storage(const CMesh::Ptr& source, const CMesh::Ptr& target);
  virtual void interpolate_field_from_to(const CField::Ptr& source, const CField::Ptr& target);
  
  void create_honeycomb();
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  CMesh::Ptr m_source_mesh;
  CMesh::Ptr m_target_mesh;
  CField::Ptr m_source_field;
  CField::Ptr m_target_field;
  
  Honeycomb m_honeycomb;

}; // end CHoneycombInterpolator

////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CHoneycombInterpolator_hpp
