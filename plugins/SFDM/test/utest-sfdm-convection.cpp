// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
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

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"
#include "math/VariablesDescriptor.hpp"

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

#include "SFDM/SFDSolver.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"

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


struct Fly
{
  Fly(const std::string& name, int age_) : first_name(name), age(age_) {}
  boost::flyweight<std::string> first_name;
  int age;
};

template <typename Term> class TermFactory;
/// This is a smart handle that releases a term when it goes out of scope
template <typename Term>
struct TermHandle
{
private:
  friend class TermFactory<Term>;
  TermHandle(boost::shared_ptr<Term>& term);
public:
  ~TermHandle();

  Term* operator->() const
  {
    return m_term;
  }

  Term& operator* () const // never throws
  {
    return *m_term;
  }
private:
  Term* m_term;
};

//template<typename T>
//class TermHandle : public common::Handle
//{
//public:
//  /// Default constructor, generating a null handle
//  TermHandle() : m_cached_ptr(0) {}

//  /// Construction from shared_ptr. This constructor may cast the argument:
//  /// - If T is a base class of Y, a static cast is done
//  /// - otherwise, a dynamic cast is done. If this fails, the resulting Handle is null.
//  template<typename Y>
//  explicit Handle(const boost::shared_ptr<Y>& ptr)
//  {
//    BOOST_MPL_ASSERT_MSG((detail::is_const_comptible<T, Y>::value), HANDLE_CONSTRUCTOR_REMOVES_CONSTNESS, (Y));
//    create_from_shared(ptr);
//  }

//  /// Construction from another handle. Casting is done as in construction from shared_ptr.
//  template<typename Y>
//  explicit Handle(const Handle<Y>& other)
//  {
//    BOOST_MPL_ASSERT_MSG((detail::is_const_comptible<T, Y>::value), HANDLE_CONSTRUCTOR_REMOVES_CONSTNESS, (Y));

//    create_from_shared(other.m_weak_ptr.lock());
//  }



/// Term base class is a component. It is counted on not many of these objects will be created
/// using a caching/locking mechanism
class TermBase : public common::Component
{
public:

  TermBase(const std::string& name) : common::Component(name)
  {
    unlock();
  }

  virtual void allocate(const Entities& entities_comp)
  {
    std::cout<<"Create new term for " << entities_comp.uri() <<std::endl;
    entities = entities_comp.handle<Entities>();
    space = entities->geometry_space().handle<mesh::Space>();
    sf = space->shape_function().handle<mesh::ShapeFunction>();
    entities->allocate_coordinates(nodes);
  }

  virtual void compute_element_data(const Uint elem_idx)
  {
    m_idx=elem_idx;
    entities->put_coordinates(nodes,m_idx);
  }

  // intrinsic state (not supposed to change)
  Handle< mesh::Entities const      > entities;
  Handle< mesh::Space const         > space;
  Handle< mesh::ShapeFunction const > sf;

  // extrinsic state (changed for every computation)
  RealMatrix nodes;
  Uint m_idx;

  // locking mechanism
  bool locked() const { return m_locked; }
  void lock() { m_locked=true; }
  void unlock() { m_locked=false; }

private:
  bool m_locked;
};

struct TermA : TermBase
{
   TermA (const std::string& name) : TermBase(name) {}
};
struct TermB : TermBase
{
   TermB (const std::string& name) : TermBase(name) {}
};
struct TermZ : TermBase
{
   TermZ (const std::string& name) : TermBase(name) {}
};

#define TERM_MAX_CACHE_SIZE 2
//#define TermHandle boost::shared_ptr

template <typename Term>
TermHandle<Term>::~TermHandle()
{
  if (is_not_null(m_term))
    m_term->unlock();
}
template <typename Term>
TermHandle<Term>::TermHandle(boost::shared_ptr<Term>& term) :
  m_term(term.get())
{
  m_term->lock();
}

template <typename Term>
class TermFactory : public common::Component
{
public:
  typedef Entities const* KEY;

  TermFactory(const std::string& name) :
    common::Component(name),
    m_max_cache_size(TERM_MAX_CACHE_SIZE)
  {
    boost::shared_ptr<Term> tmp = allocate_component<Term>("tmp");

  }

  static std::string type_name() { return "TermFactory"; }

  void set_max_cache_size(const Uint max_cache_size)
  {
    m_max_cache_size=max_cache_size;
  }

  /// destructor
  virtual ~TermFactory()
  {
    while(!m_terms.empty())
    {
      typename std::map< KEY, std::vector<boost::shared_ptr<Term> > >::iterator it = m_terms.begin();
//      std::vector<boost::shared_ptr<Term> >& cached_terms = it->second;
//      boost_foreach(Term* t, cached_terms)
//      {
//        delete t;
//      }
      m_terms.erase(it);
    }

  }

