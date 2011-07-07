// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_Proto_CProtoAction_hpp
#define CF_Solver_Actions_Proto_CProtoAction_hpp

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "Common/CAction.hpp"

namespace CF {
  namespace Common { template<typename T> class OptionComponent; }
  namespace Mesh { class CRegion; }
namespace Solver {
  class CPhysicalModel;
namespace Actions {
namespace Proto {

class Expression;
  
/// Class to encapsulate Proto actions
class CProtoAction : public Common::CAction
{
public:
  typedef boost::shared_ptr< CProtoAction > Ptr;
  typedef boost::shared_ptr< CProtoAction const> ConstPtr;

  CProtoAction(const std::string& name);
  
  ~CProtoAction();

  static std::string type_name() { return "CProtoAction"; }

  void execute();
  
  /// Set the expression. The action retains ownership of the supplied expression
  /// @param expression The proto expression to set
  void set_expression(const boost::shared_ptr<Expression> & expression);
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // namespace Proto
} // namespace Actions
} // namespace Solver
} // namespace CF

#endif // CF_Solver_Actions_Proto_CProtoAction_hpp
