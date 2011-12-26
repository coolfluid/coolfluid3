// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementCaches of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::SFDM"

#include <boost/flyweight.hpp>

#include <boost/test/unit_test.hpp>
#include <boost/assign/list_of.hpp>
#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/Environment.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/OSystem.hpp"
#include "common/OSystemLayer.hpp"
#include "common/List.hpp"
#include "common/Group.hpp"

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"
#include "math/VectorialFunction.hpp"

#include "solver/CModel.hpp"
#include "solver/Tags.hpp"

#include "physics/PhysModel.hpp"
#include "physics/Variables.hpp"

#include "mesh/Domain.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Field.hpp"
#include "mesh/FieldManager.hpp"
#include "mesh/SimpleMeshGenerator.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Region.hpp"
#include "mesh/LinearInterpolator.hpp"
#include "mesh/Space.hpp"
#include "mesh/Cells.hpp"
#include "mesh/ElementConnectivity.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/actions/BuildFaces.hpp"

#include "SFDM/ElementCaching.hpp"
#include "SFDM/Reconstructions.hpp"
#include "SFDM/SFDSolver.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Operations.hpp"

#include "Tools/Gnuplot/Gnuplot.hpp"
#include <common/Link.hpp>

using namespace boost::assign;
using namespace cf3;
using namespace cf3::math;
using namespace cf3::common;
using namespace cf3::common::PE;
using namespace cf3::mesh;
using namespace cf3::physics;
using namespace cf3::solver;
using namespace cf3::SFDM;

std::map<Real,Real> xy(const Field& field)
{
  std::map<Real,Real> map;
  for (Uint i=0; i<field.size(); ++i)
    map[field.coordinates()[i][0]] = field[i][0];
  return map;
}

struct SFDM_MPITests_Fixture
{
  /// common setup for each test case
  SFDM_MPITests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~SFDM_MPITests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( SFDM_MPITests_TestSuite, SFDM_MPITests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init_mpi )
{
  PE::Comm::instance().init(m_argc,m_argv);
  Core::instance().environment().options().configure_option("log_level", (Uint)INFO);
}

////////////////////////////////////////////////////////////////////////////////

class Term;
class ApplyBC;
// UBER shared almost global struct
class DomainDiscretizer : public Component
{
public:
  static std::string type_name() { return "DomainDiscretizer"; }
  DomainDiscretizer(const std::string& name) :
    Component(name)
  {
    shared_caches = create_static_component<SharedCaches>("shared_caches");
    term_group = create_static_component<Group>("terms");
    apply_bc = create_static_component<common::ActionDirector>("apply_bc");
  }
  virtual ~DomainDiscretizer() {}

  template <typename TermT>
  Handle<TermT> create_term(const std::string& name)
  {
    Handle<TermT> term = term_group->create_component<TermT>(name);
    term->set_discretizer(*this);
    cf3_assert(term->discretizer);
    terms.push_back(term);
    return term;
  }

  void create_boundary_condition(const std::string& name, const std::string& builder, const std::vector< Handle<Region const> >& regions );

  void initialize();

  void set_element(const Handle<Entities const>& entities, const Uint idx);

  void execute();

  Handle<Entities const> entities;
  Uint elem;

  Handle<SharedCaches> shared_caches;
  Handle<Group> term_group;
  std::vector< Handle<Term> > terms;
  Handle<Field> solution_field;
  Handle<Field> residual;
  Handle<Field> jacobian_determinant;
  Handle<Field> wave_speed;

  Handle<CacheT<SFDElement> > sfd_cache;


  Handle<common::ActionDirector> apply_bc;

};

////////////////////////////////////////////////////////////////////////////////

class Term : public Component
{
public:
  static std::string type_name() { return "Term"; }
  Term(const std::string& name) : Component(name), store_term_in_field(true) {}
  virtual ~Term() {}
  virtual void execute(const Handle<Entities const>& entities, const Uint elem_idx) = 0;

  Handle<DomainDiscretizer> discretizer;

  // Must be called right after construction
  Handle<DomainDiscretizer> set_discretizer(DomainDiscretizer& _discretizer)
  {
      discretizer = _discretizer.handle<DomainDiscretizer>();
      cf3_assert(discretizer);
      create_term_field();
      return discretizer;
  }

