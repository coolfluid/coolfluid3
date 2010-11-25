// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Solver_CMethod_hpp
#define CF_Solver_CMethod_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Common/OptionT.hpp"

#include "Solver/LibSolver.hpp"

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////

/// Solver component class
/// Solver now stores:
///   - regions which subdivide in subregions
///   - arrays containing coordinates, variables, ...
/// @author Tiago Quintino
/// @author Willem Deconinck
class Solver_API CMethod : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CMethod> Ptr;
  typedef boost::shared_ptr<CMethod const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMethod ( const std::string& name );

  /// Virtual destructor
  virtual ~CMethod();

  /// Get the class name
  static std::string type_name () { return "CMethod"; }

  /// Configuration Options
  virtual void define_config_properties ()
  {
    m_properties.add_option< Common::OptionT<bool> >("myBoolMeth", "A boolean value in a CMethod", true);
    m_properties.add_option< Common::OptionT<int> >("fourtyTwo", "An integer value in a CMethod", 42);
    m_properties.add_option< Common::OptionT<CF::Real> >("euler", "Euler number in a CMethod", 2.71);
  }

  // functions specific to the CMethod component
  
  // Signal run_operation
  void run_operation( Common::XmlNode& node ) {}
  
  CMethod& operation(const std::string& name);
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self );

};

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Solver_CMethod_hpp
