// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_Actions_CNodeOperation_hpp
#define CF_Solver_Actions_CNodeOperation_hpp

#include "Solver/Actions/CLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

class Solver_Actions_API CNodeOperation : public CLoopOperation
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<CNodeOperation> Ptr;
  typedef boost::shared_ptr<CNodeOperation const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  CNodeOperation ( const std::string& name );

  /// Virtual destructor
  virtual ~CNodeOperation() {};

  /// Get the class name
  static std::string type_name () { return "CNodeOperation"; }
    
  /// @return the nodes to loop over
  virtual Mesh::CList<Uint>& loop_list () const = 0;

};

/////////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_CNodeOperation_hpp