  void create_term_field()
  {
    term_field = discretizer->solution_field->field_group().create_field(name(),term_names()).handle<Field>();
  }

  virtual void initialize() {}

  virtual std::string term_names()
  {
    boost::shared_ptr<VariablesDescriptor> vars(allocate_component<VariablesDescriptor>("tmp"));
    cf3_assert(vars);
    cf3_assert(discretizer);
    cf3_assert(discretizer->solution_field);
    vars->set_variables(discretizer->solution_field->descriptor().description());
    vars->prefix_variable_names(name()+"_");
    return vars->description();
  }

  /// @todo move in central place
  void set_neighbour(const Handle<Entities const>& entities, const Uint elem_idx, const Uint face_nb,
                     Handle<Entities const>& neighbour_entities, Uint& neighbour_elem_idx,
                     Handle<Entities const>& face_entities, Uint& face_idx)
  {
    ElementConnectivity const& face_connectivity = *entities->get_child("face_connectivity")->handle<ElementConnectivity>();
    Entity face = face_connectivity[elem_idx][face_nb];
    face_entities = face.comp->handle<Entities>();
    face_idx = face.idx;
    FaceCellConnectivity const& cell_connectivity = *face.comp->get_child("cell_connectivity")->handle<FaceCellConnectivity>();
    if (cell_connectivity.is_bdry_face()[face.idx])
    {
      neighbour_entities = Handle<Entities const>();
    }
    else
    {
      Entity neighbour;
      if (cell_connectivity.connectivity()[face.idx][LEFT].comp == entities.get() &&
          cell_connectivity.connectivity()[face.idx][LEFT].idx == elem_idx)
        neighbour = cell_connectivity.connectivity()[face.idx][LEFT];
      else
        neighbour = cell_connectivity.connectivity()[face.idx][RIGHT];
      neighbour_entities = neighbour.comp->handle<Entities>();
      neighbour_elem_idx = neighbour.idx;
    }
  }

  bool store_term_in_field;
  Handle<Field> term_field;
};

////////////////////////////////////////////////////////////////////////////////

class BC : public common::Action
{
public:
  static std::string type_name() { return "BC"; }
  BC(const std::string& name) : common::Action(name) {}
  virtual ~BC() {}

  void set_inner_cell(const Handle<Entities const>& face_entities, const Uint face_idx, Handle<Entities const>& entities, Uint& elem_idx, Uint& face_nb)
  {
    FaceCellConnectivity const& cell_connectivity = *face_entities->get_child("cell_connectivity")->handle<FaceCellConnectivity>();
    entities = cell_connectivity.connectivity()[face_idx][LEFT].comp->handle<Entities>();
    elem_idx = cell_connectivity.connectivity()[face_idx][LEFT].idx;
    face_nb = cell_connectivity.face_number()[face_idx][LEFT];
  }

  Handle<DomainDiscretizer> discretizer;

  // Must be called right after construction
  Handle<DomainDiscretizer> set_discretizer(DomainDiscretizer& _discretizer)
  {
      discretizer = _discretizer.handle<DomainDiscretizer>();
      return discretizer;
  }

  virtual void initialize() {}

  std::vector<Handle<Region const> > regions;
};

template <Uint NEQS, Uint NDIM>
class StrongBC : public BC
{
public:
  static std::string type_name() { return "StrongBC"; }
  StrongBC(const std::string& name) : BC(name) {}
  virtual ~StrongBC() {}

  virtual void execute()
  {
    //std::cout << "executing " << type_name() << std::endl;
    boost_foreach( const Handle<Region const>& region, regions)
    {
      boost_foreach( Entities const& face_entities, find_components_recursively<Entities>(*region))
      {
        for (Uint face_idx=0; face_idx<face_entities.size(); ++face_idx)
        {
          set_inner_cell(face_entities.handle<Entities>(),face_idx, entities, elem_idx, face_nb);
          //std::cout << "face_nb = " << face_nb << std::endl;
          cf3_assert(solution_cache);
          solution_cache->cache(entities,elem_idx);
          boost_foreach(const Uint flx_pt, solution_cache->get().sf->face_flx_pts(face_nb))
          {
            for (Uint v=0; v<NEQS; ++v)
            {
              solution_cache->get().field_in_flx_pts[flx_pt][v]=0.;
            }
          }
          //std::cout << "sol_in_flx_pts = "; boost_foreach(const RealRowVector& r, solution->field_in_flx_pts)
            //std::cout << r << " ";
          //std::cout << std::endl;
          Field::View sol_in_sol_pts = solution_cache->get().field->view(solution_cache->get().space->indexes_for_element(elem_idx));
          reconstruct_from_flx_pts_cache->cache(entities).
              compute(solution_cache->get().sf->flx_pt_dirs(solution_cache->get().sf->face_flx_pts(face_nb)[0])[0],
                                                solution_cache->get().field_in_flx_pts,sol_in_sol_pts);
          //std::cout << "sol_in_sol_pts = " << to_str(sol_in_sol_pts) << std::endl;
          solution_cache->get().unlock();
        }
      }
    }
  }

