// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementData_hpp
#define CF_Solver_Actions_Proto_ElementData_hpp

#include <boost/fusion/algorithm.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/sequence.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/mpl/contains.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"

#include "Solver/Actions/Proto/ElementVariables.hpp"
#include "Solver/Actions/Proto/Terminals.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
  
/// Data to facilitate the evaluation of shape function member functions that are a function of mapped coordinates only
template<typename SF>
struct SFData
{
  /// Allocate aligned data for Eigen
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  /// Type for mapped coordinates
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  /// Type for the laplacian
  typedef Eigen::Matrix<Real, SF::nb_nodes, SF::nb_nodes> LaplacianT;
  
  /// Shape function matrix
  const typename SF::ShapeFunctionsT& shape_function(const MappedCoordsT& mapped_coords) const
  {
    SF::shape_function(mapped_coords, m_sf);
    return m_sf;
  }
  
  /// Mapped gradient computed by the shape function
  const typename SF::MappedGradientT& mapped_gradient(const MappedCoordsT& mapped_coords) const
  {
    SF::mapped_gradient(mapped_coords, m_mapped_gradient_matrix);
    return m_mapped_gradient_matrix;
  }
  
  /// Outer product of the shape function with itself
  const LaplacianT& sf_outer_product(const MappedCoordsT& mapped_coords) const
  {
    SF::shape_function(mapped_coords, m_sf);
    m_sf_outer_product.noalias() = m_sf.transpose() * m_sf;
    return m_sf_outer_product;
  }
  
private:
  mutable typename SF::ShapeFunctionsT m_sf;
  mutable typename SF::MappedGradientT m_mapped_gradient_matrix;
  mutable LaplacianT m_sf_outer_product;
};

/// Functions and operators associated with a geometric support
template<typename SF>
class GeometricSupport
  : public SFData<SF>
{
public:
  /// The shape function type
  typedef SF ShapeFunctionT;
  
  /// The value type for all element nodes
  typedef typename SF::NodeMatrixT ValueT;
 
  /// Return type of the value() method
  typedef const ValueT& ValueResultT;
  
  /// Type for passing mapped coordinates
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  /// Type of the real-world coordinates
  typedef typename SF::CoordsT CoordsT;
  
  /// We store nodes as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  GeometricSupport(const Mesh::CElements& elements) :
    m_coordinates(elements.nodes().coordinates()),
    m_connectivity(elements.connectivity_table())
  {
  }

  /// Update nodes for the current element
  void set_element(const Uint element_idx)
  {
    m_element_idx = element_idx;
    Mesh::fill(m_nodes, m_coordinates, m_connectivity[element_idx]);
  }
  
  /// Reference to the current nodes
  ValueResultT nodes() const
  {
    return m_nodes;
  }
  
  /// Connectivity data for the current element
  Mesh::CTable<Uint>::ConstRow element_connectivity()
  {
    return m_connectivity[m_element_idx];
  }
  
  Real volume() const
  {
    return SF::volume(m_nodes);
  }
  
  const CoordsT& coordinates(const MappedCoordsT& mapped_coords)
  {
    m_eval_result = SFData<SF>::shape_function(mapped_coords) * m_nodes;
    return m_eval_result;
  }
  
  /// Jacobian matrix computed by the shape function
  const typename SF::JacobianT& jacobian(const MappedCoordsT& mapped_coords)
  {
    SF::jacobian(mapped_coords, m_nodes, m_jacobian_matrix);
    return m_jacobian_matrix;
  }
  
  Real jacobian_determinant(const MappedCoordsT& mapped_coords)
  {
    return SF::jacobian_determinant(mapped_coords, m_nodes);
  }
  
  const typename SF::CoordsT& normal(const MappedCoordsT& mapped_coords)
  {
    SF::normal(mapped_coords, m_nodes, m_normal_vector);
    return m_normal_vector;
  }

private:
  /// Stored node data
  ValueT m_nodes;
  
  /// Coordinates table
  const Mesh::CTable<Real>& m_coordinates;
  
  /// Connectivity table
  const Mesh::CTable<Uint>& m_connectivity;
  
  Uint m_element_idx;
  
  /// Temp storage for non-scalar results
  typename SF::CoordsT m_eval_result;
  typename SF::JacobianT m_jacobian_matrix;
  typename SF::CoordsT m_normal_vector;
};

  
/// Storage for per-variable data
template<typename T>
struct VariableData
{
  /// Stored value type
  typedef T ValueT;
 
