// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_solver_Term_hpp
#define cf3_solver_Term_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/Component.hpp"
#include "common/StringConversion.hpp"
//#include "common/OptionList.hpp"
//#include "common/Option.hpp"
#include "physics/MatrixTypes.hpp"
#include "solver/LibSolver.hpp"

////////////////////////////////////////////////////////////////////////////////

// Forward declarations
namespace cf3 {
  namespace mesh { 
    class Dictionary; 
    class Space; 
    class ReconstructPoint; 
    class Field; 
  }
  namespace solver {
    class Time;
  }
}

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace solver {

////////////////////////////////////////////////////////////////////////////////

class solver_API Term : public common::Component
{
public:
  Term( const std::string& name );

  virtual ~Term();

  static std::string type_name() { return "Termbase"; }

  const Handle<mesh::Dictionary>& fields()  { return m_fields; }
  const Handle<mesh::Field>& solution()   { return m_solution; }

  const Handle<mesh::Dictionary>& bdry_fields()  { return m_bdry_fields; }
  const Handle<mesh::Field>& bdry_solution()   { return m_bdry_solution; }
  const Handle<mesh::Field>& bdry_solution_gradient()   { return m_bdry_solution_gradient; }

  const Handle<solver::Time>& time() const { return m_time; }

//  template <typename T>
//  common::Option& add_linked_option(Component& component, const std::string& name, T& value);

//  virtual void add_configuration_options(Component& component);

  virtual void create_fields();
  virtual void create_bdry_fields();

protected:

  Handle<mesh::Dictionary> m_fields;
  Handle<mesh::Field>      m_solution;

  Handle<mesh::Dictionary> m_bdry_fields;
  Handle<mesh::Field>      m_bdry_solution;
  Handle<mesh::Field>      m_bdry_solution_gradient;

  Handle<solver::Time>     m_time;

};

////////////////////////////////////////////////////////////////////////////////

//template <typename T>
//inline common::Option& Term::add_linked_option(Component& component, const std::string& name, T& value)
//{
//  if ( !options().check(name) )
//  {
//    options().add(name,value)
//        .link_to(&value);
//  }
//  if ( &component != this )
//  {
//    if ( component.options().check(name) )
//    {
//      component.options()[name]
//          .link_option(options().option_ptr(name));
//      options()[name] = component.options()[name].value();
//    }
//    else
//    {
//      component.options().add(name,value)
//        .link_option(options().option_ptr(name));
//    }
//  }
//  return component.options()[name];
//}

////////////////////////////////////////////////////////////////////////////////

template < Uint NB_DIM, Uint NB_EQS, Uint NB_VAR, Uint NB_GRAD >
class solver_API TermBase : public solver::Term
{
public: 

  /// @brief Constructor
  TermBase( const std::string& name ) : solver::Term(name) { }
  
  /// @brief Destructor
  virtual ~TermBase() {}

  static std::string type_name() { return "TermBase<"+common::to_str(NB_DIM)+","+common::to_str(NB_EQS)+","+common::to_str(NB_VAR)+","+common::to_str(NB_GRAD)+">"; }
  
public: // types

  enum { ENABLE_CONVECTION = false };
  enum { ENABLE_DIFFUSION  = false };
  enum { ENABLE_SOURCE     = false };

  static const Uint NDIM  = NB_DIM;
  static const Uint NEQS  = NB_EQS;
  static const Uint NVAR  = NB_VAR;
  static const Uint NGRAD = NB_GRAD;

  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::ColVector_NDIM    ColVector_NDIM;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::RowVector_NEQS    RowVector_NEQS;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::RowVector_NVAR    RowVector_NVAR;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::RowVector_NGRAD   RowVector_NGRAD;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::Matrix_NDIMxNDIM  Matrix_NDIMxNDIM;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::Matrix_NDIMxNEQS  Matrix_NDIMxNEQS;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::Matrix_NDIMxNVAR  Matrix_NDIMxNVAR;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::Matrix_NDIMxNGRAD Matrix_NDIMxNGRAD;
  typedef typename physics::MatrixTypes<NDIM,NEQS,NVAR,NGRAD>::Matrix_NEQSxNEQS  Matrix_NEQSxNEQS;

/// DONT SPOIL PERFORMANCE, DONT MAKE ANY FUNCTIONS VIRTUAL!!!

public: // Variable and PhysData computation
    
  /// @brief Compute variables and gradients in a given element point
  void get_variables( const mesh::Space& space,
                      const Uint elem_idx,
                      const ColVector_NDIM& coords,
                      const mesh::ReconstructPoint& interpolation,
                      const std::vector<mesh::ReconstructPoint>& gradient,
                      const Matrix_NDIMxNDIM& jacobian,
                      const Matrix_NDIMxNDIM& jacobian_inverse,
                      const Real& jacobian_determinant,
                      RowVector_NVAR& vars,
                      RowVector_NGRAD& gradvars,
                      Matrix_NDIMxNGRAD& gradvars_grad ) { }

  void get_bdry_variables( const mesh::Space& space,
                           const Uint elem_idx,
                           const ColVector_NDIM& coords,
                           const mesh::ReconstructPoint& interpolation,
                           const std::vector<mesh::ReconstructPoint>& gradient,
                           const Matrix_NDIMxNDIM& jacobian,
                           const Matrix_NDIMxNDIM& jacobian_inverse,
                           const Real& jacobian_determinant,
                           RowVector_NVAR& vars,
                           RowVector_NGRAD& gradvars,
                           Matrix_NDIMxNGRAD& gradvars_grad ) { }

  /// @brief Set constants in the data
  template< typename DATA>
  void set_phys_data_constants( DATA& phys_data ) { }

  /// @brief Compute the data from computed variables and gradients
  template< typename DATA>
  void compute_phys_data( const ColVector_NDIM& coords,
                          const RowVector_NVAR& vars,
                          const RowVector_NGRAD& gradvars,
                          const Matrix_NDIMxNGRAD& gradvars_grad,
                          DATA& phys_data ) { }

public: // static functions

  // These functions don't necessarily have to static.
  template< typename DATA>
  void compute_convective_flux( const DATA& p, const ColVector_NDIM& normal,
                                RowVector_NEQS& flux, Real& wave_speed )
  {
    flux.setZero();
    wave_speed=0.;
  }

  template< typename DATA >
  void compute_riemann_flux( const DATA& left, const DATA& right, const ColVector_NDIM& normal,
                             RowVector_NEQS& flux, Real& wave_speed )
  {
    flux.setZero();
    wave_speed=0.;
  }

  template< typename DATA >
  void compute_diffusive_flux( const DATA& p, const ColVector_NDIM& normal,
                               RowVector_NEQS& flux, Real& wave_speed )
  {
    flux.setZero();
    wave_speed=0.;
  }

  template< typename DATA >
  void compute_source( const DATA& p,
                       RowVector_NEQS& source )
  {
    source.setZero();
  }

};

////////////////////////////////////////////////////////////////////////////////

} // solver
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_solver_Term_hpp