  Handle<Entities const> entities;
  Uint elem_idx;
  Uint face_nb;

  virtual void initialize()
  {
    solution_cache = discretizer->shared_caches->get_cache< FluxPointField<NEQS,NDIM> >(SFDM::Tags::solution());
    solution_cache->options().configure_option("field",discretizer->solution_field->uri());
    reconstruct_from_flx_pts_cache = discretizer->shared_caches->get_cache< FluxPointReconstruct >("reconstruct_from_flx_pts");
  }

  Handle< CacheT<FluxPointField<NEQS,NDIM> > > solution_cache;
  Handle< CacheT<FluxPointReconstruct> > reconstruct_from_flx_pts_cache;

};


////////////////////////////////////////////////////////////////////////////////

void DomainDiscretizer::initialize()
{
  sfd_cache = shared_caches->get_cache<SFDElement>();
  boost_foreach(Handle<Term>& term, terms)
  {
    term->initialize();
  }
}

void DomainDiscretizer::set_element(const Handle<Entities const>& entities_c, const Uint idx)
{
  entities = entities_c;
  elem=idx;
}


void DomainDiscretizer::create_boundary_condition(const std::string& name, const std::string& builder, const std::vector< Handle<Region const> >& regions )
{
//    Handle<BC> bc = apply_bc->create_component(name,builder);
  Handle< BC > bc = apply_bc->create_component< StrongBC<1u,1u> >(name);
  bc->regions = regions;
  bc->discretizer = handle<DomainDiscretizer>();
  bc->initialize();
}

void DomainDiscretizer::execute()
{
  sfd_cache->cache(entities);

  boost_foreach(Handle<Term>& term, terms)
  {
    term->execute(entities,elem);
  }
}

////////////////////////////////////////////////////////////////////////////////

template <Uint NEQS, Uint NDIM>
class ConvectiveTerm : public Term
{
public:
  static std::string type_name() { return "ConvectiveTerm"; }
  ConvectiveTerm(const std::string& name) : Term(name) {}
  virtual ~ConvectiveTerm() {}

  Uint nb_eqs() const { return NEQS; }
  Uint flx_pt;

  virtual void initialize()
  {
    //std::cout << "initialize " << type_name() << std::endl;
    divergence_cache            = discretizer->shared_caches->get_cache< FluxPointDivergence >();
    solution_cache              = discretizer->shared_caches->get_cache< FluxPointField<NEQS,NDIM> >(SFDM::Tags::solution());
    neighbour_solution_cache    = discretizer->shared_caches->get_cache< FluxPointField<NEQS,NDIM> >(std::string("neighbour_")+SFDM::Tags::solution());
    plane_jacobian_normal_cache = discretizer->shared_caches->get_cache< PlaneJacobianNormal<NEQS,NDIM> >();

    solution_cache->options().configure_option("field",discretizer->solution_field->uri());
    neighbour_solution_cache->options().configure_option("field",discretizer->solution_field->uri());
  }

  // Convective term execution
  // -------------------------

