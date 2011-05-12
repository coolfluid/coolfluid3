// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Solver_Actions_Proto_Terminals_hpp
#define CF_Solver_Actions_Proto_Terminals_hpp

#include<iostream>

#include <boost/proto/core.hpp>

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

/// Using this on a type always gives a compile error showing the type of T
template<typename T>
void print_error()
{
  T::print_error();
}

template<typename T>
void print_error(const T&)
{
  T::print_error();
}
  
/// Creates a variable that has unique ID I
template<typename I, typename T>
struct Var : I
{
  /// Type that is wrapped
  typedef T type;
  Var() {}

  /// Index of the var
  typedef I index_type;

  template<typename T1>
  Var(const T1& par1) : variable_value(par1) {}

  template<typename T1>
  Var(T1& par1) : variable_value(par1) {}

  template<typename T1, typename T2>
  Var(const T1& par1, const T2& par2) : variable_value(par1, par2) {}

  template<typename T1, typename T2>
  Var(T1& par1, T2& par2) : variable_value(par1, par2) {}
  
  template<typename T1, typename T2, typename T3>
  Var(const T1& par1, const T2& par2, const T3& par3) : variable_value(par1, par2, par3) {}

  template<typename T1, typename T2, typename T3>
  Var(T1& par1, T2& par2, T3& par3) : variable_value(par1, par2, par3) {}
  
  type variable_value;
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
  virtual ~OptionVariable()
  {
  }

  /// Add auto-generated options to the supplied component (if they don't exist) and attach the triggers (always, even if the option existed)
  virtual void add_options(Common::Component& owner) = 0;

protected:

  template< typename OptionValueT >
  inline typename OptionType<OptionValueT>::type add_option(Common::Component& owner, const std::string& name, const std::string& description, const OptionValueT& default_val, Common::Option::Trigger_t trigger);
};

template< typename OptionValueT >
inline typename OptionType<OptionValueT>::type OptionVariable::add_option(Common::Component& owner, const std::string& name, const std::string& description, const OptionValueT& default_val, Common::Option::Trigger_t trigger)
{
  Common::Option& option = owner.properties().check(name) ? owner.properties().option(name) : *owner.properties().add_option< Common::OptionT<OptionValueT> >(name, description, default_val);
  option.mark_basic();
  option.attach_trigger( trigger );
  boost::shared_ptr< Common::OptionT<OptionValueT> > cast_option = boost::dynamic_pointer_cast< Common::OptionT<OptionValueT> >(option.shared_from_this());
  if(is_null(cast_option))
    throw Common::CastingFailed(FromHere(),"Option could not be cast");
  return cast_option;
}

template<>
inline OptionType<Common::URI>::type OptionVariable::add_option<Common::URI>(Common::Component& owner, const std::string& name, const std::string& description, const Common::URI& default_val, Common::Option::Trigger_t trigger)
{
  Common::Option& option = owner.properties().check(name) ? owner.properties().option(name) : *owner.properties().add_option< Common::OptionURI >(name, description, default_val);
  option.mark_basic();
  option.attach_trigger( trigger );
  boost::shared_ptr< Common::OptionURI > cast_option = boost::dynamic_pointer_cast< Common::OptionURI >(option.shared_from_this());
  if(is_null(cast_option))
    throw Common::CastingFailed(FromHere(),"Option could not be cast");
  return cast_option;
}

/// Base class for field data
struct FieldBase : OptionVariable
{
  FieldBase() :
    field_name("aField"),
    var_name("aVariable"),
    m_internal_name("aVariable")
  {
  }

  /// Construct a new field placeholder
  /// @param name Readable name for the represented quantity (i.e. Temperature)
  /// @param symbol Short symbol to name the variable (i.e. T)
  /// @param field_nm Name for the field to use by default
  FieldBase(const std::string& name, const std::string& symbol, const std::string& field_nm) :
    field_name(field_nm),
    var_name(symbol),
    m_internal_name(name)
  {
  }

  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.element_type();
    //return is_const ? elements.element_type() : elements.get_field_elements(field_name).element_type();
  }

  virtual void add_options(Common::Component& owner)
  {
    m_field_option = add_option<std::string>
    (
      owner,
      m_internal_name + std::string("FieldName"),
      "Field name for variable " + m_internal_name,
      field_name,
      boost::bind(&FieldBase::on_field_changed, this)
    );
    cf_assert(is_not_null(m_field_option.lock()));
    m_var_option = add_option<std::string>
    (
      owner,
      m_internal_name + std::string("VariableName"),
      "Variable name for variable " + m_internal_name,
      var_name,
      boost::bind(&FieldBase::on_var_changed, this)
    );
    cf_assert(is_not_null(m_var_option.lock()));
  }

  /// Name of the referred field
  std::string field_name;
  
  /// Name of the variable
  std::string var_name;
  
  inline const std::string& internal_name() const
  {
    return m_internal_name;
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
  
  /// Internal name, guaranteed to remain the same irrespective of user settings
  std::string m_internal_name;
};

/// Field data for a scalar field
struct ScalarField : FieldBase
{
  ScalarField() : FieldBase() {}
  ScalarField(const std::string& varname, const std::string& symbol) : FieldBase(varname, symbol, varname) {}
  ScalarField(const std::string& varname, const std::string& symbol, const std::string& field_nm) : FieldBase(varname, symbol, field_nm) {}
};

/// Field data for a vector having the dimension of the problem
struct VectorField : FieldBase
{
  VectorField() : FieldBase() {}
  VectorField(const std::string& varname, const std::string& symbol) : FieldBase(varname, symbol, varname) {}
  VectorField(const std::string& varname, const std::string& symbol, const std::string& field_nm) : FieldBase(varname, symbol, field_nm) {}
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

  template<typename T1, typename T2, typename T3>
  MeshTerm(const T1& par1, const T2& par2, const T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

  template<typename T1, typename T2, typename T3>
  MeshTerm(T1& par1, T2& par2, T3& par3) : base_type(boost::proto::make_expr<boost::proto::tag::terminal>(Var<boost::mpl::int_<I>, T>(par1, par2, par3))) {}

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
static boost::proto::terminal< double(*)(double) >::type const _sqrt = {&sqrt};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_Terminals_hpp
