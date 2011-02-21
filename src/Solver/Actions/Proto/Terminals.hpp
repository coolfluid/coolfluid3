// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_Terminals_hpp
#define CF_Solver_Actions_Proto_Terminals_hpp

#include<iostream>

#include <boost/proto/proto.hpp>

#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ElementType.hpp"

/// @file
/// Some commonly used, statically defined terminal types

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {

/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : T
{
  /// Type that is wrapped
  typedef T type;
  Var() : T() {}

  /// Index of the var
  typedef I index_type;

  template<typename T1>
  Var(const T1& par1) : T(par1) {}

  template<typename T1>
  Var(T1& par1) : T(par1) {}

  template<typename T1, typename T2>
  Var(const T1& par1, const T2& par2) : T(par1, par2) {}

  template<typename T1, typename T2>
  Var(T1& par1, T2& par2) : T(par1, par2) {}
};

/// Compute the return type of OptionVariable::add_option
template<typename T>
struct OptionType
{
  typedef boost::shared_ptr< Common::OptionT<T> > type;
};

template<>
struct OptionType<Common::URI>
{
  typedef boost::shared_ptr< Common::OptionURI > type;
};

/// Base class for variables that expose a user-controllable option
class OptionVariable
{
public:
  OptionVariable(const std::string& name, const std::string& description) : m_name(name), m_description(description)
  {
  }

  virtual ~OptionVariable()
  {
  }

  /// Add the contained option, owned by the given component
  void set_owner(const Common::Component::Ptr& owner)
  {
    m_owner = owner;
    add_options();
  }

protected:

  template< typename OptionValueT >
  inline typename OptionType<OptionValueT>::type add_option(const std::string& name, const std::string& description, Common::Option::Trigger_t trigger);

  /// Implement this to add the required options
  virtual void add_options() = 0;

  /// Name of the variable (and option)
  std::string m_name;

  /// Description of the option
  std::string m_description;

  /// Component that owns this variable, or null if it doesn't exist
  boost::weak_ptr<Common::Component> m_owner;
};

template< typename OptionValueT >
inline typename OptionType<OptionValueT>::type OptionVariable::add_option(const std::string& name, const std::string& description, Common::Option::Trigger_t trigger)
{
  Common::Option::Ptr option = m_owner.lock()->properties().add_option< Common::OptionT<OptionValueT> >(name, description, OptionValueT());
  option->mark_basic();
  option->attach_trigger( trigger );
  boost::shared_ptr< Common::OptionT<OptionValueT> > cast_option = boost::dynamic_pointer_cast< Common::OptionT<OptionValueT> >(option);
  if(is_null(cast_option))
    throw Common::CastingFailed(FromHere(),"Option could not be cast");
  return cast_option;
}

template<>
inline OptionType<Common::URI>::type OptionVariable::add_option<Common::URI>(const std::string& name, const std::string& description, Common::Option::Trigger_t trigger)
{
  Common::Option::Ptr option = m_owner.lock()->properties().add_option< Common::OptionURI >(name, description, std::string());
  option->mark_basic();
  option->attach_trigger( trigger );
  boost::shared_ptr< Common::OptionURI > cast_option = boost::dynamic_pointer_cast< Common::OptionURI >(option);
  if(is_null(cast_option))
    throw Common::CastingFailed(FromHere(),"Option could not be cast");
  return cast_option;
}

/// Base class for field data
struct FieldBase : OptionVariable
{
  FieldBase() : OptionVariable("aField", "Access to a field")
  {
  }

  FieldBase(const std::string& field_nm, const std::string varname) :
    OptionVariable(field_nm, "Field name for variable " + varname),
    field_name(field_nm),
    var_name(varname)
  {
  }

  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.element_type();
    //return is_const ? elements.element_type() : elements.get_field_elements(field_name).element_type();
  }

  std::string field_name;
  std::string var_name;

protected:
  virtual void add_options()
  {
    m_field_option = add_option<std::string>( m_name + std::string("FieldName"), "Field name", boost::bind(&FieldBase::on_field_changed, this) );
    cf_assert(is_not_null(m_field_option.lock()));
    m_var_option = add_option<std::string>( m_name + std::string("VariableName"), "Variable name", boost::bind(&FieldBase::on_var_changed, this) );
    cf_assert(is_not_null(m_var_option.lock()));
  }

private:  
  /// Called when the field name option is changed
  void on_field_changed()
  {
    field_name = m_field_option.lock()->value<std::string>();
  }

  /// Called when the var name option is changed
  void on_var_changed()
  {
    var_name = m_var_option.lock()->value<std::string>();
  }
  
  /// Option for the field name
  boost::weak_ptr< Common::OptionT<std::string> > m_field_option;

  /// Option for the variable name
  boost::weak_ptr< Common::OptionT<std::string> > m_var_option;
};

/// Constant field data
template<typename T>
struct ConstField : FieldBase
{
  ConstField() : FieldBase(), is_const(false)
  {
  }
  