  virtual void execute(const Handle<const Entities>& entities, const Uint elem_idx)
  {
    plane_jacobian_normal_cache->cache(entities,elem_idx);

    solution_cache->cache(entities,elem_idx);
    flux.resize(discretizer->sfd_cache->get().sf->nb_flx_pts());

    boost_foreach(flx_pt, discretizer->sfd_cache->get().sf->interior_flx_pts())
    {
      //std::cout << "compute analytical flux in flx_pt["<<flx_pt<<"]"<<std::endl;
      compute_analytical_flux();
      //std::cout << "flux = " << flux[flx_pt] << std::endl;
    }
    for(Uint f=0; f<discretizer->sfd_cache->get().sf->nb_faces(); ++f)
    {
      set_neighbour(entities,elem_idx,f,
                    neighbour_entities,neighbour_elem_idx,face_entities,face_idx);
      if ( is_not_null(neighbour_entities) )
      {
        //std::cout << "caching neighbour idx " << neighbour_elem_idx << std::endl;
        neighbour_solution_cache->cache(neighbour_entities,neighbour_elem_idx);
        // 2) solve riemann problem on interior-faces  ----> Flux   ( linked with (1) )
        //     Save in face (yes/no)
        boost_foreach(flx_pt, discretizer->sfd_cache->get().sf->face_flx_pts(f))
        {
          //std::cout << "compute numerical flux in flx_pt["<<flx_pt<<"]" << std::endl;
          //std::cout << "neighbour sol_in_flx_pt = " << neighbour_solution->field_in_flx_pts[f] << std::endl;
          compute_numerical_flux();
          //std::cout << "flux = " << flux[flx_pt] << std::endl;
        }
        neighbour_solution_cache->get().unlock();
      }
      else
      {
        boost_foreach(flx_pt, discretizer->sfd_cache->get().sf->face_flx_pts(f))
        {
          compute_analytical_flux();
        }
      }
    }
    // compute divergence in solution points
    Field::View term = term_field->view(discretizer->sfd_cache->get().space->indexes_for_element(elem_idx));
    divergence_cache->cache(entities).compute(flux,term);

//    Field::View jacob_det = discretizer->jacobian_determinant->view(discretizer->sfd->space->indexes_for_element(elem_idx));
//    for (Uint sol_pt=0; sol_pt<discretizer->sfd->sf->nb_sol_pts(); ++sol_pt) {
//      for (Uint v=0; v<NEQS; ++v) {
//        term[sol_pt][v] *= jacob_det[sol_pt][0];
//      }
//    }

    //std::cout << "div_flx = " << to_str(term) << std::endl; //elem->divergence_from_flux_points(elem->flx_in_flx_pts).transpose() << std::endl;

    Field::View residual = discretizer->residual->view(discretizer->sfd_cache->get().space->indexes_for_element(elem_idx));
    for (Uint sol_pt=0; sol_pt<discretizer->sfd_cache->get().sf->nb_sol_pts(); ++sol_pt) {
      for (Uint v=0; v<NEQS; ++v) {
        residual[sol_pt][v] -= term[sol_pt][v];
      }
    }

    plane_jacobian_normal_cache->get().unlock();
    solution_cache->get().unlock();
  }

  // Flux evaluations
  // ----------------
  virtual void compute_analytical_flux() = 0;
  virtual void compute_numerical_flux() = 0;

  // Data

  Handle<Entities const> neighbour_entities;
  Uint neighbour_elem_idx;
  Handle<Entities const> face_entities;
  Uint face_idx;

  // In flux points:
  Handle< CacheT<FluxPointDivergence> > divergence_cache;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > solution_cache;
  Handle< CacheT<FluxPointField<NEQS,NDIM> > > neighbour_solution_cache;
  Handle< CacheT<PlaneJacobianNormal<NEQS,NDIM> > >plane_jacobian_normal_cache;

  std::vector< typename FluxPointField<NEQS,NDIM>::field_t > flux;
};

////////////////////////////////////////////////////////////////////////////////

class LinearAdvection : public ConvectiveTerm<1u,1u>
{
public:
  static std::string type_name() { return "LinearAdvection"; }
  LinearAdvection(const std::string& name) : ConvectiveTerm(name)
  {
    analytical_flux.functions("2.*x");
    analytical_flux.variables("x");
    analytical_flux.parse();
  }
  virtual ~LinearAdvection() {}

  virtual void precompute() {}

