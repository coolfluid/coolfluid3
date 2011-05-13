// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CProtoElementsAction_hpp
#define CF_Solver_Actions_CProtoElementsAction_hpp

#include "Mesh/SF/Types.hpp"

#include "CProtoFieldAction.hpp"
#include "ElementLooper.hpp"

namespace CF {
namespace Solver {
namespace Actions {
namespace Proto {
 
/// A CAction that runs a proto expression over all cells referred to by the support.
/// The support is deduced from the expression itself
template<typename ExprT>
class CProtoElementsAction : public CProtoFieldAction<ExprT>
{
  typedef CProtoFieldAction<ExprT> BaseT;
  
  /// Supported element types
  typedef boost::mpl::vector4
  <
    Mesh::SF::Line1DLagrangeP1,
    Mesh::SF::Triag2DLagrangeP1,
    Mesh::SF::Quad2DLagrangeP1,
    Mesh::SF::Hexa3DLagrangeP1
  > ElementTypes;
  
public:
  typedef boost::shared_ptr< CProtoElementsAction<ExprT> > Ptr;
  typedef boost::shared_ptr< CProtoElementsAction<ExprT> const> ConstPtr;
  
  CProtoElementsAction(const std::string& name) :
    BaseT(name)
  {
  }
  
  static std::string type_name() { return "CProtoElementsAction"; }
  
  /// Run the expression, looping over all elements
  virtual void execute()
  { 
    // Traverse all CElements under the root and evaluate the expression
    BOOST_FOREACH(Mesh::CElements& elements, Common::find_components_recursively<Mesh::CElements>( BaseT::root_region() ) )
    {
      boost::mpl::for_each<ElementTypes>( ElementLooper<ElementTypes, typename BaseT::CopiedExprT>(elements, *BaseT::m_expr, BaseT::m_variables) );
    }
  }
};

/// Returns a configurable CAction object that will execute the supplied expression for all elements
template<typename ExprT>
Common::CAction& build_elements_action(const std::string& name, Common::Component& parent, Common::Component& option_owner, PhysicalModel& physical_model, const ExprT& expr, Common::OptionURI::Ptr region_option = Common::OptionURI::Ptr())
{
  boost::shared_ptr< CProtoElementsAction<ExprT> > result = parent.create_component_ptr< CProtoElementsAction<ExprT> >(name);
  regist_typeinfo(result.get());
  result->set_expression(expr, option_owner, physical_model, region_option);
  return *result;
}

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Actions_CProtoElementsAction_hpp
