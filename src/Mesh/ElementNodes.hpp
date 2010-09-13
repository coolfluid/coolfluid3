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
template<typename IteratorT, typename RowT>
inline void fill_node_list(IteratorT iterator, const CArray& coordinates, const RowT& element_row) {
  BOOST_FOREACH(const Uint point_idx, element_row) {
    *(iterator++) = coordinates[point_idx];
  }
}

/// Starting at the given iterator, fill its sequence with the node coordinates
template<typename IteratorT>
inline void fill_node_list(IteratorT iterator, const CArray& coordinates, const CTable& connectivity, const Uint element) {
  fill_node_list(iterator, coordinates, connectivity[element]);
}

////////////////////////////////////////////////////////////////////////////////

/// Convenience function to create a 1D point
inline RealVector point1(const Real x)
{
  RealVector result(1);
  result[0] = x;
  return result;
}

/// Convenience function to create a 2D point
inline RealVector point2(const Real x, const Real y)
{
  RealVector result(2);
  result[0] = x;
  result[1] = y;
  return result;
}

/// Convenience function to create a 3D point
inline RealVector point3(const Real x, const Real y, const Real z)
{
  RealVector result(3);
  result[0] = x;
  result[1] = y;
  result[2] = z;
  return result;
}

////////////////////////////////////////////////////////////////////////////////

std::ostream& operator<<(std::ostream& output, const ElementNodeView& nodeVector);
std::ostream& operator<<(std::ostream& output, const ConstElementNodeView& nodeVector);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_ElementNodes_hpp