  virtual void compute_analytical_flux()
  {
    analytical_flux.evaluate(solution_cache->get().field_in_flx_pts[flx_pt],flux[flx_pt]);
  }
  virtual void compute_numerical_flux()
  {
    analytical_flux.evaluate(solution_cache->get().field_in_flx_pts[flx_pt],flux[flx_pt]);
  }

private:
  math::VectorialFunction analytical_flux;
};

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( sandbox )
{

  Handle<Group> sandbox = Core::instance().root().create_component<Group>("sandbox");
  Handle<Mesh> mesh = sandbox->create_component<Mesh>("mesh");

  std::vector<Uint> nb_cells = list_of( 4  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -4.  );

  Handle<SimpleMeshGenerator> generate_mesh = sandbox->create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh->options().configure_option("mesh",mesh->uri());
  generate_mesh->options().configure_option("nb_cells",nb_cells);
  generate_mesh->options().configure_option("lengths",lengths);
  generate_mesh->options().configure_option("offsets",offsets);
  generate_mesh->options().configure_option("bdry",true);
  generate_mesh->execute();
//  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);

  Handle<mesh::actions::BuildFaces> build_faces = sandbox->create_component<mesh::actions::BuildFaces>("build_faces");
  build_faces->options().configure_option("store_cell2face",true);
  build_faces->transform(*mesh);

  SpaceFields& sfd_space_fields = mesh->create_space_and_field_group("sfd_space",SpaceFields::Basis::CELL_BASED,"cf3.SFDM.P4");
  Field& solution = sfd_space_fields.create_field("solution","solution[vector]");
  Field& residual = sfd_space_fields.create_field("residual","residual[vector]");

  Handle<DomainDiscretizer> discretizer = sandbox->create_component<DomainDiscretizer>("discretizer");
  discretizer->solution_field = solution.handle<Field>();
  discretizer->residual = residual.handle<Field>();
  Handle<LinearAdvection> advection = discretizer->create_term<LinearAdvection>("advection");

  std::vector< Handle<Region const> > bc_regions;
  bc_regions.push_back(mesh->topology().access_component("xneg")->handle<Region>());
  bc_regions.push_back(mesh->topology().access_component("xpos")->handle<Region>());
  discretizer->create_boundary_condition("zero","cf3.common.Group",bc_regions);


  boost_foreach(const Cells& elements, find_components_recursively<Cells>(*mesh))
  {
    // Initial condition
    Handle<Entities const> entities = elements.handle<Entities>();

    SFDElement& sfd_elem = discretizer->shared_caches->get_cache<SFDElement>()->cache(entities);
    GeometryElement& geo_elem = discretizer->shared_caches->get_cache<GeometryElement>()->cache(entities);

    Reconstruct reconstruct_to_sfd;
    reconstruct_to_sfd.build_coefficients(geo_elem.sf,sfd_elem.sf);

    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      geo_elem.compute_element(elem);
      Field::View sfd_sol_in_sol_pts = solution.view(sfd_elem.space->indexes_for_element(elem));
      reconstruct_to_sfd(geo_elem.nodes,sfd_sol_in_sol_pts);
    }
  }
  // Domain discretization
  discretizer->initialize();
  for (Uint t=0; t<1; ++t)
  {
    std::cout << "t = " << t << std::endl;
    boost_foreach(const Cells& elements, find_components_recursively<Cells>(*mesh))
    {
      Handle<Entities const> entities = elements.handle<Entities>();
      for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
      {
        // Actions shared before for all terms are computed
        // ------------------------------------------------
        discretizer->set_element(entities,elem_idx);

        // Computation of terms
        // --------------------
        discretizer->execute();

        // Actions shared after all terms are computed
        // -------------------------------------------
        // - Add all terms to residual
      }
    }
  }
  discretizer->apply_bc->execute();

  std::cout << "operations = " << ReconstructBase::elementary_operations << std::endl;
  std::vector<URI> fields;
  fields.push_back(solution.uri());
  fields.push_back(residual.uri());
  fields.push_back(advection->term_field->uri());
  mesh->write_mesh("sandbox.plt",fields);

}

