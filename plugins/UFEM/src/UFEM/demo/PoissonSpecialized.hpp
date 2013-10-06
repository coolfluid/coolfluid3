// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_UFEM_PoissonSpecialized_hpp
#define cf3_UFEM_PoissonSpecialized_hpp

#include <boost/scoped_ptr.hpp>

#include "UFEM/LSSAction.hpp"
#include "solver/actions/Proto/ProtoAction.hpp"

#include "LibUFEMDemo.hpp"

namespace cf3 {
namespace UFEM {
namespace demo {

/// Poisson problem using generic Proto expressions
class UFEM_API PoissonSpecialized : public LSSAction
{
public: // functions

  /// Contructor
  /// @param name of the component
  PoissonSpecialized ( const std::string& name );

  /// Get the class name
  static std::string type_name () { return "PoissonSpecialized"; }
  
private:
  math::LSS::BlockAccumulator m_block_accumulator;
};

} // demo
} // UFEM
} // cf3


#endif // cf3_UFEM_PoissonSpecialized_hpp