  /// Create term if non-existant, else get the term and lock it through TermHandle constructor
  TermHandle<Term> get_term(const Entities& entities)
  {
    typename std::map< KEY, std::vector<boost::shared_ptr<Term> > >::iterator it = m_terms.find(&entities);
    if(it != m_terms.end())
    {
      boost_foreach(boost::shared_ptr<Term>& t, it->second)
      {
        if(t->locked()==false) // not in use
        {
          std::cout << "Use available term for " << entities.uri() << std::endl;
          return TermHandle<Term>(t);
        }
      }
      if (it->second.size() == m_max_cache_size) // Check for caching
        throw InvalidStructure(FromHere(),"All terms are locked for "+entities.uri().string()+" created. (max_cache_size="+to_str(m_max_cache_size)+")");
    }

    boost::shared_ptr<Term> term = allocate_component<Term>(Term::type_name());
    term->allocate(entities);

    if(it != m_terms.end())
      it->second.push_back(term);
    else
      m_terms[&entities]=std::vector< boost::shared_ptr<Term> >(1,term);

    return TermHandle<Term>(term);
  }

private:
  Uint m_max_cache_size;
  std::map< KEY, std::vector<boost::shared_ptr<Term> > > m_terms;
};

BOOST_AUTO_TEST_CASE( sandbox )
{

  Fly willem("willem",5);
  Fly bart("bart",10);
  Fly another_willem("willem",12);

  std::cout << willem.age << " " << bart.age << " " << another_willem.age << std::endl;
  std::cout << &willem.first_name.get() << " " << &bart.first_name.get() << " " << &another_willem.first_name.get() << std::endl;

  Handle<Mesh> mesh = Core::instance().root().create_component<Mesh>("sandbox");

  std::vector<Uint> nb_cells = list_of( 4  );
  std::vector<Real> lengths  = list_of(  8.  );
  std::vector<Real> offsets  = list_of(  -4.  );

  Handle<SimpleMeshGenerator> generate_mesh = Core::instance().root().create_component<SimpleMeshGenerator>("generate_mesh");
  generate_mesh->options().configure_option("mesh",mesh->uri());
  generate_mesh->options().configure_option("nb_cells",nb_cells);
  generate_mesh->options().configure_option("lengths",lengths);
  generate_mesh->options().configure_option("offsets",offsets);
  generate_mesh->options().configure_option("bdry",true);
  generate_mesh->execute();
//  build_component_abstract_type<MeshTransformer>("cf3.mesh.actions.LoadBalance","load_balance")->transform(mesh);

  Handle<TermFactory<TermA> > factory = Core::instance().root().create_component<TermFactory<TermA> >("TermA_factory");
  factory->set_max_cache_size(20u);

  boost_foreach(const Entities& elements, find_components_recursively<Entities>(*mesh))
  {
    TermHandle<TermA> term = factory->get_term(elements);
    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      term->compute_element_data(elem);
      TermHandle<TermA> term2 = factory->get_term(elements);
      TermHandle<TermA> term3 = factory->get_term(elements);
    }
  }

  std::string document = "AAZZBBZB";
  const char* chars = document.c_str();


  // For each Term use a flyweight object
  for(size_t i = 0; i < document.length(); i++)
  {
//    TermHandle term1 = factory->GetTerm(std::string(chars[i]));
    std::cout << "use term with key " << chars[i] << std::endl;
//    Term* term2 = factory->GetTerm(chars[i]);
  }

}

#if 0
BOOST_AUTO_TEST_CASE( test_P0 )
{

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
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
  solver.domain_discretization().create_term("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set terminal png\n";
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

  std::cout << "solution_field.max = " << max.transpose() << std::endl;
  std::cout << "solution_field.min = " << min.transpose() << std::endl;

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
  solver.domain_discretization().create_term("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set terminal png\n";
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

  std::cout << "solution_field.max = " << max.transpose() << std::endl;
  std::cout << "solution_field.min = " << min.transpose() << std::endl;

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
  solver.domain_discretization().create_term("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set terminal png\n";
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

  std::cout << "solution_field.max = " << max.transpose() << std::endl;
  std::cout << "solution_field.min = " << min.transpose() << std::endl;

}

BOOST_AUTO_TEST_CASE( test_P3 )
{

  //////////////////////////////////////////////////////////////////////////////
  // create and configure SFD - LinEuler 2D model
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
  solver.domain_discretization().create_term("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Boundary condition
  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  Term& dirichlet = solver.domain_discretization().create_term("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set terminal png\n";
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

  std::cout << "solution_field.max = " << max.transpose() << std::endl;
  std::cout << "solution_field.min = " << min.transpose() << std::endl;

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
