// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_actions_Proto_ElementData_hpp
#define cf3_solver_actions_Proto_ElementData_hpp

#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/mpl.hpp>
#include <boost/fusion/container/vector/convert.hpp>
#include <boost/fusion/container/vector.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector_c.hpp>

#include "common/Component.hpp"
#include "common/FindComponents.hpp"

#include "math/VariablesDescriptor.hpp"
#include "math/LSS/BlockAccumulator.hpp"

#include "mesh/Elements.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Geometry.hpp"
#include "mesh/ElementData.hpp"

#include "ElementMatrix.hpp"
#include "ElementOperations.hpp"
#include "Terminals.hpp"

namespace cf3 {
namespace solver {
namespace actions {
namespace Proto {

/// Grammar matching expressions if they have a terminal with the index given in the template parameter
template<Uint I>
struct UsesVar :
  boost::proto::or_
  <
    boost::proto::when
    <
      boost::proto::terminal< Var<boost::mpl::int_<I>, boost::proto::_> >,
      boost::mpl::true_()
    >,
    boost::proto::when
    <
      boost::proto::terminal< boost::proto::_ >,
      boost::mpl::false_()
    >,
    boost::proto::when
    <
      boost::proto::nary_expr<boost::proto::_, boost::proto::vararg<boost::proto::_> >,
      boost::proto::fold< boost::proto::_, boost::mpl::false_(), boost::mpl::max< boost::proto::_state, boost::proto::call< UsesVar<I> > >() >
    >
  >
{
};

/// Functions and operators associated with a geometric support
template<typename ETYPE>
class GeometricSupport
{
public:
  /// The shape function type
  typedef ETYPE EtypeT ;

  /// The value type for all element nodes
  typedef typename EtypeT::NodesT ValueT;

  /// Return type of the value() method
  typedef const ValueT& ValueResultT;

  /// We store nodes as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  GeometricSupport(const mesh::Elements& elements) :
    m_coordinates(elements.geometry_fields().coordinates()),
    m_connectivity(elements.node_connectivity())
  {
  }

  /// Update nodes for the current element and set the connectivity for the passed block accumulator
  void set_element(const Uint element_idx)
  {
    m_element_idx = element_idx;
    mesh::fill(m_nodes, m_coordinates, m_connectivity[element_idx]);
  }

  void update_block_connectivity(math::LSS::BlockAccumulator& block_accumulator)
  {
    block_accumulator.neighbour_indices(m_connectivity[m_element_idx]);
  }

  /// Reference to the current nodes
  ValueResultT nodes() const
  {
    return m_nodes;
  }

  /// Connectivity data for the current element
  common::Table<Uint>::ConstRow element_connectivity() const
  {
    return m_connectivity[m_element_idx];
  }

  Real volume() const
  {
    return EtypeT::volume(m_nodes);
  }

  const typename EtypeT::CoordsT& coordinates(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_value(mapped_coords, m_sf);
    m_eval_result.noalias() = m_sf * m_nodes;
    return m_eval_result;
  }

  /// Precomputed coordinates
  const typename EtypeT::CoordsT& coordinates() const
  {
    return m_eval_result;
  }

  /// Jacobian matrix computed by the shape function
  const typename EtypeT::JacobianT& jacobian(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::compute_jacobian(mapped_coords, m_nodes, m_jacobian_matrix);
    return m_jacobian_matrix;
  }

  /// Precomputed jacobian
  const typename EtypeT::JacobianT& jacobian() const
  {
    return m_jacobian_matrix;
  }

  /// Precomputed jacobian inverse
  const typename EtypeT::JacobianT& jacobian_inverse() const
  {
    return m_jacobian_inverse;
  }

  Real jacobian_determinant(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    m_jacobian_determinant = EtypeT::jacobian_determinant(mapped_coords, m_nodes);
    return m_jacobian_determinant;
  }

  /// Precomputed jacobian determinant
  Real jacobian_determinant() const
  {
    return m_jacobian_determinant;
  }

  const typename EtypeT::CoordsT& normal(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::normal(mapped_coords, m_nodes, m_normal_vector);
    return m_normal_vector;
  }

  const typename EtypeT::CoordsT& normal() const
  {
    return m_normal_vector;
  }

  /// Precompute the shape function matrix
  void compute_shape_functions(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_value(mapped_coords, m_sf);
  }

  /// Precompute jacobian for the given mapped coordinates
  void compute_jacobian(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    compute_jacobian_dispatch(boost::mpl::bool_<EtypeT::dimension == EtypeT::dimensionality>(), mapped_coords);
  }

