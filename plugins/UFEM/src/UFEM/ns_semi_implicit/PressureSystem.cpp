// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <Epetra_CrsMatrix.h>
#include <EpetraExt_MatrixMatrix.h>

#include <Thyra_describeLinearOp.hpp>

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/Option.hpp"
#include "common/OptionList.hpp"

#include "math/LSS/System.hpp"
#include "math/LSS/Trilinos/ThyraOperator.hpp"
#include "math/LSS/Trilinos/TrilinosCrsMatrix.hpp"
#include "math/LSS/Trilinos/TrilinosVector.hpp"

#include "mesh/Dictionary.hpp"
#include "mesh/Field.hpp"
#include "mesh/LagrangeP1/ElementTypes.hpp"

#include "solver/Action.hpp"
#include "solver/Tags.hpp"
#include "solver/actions/Proto/ElementLooper.hpp"

#include "../SUPG.hpp"

#include "PressureSystem.hpp"

#include <coolfluid-ufem-config.hpp>

namespace cf3 {
namespace UFEM {

using namespace solver::actions::Proto;
using boost::proto::lit;

common::ComponentBuilder < PressureSystem, common::ActionDirector, LibUFEM > PressureSystem_Builder;

void write_mat(const Teuchos::RCP<const Thyra::LinearOpBase<Real> >& mat, const std::string& filename)
{
  Teuchos::RCP<std::ofstream> mat_out = Teuchos::rcp(new  std::ofstream(filename.c_str(), std::ios::out));
  Teuchos::RCP<Teuchos::FancyOStream> mat_fancy_out = Teuchos::fancyOStream(mat_out);
  Thyra::describeLinearOp(*mat, *mat_fancy_out, Teuchos::VERB_EXTREME);
}

PressureSystem::PressureSystem(const std::string& name) :
  LSSActionUnsteady(name)
{
}

PressureSystem::~PressureSystem()
{
}

void PressureSystem::do_create_lss(common::PE::CommPattern &cp, const math::VariablesDescriptor &vars, std::vector<Uint> &node_connectivity, std::vector<Uint> &starting_indices, const std::vector<Uint> &periodic_links_nodes, const std::vector<bool> &periodic_links_active)
{
  const Uint nb_nodes = starting_indices.size()-1;
  cf3_assert(starting_indices.back() == node_connectivity.size());
  
  // Extend the node connectivity to include periodic nodes
  std::vector< std::set<Uint> > node_connectivity_sets(nb_nodes);
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    node_connectivity_sets[i].insert(node_connectivity.begin()+starting_indices[i], node_connectivity.begin()+starting_indices[i+1]);
  }
  
//   if(periodic_links_active.size() != 0)
//   {
//     for(Uint i = 0; i != nb_nodes; ++i)
//     {
//       std::list<Uint> connected_nodes;
//       connected_nodes.push_back(i);
//       Uint target_node = i;
//       while(periodic_links_active[target_node])
//       {
//         target_node = periodic_links_nodes[target_node];
//         connected_nodes.push_back(target_node);
//       }
//       BOOST_FOREACH(Uint j, connected_nodes)
//       {
//         BOOST_FOREACH(Uint k, connected_nodes)
//         {
//           node_connectivity_sets[j].insert(node_connectivity.begin()+starting_indices[k], node_connectivity.begin()+starting_indices[k+1]);
//           node_connectivity_sets[k].insert(node_connectivity.begin()+starting_indices[j], node_connectivity.begin()+starting_indices[j+1]);
//         }
//       }
//       
//     }
//   }
  std::vector<Uint> new_node_connectivity; new_node_connectivity.reserve(node_connectivity.size()*4);
  std::vector<Uint> new_starting_indices; new_starting_indices.reserve(nb_nodes+1);
  new_starting_indices.push_back(0);
  std::vector< std::set<Uint> > new_node_connectivity_sets(nb_nodes);
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    std::set<Uint> row_set;
    const Uint row_begin = starting_indices[i];
    const Uint row_end = starting_indices[i+1];
    for(Uint j = row_begin; j != row_end; ++j)
    {
      Uint target_node = node_connectivity[j];
      while(!periodic_links_active.empty() && periodic_links_active[target_node])
        target_node = periodic_links_nodes[target_node];
      row_set.insert(target_node);
    }
    for(Uint j = 0; j != nb_nodes; ++j)
    {
      const Uint col_begin = starting_indices[j];
      const Uint col_end = starting_indices[j+1];
      for(Uint k = col_begin; k != col_end; ++k)
      {
        Uint target_node = node_connectivity[k];
        while(!periodic_links_active.empty() && periodic_links_active[target_node])
          target_node = periodic_links_nodes[target_node];
        if(row_set.count(node_connectivity[k]) != 0 || row_set.count(target_node) != 0)
        {
          new_node_connectivity.push_back(j);
          break;
        }
      }
    }
    new_starting_indices.push_back(new_node_connectivity.size());
  }

