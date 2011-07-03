// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_Model_hpp
#define CF_UFEM_Model_hpp

#include <boost/scoped_ptr.hpp>

#include "LibUFEM.hpp"
#include "Solver/Actions/Proto/CProtoActionDirector.hpp"

namespace CF {

namespace UFEM {

class LinearProblem;
  
/// Abstract base class for a model, providing a mesh, a physical model and a linear problem
class UFEM_API Model : public Solver::Actions::Proto::CProtoActionDirector
{
public: // typedefs

  typedef boost::shared_ptr<Model> Ptr;
  typedef boost::shared_ptr<Model const> ConstPtr;

public: // functions
  
  /// Contructor
  /// @param name of the component
  Model ( const std::string& name );
  
  virtual ~Model();

  /// Get the class name
  static std::string type_name () { return "Model"; }
  
  /// Returns a reference to the actual linear problem that is executed each iteration
  virtual LinearProblem& problem() = 0;
  
  virtual void execute();
  
  /// Reads the mesh, as configured by the options
  void signal_read_mesh(Common::SignalArgs& node);
  
  /// Writes the mesh, as configured by the options
  void signal_write_mesh(Common::SignalArgs& node);
  
private:
  class Implementation;
  boost::scoped_ptr<Implementation> m_implementation;
};

} // UFEM
} // CF


#endif // CF_UFEM_Model_hpp