  /// Precompute the interpolated value (requires a computed EtypeT)
  void compute_coordinates() const
  {
    m_eval_result.noalias() = m_sf * m_nodes;
  }

  /// Precompute normal (if we have a "face" type)
  void compute_normal(const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    compute_normal_dispatch(boost::mpl::bool_<EtypeT::dimension - EtypeT::dimensionality == 1>(), mapped_coords);
  }

private:
  void compute_normal_dispatch(boost::mpl::false_, const typename EtypeT::MappedCoordsT&) const
  {
  }

  void compute_normal_dispatch(boost::mpl::true_, const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::normal(mapped_coords, m_nodes, m_normal_vector);
  }

  void compute_jacobian_dispatch(boost::mpl::false_, const typename EtypeT::MappedCoordsT&) const
  {
  }

  void compute_jacobian_dispatch(boost::mpl::true_, const typename EtypeT::MappedCoordsT& mapped_coords) const
  {
    EtypeT::compute_jacobian(mapped_coords, m_nodes, m_jacobian_matrix);
    bool is_invertible;
    m_jacobian_matrix.computeInverseAndDetWithCheck(m_jacobian_inverse, m_jacobian_determinant, is_invertible);
    cf3_assert(is_invertible);
  }

  /// Stored node data
  ValueT m_nodes;

  /// Coordinates table
  const common::Table<Real>& m_coordinates;

  /// Connectivity table
  const common::Table<Uint>& m_connectivity;

  /// Index for the current element
  Uint m_element_idx;

  /// Temp storage for non-scalar results
  mutable typename EtypeT::SF::ValueT m_sf;
  mutable typename EtypeT::CoordsT m_eval_result;
  mutable typename EtypeT::JacobianT m_jacobian_matrix;
  mutable typename EtypeT::JacobianT m_jacobian_inverse;
  mutable Real m_jacobian_determinant;
  mutable typename EtypeT::CoordsT m_normal_vector;
};

/// Helper function to find a field starting from a region
inline mesh::Field& find_field(mesh::Elements& elements, const std::string& tag)
{
  mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
  return common::find_component_recursively_with_tag<mesh::Field>(mesh, tag);
}

/// Dummy shape function type used for element-based fields
struct ElementBased
{
};

/// Data associated with field variables
template<typename ETYPE, typename SupportEtypeT, Uint Dim, bool IsEquationVar>
class EtypeTVariableData
{
public:
  /// The shape function type
  typedef ETYPE EtypeT;

private:
  /// Interpolation of a field
  template<Uint VarDim, int Dummy=0>
  struct InterpolationImpl
  {
    typedef Eigen::Matrix<Real, 1, VarDim> MatrixT;
    typedef const MatrixT& result_type;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    template<typename NodeValuesT>
    result_type operator()(const typename EtypeT::SF::ValueT& sf, const NodeValuesT& values) const
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
    result_type operator()(const typename EtypeT::SF::ValueT& sf, const NodeValuesT& values) const
    {
      stored_result = sf * values;
      return stored_result;
    }

    mutable Real stored_result;
  };

public:

  typedef GeometricSupport<SupportEtypeT> SupportT;

  /// The value type for all element values
  typedef Eigen::Matrix<Real, EtypeT::nb_nodes, Dim> ValueT;

  /// Return type of the value() method
  typedef const ValueT& ValueResultT;

  /// Type for passing mapped coordinates
  typedef typename EtypeT::MappedCoordsT MappedCoordsT;

  /// The result type of an interpolation at given mapped coordinates
  typedef typename InterpolationImpl<Dim>::result_type EvalT;

  // Specialization of InterpolationImpl should ensure that 1x1 matrices are replaced by Reals
  BOOST_MPL_ASSERT_NOT(( boost::is_same<EvalT, const Eigen::Matrix<Real, 1, 1>&> ));

  /// Type of the gradient
  typedef typename EtypeT::SF::GradientT GradientT;

  /// Type of the linearized form of the divergence
  typedef Eigen::Matrix<Real, 1, Dim * EtypeT::nb_nodes> DivergenceLinT;

  /// The dimension of the variable
  static const Uint dimension = Dim;

  /// True if this variable is an unknow in the system of equations
  static const bool is_equation_variable = IsEquationVar;

  /// We store data as a fixed-size Eigen matrix, so we need to make sure alignment is respected
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  template<typename VariableT>
  EtypeTVariableData(const VariableT& placeholder, mesh::Elements& elements, const SupportT& support) :
    m_field(find_field(elements, placeholder.field_tag())),
    m_connectivity(elements.node_connectivity()),
    m_support(support)
  {
    offset = m_field.descriptor().offset(placeholder.name());
  }

