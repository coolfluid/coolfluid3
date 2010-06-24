#ifndef CF_Mesh_ElementNodes_hpp
#define CF_Mesh_ElementNodes_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Math/RealVector.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Provide a mutable view of the nodes of a single element, offering operator[] and size() functions
/// Copying this creates a shallow copy and modifying a copy modifies the original coordinate data
struct ElementNodeView
{
  ElementNodeView() {}
  ElementNodeView(CArray& coordinates, const CTable::ConstRow& connectivity);
  Uint size() const;
  CArray::ConstRow operator[](const Uint idx) const;
  CArray::Row operator[](const Uint idx);
private:
  friend class ConstElementNodeView; // Allows construction of the const version from the mutable version
  struct Data;
  boost::shared_ptr<Data> m_data;
};

////////////////////////////////////////////////////////////////////////////////

/// Provide a constant view of the nodes of a single element, offering operator[] and size() functions
/// Copying this creates a shallow copy.
struct ConstElementNodeView
{
  ConstElementNodeView() {}
  ConstElementNodeView(CArray const& coordinates, const CTable::ConstRow& connectivity);
  ConstElementNodeView(const ElementNodeView& elementNodeVector);
  Uint size() const;
  CArray::ConstRow operator[](const Uint idx) const;
  CArray::Row operator[](const Uint idx);
private:
  struct Data;
  boost::shared_ptr<Data> m_data;
};

////////////////////////////////////////////////////////////////////////////////

/// Copy of the node coordinates
typedef std::vector<RealVector> ElementNodeVector;

////////////////////////////////////////////////////////////////////////////////

/// Starting at the given iterator, fill its sequence with the node coordinates
template<typename IteratorT, typename ArrayT, typename RowT>
void fill_node_list(IteratorT iterator, ArrayT& coordinates, RowT& element_row) {
  BOOST_FOREACH(const Uint point_idx, element_row) {
    *(iterator++) = coordinates[point_idx];
  }
}

/// Starting at the given iterator, fill its sequence with the node coordinates
template<typename IteratorT, typename ArrayT>
inline void fill_node_list(IteratorT iterator, ArrayT& coordinates, const CRegion& region, const Uint element) {
  const CTable::ConstRow row = region.get_row(element);
  fill_node_list(iterator, coordinates, row);
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, const ElementNodeView& nodeVector);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementNodes_hpp
