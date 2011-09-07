// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UFEM_Tags_hpp
#define CF_UFEM_Tags_hpp

#include "UFEM/LibUFEM.hpp"

namespace CF {
namespace UFEM {

////////////////////////////////////////////////////////////////////////////////////////////

/// Class defines the tags for the UFEM solvers
/// @author Bart Janssens
class UFEM_API Tags : public NonInstantiable<Tags> {
public:

  /// Tag used for i.e. the solution field.
  static const char * solution ();

  /// Tag used for the source terms
  static const char * source_terms();

}; // Tags

////////////////////////////////////////////////////////////////////////////////////////////

} // UFEM
} // CF

#endif // CF_UFEM_Tags_hpp
