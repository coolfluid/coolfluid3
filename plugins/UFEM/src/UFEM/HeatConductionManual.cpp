// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_PROTO_MAX_ARITY 10
#define BOOST_MPL_LIMIT_METAFUNCTION_ARITY 10

#include "common/Builder.hpp"
#include "common/OptionList.hpp"

#include "mesh/LagrangeP2/Line1D.hpp"
#include "mesh/LagrangeP2/LibLagrangeP2.hpp"
#include "mesh/LagrangeP1/LibLagrangeP1.hpp"

#include "math/LSS/SolveLSS.hpp"
#include "math/LSS/ZeroLSS.hpp"

#include "solver/actions/Proto/Expression.hpp"
#include "solver/Tags.hpp"

#include "HeatConductionManual.hpp"
#include "Tags.hpp"

namespace cf3 {
namespace UFEM {

using namespace common;
using namespace solver;
using namespace solver::actions;
using namespace solver::actions::Proto;
using namespace mesh;

ComponentBuilder < HeatConductionManual, LSSAction, LibUFEM > HeatConductionManual_builder;

class HeatConductionManualAssembly : public solver::Action
{
public:
  HeatConductionManualAssembly ( const std::string& name ) : solver::Action(name)
  {
  }
  
  static std::string type_name () { return "HeatConductionManualAssembly"; }
  
  virtual void execute()
  {
    if(is_null(m_lss))
      throw common::SetupError(FromHere(), "LSS not set for " + uri().path());
    
    BOOST_FOREACH(const Handle<mesh::Region>& region, m_loop_regions)
    {
      BOOST_FOREACH(const mesh::Elements& elements, find_components_recursively_with_filter<Elements>(*region, IsElementType<mesh::LagrangeP1::Triag2D>()))
      {
        const Uint nb_elems = elements.size();
        const mesh::Connectivity& connectivity = elements.geometry_space().connectivity();
        const mesh::Field& coordinates = elements.geometry_fields().coordinates();
        Eigen::Matrix<Real, 3, 2> nodes, normals;
        math::LSS::BlockAccumulator acc;
        acc.resize(3, 1);
        
        Eigen::Matrix<Real, 3, 3> A;
        Eigen::Matrix<Real, 3, 1> x;
        
        const mesh::Mesh& mesh = common::find_parent_component<mesh::Mesh>(elements);
        const mesh::Field& field = common::find_component_recursively_with_tag<mesh::Field>(mesh, "heat_conduction_solution");
        
        for(Uint i = 0; i != nb_elems; ++i)
        {
          acc.neighbour_indices(connectivity[i]);
          mesh::fill(nodes, coordinates, connectivity[i]);
          mesh::fill(x, field, connectivity[i]);
          
          normals(0, XX) = nodes(1, YY) - nodes(2, YY); normals(0, YY) = nodes(2, XX) - nodes(1, XX);
          normals(1, XX) = nodes(2, YY) - nodes(0, YY); normals(1, YY) = nodes(0, XX) - nodes(2, XX);
          normals(2, XX) = nodes(0, YY) - nodes(1, YY); normals(2, YY) = nodes(1, XX) - nodes(0, XX);
          
          const Real c = 1. / (2.*(normals(2, YY)*normals(1, XX) - normals(1, YY)*normals(2, XX)));
          
          for(Uint i = 0; i != 3; ++i)
            for(Uint j = 0; j != 3; ++j)
              A(i, j) = c * (normals(i, XX)*normals(j, XX) + normals(i, YY)*normals(j, YY));
          
          static const Uint mat_size = 3;
          static const Uint nb_dofs = 1;
          static const Uint nb_nodes = 3;
            
          for(Uint row = 0; row != mat_size; ++row)
          {
            // This converts u1,u2...pn to u1v1p1...
            const Uint block_row = (row % nb_nodes)*nb_dofs + row / nb_nodes;
            for(Uint col = 0; col != mat_size; ++col)
            {
              const Uint block_col = (col % nb_nodes)*nb_dofs + col / nb_nodes;
              acc.mat(block_row, block_col) = A(row, col);
            }
          }
          
          acc.rhs.noalias() = A*x;
          m_lss->add_values(acc);
        }
      }
    }
  }
  
  Handle<math::LSS::System> m_lss;
};

ComponentBuilder < HeatConductionManualAssembly, common::Component, LibUFEM > HeatConductionManualAssembly_builder;

HeatConductionManual::HeatConductionManual ( const std::string& name ) : LSSAction ( name )
{
  set_solution_tag("heat_conduction_solution");
  
  ConfigurableConstant<Real> k("k", "Thermal conductivity (J/(mK))", 1.);
  FieldVariable<0, ScalarField> T("Temperature", "heat_conduction_solution");
  
  create_component<math::LSS::ZeroLSS>("ZeroLSS");
  Handle<HeatConductionManualAssembly> assembly = create_component<HeatConductionManualAssembly>("Assembly");
  
  Handle<BoundaryConditions> bc = create_component<BoundaryConditions>("BoundaryConditions");
  bc->mark_basic();
  bc->set_solution_tag(solution_tag());
  
  create_component<math::LSS::SolveLSS>("SolveLSS");
  create_component<ProtoAction>("SetSolution")->set_expression(nodes_expression(T += solution(T)));
  
  configure_option_recursively(solver::Tags::physical_model(), m_physical_model);
  
  options().option("lss").link_to(&assembly->m_lss);
}

void HeatConductionManual::on_initial_conditions_set(InitialConditions& initial_conditions)
{
  initial_conditions.create_initial_condition(solution_tag());
}


} // UFEM
} // cf3
