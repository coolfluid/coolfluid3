// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_FVM_Core_PolynomialReconstructor_hpp
#define CF_FVM_Core_PolynomialReconstructor_hpp

#include "Common/Component.hpp"
#include "FVM/Core/LibCore.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Mesh { class CStencilComputerRings; }
namespace FVM {
namespace Core {

///////////////////////////////////////////////////////////////////////////////////////

class FVM_Core_API PolynomialReconstructor : public Common::Component
{
public: // typedefs

  /// pointers
  typedef boost::shared_ptr<PolynomialReconstructor> Ptr;
  typedef boost::shared_ptr<PolynomialReconstructor const> ConstPtr;

public: // functions
  /// Contructor
  /// @param name of the component
  PolynomialReconstructor ( const std::string& name );

  /// Virtual destructor
  virtual ~PolynomialReconstructor() {};

  /// Get the class name
  static std::string type_name () { return "PolynomialReconstructor"; }

  void set_cell(const Uint unified_elem_idx);

private: // helper functions
private: // data

  boost::shared_ptr<Mesh::CStencilComputerRings> m_stencil_computer;

  Uint m_order;
  Uint m_dim;
  
  std::vector<std::vector<Uint> > m_stencils;
  std::vector<std::vector<Real> > m_weights;
  
  Uint m_idx;
};

////////////////////////////////////////////////////////////////////////////////

} // Core
} // FVM
} // CF

/////////////////////////////////////////////////////////////////////////////////////

#endif // CF_FVM_Core_PolynomialReconstructor_hpp