  /// Update nodes for the current element
  void set_element(const Uint element_idx)
  {
    m_element_idx = element_idx;
    mesh::fill(m_element_values, m_field, m_connectivity[element_idx], offset);
  }

  const common::Table<Uint>::ConstRow element_connectivity() const
  {
    return m_connectivity[m_element_idx];
  }

  /// Reference to the geometric support
  const SupportT& support() const
  {
    return m_support;
  }

  /// Reference to the stored data, i.e. the matrix of nodal values
  ValueResultT value() const
  {
    return m_element_values;
  }

  /// Precompute all the cached values for the given geometric support and mapped coordinates.
  void compute_values(const MappedCoordsT& mapped_coords) const
  {
    compute_values_dispatch(boost::mpl::bool_<EtypeT::dimension == EtypeT::dimensionality>(), mapped_coords);
  }

  /// Calculate and return the interpolation at given mapped coords
  EvalT eval(const MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_value(mapped_coords, m_sf);
    return m_eval(m_sf, m_element_values);
  }

  /// Return previously computed evaluation
  EvalT eval() const
  {
    return m_eval.stored_result;
  }


  /// Shape function matrix at mapped coordinates (calculates and returns)
  const typename EtypeT::SF::ValueT& shape_function(const MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_value(mapped_coords, m_sf);
    return m_sf;
  }

  /// Previously calculated shape function matrix
  const typename EtypeT::SF::ValueT& shape_function() const
  {
    return m_sf;
  }

  /// Return the gradient matrix
  const GradientT& nabla(const MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_gradient(mapped_coords, m_mapped_gradient_matrix);
    m_gradient.noalias() = m_support.jacobian(mapped_coords).inverse() * m_mapped_gradient_matrix;
    return m_gradient;
  }

  /// Previously calculated gradient matrix
  const GradientT& nabla() const
  {
    return m_gradient;
  }

  /// Index of where the variable we need is in the field data row
  Uint offset;

private:
  /// Precompute for non-volume EtypeT
  void compute_values_dispatch(boost::mpl::false_, const MappedCoordsT& mapped_coords) const
  {
    EtypeT::SF::compute_value(mapped_coords, m_sf);
    m_eval(m_sf, m_element_values);
  }

  /// Precompute for volume EtypeT
  void compute_values_dispatch(boost::mpl::true_, const MappedCoordsT& mapped_coords) const
  {
    compute_values_dispatch(boost::mpl::false_(), mapped_coords);
    EtypeT::SF::compute_gradient(mapped_coords, m_mapped_gradient_matrix);
    m_gradient.noalias() = m_support.jacobian_inverse() * m_mapped_gradient_matrix;
  }

  /// Value of the field in each element node
  ValueT m_element_values;

  /// Data table
  const mesh::Field& m_field;

  /// Connectivity table
  const mesh::Connectivity& m_connectivity;

  /// Gemetric support
  const SupportT& m_support;

  Uint m_element_idx;

  /// Cached data
  mutable typename EtypeT::SF::ValueT m_sf;
  mutable typename EtypeT::SF::GradientT m_mapped_gradient_matrix;
  mutable GradientT m_gradient;

  InterpolationImpl<Dim> m_eval;
};

/// Data for element-based fields
template<typename SupportEtypeT, Uint Dim, bool IsEquationVar>
class EtypeTVariableData<ElementBased, SupportEtypeT, Dim, IsEquationVar>
{

  BOOST_MPL_ASSERT_MSG(
    Dim == 1
    , VECTOR_ELEMENT_FIELDS_NOT_SUPPORTED
    , (EtypeTVariableData<ElementBased, SupportEtypeT, Dim, IsEquationVar>)
  );

public:
  typedef ElementBased EtypeT;

  /// Type of returned value
  typedef Real& ValueResultT;

  /// Data type for the geometric support
  typedef GeometricSupport<SupportEtypeT> SupportT;

  /// The dimension of the variable
  static const Uint dimension = Dim;

  /// True if this variable is an unknow in the system of equations
  static const bool is_equation_variable = IsEquationVar;

  template<typename VariableT>
  EtypeTVariableData(const VariableT& placeholder, mesh::Elements& elements, const SupportT& support) :
    m_field(find_field(elements, placeholder.field_tag())),
    m_support(support),
    m_elements_begin(m_field.field_group().space(elements).elements_begin()),
    offset(m_field.descriptor().offset(placeholder.name()))
  {
  }

