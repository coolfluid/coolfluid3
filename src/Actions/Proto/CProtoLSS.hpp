// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Actions_Proto_CProtoLSS_hpp
#define CF_Actions_Proto_CProtoLSS_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Math/MatrixTypes.hpp"

namespace CF {
namespace Actions {
namespace Proto {

////////////////////////////////////////////////////////////////////////////////

/// CProtoLSS component class
/// This class stores a linear system for use by proto expressions
/// @author Bart Janssens
class Mesh_API CProtoLSS : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CProtoLSS> Ptr;
  typedef boost::shared_ptr<CProtoLSS const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CProtoLSS ( const std::string& name ) : Component(name)
  {
  }

  /// Get the class name
  static std::string type_name () { return "CProtoLSS"; }    
  
  /// Set the number of equations
  void resize ( Uint nb_dofs )
  {
    m_system_matrix.resize(nb_dofs, nb_dofs);
    m_rhs.resize(nb_dofs);
  }
  
  Uint size() const
  {
    return m_system_matrix.cols();
  }
  
  /// Reference to the system matrix
  RealMatrix& matrix()
  {
    return m_system_matrix;
  }
  
  /// Reference to the RHS vector
  RealVector& rhs()
  {
    return m_rhs;
  }
  
private:
  /// System matrix
  RealMatrix m_system_matrix;
  
  /// Right hand side
  RealVector m_rhs;

};
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Proto
} // namespace Actions
} // namespace CF


////////////////////////////////////////////////////////////////////////////////

#endif // CF_Actions_Proto_CProtoLSS_hpp
