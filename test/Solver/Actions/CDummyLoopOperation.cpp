// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/Log.hpp"
#include "Common/RegistLibrary.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Mesh/CFieldElements.hpp"

#include "Solver/Actions/CLoopOperation.hpp"
#include "CDummyLoopOperation.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver::Actions;

namespace CF {
namespace TestActions {

/// Class defines the initialization and termination of the library Actions
class LibTestActions : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<LibTestActions> Ptr;
  typedef boost::shared_ptr<LibTestActions const> ConstPtr;

  /// Constructor
  LibTestActions ( const std::string& name) : Common::CLibrary(name) {   }

public: // functions

  /// @return string of the library namespace
  static std::string library_namespace() { return "CF.TestActions"; }


  /// Static function that returns the module name.
  /// Must be implemented for CLibrary registration
  /// @return name of the library
  static std::string library_name() { return "TestActions"; }

  /// Static function that returns the description of the module.
  /// Must be implemented for CLibrary registration
  /// @return description of the library

  static std::string library_description()
  {
    return "This library implements the TestActions API.";
  }

  /// Gets the Class name
  static std::string type_name() { return "LibTestActions"; }

  /// initiate library
  virtual void initiate() {}

  /// terminate library
  virtual void terminate() {}

}; // end LibTestActions

///////////////////////////////////////////////////////////////////////////////////////

Common::RegistLibrary<LibTestActions> libTestActions;

Common::ComponentBuilder < CDummyLoopOperation, CLoopOperation, LibTestActions > CDummyLoopOperation_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CDummyLoopOperation::CDummyLoopOperation ( const std::string& name ) :
  CNodeOperation(name)
{

}

/////////////////////////////////////////////////////////////////////////////////////

void CDummyLoopOperation::execute()
{
  CFinfo << "  looping index " << idx() << CFendl;
}

/////////////////////////////////////////////////////////////////////////////////////

void CDummyLoopOperation::create_loop_helper ( CElements& geometry_elements )
{
  CFinfo << type_name() << " set to loop over " << geometry_elements.full_path().path() << CFendl;
  m_loop_helper = boost::shared_ptr<LoopHelper> ( new LoopHelper(geometry_elements) );
}

/////////////////////////////////////////////////////////////////////////////////////

CList<Uint>& CDummyLoopOperation::loop_list() const
{
	return m_loop_helper->used_nodes;
}

////////////////////////////////////////////////////////////////////////////////////

} // TestActions
} // CF

////////////////////////////////////////////////////////////////////////////////////

