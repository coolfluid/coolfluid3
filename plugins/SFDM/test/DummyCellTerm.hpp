// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_DummyCellTerm_hpp
#define CF_SFDM_DummyCellTerm_hpp

#include "SFDM/CellTerm.hpp"

namespace CF {
namespace SFDM {

/////////////////////////////////////////////////////////////////////////////////////

class SFDM_API DummyCellTerm : public CellTerm {

public: // typedefs

  /// provider
  typedef boost::shared_ptr< DummyCellTerm > Ptr;
  typedef boost::shared_ptr< DummyCellTerm const > ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  DummyCellTerm ( const std::string& name );

  /// Virtual destructor
  virtual ~DummyCellTerm();

  /// Get the class name
  static std::string type_name () { return "DummyCellTerm"; }

  virtual void execute();

};

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

#endif // CF_SFDM_DummyCellTerm_hpp
