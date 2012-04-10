// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TrilinosDetail_hpp
#define cf3_Math_LSS_TrilinosDetail_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosDetail.hpp Shared functions between Trilinos classes
  @author Tamas Banyai, Bart Janssens

**/

////////////////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace common { namespace PE { class CommPattern; } }
namespace math {
  class VariablesDescriptor;
namespace LSS {

/// Create a local node index to matrix local index lookup
/// @param cp The comm pattern that governs the node distribution
/// @param variables The variables to use. Equations will be grouped per variable
/// @param p2m Mapping from node index to local matrix index
/// @param my_global_elements Indices for each non-zero
/// @param num_my_elements The number of non-ghosts owned by this rank
void create_map_data(cf3::common::PE::CommPattern& cp,
                      const VariablesDescriptor& variables,
                      std::vector<int>& p2m,
                      std::vector<int>& my_global_elements,
                      int& num_my_elements);

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TrilinosDetail_hpp