  /// Return type of the value() method
  typedef ValueT& ValueResultT;
  
  /// Constructor must take the associated type and CElements as arguments
  VariableData(T& var, const Mesh::CElements&) : m_var(var)
  {
  }
  
  /// Set element method must be provided
  void set_element(const Uint)
  {
  }
  
  /// By default, value just returns the supplied value
  ValueResultT value()
  {
    return m_var;
  }
  
private:
  T& m_var;
};

/// Storage for per-variable data
template<typename T>
struct VariableData< ConfigurableConstant<T> >
{
  /// Stored value type
  typedef T ValueT;
 
  /// Return type of the value() method
  typedef const ValueT& ValueResultT;
  
  /// Constructor must take the associated type and CElements as arguments
  VariableData(const ConfigurableConstant<T>& var, const Mesh::CElements&) : m_value(var.stored_value)
  { 
  }
  
  /// Set element method must be provided
  void set_element(const Uint)
  {
  }
  
  /// By default, value just returns the supplied value
  ValueResultT value()
  {
    return m_value;
  }
  
private:
  const T& m_value;
};

/// Data associated with fields
template<typename FieldSF>
struct FieldData : public SFData<FieldSF>
{
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  
  typedef typename FieldSF::MappedGradientT GradientT;
  typedef typename SFData<FieldSF>::LaplacianT LaplacianT;
  typedef typename FieldSF::MappedCoordsT MappedCoordsT;
  
  /// Return the gradient
  template<typename SupportT>
  const GradientT& gradient(const MappedCoordsT& mapped_coords, SupportT& support) const
  {
    m_gradient.noalias() = support.jacobian(mapped_coords).inverse() * support.mapped_gradient(mapped_coords);
    return m_gradient;
  }
  
  /// Return the laplacian
  template<typename SupportT>
  const LaplacianT& laplacian(const MappedCoordsT& mapped_coords, SupportT& support) const
  {
    // Calculate the gradient
    gradient(mapped_coords, support);
    m_laplacian.noalias() = m_gradient.transpose() * m_gradient;
    return m_laplacian;
  }
  
private:
  mutable GradientT m_gradient;
  mutable LaplacianT m_laplacian;
};


/// Storage for per-variable data for variables that depend on a shape function
template<typename SF, typename T>
struct SFVariableData;

/// Data associated with ConstField<Real> variables
template<typename SF>
class SFVariableData<SF, ConstField<Real> >
  : public FieldData<SF>
{
public:
  /// The shape function type
  typedef SF ShapeFunctionT;
  
  /// Types of the base classes
  typedef FieldData<SF> FieldDataT;
  typedef SFData<SF> SFDataT;
  
  /// The value type for all element values
  typedef Eigen::Matrix<Real, SF::nb_nodes, 1> ValueT;
 
  /// Return type of the value() method
  typedef const ValueT& ValueResultT;
  
  /// Type for passing mapped coordinates
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  /// The result type of an interpolation at given mapped coordinates
  typedef Real EvalT;
  
  /// We store data as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  SFVariableData(const ConstField<Real>& placeholder, const Mesh::CElements& elements) : m_data(0), m_connectivity(0)
  {
    if(placeholder.is_const) // the field has a single constant value over the entire domain
    {
      m_element_values.setConstant(placeholder.value);
    }
    else
    {
      m_data = &elements.get_field_elements(placeholder.field_name).data();
      cf_assert(m_data);
      
      m_connectivity = &elements.get_field_elements(placeholder.field_name).connectivity_table();
      cf_assert(m_connectivity);
      
      Mesh::CField::ConstPtr field = boost::dynamic_pointer_cast<Mesh::CField const>(elements.get_field_elements(placeholder.field_name).get_parent());
      cf_assert(field);
      
      var_begin = field->var_index(placeholder.var_name);
      cf_assert(field->var_length(placeholder.var_name) == 1);
    }
  }

  /// Update nodes for the current element
  void set_element(const Uint element_idx)
  {
    if(!m_data)
      return;
    m_element_idx = element_idx;
    Mesh::fill(m_element_values, *m_data, (*m_connectivity)[element_idx], var_begin);
  }
  
  const Mesh::CTable<Uint>::ConstRow element_connectivity()
  {
    return m_connectivity[m_element_idx];
  }
  
  /// Reference to the stored data
  ValueResultT value() const
  {
    return m_element_values;
  }
  
  EvalT eval(const MappedCoordsT& mapped_coords)
  {
    return SFData<SF>::shape_function(mapped_coords) * m_element_values;
  }
  
private:
  /// Value of the field in each element node
  ValueT m_element_values;
  
  /// Coordinates table
  Mesh::CTable<Real> const* m_data;
  
  /// Connectivity table
  Mesh::CTable<Uint> const* m_connectivity;
  
  Uint m_element_idx;
  
  /// Index of where the variable we need is in the field data row
  Uint var_begin;
};