#if 0
BOOST_AUTO_TEST_CASE( test_P0 )
{

  //////////////////////////////////////////////////////////////////////////////
  Uint dim=1;

  CModel& model   = *Core::instance().root().create_component<CModel>("test_P0");
  model.setup("cf3.SFDM.SFDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = *model.solver().handle<SFDSolver>();
  Domain&   domain  = model.domain();

  physics.options().configure_option("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 1;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().configure_option("mesh",mesh.uri());
  generate_mesh.options().configure_option("nb_cells",nb_cells);
  generate_mesh.options().configure_option("lengths",lengths);
  generate_mesh.options().configure_option("offsets",offsets);
  generate_mesh.options().configure_option("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().configure_option(SFDM::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().configure_option(SFDM::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().configure_option(SFDM::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().configure_option("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().configure_option("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::solution()))->handle<Field>();
  solution_field.field_group().create_coordinates();

  // Discretization
  solver.domain_discretization().create_ElementCache("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0.");
//  dirichlet.configure_option("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().configure_option("time_step",100.);
  solver.time_stepping().time().options().configure_option("end_time" , 2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 1.);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 100.);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set ElementCacheinal png\n";
  gp << "set output 'test_P0.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( xy(solution_field) );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( xy(solution_field) );
  gp.send( xy(residual_field) );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field[0][0] ,  0.);

  BOOST_CHECK_EQUAL(residual_field[1][0] , -1.);

  BOOST_CHECK_EQUAL(residual_field[2][0] , 1.);

  BOOST_CHECK_EQUAL(residual_field[3][0] , 1.);


  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.field_group().create_field("rank");
  Field& rank_sync = solution_field.field_group().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.field_group().field("residual").uri());
  fields.push_back(solution_field.field_group().field("solution_backup").uri());
  mesh.write_mesh("linearadv1d.plt",fields);

  RealVector max( solution_field.row_size() ); max.setZero();
  RealVector min( solution_field.row_size() ); min.setZero();
  for (Uint i=0; i<solution_field.size(); ++i)
  {
    for (Uint j=0; j<solution_field.row_size(); ++j)
    {
      max[j] = std::max(max[j],solution_field[i][j]);
      min[j] = std::min(min[j],solution_field[i][j]);
    }
  }

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P1 )
{
  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=1;

  CModel& model   = *Core::instance().root().create_component<CModel>("test_P1");
  model.setup("cf3.SFDM.SFDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = *model.solver().handle<SFDSolver>();
  Domain&   domain  = model.domain();

  physics.options().configure_option("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 2;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().configure_option("mesh",mesh.uri());
  generate_mesh.options().configure_option("nb_cells",nb_cells);
  generate_mesh.options().configure_option("lengths",lengths);
  generate_mesh.options().configure_option("offsets",offsets);
  generate_mesh.options().configure_option("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().configure_option(SFDM::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().configure_option(SFDM::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().configure_option(SFDM::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().configure_option("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().configure_option("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::solution()))->handle<Field>();
  solution_field.field_group().create_coordinates();

  // Discretization
  solver.domain_discretization().create_ElementCache("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.configure_option("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().configure_option("time_step",100.);
  solver.time_stepping().time().options().configure_option("end_time" , 2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 1.);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 100.);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set ElementCacheinal png\n";
  gp << "set output 'test_P1.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 8u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  BOOST_CHECK_EQUAL(residual_field[0][0] , -1.);
  BOOST_CHECK_EQUAL(residual_field[1][0] , -1.);

  BOOST_CHECK_EQUAL(residual_field[2][0] , 0.);
  BOOST_CHECK_EQUAL(residual_field[3][0] , 0.);

  BOOST_CHECK_EQUAL(residual_field[4][0] , 1.);
  BOOST_CHECK_EQUAL(residual_field[5][0] , 1.);

  BOOST_CHECK_EQUAL(residual_field[6][0] , 1.);
  BOOST_CHECK_EQUAL(residual_field[7][0] , 1.);

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.field_group().create_field("rank");
  Field& rank_sync = solution_field.field_group().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.field_group().field("residual").uri());
  fields.push_back(solution_field.field_group().field("solution_backup").uri());
  mesh.write_mesh("linearadv1d.plt",fields);

  RealVector max( solution_field.row_size() ); max.setZero();
  RealVector min( solution_field.row_size() ); min.setZero();
  for (Uint i=0; i<solution_field.size(); ++i)
  {
    for (Uint j=0; j<solution_field.row_size(); ++j)
    {
      max[j] = std::max(max[j],solution_field[i][j]);
      min[j] = std::min(min[j],solution_field[i][j]);
    }
  }

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P2 )
{

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
  Uint dim=1;

  CModel& model   = *Core::instance().root().create_component<CModel>("test_P2");
  model.setup("cf3.SFDM.SFDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = *model.solver().handle<SFDSolver>();
  Domain&   domain  = model.domain();

  physics.options().configure_option("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 3;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().configure_option("mesh",mesh.uri());
  generate_mesh.options().configure_option("nb_cells",nb_cells);
  generate_mesh.options().configure_option("lengths",lengths);
  generate_mesh.options().configure_option("offsets",offsets);
  generate_mesh.options().configure_option("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().configure_option(SFDM::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().configure_option(SFDM::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().configure_option(SFDM::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().configure_option("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().configure_option("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::solution()))->handle<Field>();
  solution_field.field_group().create_coordinates();

  // Discretization
  solver.domain_discretization().create_ElementCache("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.configure_option("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().configure_option("time_step",100.);
  solver.time_stepping().time().options().configure_option("end_time" , 2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 1.);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 100.);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set ElementCacheinal png\n";
  gp << "set output 'test_P2.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  Real fraction = 100*math::Consts::eps();
  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 12u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[0][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[1][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[2][0] , -1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[3][0] , -2.  , fraction);
  BOOST_CHECK_SMALL(residual_field[4][0]                 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[5][0] ,  2.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[6][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[7][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[8][0] ,  1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[9][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[10][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[11][0],  1.  , fraction);

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.field_group().create_field("rank");
  Field& rank_sync = solution_field.field_group().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.field_group().field("residual").uri());
  fields.push_back(solution_field.field_group().field("solution_backup").uri());
  mesh.write_mesh("linearadv1d.plt",fields);

  RealVector max( solution_field.row_size() ); max.setZero();
  RealVector min( solution_field.row_size() ); min.setZero();
  for (Uint i=0; i<solution_field.size(); ++i)
  {
    for (Uint j=0; j<solution_field.row_size(); ++j)
    {
      max[j] = std::max(max[j],solution_field[i][j]);
      min[j] = std::min(min[j],solution_field[i][j]);
    }
  }

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P3 )
{

  //////////////////////////////////////////////////////////////////////////////
  Uint dim=1;

  CModel& model   = *Core::instance().root().create_component<CModel>("test_P3");
  model.setup("cf3.SFDM.SFDSolver","cf3.physics.Scalar.Scalar1D");
  PhysModel& physics = model.physics();
  SFDSolver& solver  = *model.solver().handle<SFDSolver>();
  Domain&   domain  = model.domain();

  physics.options().configure_option("v",1.);

  //////////////////////////////////////////////////////////////////////////////
  // create and configure mesh

  // Create a 2D rectangular mesh
  Mesh& mesh = *domain.create_component<Mesh>("mesh");

  Uint res        = 4;
  Uint sol_order  = 4;
  Uint time_order = 1;

  std::vector<Uint> nb_cells = list_of( res  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -3.  );

  SimpleMeshGenerator& generate_mesh = *domain.create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh.options().configure_option("mesh",mesh.uri());
  generate_mesh.options().configure_option("nb_cells",nb_cells);
  generate_mesh.options().configure_option("lengths",lengths);
  generate_mesh.options().configure_option("offsets",offsets);
  generate_mesh.options().configure_option("bdry",false);
  generate_mesh.execute();
  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);
  solver.options().configure_option(SFDM::Tags::mesh(),mesh.handle<Mesh>());

  //////////////////////////////////////////////////////////////////////////////
  // Prepare the mesh

  solver.options().configure_option(SFDM::Tags::solution_vars(),std::string("cf3.physics.Scalar.LinearAdv1D"));
  solver.options().configure_option(SFDM::Tags::solution_order(),sol_order);
  solver.iterative_solver().options().configure_option("nb_stages",time_order);
  solver.prepare_mesh().execute();

  //////////////////////////////////////////////////////////////////////////////
  // Configure simulation


  // Initial condition
  solver::Action& init = solver.initial_conditions().create_initial_condition("gaussian");
  std::vector<std::string> functions;
  // Gaussian wave
  functions.push_back("3-abs(x)");
  init.options().configure_option("functions",functions);
  solver.initial_conditions().execute();

  Field& solution_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::solution()))->handle<Field>();
  solution_field.field_group().create_coordinates();

  // Discretization
  solver.domain_discretization().create_ElementCache("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Boundary condition
  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementCache& dirichlet = solver.domain_discretization().create_ElementCache("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
//  std::vector<std::string> dirichlet_functions;
//  dirichlet_functions.push_back("0");
//  dirichlet.configure_option("functions",dirichlet_functions);

  // Time stepping
  solver.time_stepping().time().options().configure_option("time_step",100.);
  solver.time_stepping().time().options().configure_option("end_time" , 2.); // instead of 0.3
  solver.time_stepping().configure_option_recursively("cfl" , 1.);
  solver.time_stepping().configure_option_recursively("milestone_dt" , 100.);

  //////////////////////////////////////////////////////////////////////////////
  // Run simulation

  Field& residual_field = *follow_link(solver.field_manager().get_child(SFDM::Tags::residual()))->handle<Field>();


#ifdef GNUPLOT_FOUND
  Gnuplot gp(std::string(GNUPLOT_COMMAND));
  gp << "set ElementCacheinal png\n";
  gp << "set output 'test_P3.png'\n";
//  gp << "set yrange [-1.2:1.2]\n";
  gp << "set grid\n";
  gp << "set xlabel 'x'\n";
  gp << "set ylabel 'U'\n";
  gp << "set title 'Rank "<<PE::Comm::instance().rank()<<" , P"<<sol_order-1<<"  RK"<<time_order<<"'\n";
  gp << "plot ";
  gp << "'-' with linespoints title 'initial solution'"    << ", ";
  gp << "'-' with linespoints title 'final solution'"      << ", ";
  gp << "'-' with linespoints title 'residual'"            << "\n";
  gp.send( solution_field.coordinates().array() , solution_field.array() );
#endif

  model.simulate();

#ifdef GNUPLOT_FOUND
  gp.send( solution_field.coordinates().array() , solution_field.array() );
  gp.send( residual_field.coordinates().array() , residual_field.array() );
#endif

  CFinfo << "memory: " << OSystem::instance().layer()->memory_usage_str() << CFendl;

  /// CHECKS
  Real fraction = 100*math::Consts::eps();

  BOOST_CHECK_EQUAL(solver.time_stepping().properties().value<Uint>("iteration") , 2u);
  BOOST_CHECK_EQUAL(solver.time_stepping().time().dt() , 2.);

  BOOST_CHECK_EQUAL(residual_field.size()     , 16u);
  BOOST_CHECK_EQUAL(residual_field.row_size() , 1u);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[0][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[1][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[2][0] , -1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[3][0] , -1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[4][0] , -1.1270166537925839  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[5][0] , -0.87298334620741969 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[6][0] ,  0.87298334620741969 , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[7][0] ,  1.1270166537925839  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[8][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[9][0] ,  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[10][0] , 1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[11][0] , 1.  , fraction);

  BOOST_CHECK_CLOSE_FRACTION(residual_field[12][0] , 1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[13][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[14][0],  1.  , fraction);
  BOOST_CHECK_CLOSE_FRACTION(residual_field[15][0],  1.  , fraction);

  //////////////////////////////////////////////////////////////////////////////
  // Output

  std::vector<URI> fields;
  Field& rank = solution_field.field_group().create_field("rank");
  Field& rank_sync = solution_field.field_group().create_field("rank_sync");
  for (Uint r=0; r<rank.size(); ++r)
  {
    rank[r][0] = rank.rank()[r];
    rank_sync[r][0] = PE::Comm::instance().rank();
  }
  rank_sync.parallelize();
  rank_sync.synchronize();

  fields.push_back(solution_field.uri());
  fields.push_back(solution_field.field_group().field("residual").uri());
  fields.push_back(solution_field.field_group().field("solution_backup").uri());
  mesh.write_mesh("linearadv1d.plt",fields);

  RealVector max( solution_field.row_size() ); max.setZero();
  RealVector min( solution_field.row_size() ); min.setZero();
  for (Uint i=0; i<solution_field.size(); ++i)
  {
    for (Uint j=0; j<solution_field.row_size(); ++j)
    {
      max[j] = std::max(max[j],solution_field[i][j]);
      min[j] = std::min(min[j],solution_field[i][j]);
    }
  }

  //std::cout << "solution_field.max = " << max.transpose() << std::endl;
  //std::cout << "solution_field.min = " << min.transpose() << std::endl;

}
#endif
////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( finalize_mpi )
{
  PE::Comm::instance().finalize();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////
