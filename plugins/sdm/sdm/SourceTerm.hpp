// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_sdm_SourceTerm_hpp
#define cf3_sdm_SourceTerm_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PropertyList.hpp"

#include "math/MatrixTypes.hpp"

#include "mesh/Connectivity.hpp"

#include "sdm/Tags.hpp"
#include "sdm/Term.hpp"
#include "sdm/ShapeFunction.hpp"
#include "sdm/Operations.hpp"
#include "sdm/PhysDataBase.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace sdm {

//////////////////////////////////////////////////////////////////////////////

/// A proposed base class for simple Source terms.
/// Classes inheriting only need to implement functions to compute
/// the source given a physical data object
/// @author Willem Deconinck
template <typename PHYSDATA>
class SourceTerm : public Term
{
public: // typedefs
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  enum { NDIM = PHYSDATA::NDIM }; ///< number of dimensions
  enum { NEQS = PHYSDATA::NEQS }; ///< number of independent variables or equations
  typedef PHYSDATA PhysData;
  typedef Eigen::Matrix<Real,NDIM,1> RealVectorNDIM;
  typedef Eigen::Matrix<Real,NEQS,1> RealVectorNEQS;

public: // functions

  /// constructor
  SourceTerm( const std::string& name );

  /// Gets the Class name
  static std::string type_name() { return "SourceTerm"; }

  virtual void execute();

private: // functions

  // Pure virtual evaluations
  // -----------------------------
  /// @brief Compute analytical flux
  /// @param [in]  data          Physical data necessary to compute the source
  /// @param [out] source        Computed flux, projected on unit_normal
  virtual void compute_source(PHYSDATA& data, RealVectorNEQS& source) = 0;

protected: // configuration

  /// @brief Initialize this term
  virtual void initialize();

  /// @brief Initialize caches for a given Entities component
  virtual void set_entities(const mesh::Entities& entities);

  /// @brief Set caches for element with given index
  virtual void set_element(const Uint elem_idx);

  /// @brief Standard computation of solution and coordinates in a solution point
  ///
  /// This function has to be used for boundaries, where the neighbour-element is a face entities
  /// The physical data is then located inside the face itself.
  virtual void compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PHYSDATA& phys_data );

  virtual void allocate_sol_pt_data();

  /// @brief free the element caches
  virtual void unset_element();

protected: // fast-access-data (for convenience no "m_" prefix)

  PHYSDATA sol_pt_data;             ///< Physical data (for interior points)

  Uint sol_pt;                                ///< Current solution point

  Handle< CacheT<SFDElement> > elem;          ///< This cell

  // In sol points:
  RealVectorNEQS sol_pt_source;               ///< Storage of source in sol points

}; // end SourceTerm

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
SourceTerm<PHYSDATA>::SourceTerm( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Source Spectral Difference term");
  properties()["description"] = std::string("Computes on a per cell basis the source term");
}

/////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::execute()
{
  mesh::Field::View term      = m_term_field->view(elem->get().space->connectivity()[m_elem_idx]);
  Uint v(0);

  /// 1) Calculate source term in solution points
  for (sol_pt=0; sol_pt < elem->get().sf->nb_sol_pts(); ++sol_pt) // sol_pt is a member variable!!!
  {
    compute_sol_pt_phys_data(elem->get(),sol_pt,sol_pt_data);
    compute_source(sol_pt_data, sol_pt_source );

    for (v=0; v<NEQS; ++v)
    {
      term[sol_pt][v] = sol_pt_source[v];
    }
  }

  /// 2) Add this term from the residual field
  mesh::Field::View residual = residual_field().view(elem->get().space->connectivity()[m_elem_idx]);
  for (sol_pt=0; sol_pt<elem->get().sf->nb_sol_pts(); ++sol_pt) {
    for (v=0; v<NEQS; ++v) {
      residual[sol_pt][v] += term[sol_pt][v];
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::initialize()
{
  Term::initialize();
  elem = shared_caches().template get_cache< SFDElement >();
  elem->options().configure_option("space",solution_field().dict().handle<mesh::Dictionary>());
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::allocate_sol_pt_data()
{
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::set_entities(const mesh::Entities& entities)
{
  Term::set_entities(entities);
  elem->cache(m_entities);
  allocate_sol_pt_data();
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::set_element(const Uint elem_idx)
{
  Term::set_element(elem_idx);
  elem->set_cache(m_elem_idx);
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::compute_sol_pt_phys_data(const SFDElement& elem, const Uint sol_pt, PHYSDATA& phys_data )
{
  mesh::Field::View sol_pt_solution = solution_field().view(elem.space->connectivity()[elem.idx]);
  mesh::Field::View sol_pt_coords   = solution_field().dict().coordinates().view(elem.space->connectivity()[elem.idx]);
  for (Uint var=0; var<NEQS; ++var)
    phys_data.solution[var] = sol_pt_solution[sol_pt][var];
  for (Uint dim=0; dim<NDIM; ++dim)
    phys_data.coord[dim] = sol_pt_coords[sol_pt][dim];
}

////////////////////////////////////////////////////////////////////////////////

template <typename PHYSDATA>
void SourceTerm<PHYSDATA>::unset_element()
{
  Term::unset_element();
  elem->get().unlock();
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_sdm_SourceTerm_hpp
