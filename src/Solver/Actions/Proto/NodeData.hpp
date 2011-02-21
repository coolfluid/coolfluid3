// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_NodeData_hpp
#define CF_Solver_Actions_Proto_NodeData_hpp

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>

#include "Common/ComponentPredicates.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

#include "Terminals.hpp"
#include "Transforms.hpp"

/// @file
/// Data associated with node expressions

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Extract the coordinates, given a specific region
inline const Mesh::CTable<Real>& extract_coordinates(const Mesh::CRegion& region)
{
  const Mesh::CTable<Real>* coordinates = nullptr;
  coordinates = Common::find_component_ptr_with_tag<Mesh::CTable<Real> >(region, "coordinates").get();
  if(!coordinates)
  {
    BOOST_FOREACH(const Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(region))
    {
      if(coordinates)
      {
        cf_assert(coordinates == &elements.nodes().coordinates());
        continue;
      }
      coordinates = &elements.nodes().coordinates();
    }
  }
  
  return *coordinates;
}

/// Struct keeping track of data associated with numbered variables in node expressions
template<typename T, Uint Dim = 1>
struct NodeVarData
{
  /// Stored value type
  typedef T ValueT;
 
  /// Return type of the value() method
  typedef ValueT& ValueResultT;
  
  NodeVarData(T& var, Mesh::CRegion&) : m_var(var)
  {
  }
  
  void set_node(const Uint) {}
  
  /// By default, value just returns the supplied value
  ValueResultT value()
  {
    return m_var;
  }
  
private:
  T& m_var;
};

template<>
struct NodeVarData< Field<Real> >
{
  NodeVarData(const Field<Real>& placeholder, Mesh::CRegion& region) :
    m_field( *Common::find_parent_component<Mesh::CMesh>(region).get_child<Mesh::CField2>(placeholder.field_name) ),
    m_data( m_field.data() )
  {
    m_var_begin = m_field.var_index(placeholder.var_name);
    cf_assert(m_field.var_type(placeholder.var_name) == 1);
  }
  
  void set_node(const Uint idx)
  {
    m_value = &m_data[idx][m_var_begin];
  }
  
  typedef Real ValueT;
  typedef Real& ValueResultT;
  
  /// Value is intended to be mutable, so we return a reference
  ValueResultT value() const
  {
    return *m_value;
  }
  
private:
  Mesh::CField2& m_field;
  Uint m_var_begin;
  Mesh::CTable<Real>& m_data;
  Real* m_value;
};

template<>
struct NodeVarData< ConstField<Real> >
{
  NodeVarData(const ConstField<Real>& placeholder, Mesh::CRegion& region) :
    m_field( *Common::find_parent_component<Mesh::CMesh>(region).get_child<Mesh::CField2>(placeholder.field_name) ),
    m_data( m_field.data() )
  {
    m_var_begin = m_field.var_index(placeholder.var_name);
    cf_assert(m_field.var_type(placeholder.var_name) == 1);
  }
  
  void set_node(const Uint idx)
  {
    m_value = m_data[idx][m_var_begin];
  }
  
  typedef Real ValueT;
  typedef Real ValueResultT;
  
  /// Value is intended to be const, so we return a copy
  ValueResultT value() const
  {
    return m_value;
  }
  
private:
  Mesh::CField2& m_field;
  Uint m_var_begin;
  Mesh::CTable<Real>& m_data;
  Real m_value;
};

template<Uint Dim>
struct NodeVarData<VectorField, Dim>
{
  typedef Eigen::Matrix<Real, Dim, 1> ValueT;
  typedef const ValueT& ValueResultT;
  
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  NodeVarData(const VectorField& placeholder, Mesh::CRegion& region) :
    m_field( *Common::find_parent_component<Mesh::CMesh>(region).get_child<Mesh::CField2>(placeholder.field_name) ),
    m_data( m_field.data() )
  {
    m_var_begin = m_field.var_index(placeholder.var_name);
  }
  
  void set_node(const Uint idx)
  {
    m_idx = idx;
    for(Uint i = 0; i != Dim; ++i)
      m_value[i] = m_data[idx][m_var_begin + i];
  }
  
  /// Return a reference to the stored value
  ValueResultT value() const
  {
    return m_value;
  }
  
  /// Sets values using a vector-like container
  template<typename VectorT>
  void set_values(const VectorT& v)
  {
    for(Uint i = 0; i != Dim; ++i)
      m_data[m_idx][m_var_begin + i] = v[i];
  }
  
private:
  Mesh::CField2& m_field;
  Uint m_var_begin;
  Mesh::CTable<Real>& m_data;
  ValueT m_value;
  Uint m_idx;
};

template<typename T>
struct NodeVarData< ConfigurableConstant<T> >
{
  NodeVarData(const ConfigurableConstant<T>& placeholder, Mesh::CRegion& region) :
    m_value(placeholder.stored_value)
  {
  }
  
  void set_node(const Uint)
  {
  }
  
  typedef T ValueT;
  typedef const T& ValueResultT;
  
  ValueResultT value() const
  {
    return m_value;
  }
  
private:
  ValueResultT m_value;
};

/// MPL transform operator to wrap a variable in its data type
template<Uint Dim>
struct AddNodeData
{
  template<typename VarT, int Dummy=0>
  struct apply
  {
    typedef NodeVarData<VarT>* type;
  };
  
  template<int Dummy>
  struct apply<VectorField, Dummy>
  {
    typedef NodeVarData<VectorField, Dim>* type;
  };
};