  ConstField(const std::string& name, const T& val) :
    FieldBase(name, name),
    is_const(true),
    value(val)
  {
  }

  ConstField(const std::string& field_nm, const std::string varname) :
    FieldBase(field_nm, varname),
    is_const(false)
  {
  }

  bool is_const;
  T value;

protected:
  virtual void add_options()
  {
    FieldBase::add_options();
    m_is_const_option = add_option<bool>( m_name + std::string("IsConst"), "True if this field is just a constant value", boost::bind(&ConstField::on_is_const_changed, this) );
    cf_assert(is_not_null(m_is_const_option.lock()));
    m_value_option = add_option<T>( m_name + std::string("Value"), "Value to use if the field is to be treated as constant", boost::bind(&ConstField::on_value_changed, this) );
    cf_assert(is_not_null(m_value_option.lock()));
  }

private:
  /// Called when the IsConst option is changed
  void on_is_const_changed()
  {
    is_const = m_is_const_option.lock()->template value<bool>();
  }
  
  /// Called when the Value option is changed
  void on_value_changed()
  {
    value = m_value_option.lock()->template value<T>();
  }

  /// Option to indicate the field really is a constant that has the same value everywhere
  boost::weak_ptr< Common::OptionT<bool> > m_is_const_option;

  /// Value to store
  boost::weak_ptr< Common::OptionT<T> > m_value_option;
};

/// Mutable field data
template<typename T>
struct Field : ConstField<T>
{
  Field() : ConstField<T>() {}
  Field(const std::string& field_nm, const std::string var_nm) : ConstField<T>(field_nm, var_nm) {}
  Field(const std::string& name, const T& val) : ConstField<T>(name, val) {}
};

/// Field data for a vector having the dimension of the problem
struct VectorField : FieldBase
{
  VectorField() : FieldBase() {}
  VectorField(const std::string& field_nm, const std::string var_nm) : FieldBase(field_nm, var_nm) {}
};

/// Store a user-configurable constant value, used i.e. for constant boundary conditions or model constants
template<typename T>
struct ConfigurableConstant : OptionVariable
{
  ConfigurableConstant() : OptionVariable("aConfigurableConstant", "Configurable constant")
  {
  }

  ConfigurableConstant(const std::string& name, const std::string& description, const T& value = T()) :
    OptionVariable(name, description),
    stored_value(value)
  {
  }

  T stored_value;

protected:
  virtual void add_options()
  {
    m_value_option = add_option<T>( m_name, "Option to set constant", boost::bind(&ConfigurableConstant::on_value_changed, this) );
  }

private:
  /// Called when the field name option is changed
  void on_value_changed()
  {
    stored_value = m_value_option.lock()->template value<T>();
  }

  /// Option for the field name
  boost::weak_ptr< Common::OptionT<T> > m_value_option;
};

/// Represents storage for an element matrix (i.e. of size nb_nodes x nb_nodes) corresponding to the variable with the index supplied in the template parameter
template<Uint I>
struct ElementMatrix
{
};

/// Shorthand for terminals containing a numbered variable
template<Uint I, typename T>
struct NumberedTermType
{
  typedef typename boost::proto::result_of::make_expr
  <
    boost::proto::tag::terminal
  , Var<boost::mpl::int_<I>, T>
  >::type type;
};

template<Uint I, typename T>
struct MeshTerm :
  boost::proto::extends< typename NumberedTermType<I, T>::type, MeshTerm<I, T> >
{
  typedef boost::proto::extends< typename NumberedTermType<I, T>::type, MeshTerm<I, T> > base_type;

  MeshTerm() : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>())) {}

  template<typename T1>
  MeshTerm(const T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1>
  MeshTerm(T1& par1) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1))) {}

  template<typename T1, typename T2>
  MeshTerm(const T1& par1, const T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  template<typename T1, typename T2>
  MeshTerm(T1& par1, T2& par2) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2))) {}

  BOOST_PROTO_EXTENDS_USING_ASSIGN(MeshTerm)
};

/// Store a given reference, so it can safely be shallow-copied when deep-copying expressions
template<typename T>
struct StoredReference
{
  typedef T value_type;
  explicit StoredReference(T& t) : m_t(&t)
  {
  }
  
  /// Get a reference to the originally referenced object
  T& get() const
  {
    return *m_t;
  }
  
private:
  T* m_t;
};

/// Easily store a reference
template<typename T>
StoredReference<T> store(T& t)
{
  return StoredReference<T>(t);
}

/// Wrap std::cout
static boost::proto::terminal< std::ostream & >::type _cout = {std::cout};

/// Accept a 2D realvector for atan2
inline Real atan_vec(const RealVector2& vec)
{
  return atan2(vec[1], vec[0]);
}

// Wrap some math functions
static boost::proto::terminal< double(*)(double) >::type const _sin = {&sin};
static boost::proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};
static boost::proto::terminal< double(*)(const RealVector2&) >::type const _atan_vec = {&atan_vec};
static boost::proto::terminal< double(*)(double) >::type const _exp = {&exp};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_Terminals_hpp
