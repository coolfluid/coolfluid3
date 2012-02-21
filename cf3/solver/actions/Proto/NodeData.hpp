// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_NodeData_hpp
#define cf3_solver_actions_Proto_NodeData_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>

#include "common/FindComponents.hpp"

#include "math/VariablesDescriptor.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/Dictionary.hpp"
#include "mesh/Elements.hpp"
#include "mesh/Region.hpp"

#include "Transforms.hpp"

/// @file
/// Data associated with node expressions

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Extract the coordinates, given a specific region
inline const common::Table<Real>& extract_coordinates(const mesh::Region& region)
{
  const common::Table<Real>* coordinates = nullptr;
  coordinates = common::find_component_ptr_with_tag<common::Table<Real> >(region, mesh::Tags::coordinates()).get();
  if(!coordinates)
  {
    BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(region))
    {
      if(coordinates)
      {
        cf3_assert(coordinates == &elements.geometry_fields().coordinates());
        continue;
      }
      coordinates = &elements.geometry_fields().coordinates();
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

  NodeVarData(T& var, mesh::Region&) : m_var(var)
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

/// Helper function to find a field starting from a region
inline mesh::Field& find_field(mesh::Region& region, const std::string& tag)
{
  mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(region);
  mesh::Dictionary& dict =  mesh.geometry_fields();
  return common::find_component_with_tag<mesh::Field>(dict, tag);
}

template<>
struct NodeVarData< ScalarField >
{
  static const Uint dimension = 1;

  NodeVarData(const ScalarField& placeholder, mesh::Region& region) :
    m_field(find_field(region, placeholder.field_tag()))
  {
    const math::VariablesDescriptor& descriptor = m_field.descriptor();
    m_var_begin = descriptor.offset(placeholder.name());

    // Variable must be a scalar
    cf3_assert(descriptor.size(placeholder.name()) == 1);

    offset = descriptor.offset(placeholder.name());
    nb_dofs = descriptor.size();
  }

  void set_node(const Uint idx)
  {
    m_idx = idx;
    m_value = m_field[idx][m_var_begin];
  }

  typedef Real ValueT;
  typedef Real ValueResultT;

  /// Value is intended to be const, so we return a copy
  ValueResultT value() const
  {
    return m_value;
  }

  /// Sets value
  void set_value(boost::proto::tag::assign, const Real v)
  {
    m_value = v;
    m_field[m_idx][m_var_begin] = m_value;
  }

  void set_value(boost::proto::tag::plus_assign, const Real v)
  {
    m_value += v;
    m_field[m_idx][m_var_begin] = m_value;
  }

  void set_value(boost::proto::tag::minus_assign, const Real v)
  {
    m_value -= v;
    m_field[m_idx][m_var_begin] = m_value;
  }

  /// Offset for the variable in the field
  Uint offset;

  /// Total nbdofs in the field that this variable is in
  Uint nb_dofs;

private:
  mesh::Field& m_field;
  Uint m_var_begin;
  Uint m_idx;
  Real m_value;
};

template<Uint Dim>
struct NodeVarData<VectorField, Dim>
{
  typedef Eigen::Matrix<Real, Dim, 1> ValueT;
  typedef const ValueT& ValueResultT;

  static const Uint dimension = Dim;

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  NodeVarData(const VectorField& placeholder, mesh::Region& region) :
    m_field( find_field(region, placeholder.field_tag()) )
  {
    const math::VariablesDescriptor& descriptor = m_field.descriptor();
    m_var_begin = descriptor.offset(placeholder.name());

    // Variable must be a vector
    cf3_assert(descriptor.size(placeholder.name()) == dimension);

    offset = descriptor.offset(placeholder.name());
    nb_dofs = descriptor.size();
  }

  void set_node(const Uint idx)
  {
    m_idx = idx;
    for(Uint i = 0; i != Dim; ++i)
      m_value[i] = m_field[idx][m_var_begin + i];
  }

  /// Return a reference to the stored value
  ValueResultT value() const
  {
    return m_value;
  }

  /// Sets values using a vector-like container
  template<typename VectorT>
  void set_value(boost::proto::tag::assign, const VectorT& v)
  {
    m_value = v;
    for(Uint i = 0; i != Dim; ++i)
      m_field[m_idx][m_var_begin + i] = v[i];
  }

  template<typename VectorT>
  void set_value(boost::proto::tag::plus_assign, const VectorT& v)
  {
    m_value += v;
    for(Uint i = 0; i != Dim; ++i)
      m_field[m_idx][m_var_begin + i] += v[i];
  }

  template<typename VectorT>
  void set_value(boost::proto::tag::minus_assign, const VectorT& v)
  {
    m_value -= v;
    for(Uint i = 0; i != Dim; ++i)
      m_field[m_idx][m_var_begin + i] -= v[i];
  }

  /// Offset for the variable in the field
  Uint offset;

  /// Total nb dofs in the field
  Uint nb_dofs;

private:
  mesh::Field& m_field;
  Uint m_var_begin;
  ValueT m_value;
  Uint m_idx;
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

  /// The dimension of the problem
  static const Uint dimension = NbDims::value;

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
          VariablesDataT, typename boost::remove_reference<I>::type
        >::type
      >::type
    >::type type;
  };

  /// Return the data stored at index I
  template<typename I>
  typename DataType<I>::type& var_data(const I&)
  {
    return *boost::fusion::at<I>(m_variables_data);
  }

  /// Type of the coordinates
  typedef Eigen::Matrix<Real, NbDims::value, 1> CoordsT;

  template<typename ExprT>
  NodeData(VariablesT& variables, mesh::Region& region, const common::Table<Real>& coords, const ExprT& expr) :
    m_variables(variables),
    m_region(region),
    m_coordinates(coords)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(InitVariablesData(m_variables, m_region, m_variables_data));
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
    const common::Table<Real>::ConstRow row = m_coordinates[node_idx];
    for(Uint i = 0; i != NbDims::value; ++i)
    {
      m_position[i] = row[i];
    }
    return m_position;
  }

private:
  /// Variables used in the expression
  VariablesT& m_variables;

  /// Referred region
  mesh::Region& m_region;

  /// Aray holding the coordinate values
  const common::Table<Real>& m_coordinates;

  /// Data associated with each numbered variable
  VariablesDataT m_variables_data;

  /// Current coordinates
  mutable CoordsT m_position;

  ///////////// helper functions and structs /////////////
private:
  /// Initializes the pointers in a VariablesDataT fusion sequence
  struct InitVariablesData
  {
    InitVariablesData(VariablesT& vars, mesh::Region& reg, VariablesDataT& vars_data) :
      variables(vars),
      region(reg),
      variables_data(vars_data)
    {
    }

    template<typename I>
    void operator()(const I&)
    {
      apply(boost::fusion::at<I>(variables), boost::fusion::at<I>(variables_data));
    }

    template<typename VarDataT>
    void apply(boost::mpl::void_, VarDataT*& data)
    {
      data = 0;
    }

    template<typename VarT, typename VarDataT>
    void apply(const VarT& var, VarDataT*& data)
    {
      const std::string& var_name = var.name();
      data = new VarDataT(var, region);
    }

    VariablesT& variables;
    mesh::Region& region;
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
inline void make_node_list(const mesh::Region& region, const common::Table<Real>& coordinates, std::vector<Uint>& nodes)
{
  std::vector<bool> node_is_used(coordinates.size(), false);

  // First count the number of unique nodes
  Uint nb_nodes = 0;
  BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(region))
  {
    const common::Table<Uint>& conn_tbl = elements.geometry_space().connectivity();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_elem_nodes = conn_tbl.row_size();

    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const common::Table<Uint>::ConstRow row = conn_tbl[elem_idx];
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
  BOOST_FOREACH(const mesh::Elements& elements, common::find_components_recursively<mesh::Elements>(region))
  {
    const common::Table<Uint>& conn_tbl = elements.geometry_space().connectivity();
    const Uint nb_elems = conn_tbl.size();
    const Uint nb_nodes = conn_tbl.row_size();

    for(Uint elem_idx = 0; elem_idx != nb_elems; ++elem_idx)
    {
      const common::Table<Uint>::ConstRow row = conn_tbl[elem_idx];
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
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_NodeData_hpp
