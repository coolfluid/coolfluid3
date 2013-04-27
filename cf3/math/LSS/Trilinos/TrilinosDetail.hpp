// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Math_LSS_TrilinosDetail_hpp
#define cf3_Math_LSS_TrilinosDetail_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include "common/CF.hpp"
#include "common/List.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

/**
  @file TrilinosDetail.hpp Shared functions between Trilinos classes
  @author Tamas Banyai, Bart Janssens

**/

////////////////////////////////////////////////////////////////////////////////////////////

class Epetra_Operator;
namespace cf3 {
  namespace common { namespace PE { class CommPattern; } }
namespace math {
  class VariablesDescriptor;
namespace LSS {

class Vector;

/// Create a local node index to matrix local index lookup
/// @param cp The comm pattern that governs the node distribution
/// @param variables The variables to use. Equations will be grouped per variable
/// @param p2m Mapping from node index to local matrix index
/// @param my_global_elements GID for each column in the matrix
/// @param num_my_elements The number of non-ghosts owned by this rank. This corresponds to the first num_my_elements rows items in my_global elements
void create_map_data(cf3::common::PE::CommPattern& cp,
                      const VariablesDescriptor& variables,
                      std::vector<int>& p2m,
                      std::vector<int>& my_global_elements,
                      std::vector<Uint>& my_ranks,
                      int& num_my_elements,
                      const std::vector<Uint>& periodic_links_nodes = std::vector<Uint>(),
                      const std::vector<bool>& periodic_links_active = std::vector<bool>());

/// Create the sparsitystructure of the matrix in a generic way
/// @param cp The comm pattern that governs the node distribution
/// @param variables The variables to use. Equations will be grouped per variable
/// @param node_connectivity The connected nodes for each node
/// @param starting_indices For each node, the start index into node_connectivity to find its connected nodes
/// @param p2m Mapping from node index to local matrix index
/// @param num_indices_per_row Will contain the number of indices for each row
/// @param indices_per_row Flattened list of the indices for each row
/// @param periodic_links_nodes For each node, its periodic link. Empty if no periodicity
/// @param periodic_links_active For each node, indicate if it has a periodic link. Empty if no periodicity.
void create_indices_per_row(cf3::common::PE::CommPattern& cp,
                     const VariablesDescriptor& variables,
                     const std::vector<Uint>& node_connectivity,
                     const std::vector<Uint>& starting_indices,
                     const std::vector<int>& p2m,
                     std::vector<int>& num_indices_per_row,
                     std::vector<int>& indices_per_row,
                     const std::vector<Uint>& periodic_links_nodes = std::vector<Uint>(),
                     const std::vector<bool>& periodic_links_active = std::vector<bool>()
                    );

/// Compute y = alpha*op*x + beta*y
void apply_matrix(const Epetra_Operator& op, const Handle<Vector>& y, const Handle<Vector const>& x, const Real alpha = 1., const Real beta = 0.);

} // namespace LSS
} // namespace math
} // namespace cf3

#endif // cf3_Math_LSS_TrilinosDetail_hpp