template<typename VariablesT, typename NbDims>
class NodeData
{
public:
  /// Number of variales that we have stored
  typedef typename boost::fusion::result_of::size<VariablesT>::type NbVarsT;
  
  /// Type of the per-variable data
  typedef typename boost::mpl::transform< VariablesT, AddNodeData<NbDims::value> >::type VariablesDataT;
  
  /// Return the type of the data stored for variable I (I being an Integral Constant in the boost::mpl sense)
  template<typename I>
  struct DataType
  {
    typedef typename boost::remove_pointer
    <
      typename boost::remove_reference
      <
        typename boost::fusion::result_of::at
        <
          VariablesDataT, I
        >::type 
      >::type
    >::type type;
  };
  
  /// Return the data stored at index I
  template<typename I>
  typename DataType<I>::type& var_data()
  {
    return *boost::fusion::at<I>(m_variables_data);
  }
  
  /// Type of the coordinates
  typedef Eigen::Matrix<Real, NbDims::value, 1> CoordsT;
  
  NodeData(VariablesT& variables, Mesh::CRegion& region, const Mesh::CTable<Real>& coords) :
    m_variables(variables),
    m_region(region),
    m_coordinates(coords)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(InitVariablesData(m_variables, m_region, m_variables_data));
    boost::fusion::for_each( m_variables, CalculateOffsets(m_offsets, NbDims::value) );
  }
  
  ~NodeData()
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(DeleteVariablesData(m_variables_data));
  }
  
  /// Update node index
  void set_node(const Uint idx)
  {
    node_idx = idx;
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(SetNode(m_variables_data, node_idx));
  }
  
  /// Current node index
  Uint node_idx;
  
  /// Access to the current coordinates
  const CoordsT& coordinates() const
  {
    const Mesh::CTable<Real>::ConstRow row = m_coordinates[node_idx];
    for(Uint i = 0; i != NbDims::value; ++i)
    {
      m_position[i] = row[i];
    }
    return m_position;
  }
  
  const std::vector<Uint>& variable_offsets() const
  {
    return m_offsets;
  }
  
private:
  /// Variables used in the expression
  VariablesT& m_variables;
  
  /// Referred region
  Mesh::CRegion& m_region;
  
  /// Aray holding the coordinate values
  const Mesh::CTable<Real>& m_coordinates;
  
  /// Data associated with each numbered variable
  VariablesDataT m_variables_data;
  
  /// Current coordinates
  mutable CoordsT m_position;
  
  /// Offsets for field variables (so they can be referenced in a linear system, if any)
  std::vector<Uint> m_offsets;
  
  ///////////// helper functions and structs /////////////
private:
  /// Initializes the pointers in a VariablesDataT fusion sequence
  struct InitVariablesData
  {
    InitVariablesData(VariablesT& vars, Mesh::CRegion& reg, VariablesDataT& vars_data) :
      variables(vars),
      region(reg),
      variables_data(vars_data)
    {
    }
    
    template<typename I>
    void operator()(const I&)
    {
      typedef typename DataType<I>::type VarDataT;
      boost::fusion::at<I>(variables_data) = new VarDataT(boost::fusion::at<I>(variables), region);
    }
    
    VariablesT& variables;
    Mesh::CRegion& region;
    VariablesDataT& variables_data;
  };
  
  /// Delete stored per-variable data
  struct DeleteVariablesData
  {
    DeleteVariablesData(VariablesDataT& vars_data) : variables_data(vars_data)
    {
    }
    
    template<typename I>
    void operator()(const I&)
    {
      delete boost::fusion::at<I>(variables_data);
    }
    
    VariablesDataT& variables_data;
  };
  
  /// Set the element on each stored data item
  struct SetNode
  {
    SetNode(VariablesDataT& vars_data, const Uint idx) :
      variables_data(vars_data),
      node_idx(idx)
    {
    }
    
    template<typename I>
    void operator()(const I&)
    {
      boost::fusion::at<I>(variables_data)->set_node(node_idx);
    }
    
    VariablesDataT& variables_data;
    const Uint node_idx;
  };  
};

/// Creates a list of unique nodes in the region
inline void make_node_list(const Mesh::CRegion& region, const Mesh::CTable<Real>& coordinates, std::vector<Uint>& nodes)
{
  std::vector<bool> node_is_used(coordinates.size(), false);
  
  // First count the number of unique nodes
  Uint nb_nodes = 0;
  BOOST_FOREACH(const Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(region))
  {
    const Mesh::CTable<Uint>& conn_tbl = elements.connectivity_table();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_elem_nodes = conn_tbl.row_size();
    
    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const Mesh::CTable<Uint>::ConstRow row = conn_tbl[elem_idx];
      for(Uint node_idx = 0; node_idx != nb_elem_nodes; ++node_idx)
      {
        const Uint node = row[node_idx];
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          ++nb_nodes;
        }
      }
    }
  }
  
  // reserve space for all unique nodes
  nodes.clear();
  nodes.reserve(nb_nodes);
  
  // Add the unique node indices
  node_is_used.assign(coordinates.size(), false);
  BOOST_FOREACH(const Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>(region))
  {
    const Mesh::CTable<Uint>& conn_tbl = elements.connectivity_table();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_nodes = conn_tbl.row_size();
    
    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const Mesh::CTable<Uint>::ConstRow row = conn_tbl[elem_idx];
      for(Uint node_idx = 0; node_idx != nb_nodes; ++node_idx)
      {
        const Uint node = row[node_idx];
        if(!node_is_used[node])
        {
          node_is_used[node] = true;
          nodes.push_back(node);
        }
      }
    }
  }
}
  
} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_NodeData_hpp