/// Stores data that is used when looping over elements to execut Proto expressions. "Data" is meant here in the boost::proto sense,
/// i.e. it is intended for use as 3rd argument for proto transforms.
/// VariablesT is a fusion sequence containing each unique variable in the expression
/// VariablesDataT is a fusion sequence of pointers to the data (also in proto sense) associated with each of the variables
/// SupportSF is the shape function for the geometric support
template<typename VariablesT, typename VariablesDataT, typename SupportSF>
class ElementData
{
public:
  
  /// Number of variales that we have stored
  typedef typename boost::fusion::result_of::size<VariablesT>::type NbVarsT;
  
  ElementData(VariablesT& variables, Mesh::CElements& elements) :
    m_variables(variables),
    m_elements(elements),
    m_support(elements)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(InitVariablesData(m_variables, m_elements, m_variables_data));
  }
  
  ~ElementData()
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(DeleteVariablesData(m_variables_data));
  }
  
  /// Update element index
  void set_element(const Uint element_idx)
  {
    m_element_idx = element_idx;
    m_support.set_element(element_idx);
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(SetElement(m_variables_data, element_idx));
  }
  
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
  
  /// Return the type of the stored variable I (I being an Integral Constant in the boost::mpl sense)
  template<typename I>
  struct VariableType
  {
    typedef typename boost::remove_pointer
    <
      typename boost::remove_reference
      <
        typename boost::fusion::result_of::at
        <
          VariablesT, I
        >::type 
      >::type
    >::type type;
  };
  
  typedef GeometricSupport<SupportSF> SupportT;
  
  /// Return the data stored at index I
  template<typename I>
  typename DataType<I>::type& var_data()
  {
    return *boost::fusion::at<I>(m_variables_data);
  }
  
  /// Return the variable stored at index I
  template<typename I>
  const typename VariableType<I>::type& variable()
  {
    return boost::fusion::at<I>(m_variables);
  }
  
  /// Get the data associated with the geometric support
  SupportT& support()
  {
    return m_support;
  }
  
private:
  /// Variables used in the expression
  VariablesT& m_variables;
  
  /// Referred CElements
  Mesh::CElements& m_elements;
  
  /// Data for the geometric support
  SupportT m_support;
  
  /// Data associated with each numbered variable
  VariablesDataT m_variables_data;
  
  Uint m_element_idx;
  
  ///////////// helper functions and structs /////////////
  
  /// Initializes the pointers in a VariablesDataT fusion sequence
  struct InitVariablesData
  {
    InitVariablesData(VariablesT& vars, Mesh::CElements& elems, VariablesDataT& vars_data) :
      variables(vars),
      elements(elems),
      variables_data(vars_data)
    {
    }
    
    template<typename I>
    void operator()(const I&)
    {
      typedef typename DataType<I>::type VarDataT;
      boost::fusion::at<I>(variables_data) = new VarDataT(boost::fusion::at<I>(variables), elements);
    }
    
    VariablesT& variables;
    Mesh::CElements& elements;
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
  struct SetElement
  {
    SetElement(VariablesDataT& vars_data, const Uint elem_idx) :
      variables_data(vars_data),
      element_idx(elem_idx)
    {
    }
    
    template<typename I>
    void operator()(const I&)
    {
      boost::fusion::at<I>(variables_data)->set_element(element_idx);
    }
    
    VariablesDataT& variables_data;
    const Uint element_idx;
  };
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementData_hpp
