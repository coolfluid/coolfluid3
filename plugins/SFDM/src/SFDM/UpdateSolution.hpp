// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_UpdateSolution_hpp
#define CF_SFDM_UpdateSolution_hpp

#include "Common/CAction.hpp"
#include "Mesh/CFieldView.hpp"
#include "SFDM/LibSFDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh   { class CField; }
namespace Solver { class CTime;   }
namespace SFDM {

class SFDM_API UpdateSolution : public Common::CAction
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

  Mesh::CScalarFieldView::Ptr m_update_coeff_view;
  Mesh::CMultiStateFieldView::Ptr m_residual_view;
  Mesh::CMultiStateFieldView::Ptr m_solution_view;

  boost::weak_ptr<Mesh::CField> m_solution;
  boost::weak_ptr<Mesh::CField> m_residual;
  boost::weak_ptr<Mesh::CField> m_update_coeff;
};

////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_UpdateSolution_hpp