  Uint ghost_id = 0;
  for(Uint i = 0; i != nb_nodes; ++i)
  {
    if(periodic_links_active[i])
    {
      std::cout << "ghost ";
    }
    std::cout << "node " << i << "(" << ghost_id << ") connects to:";
    for(Uint j = new_starting_indices[i]; j != new_starting_indices[i+1]; ++j)
      std::cout << " " << new_node_connectivity[j];
    std::cout << std::endl;
    
    if(!periodic_links_active[i])
      ++ghost_id;
  }
  
  // The standard LSS contains the original sparsity
  LSSActionUnsteady::do_create_lss(cp, vars, node_connectivity, starting_indices, periodic_links_nodes, periodic_links_active);
  
  Handle<math::LSS::System> lss = options().option("lss").value< Handle<math::LSS::System> >();
  
  // We also create a new matrix with the correct sparsity to store the product
  Handle<math::LSS::TrilinosCrsMatrix> result_matrix = create_component<math::LSS::TrilinosCrsMatrix>("ProductMatrix");
  result_matrix->create(cp, 1, new_node_connectivity, new_starting_indices, *lss->solution(), *lss->rhs(), periodic_links_nodes, periodic_links_active);
  
  result_matrix->reset(1.);
  
  write_mat(result_matrix->thyra_operator(), "p_mat_sparsity.txt");
}

class PressureSystemAssembly : public solver::Action
{
public:
  PressureSystemAssembly(const std::string& name) :
    solver::Action(name),
    u("Velocity", "navier_stokes_u_solution"),
    p("Pressure", "navier_stokes_p_solution"),
    u_adv("AdvectionVelocity", "linearized_velocity"),
    nu_eff("EffectiveViscosity", "navier_stokes_viscosity"),
    theta(0.5)
  {
    options().add("pressure_lss_action", m_pressure_lss_action)
      .pretty_name("Pressure LSS Action")
      .description("LSS action for the pressure system")
      .link_to(&m_pressure_lss_action);

    options().add("theta", theta)
      .pretty_name("Theta")
      .description("Theta parameter for the theta scheme")
      .link_to(&theta);
  }

  static std::string type_name () { return "PressureSystemAssembly"; }

  virtual void execute()
  {
    if(is_null(m_pressure_lss_action))
      throw common::SetupError(FromHere(), "pressure_lss_action not set for PressureSystemAssembly " + uri().string());

    Handle<math::LSS::System> p_lss(m_pressure_lss_action->get_child("LSS"));
    cf3_assert(is_not_null(p_lss));

    Handle<math::LSS::TrilinosCrsMatrix> p_crs_mat(p_lss->matrix());
    cf3_assert(is_not_null(p_crs_mat));

    Epetra_CrsMatrix& p_lss_mat = *p_crs_mat->epetra_matrix();
    p_lss_mat.PutScalar(0.);
    Epetra_CrsMatrix assembly_result(p_lss_mat);
    
    // This one has the right sparsity for the product result:
    Handle<math::LSS::TrilinosCrsMatrix> product_mat(m_pressure_lss_action->get_child("ProductMatrix"));
    cf3_assert(is_not_null(product_mat));
    product_mat->reset(1.);
    Teuchos::RCP<Epetra_CrsMatrix> product_result = Teuchos::rcp(new Epetra_CrsMatrix(Copy, p_lss_mat.RowMap(), 1));
    Teuchos::RCP<Epetra_CrsMatrix> sum_result = Teuchos::rcp(new Epetra_CrsMatrix(Copy, p_lss_mat.RowMap(), 1));

#ifdef CF3_UFEM_ENABLE_QUADS
    assemble_mass_matrix<mesh::LagrangeP1::Quad2D>();
#endif

    // Compute the reciprocal of the lumped mass matrix
    Handle<math::LSS::TrilinosVector> trilinos_rhs(p_lss->rhs());
    Epetra_Vector& mass_mat_lumped = *trilinos_rhs->epetra_vector();
    if(mass_mat_lumped.Reciprocal(mass_mat_lumped) != 0)
      throw common::BadValue(FromHere(), "Error computing mass matrix inverse");

    const int dim = m_physical_model->ndim();
    for(int i = 0; i != dim; ++i)
    {
      p_lss_mat.PutScalar(0.);
      // Assemble the PU matrix
#ifdef CF3_UFEM_ENABLE_QUADS
      assemble_pu<mesh::LagrangeP1::Quad2D>(i);
#endif

      // Store the result
      EpetraExt::MatrixMatrix::Add(p_lss_mat, false, 1., assembly_result, 0.);

      p_lss_mat.PutScalar(0.);
      // Assemble the up matrix
#ifdef CF3_UFEM_ENABLE_QUADS
      assemble_up<mesh::LagrangeP1::Quad2D>(i);
#endif

      assembly_result.RightScale(mass_mat_lumped);
      //EpetraExt::MatrixMatrix::Multiply(assembly_result, false, p_lss_mat, false, *product_result);
      //EpetraExt::MatrixMatrix::Add(*product_result, false, 1., *sum_result, 1.);
    }

    p_lss_mat.PutScalar(0.);
    // Assemble pressure laplacian part
#ifdef CF3_UFEM_ENABLE_QUADS
    assemble_pp<mesh::LagrangeP1::Quad2D>();
#endif

    //EpetraExt::MatrixMatrix::Add(p_lss_mat, false, 1., *sum_result, 1.);
    //p_crs_mat->replace_epetra_matrix(sum_result);
    EpetraExt::MatrixMatrix::Multiply(p_lss_mat, false, p_lss_mat, true, *product_result);
    p_crs_mat->reset(1.);
    write_mat(p_crs_mat->thyra_operator(), "pp.txt");
    p_crs_mat->replace_epetra_matrix(product_result);

    p_crs_mat->reset(1.);
    write_mat(p_crs_mat->thyra_operator(), "p_mat.txt");
  }