  /// Update nodes for the current element
  void set_element(const Uint element_idx)
  {
    m_field_idx = element_idx + m_elements_begin;
  }

  Real& value()
  {
    return m_field[m_field_idx][offset];
  }

private:
  mesh::Field& m_field;
  const SupportT& m_support;
  const Uint m_elements_begin;
  Uint m_field_idx;

public:
  /// Index in the field array for this variable
  const Uint offset;
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
template<typename VariablesT, typename SupportEtypeT, typename ShapeFunctionsT, typename EquationVariablesT, typename MatrixSizesT, typename EMatrixSizeT>
struct MakeVarData
{
  template<typename I>
  struct apply
  {
    typedef typename boost::mpl::at<VariablesT, I>::type VarT;
    typedef typename boost::mpl::at<ShapeFunctionsT, I>::type EtypeT;
    typedef typename boost::mpl::at<EquationVariablesT, I>::type IsEquationVar;
    typedef typename boost::mpl::if_<IsEquationVar, EMatrixSizeT, typename boost::mpl::at<MatrixSizesT, I>::type>::type MatSize;

    typedef typename boost::mpl::if_c<EtypeT::order == 0, ElementBased, EtypeT>::type EEtypeT;

    typedef typename boost::mpl::if_
    <
      boost::mpl::is_void_<VarT>,
      boost::mpl::void_,
      EtypeTVariableData<EEtypeT, SupportEtypeT, FieldWidth<VarT, EtypeT>::value, IsEquationVar::value>*
    >::type type;
  };
};

template<typename VariablesT, typename EtypeT, typename EquationVariablesT, typename MatrixSizesT, typename EMatrixSizeT>
struct MakeVarData<VariablesT, EtypeT, EtypeT, EquationVariablesT, MatrixSizesT, EMatrixSizeT>
{
  template<typename I>
  struct apply
  {
    typedef typename boost::mpl::at<VariablesT, I>::type VarT;
    typedef typename boost::mpl::at<EquationVariablesT, I>::type IsEquationVar;
    typedef typename boost::mpl::if_<IsEquationVar, EMatrixSizeT, typename boost::mpl::at<MatrixSizesT, I>::type>::type MatSize;

    typedef typename boost::mpl::if_
    <
      boost::mpl::is_void_<VarT>,
      boost::mpl::void_,
      EtypeTVariableData<EtypeT, EtypeT, FieldWidth<VarT, EtypeT>::value, IsEquationVar::value>*
    >::type type;
  };
};

/// Stores data that is used when looping over elements to execute Proto expressions. "Data" is meant here in the boost::proto sense,
/// i.e. it is intended for use as 3rd argument for proto transforms.
/// VariablesT is a fusion sequence containing each unique variable in the expression
/// VariablesDataT is a fusion sequence of pointers to the data (also in proto sense) associated with each of the variables
/// SupportEtypeT is the shape function for the geometric support
template<typename VariablesT, typename VariablesEtypeTT, typename SupportEtypeT, typename EquationVariablesT>
class ElementData
{
public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
  /// Number of variales that we have stored
  typedef typename boost::fusion::result_of::size<VariablesT>::type NbVarsT;

  /// Shape function for the support
  typedef SupportEtypeT SupportShapeFunction;
  
  /// The dimension of the problem
  static const Uint dimension = SupportShapeFunction::dimension;

  /// Element matrix size per var
  typedef typename MatrixSizePerVar<VariablesT, VariablesEtypeTT>::type MatrixSizesT;

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
      MakeVarData<VariablesT, SupportEtypeT, VariablesEtypeTT, EquationVariablesT, MatrixSizesT, EMatrixSizeT>
    >::type
  >::type VariablesDataT;

  /// A view of only the data used in the element matrix
  typedef boost::fusion::filter_view< VariablesDataT, IsEquationData > EquationDataT;

  ElementData(VariablesT& variables, mesh::Elements& elements) :
    m_variables(variables),
    m_elements(elements),
    m_support(elements),
    m_equation_data(m_variables_data)
  {
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(InitVariablesData(m_variables, m_elements, m_variables_data, m_support));
    for(Uint i = 0; i != CF3_PROTO_MAX_ELEMENT_MATRICES; ++i)
      m_element_matrices[i].setZero();

    // TODO Fix for multi-shapefunction case
    BOOST_MPL_ASSERT_MSG(EMatrixSizeT::value%SupportShapeFunction::nb_nodes == 0, ERROR_CALCULATING_NUMBER_OF_DOFS, (boost::mpl::int_<SupportShapeFunction::nb_nodes>));
    block_accumulator.resize(SupportShapeFunction::nb_nodes, EMatrixSizeT::value/SupportShapeFunction::nb_nodes);
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
    update_blocks(typename boost::fusion::result_of::empty<EquationDataT>::type());
  }

