#ifndef CF_Mesh_ElementNodes_hpp
#define CF_Mesh_ElementNodes_hpp

////////////////////////////////////////////////////////////////////////////////

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

/// Vector of rows, where each row represents the coordinate of a node
typedef std::vector<CArray::Row> ElementNodeVector;

////////////////////////////////////////////////////////////////////////////////

/// Vector of rows, where each row represents the coordinate of a node
/// Const version
typedef std::vector<CArray::ConstRow> ConstElementNodeVector;

typedef std::vector<RealVector> ElementNodeVectorCopy;

////////////////////////////////////////////////////////////////////////////////

/// Starting at the given iterator, fill its sequence with the node coordinates
template<typename IteratorT, typename ArrayT>
void fill_node_list(IteratorT iterator, ArrayT& coordinates, const CRegion& region, const Uint element) {
  const CTable::ConstRow row = region.get_row(element);
  BOOST_FOREACH(const Uint point_idx, row) {
    *(++iterator) = coordinates[point_idx];
  }
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, const ElementNodeView& nodeVector);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementNodes_hpp
