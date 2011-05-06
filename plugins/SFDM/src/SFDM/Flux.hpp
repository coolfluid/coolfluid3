// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_Flux_hpp
#define CF_SFDM_Flux_hpp

////////////////////////////////////////////////////////////////////////////////

#include "SFDM/LibSFDM.hpp"
#include "Math/MatrixTypes.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace SFDM {

class ShapeFunction;

//////////////////////////////////////////////////////////////////////////////

/// This class defines an action that Fluxs a state from one
/// shapefunction to another.
/// Example: States defined in solution points need to be Fluxed
///          in Flux points.
/// @author Willem Deconinck
class SFDM_API Flux : public Common::Component
{
public: // typedefs

    typedef boost::shared_ptr<Flux> Ptr;
    typedef boost::shared_ptr<Flux const> ConstPtr;

public: // functions
  
  /// constructor
  Flux( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "Flux"; }

  /// compute the flux of the states
  /// @param states  states with dimension (nb_states , states_size)
  RealMatrix operator()(const RealMatrix& states) const;

private:

}; // end Flux


////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_SFDM_Flux_hpp