  /// Update block accumulator only if a system of equations is accessed in the expressions
  void update_blocks(boost::mpl::false_)
  {
    block_accumulator.reset();
    m_support.update_block_connectivity(block_accumulator);
  }

  void update_blocks(boost::mpl::true_)
  {
  }

  /// Precompute element matrices, for the variables found in expr
  template<typename ExprT>
  void precompute_element_matrices(const typename SupportEtypeT::MappedCoordsT& mapped_coords, const ExprT& e)
  {
    // TODO: Add some granularity in here
    m_support.compute_shape_functions(mapped_coords);
    m_support.compute_coordinates();
    m_support.compute_jacobian(mapped_coords);
    m_support.compute_normal(mapped_coords);
    boost::mpl::for_each< boost::mpl::range_c<int, 0, NbVarsT::value> >(PrecomputeData<ExprT>(m_variables_data, mapped_coords));
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

  typedef GeometricSupport<SupportEtypeT> SupportT;

  /// Return the data stored at index I
  template<typename I>
  typename DataType<I>::type& var_data(const I&)
  {
    return *boost::fusion::at<typename IndexType<I>::type>(m_variables_data);
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

  /// Stores a mutable block accululator, always up-to-date with index mapping and correct size
  mutable math::LSS::BlockAccumulator block_accumulator;

private:
  /// Variables used in the expression
  VariablesT& m_variables;

  /// Referred Elements
  mesh::Elements& m_elements;

  /// Data for the geometric support
  SupportT m_support;

  /// Data associated with each numbered variable
  VariablesDataT m_variables_data;

  Uint m_element_idx;

  ElementMatrixT m_element_matrices[CF3_PROTO_MAX_ELEMENT_MATRICES];
  ElementVectorT m_element_rhs;

  /// Filtered view of the data associated with equation variables
  const EquationDataT m_equation_data;

  ///////////// helper functions and structs /////////////

  /// Initializes the pointers in a VariablesDataT fusion sequence
  struct InitVariablesData
  {
    InitVariablesData(VariablesT& vars, mesh::Elements& elems, VariablesDataT& vars_data, const SupportT& sup) :
      m_variables(vars),
      m_elements(elems),
      m_variables_data(vars_data),
      m_support(sup)
    {
    }

    template<typename I>
    void operator()(const I&)
    {
      apply(boost::fusion::at<I>(m_variables), boost::fusion::at<I>(m_variables_data));
    }

    void apply(const boost::mpl::void_&, const boost::mpl::void_&)
    {
    }

    template<typename VarT, typename VarDataT>
    void apply(const VarT& v, VarDataT*& d)
    {
      d = new VarDataT(v, m_elements, m_support);
    }

    VariablesT& m_variables;
    mesh::Elements& m_elements;
    VariablesDataT& m_variables_data;
    const SupportT& m_support;
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

  /// Precompute variables data
  template<typename ExprT>
  struct PrecomputeData
  {
    PrecomputeData(VariablesDataT& vars_data, const typename SupportEtypeT::MappedCoordsT& mapped_coords) :
      m_variables_data(vars_data),
      m_mapped_coords(mapped_coords)
    {
    }

    template<typename I>
    void operator()(const I&)
    {
      apply(typename boost::result_of<UsesVar<I::value>(ExprT)>::type(), boost::fusion::at<I>(m_variables_data));
    }

    void apply(boost::mpl::false_, const boost::mpl::void_&)
    {
    }

    template<typename T>
    void apply(boost::mpl::true_, T*& d)
    {
      d->compute_values(m_mapped_coords);
    }

    // Variable is not used - do nothing
    template<typename T>
    void apply(boost::mpl::false_, T*& d)
    {
    }

  private:
    VariablesDataT& m_variables_data;
    const typename SupportEtypeT::MappedCoordsT& m_mapped_coords;
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
        element_vector.template segment<rows>( (data->offset + i)*rows ) = v.col(i);
      }
    }

    ElementVectorT& element_vector;
  };
};

} // namespace Proto
} // namespace actions
} // namespace solver
} // namespace cf3

#endif // cf3_solver_actions_Proto_ElementData_hpp
