// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/RegistLibrary.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"

#include "solver/Actions/CLoopOperation.hpp"
#include "CDummyLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::solver::Actions;

namespace cf3 {
namespace TestActions {

/// Class defines the initialization and termination of the library Actions
class LibTestActions : public common::Library {

public:

  typedef boost::shared_ptr<LibTestActions> Ptr;
  typedef boost::shared_ptr<LibTestActions const> ConstPtr;

  /// Constructor
  LibTestActions ( const std::string& name) : common::Library(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "cf3.TestActions"; }


  /// Static function that returns the library name.
  /// Must be implemented for Library registration
  /// @return name of the library
  static std::string library_name() { return "TestActions"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for Library registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the TestActions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTestActions"; }

protected:

  /// initiate library
  virtual void initiate_impl() {}

  /// terminate library
  virtual void terminate_impl() {}

}; // end LibTestActions

///////////////////////////////////////////////////////////////////////////////////////

common::RegistLibrary<LibTestActions> libTestActions;

common::ComponentBuilder < CDummyLoopOperation, CLoopOperation, LibTestActions > CDummyLoopOperation_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CDummyLoopOperation::CDummyLoopOperation ( const std::string& name ) :
  CLoopOperation(name)
{

}

/////////////////////////////////////////////////////////////////////////////////////

void CDummyLoopOperation::execute()
{
  CFinfo << "  looping index " << idx() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////////

} // TestActions
} // cf3

////////////////////////////////////////////////////////////////////////////////////