  template<typename ElementT>
  void assemble_mass_matrix()
  {
    for_each_element<ElementT>(group
    (
      _A = _0,
      element_quadrature( _A(p, p) += transpose(N(p)) * N(p) ),
      lump(_A),
      m_pressure_lss_action->system_rhs += diagonal(_A)
    ));
  }

  template<typename ElementT>
  void assemble_pu(const int i)
  {
    const Real u_ref = physical_model().options().value<Real>("reference_velocity");
    for_each_element<ElementT>(group
    (
      _A = _0, _T = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _T(p, p) += tau_ps * transpose(nabla(p)[i]) * N(p),
        _A(p, p) += transpose(N(p)) * nabla(p)[i] //+ tau_ps * transpose(nabla(p)[i]) * u_adv*nabla(p)
      ),
      m_pressure_lss_action->system_matrix += _T + lit(m_pressure_lss_action->dt()) * _A
    ));
  }

  template<typename ElementT>
  void assemble_up(const int i)
  {
    const Real u_ref = physical_model().options().value<Real>("reference_velocity");
    for_each_element<ElementT>(group
    (
      _A = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature
      (
        _A(p, p) += -lit(theta) * transpose(nabla(u)[_i]) * N(p)
      ),
      m_pressure_lss_action->system_matrix += _A
    ));
  }

  template<typename ElementT>
  void assemble_pp()
  {
    const Real u_ref = physical_model().options().value<Real>("reference_velocity");
    for_each_element<ElementT>(group
    (
      _A = _0,
      compute_tau(u, nu_eff, u_ref, lit(tau_ps), lit(tau_su), lit(tau_bulk)),
      element_quadrature( _A(p, p) += transpose(nabla(p)) * nabla(p) ),
      //m_pressure_lss_action->system_matrix += lit(theta) * (lit(tau_ps) + lit(m_pressure_lss_action->dt())) * _A
      m_pressure_lss_action->system_matrix += -lit(tau_ps)*_A
    ));
  }

private:
  // Helper function to loop over all regions
  template<typename ElementT, typename ExprT>
  void for_each_element(const ExprT& expr)
  {
    BOOST_FOREACH(const Handle<mesh::Region> region, m_loop_regions)
    {
      solver::actions::Proto::for_each_element< boost::mpl::vector1<ElementT> >(*region, expr);
    }
  }

  Handle<LSSActionUnsteady> m_pressure_lss_action;

  /// The velocity solution field
  FieldVariable<0, VectorField> u;
  /// The pressure solution field
  FieldVariable<1, ScalarField> p;
  /// The linearized advection velocity
  FieldVariable<2, VectorField> u_adv;
  /// Effective viscosity field
  FieldVariable<3, ScalarField> nu_eff;

  Real tau_ps, tau_su, tau_bulk;
  Real theta;
};

common::ComponentBuilder < PressureSystemAssembly, common::Action, LibUFEM > PressureSystemAssembly_Builder;

} // UFEM
} // cf3
