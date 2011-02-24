// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_UpdateSolution_hpp
#define CF_FVM_UpdateSolution_hpp

#include "Solver/Actions/CLoopOperation.hpp"
#include "FVM/LibFVM.hpp"
#include "FVM/RoeFluxSplitter.hpp"

#include "Mesh/CCellFaces.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/CFaceCellConnectivity.hpp"
#include "Mesh/CTable.hpp"
#include "Common/Foreach.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField2; }
namespace Solver { class CTime;   }
namespace FVM {

class FVM_API UpdateSolution : public Common::CAction
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<UpdateSolution> Ptr;
  typedef boost::shared_ptr<UpdateSolution const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  UpdateSolution ( const std::string& name );

  /// Virtual destructor
  virtual ~UpdateSolution() {};

  /// Get the class name
  static std::string type_name () { return "UpdateSolution"; }
  
  /// execute the action
  virtual void execute ();
  
private: // data

  boost::weak_ptr<Mesh::CField2> m_solution;
  boost::weak_ptr<Mesh::CField2> m_residual;  
  boost::weak_ptr<Mesh::CField2> m_update_coeff;
};

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_UpdateSolution_hpp
