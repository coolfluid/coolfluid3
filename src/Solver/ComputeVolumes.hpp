#ifndef CF_ComputeVolumes_hpp
#define CF_ComputeVolumes_hpp

#include "Mesh/CRegion.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/ElementNodes.hpp"


/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

/////////////////////////////////////////////////////////////////////////////////////

struct ComputeVolumes
{
  CArray::Ptr volumes;
  CArray::Ptr coordinates;
  CTable::Ptr connectivity_table;

  ComputeVolumes () { }
  
  void setup (CElements& field_elements )
  {
    volumes = field_elements.elemental_data().get_type<CArray>();
    volumes->array().resize(boost::extents[field_elements.elements_count()][1]);
    
    coordinates = field_elements.get_geometry_elements().coordinates().get_type<CArray>();
    connectivity_table = field_elements.get_geometry_elements().connectivity_table().get_type<CTable>();
    
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(volumes.get());
    std::vector<RealVector> nodes;
    fill_node_list( std::inserter(nodes, nodes.begin()), *coordinates, *connectivity_table, elem );
    volumes->array()[elem][0] = SFType::volume( nodes );
  }

};
  
/////////////////////////////////////////////////////////////////////////////////////

struct OutputVolumes
{
  CArray::Ptr volumes;

  OutputVolumes () { }
  
  void setup (CElements& field_elements )
  {
    volumes = field_elements.elemental_data().get_type<CArray>();
    CFinfo << field_elements.full_path().string() << CFendl;
  }
  
  template < typename SFType >
  void execute ( Uint elem )
  {
    cf_assert(volumes.get());
    CFinfo << "   volume["<<elem<<"] = " << volumes->array()[elem][0] << CFendl;
  }
  
};
  
/////////////////////////////////////////////////////////////////////////////////////

template < typename OP1, typename OP2 >
struct OperationMerge
{
  OP1 m_op1;
  OP2 m_op2;

  void setup (CElements& field_elements )
  {
    m_op1.setup(field_elements);
    m_op2.setup(field_elements);
  }
  
  template < typename EType >
  void execute (  Uint elem )
  {
    m_op1.template execute<EType>( elem );
    m_op2.template execute<EType>( elem );
  }
};

/////////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_ComputeVolumes_hpp
