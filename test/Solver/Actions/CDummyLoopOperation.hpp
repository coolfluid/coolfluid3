// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CDummyLoopOperation_hpp
#define CF_Solver_Actions_CDummyLoopOperation_hpp

#include "Mesh/CElements.hpp"

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CFieldElements;
  template <typename T> class CList;
}
namespace TestActions {

///////////////////////////////////////////////////////////////////////////////////////

class CDummyLoopOperation : public Solver::Actions::CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CDummyLoopOperation> Ptr;
  typedef boost::shared_ptr<CDummyLoopOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CDummyLoopOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CDummyLoopOperation() {};

  /// Get the class name
  static std::string type_name () { return "CDummyLoopOperation"; }
	
  /// execute the action
  virtual void execute ();
	
		
private: // data

};

} // TestActions
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_TestActions_CDummyLoopOperation_hpp
