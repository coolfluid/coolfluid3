// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_ElementData_hpp
#define CF_Solver_Actions_Proto_ElementData_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "Common/Component.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/ElementData.hpp"

#include "ElementMatrix.hpp"
#include "ElementOperations.hpp"
#include "Terminals.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Functions and operators associated with a geometric support
template<typename ShapeFunctionT>
class GeometricSupport
{
public:
  /// The shape function type
  typedef ShapeFunctionT SF ;
  
  /// The value type for all element nodes
  typedef typename SF::NodeMatrixT ValueT;
 
  /// Return type of the value() method
  typedef const ValueT& ValueResultT;
  
  /// We store nodes as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  GeometricSupport(const Mesh::CElements& elements) :
    m_coordinates(elements.nodes().coordinates()),
    m_connectivity(elements.node_connectivity())
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
  Mesh::CTable<Uint>::ConstRow element_connectivity() const
  {
    return m_connectivity[m_element_idx];
  }
  
  Real volume() const
  {
    return SF::volume(m_nodes);
  }
  
  const typename SF::CoordsT& coordinates(const typename SF::MappedCoordsT& mapped_coords) const
  {
    SF::shape_function_value(mapped_coords, m_sf);
    m_eval_result.noalias() = m_sf * m_nodes;
    return m_eval_result;
  }
  
  /// Jacobian matrix computed by the shape function
  const typename SF::JacobianT& jacobian(const typename SF::MappedCoordsT& mapped_coords) const
  {
    SF::jacobian(mapped_coords, m_nodes, m_jacobian_matrix);
    return m_jacobian_matrix;
  }
  
  Real jacobian_determinant(const typename SF::MappedCoordsT& mapped_coords) const
  {
    return SF::jacobian_determinant(mapped_coords, m_nodes);
  }
  
  const typename SF::CoordsT& normal(const typename SF::MappedCoordsT& mapped_coords) const
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
  
  /// Index for the current element
  Uint m_element_idx;
  
  /// Temp storage for non-scalar results
  mutable typename SF::ShapeFunctionsT m_sf;
  mutable typename SF::CoordsT m_eval_result;
  mutable typename SF::JacobianT m_jacobian_matrix;
  mutable typename SF::CoordsT m_normal_vector;
};

/// Data associated with VectorField variables
template<typename ShapeFunctionT, Uint Dim, Uint Offset, Uint MatrixSize, bool IsEquationVar>
class SFVariableData
{
private:
  // Forward declaration
  template<Uint VarDim, int Dummy = 0>
  struct InterpolationImpl;
public:
  /// The shape function type
  typedef ShapeFunctionT SF;
  
  /// The value type for all element values
  typedef Eigen::Matrix<Real, SF::nb_nodes, Dim> ValueT;
 
  /// Return type of the value() method
  typedef const ValueT& ValueResultT;
  
  /// Type for passing mapped coordinates
  typedef typename SF::MappedCoordsT MappedCoordsT;
  
  /// The result type of an interpolation at given mapped coordinates
  typedef typename InterpolationImpl<Dim>::result_type EvalT;
  
  /// Type of the gradient
  typedef typename SF::MappedGradientT GradientT;
  
  /// Type of the linearized form of the divergence
  typedef Eigen::Matrix<Real, 1, Dim * SF::nb_nodes> DivergenceLinT;
  
  /// The dimension of the variable
  static const Uint dimension = Dim;
  
  /// The offset of the variable in the element matrix
  static const Uint offset = Offset;
  
  /// Size of the element matrix
  static const Uint matrix_size = MatrixSize;
  
  /// True if this variable is an unknow in the system of equations
  static const bool is_equation_variable = IsEquationVar;
  
  /// We store data as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  template<typename VariableT>
  SFVariableData(const VariableT& placeholder, const Mesh::CElements& elements) : m_data(0), m_connectivity(0)
  {
    const Mesh::CMesh& mesh = Common::find_parent_component<Mesh::CMesh>(elements);
    Common::Component::ConstPtr field_comp = mesh.get_child_ptr(placeholder.field_name);
    Mesh::CField::ConstPtr field = field_comp->as_ptr<Mesh::CField>();
    cf_assert(field);
    
    m_data = &field->data();
    
    m_connectivity = &elements.node_connectivity();    
    
    var_begin = field->var_index(placeholder.var_name);
  }

