// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Actions_Proto_Terminals_hpp
#define CF_Actions_Proto_Terminals_hpp

#include<iostream>

#include <boost/proto/proto.hpp>

#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/ElementType.hpp"

#include "Solver/CPhysicalModel.hpp"


/// @file
/// Some commonly used, statically defined terminal types

namespace CF {
namespace Common {

/// Specialize OptionT for URI so it can be used uniformy with other types
template<>
class OptionT<URI> : public OptionURI
{
public:
  OptionT(const std::string & name, const std::string & desc,
              const URI & def) : OptionURI(name, desc, def)
  {
  }
};

} // Common
} // CF

namespace CF {
namespace Actions {
namespace Proto {

// Placeholders for different types of data
struct ElementIdxHolder {};
boost::proto::terminal<ElementIdxHolder>::type const _elem = {{}}; // Represents an element index ({{{}}} makes this a static initialization)

struct ElementNodeIdxHolder {};
boost::proto::terminal<ElementNodeIdxHolder>::type const _elem_node = {{}}; // Represents a node index relative to the element

struct MappedCoordHolder {};
boost::proto::terminal<MappedCoordHolder>::type const _mapped_coord = {{}};

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
  typename boost::shared_ptr< Common::OptionT<OptionValueT> > add_option(const std::string& name, const std::string& description, Common::Option::Trigger_t trigger)
  {
    Common::Option::Ptr option = m_owner.lock()->properties().add_option< Common::OptionT<OptionValueT> >(m_name, m_description, OptionValueT());
    option->mark_basic();
    option->attach_trigger( trigger );
    return boost::dynamic_pointer_cast< Common::OptionT<OptionValueT> >(option);
  }
  
  /// Implement this to add the required options
  virtual void add_options() = 0;
  
  /// Name of the variable (and option)
  std::string m_name;
  
  /// Description of the option
  std::string m_description;
  
  /// Component that owns this variable, or null if it doesn't exist
  boost::weak_ptr<Common::Component> m_owner;
};

/// Represent const element nodes. Using this in an expression is like passing the nodes of the current element
struct ConstNodes : OptionVariable
{
  ConstNodes() : OptionVariable("aConstNodes", "Geometric support region")
  {
  }
  
  ConstNodes(const std::string& aname, Mesh::CRegion::Ptr r = Mesh::CRegion::Ptr()) :
    OptionVariable(aname, "Geometric support region for " + aname),
    m_region(r)
  {
  }
  
  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.element_type();
  }
  
  Mesh::CRegion& region()
  {
    return *m_region.lock();
  }

protected:
  virtual void add_options()
  {
    m_region_path = add_option<Common::URI>(m_name, m_description, boost::bind(&ConstNodes::on_trigger, this));
    m_region_path.lock()->supported_protocol("cpath");
    m_physical_model = Common::find_component_ptr<Solver::CPhysicalModel>(*m_owner.lock()->get_parent());
  }

private:
  /// Root region with which the nodes are associated
  boost::weak_ptr<Mesh::CRegion> m_region;
  boost::weak_ptr<Solver::CPhysicalModel> m_physical_model;
  
  /// Option with the path to the region
  boost::weak_ptr<Common::OptionURI> m_region_path;
  
  void on_trigger()
  {
    m_region = m_owner.lock()->look_component_type<Mesh::CRegion>( m_region_path.lock()->value<std::string>() );
    m_physical_model.lock()->configure_property("DOFs", region().recursive_nodes_count());
  }
};

/// Constant field data
template<typename T>
struct ConstField : OptionVariable
{
  ConstField() : OptionVariable("aConstField", "Const access to a field")
  {
  }
  
  ConstField(const std::string& name) :
    OptionVariable(name, "Const access to a field")
  {
  }
  
  ConstField(const std::string& field_nm, const std::string varname) :
    OptionVariable(varname, "Field name for variable " + varname),
    field_name(field_nm),
    var_name(varname)
  {
  }
  
  /// Get the element type, based on the CElements currently traversed.
  const Mesh::ElementType& element_type(const Mesh::CElements& elements) const
  {
    return elements.get_field_elements(field_name).element_type();
  }
  
  std::string field_name;
  std::string var_name;
  
protected:
  virtual void add_options()
  {
    m_field_option = add_option<std::string>( m_name + "FieldName", "Field name", boost::bind(&ConstField::on_field_changed, this) );
    m_var_option = add_option<std::string>( m_name + "VariableName", "Variable name", boost::bind(&ConstField::on_var_changed, this) );
  }
  
private:
  /// Called when the field name option is changed
  void on_field_changed()
  {
    field_name = m_field_option.lock()->template value<std::string>();
  }
  
  /// Called when the var name option is changed
  void on_var_changed()
  {
    var_name = m_var_option.lock()->template value<std::string>();
  }
  
  /// Option for the field name
  boost::weak_ptr< Common::OptionT<std::string> > m_field_option;
  
  /// Option for the variable name
  boost::weak_ptr< Common::OptionT<std::string> > m_var_option;
  
};

/// Mutable field data
template<typename T>
struct Field : ConstField<T>
{
  Field() : ConstField<T>() {}
  Field(const std::string& name) : ConstField<T>(name) {}
  Field(const std::string& field_nm, const std::string var_nm) : ConstField<T>(field_nm, var_nm) {}
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

  
/// Wrap std::cout
boost::proto::terminal< std::ostream & >::type _cout = {std::cout};

/// Accept a 2D realvector for atan2
Real atan_vec(const RealVector2& vec)
{
  return atan2(vec[1], vec[0]);
}

// Wrap some math functions
boost::proto::terminal< double(*)(double) >::type const _sin = {&sin};
boost::proto::terminal< double(*)(double, double) >::type const _atan2 = {&atan2};
boost::proto::terminal< double(*)(const RealVector2&) >::type const _atan_vec = {&atan_vec};

} // namespace Proto
} // namespace Actions
} // namespace CF

#endif // CF_Actions_Proto_Terminals_hpp
