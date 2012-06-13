// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_ComputeLNorm_hpp
#define cf3_sdm_ComputeLNorm_hpp

#include "common/Action.hpp"

#include "sdm/LibSDM.hpp"

/////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh   { class Field; }
namespace sdm {

class sdm_API ComputeLNorm : public common::Action {

public: // functions
  /// Contructor
  /// @param name of the component
  ComputeLNorm ( const std::string& name );

  /// Virtual destructor
  virtual ~ComputeLNorm() {}

  /// Get the class name
  static std::string type_name () { return "ComputeLNorm"; }

  /// execute the action
  virtual void execute ();

  std::vector<Real> compute_norm(const mesh::Field& field) const;

private:

  Uint compute_nb_rows(const mesh::Field& field) const;
};

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

#endif // cf3_sdm_ComputeLNorm_hpp