  /// Update nodes for the current element
  void set_element(const Uint element_idx)
  {
    if(!m_data)
      return;
    m_element_idx = element_idx;
    Mesh::fill(m_element_values, *m_data, (*m_connectivity)[element_idx], var_begin);
  }
  
  const Mesh::CTable<Uint>::ConstRow element_connectivity() const
  {
    return (*m_connectivity)[m_element_idx];
  }
  
  /// Reference to the stored data, i.e. the matrix of nodal values
  ValueResultT value() const
  {
    return m_element_values;
  }
  
  /// Precompute all the cached values for the given geometric support and mapped coordinates.
  template<typename SupportT>
  void compute_values(const MappedCoordsT& mapped_coords, const SupportT& support) const
  {
    SF::shape_function_value(mapped_coords, m_sf);
    SF::shape_function_gradient(mapped_coords, m_mapped_gradient_matrix);
    m_gradient.noalias() = support.jacobian(mapped_coords).inverse() * m_mapped_gradient_matrix;
    m_eval(m_sf, m_element_values);
    m_advection = m_eval.stored_result * m_gradient;
  }
  
  /// Calculate and return the interpolation at given mapped coords
  EvalT eval(const MappedCoordsT& mapped_coords) const
  {
    SF::shape_function_value(mapped_coords, m_sf);
    return m_eval(m_sf, m_element_values);
  }
  
  /// Return previously computed evaluation
  EvalT eval() const
  {
    return m_eval.stored_result;
  }
  
  
  /// Shape function matrix at mapped coordinates (calculates and returns)
  const typename SF::ShapeFunctionsT& shape_function(const MappedCoordsT& mapped_coords) const
  {
    SF::shape_function_value(mapped_coords, m_sf);
    return m_sf;
  }
  
  /// Previously calculated shape function matrix
  const typename SF::ShapeFunctionsT& shape_function() const
  {
    return m_sf;
  }
  
  /// Return the gradient
  template<typename SupportT>
  const GradientT& gradient(const MappedCoordsT& mapped_coords, const SupportT& support) const
  {
    SF::shape_function_gradient(mapped_coords, m_mapped_gradient_matrix);
    m_gradient.noalias() = support.jacobian(mapped_coords).inverse() * m_mapped_gradient_matrix;
    return m_gradient;
  }
  
  /// Previously calculated gradient matrix
  const GradientT& gradient() const
  {
    return m_gradient;
  }
  
  /// Return the advection operator
  template<typename SupportT>
  const typename SF::ShapeFunctionsT& advection(const MappedCoordsT& mapped_coords, const SupportT& support) const
  {
    SF::shape_function_value(mapped_coords, m_sf);
    m_advection = m_sf * m_element_values * gradient(mapped_coords, support);
    return m_advection;
  }
  
  /// Previously calculated advection operator
  const typename SF::ShapeFunctionsT& advection() const
  {
    return m_advection;
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
  
  /// Cached data
  mutable typename SF::ShapeFunctionsT m_sf;
  mutable typename SF::MappedGradientT m_mapped_gradient_matrix;
  mutable GradientT m_gradient;
  mutable typename SF::ShapeFunctionsT m_advection;
  
  /// Interpolation of a field
  template<Uint VarDim, int Dummy>
  struct InterpolationImpl
  { 
    typedef Eigen::Matrix<Real, 1, VarDim> MatrixT;
    typedef const MatrixT& result_type;
    
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    
    template<typename NodeValuesT>
    result_type operator()(const typename SF::ShapeFunctionsT& sf, const NodeValuesT& values) const
    {
      stored_result.noalias() = sf * values;
      return stored_result;
    }
    
    mutable MatrixT stored_result;
  };

  /// Interpolation of a scalar field
  template<int Dummy>
  struct InterpolationImpl<1, Dummy>
  {
    typedef Real result_type;
    
    template<typename NodeValuesT>
    result_type operator()(const typename SF::ShapeFunctionsT& sf, const NodeValuesT& values) const
    {
      stored_result = sf * values;
      return stored_result;
    }
    
    mutable Real stored_result;
  };
  
  InterpolationImpl<Dim> m_eval;
};

/// Predicate to check if data belongs to an equation variable
struct IsEquationData
{
  template<typename DataT, int Dummy = 0>
  struct apply
  {
    typedef boost::mpl::bool_<boost::remove_pointer<DataT>::type::is_equation_variable> type;
  };
  
  template<int Dummy>
  struct apply<boost::mpl::void_, Dummy>
  {
    typedef boost::mpl::false_ type;
  };
};

/// Metafunction class for creating an appropriate data type
template<typename VariablesT, typename SupportSF, typename ShapeFunctionsT, typename EquationVariablesT, typename MatrixSizesT, typename EMatrixSizeT>
struct MakeVarData
{
  template<typename I>
  struct apply
  {
    typedef typename boost::mpl::at<VariablesT, I>::type VarT;
    typedef typename boost::mpl::at<ShapeFunctionsT, I>::type SF;
    typedef typename boost::mpl::at<EquationVariablesT, I>::type IsEquationVar;
    typedef typename boost::mpl::if_<IsEquationVar, EMatrixSizeT, typename boost::mpl::at<MatrixSizesT, I>::type>::type MatSize;
    typedef typename boost::mpl::eval_if< IsEquationVar, VarOffset<MatrixSizesT, EquationVariablesT, I>, boost::mpl::int_<0> >::type Offset;
    
    typedef typename boost::mpl::if_
    <
      boost::mpl::is_void_<VarT>,
      boost::mpl::void_,
      SFVariableData<SF, FieldWidth<VarT, SF>::value, Offset::value, MatSize::value, IsEquationVar::value>*
    >::type type;
  };
};

template<typename VariablesT, typename SF, typename EquationVariablesT, typename MatrixSizesT, typename EMatrixSizeT>
struct MakeVarData<VariablesT, SF, SF, EquationVariablesT, MatrixSizesT, EMatrixSizeT>
{
  template<typename I>
  struct apply
  {
    typedef typename boost::mpl::at<VariablesT, I>::type VarT;
    typedef typename boost::mpl::at<EquationVariablesT, I>::type IsEquationVar;
    typedef typename boost::mpl::if_<IsEquationVar, EMatrixSizeT, typename boost::mpl::at<MatrixSizesT, I>::type>::type MatSize;
    typedef typename boost::mpl::eval_if< IsEquationVar, VarOffset<MatrixSizesT, EquationVariablesT, I>, boost::mpl::int_<0> >::type Offset;
    
    typedef typename boost::mpl::if_
    <
      boost::mpl::is_void_<VarT>,
      boost::mpl::void_,
      SFVariableData<SF, FieldWidth<VarT, SF>::value, Offset::value, MatSize::value, IsEquationVar::value>*
    >::type type;
  };
};

/// Stores data that is used when looping over elements to execute Proto expressions. "Data" is meant here in the boost::proto sense,
/// i.e. it is intended for use as 3rd argument for proto transforms.
/// VariablesT is a fusion sequence containing each unique variable in the expression
/// VariablesDataT is a fusion sequence of pointers to the data (also in proto sense) associated with each of the variables
/// SupportSF is the shape function for the geometric support
template<typename VariablesT, typename VariablesSFT, typename SupportSF, typename EquationVariablesT>
class ElementData
{
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  /// Number of variales that we have stored
  typedef typename boost::fusion::result_of::size<VariablesT>::type NbVarsT;
  
  /// Shape function for the support
  typedef SupportSF SupportShapeFunction;
  
  /// Element matrix size per var
  typedef typename MatrixSizePerVar<VariablesT, VariablesSFT>::type MatrixSizesT;
  
  /// Size for the element matrix
  typedef typename ElementMatrixSize<MatrixSizesT, EquationVariablesT>::type EMatrixSizeT;
  
  /// Type for the element matrix (combined for all equations)
  typedef Eigen::Matrix<Real, EMatrixSizeT::value, EMatrixSizeT::value> ElementMatrixT;
  
  /// Type for the element vector (combined for all equations)
  typedef Eigen::Matrix<Real, EMatrixSizeT::value, 1> ElementVectorT;
  
  typedef typename boost::fusion::result_of::as_vector
  <
    typename boost::mpl::transform
    <
      typename boost::mpl::copy<boost::mpl::range_c<int,0,NbVarsT::value>, boost::mpl::back_inserter< boost::mpl::vector_c<Uint> > >::type, //range from 0 to NbVarsT
      MakeVarData<VariablesT, SupportSF, VariablesSFT, EquationVariablesT, MatrixSizesT, EMatrixSizeT>
    >::type
  >::type VariablesDataT;
  
  /// A view of only the data used in the element matrix
  typedef boost::fusion::filter_view< VariablesDataT, IsEquationData > EquationDataT;
  
  ElementData(VariablesT& variables, Mesh::CElements& elements) :
    m_variables(variables),
    m_elements(elements),
    m_support(elements),
    m_equation_data(m_variables_data)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(InitVariablesData(m_variables, m_elements, m_variables_data));
    for(Uint i = 0; i != CF_PROTO_MAX_ELEMENT_MATRICES; ++i)
      m_element_matrices[i].setZero();
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
    boost::fusion::for_each(m_equation_data, FillRhs(m_element_rhs));
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
  typename DataType<I>::type& var_data(const I&)
  {
    return *boost::fusion::at<I>(m_variables_data);
  }
  
  /// Return the variable stored at index I
  template<typename I>
  const typename VariableType<I>::type& variable(const I&)
  {
    return boost::fusion::at<I>(m_variables);
  }
  
  /// Get the data associated with the geometric support
  SupportT& support()
  {
    return m_support;
  }
  
  const SupportT& support() const
  {
    return m_support;
  }
  
  /// Retrieve the element matrix at index i
  ElementMatrixT& element_matrix(const int i)
  {
    return m_element_matrices[i];
  }
  
  /// Retrieve the element RHS
  ElementVectorT& element_rhs()
  {
    return m_element_rhs;
  };
  
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
  
  ElementMatrixT m_element_matrices[CF_PROTO_MAX_ELEMENT_MATRICES];
  ElementVectorT m_element_rhs;
  
  /// Filtered view of the data associated with equation variables
  const EquationDataT m_equation_data;
  
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
      apply(boost::fusion::at<I>(variables), boost::fusion::at<I>(variables_data));
    }
    
    void apply(const boost::mpl::void_&, const boost::mpl::void_&)
    {
    }
    
    template<typename VarT, typename VarDataT>
    void apply(const VarT& v, VarDataT*& d)
    {
      d = new VarDataT(v, elements);
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
      apply(boost::fusion::at<I>(variables_data));
    }
    
    void apply(const boost::mpl::void_&)
    {
    }
    
    template<typename T>
    void apply(T*& d)
    {
      delete d;
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
      apply(boost::fusion::at<I>(variables_data));
    }
    
    void apply(const boost::mpl::void_&)
    {
    }
    
    template<typename T>
    void apply(T*& d)
    {
      d->set_element(element_idx);
    }
    
    VariablesDataT& variables_data;
    const Uint element_idx;
  };
  
  /// Set the element on each stored data item
  struct FillRhs
  {
    FillRhs(ElementVectorT& elm_v) :
      element_vector(elm_v)
    {
    }
    
    template<typename DataT>
    void operator()(const DataT* data) const
    {
      typename DataT::ValueResultT v = data->value();
      static const Uint rows = DataT::ValueT::RowsAtCompileTime;
      static const Uint cols = DataT::ValueT::ColsAtCompileTime;
      for(Uint i = 0; i != cols; ++i)
      {
        element_vector.template segment<rows>(DataT::offset + i*rows) = v.col(i);
      }
    }
    
    ElementVectorT& element_vector;
  };
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_ElementData_hpp
