// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the ElementDatas of the
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

#include "SFDM/SFDSolver.hpp"
#include "SFDM/Term.hpp"
#include "SFDM/Tags.hpp"
#include "SFDM/ShapeFunction.hpp"

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

template <typename ElementData> class ElementDataFactory;
/// This is a smart handle that releases a ElementData when it goes out of scope
template <typename ElementData>
struct ElementDataHandle
{
private:
  friend class ElementDataFactory<ElementData>;
  ElementDataHandle(boost::shared_ptr<ElementData>& element_data);
public:
  ElementDataHandle();
  ~ElementDataHandle();

  ElementData* operator->() const
  {
    return m_element_data;
  }

  ElementData* get() const
  {
    return m_element_data;
  }

  ElementData& operator* () const // never throws
  {
    return *m_element_data;
  }
private:
  ElementData* m_element_data;
};

//template<typename T>
//class ElementDataHandle : public common::Handle
//{
//public:
//  /// Default constructor, generating a null handle
//  ElementDataHandle() : m_cached_ptr(0) {}

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


#define ReconstructBase_Operation_Count
#ifdef ReconstructBase_Operation_Count
#define increase_elementary_operations ++ReconstructBase::elementary_operations
#else
#define increase_elementary_operations
#endif
struct ReconstructBase
{
  template <typename matrix_type>
  static Uint nb_vars(const matrix_type& m)
  {
    return m[0].size();
  }
  static Uint nb_vars(const RealMatrix& m)
  {
    return m.cols();
  }
  static Uint nb_vars(const boost::multi_array<Real, 2>& m)
  {
    return m.shape()[1];
  }
  static Uint nb_vars(const boost::detail::multi_array::multi_array_view<Real, 2>& m)
  {
    return m.shape()[1];
  }
  template <typename matrix_type>
  static const Real& access(const matrix_type& m, Uint i, Uint j)
  {
    return m[i][j];
  }
  static const Real& access(const RealMatrix& m, Uint i, Uint j)
  {
    return m(i,j);
  }

  /// Reconstruct values from matrix with values in row-vectors to vector
  /// @param [in]  from  matrix with values in row-vectors
  /// @param [out] to    vector
  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void operator()(const matrix_type& from, const vector_type& to) const
  {
    equal(from,to);
  }

  template <typename matrix_type, typename vector_type>
  void equal(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    set_zero(to);
    add(from,to);
  }

  template <typename matrix_type, typename vector_type>
  void add(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_plus(from,to,pt);
  }

  template <typename matrix_type, typename vector_type>
  void subtract(const matrix_type& from, const vector_type& to) const
  {
    cf3_assert(m_pts.size()>0);
    boost_foreach(const Uint pt, m_pts)
      contribute_minus(from,to,pt);
  }


  /// @note vec is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename vector_type>
  static void set_zero(const vector_type& vec)
  {
    //increase_elementary_operations;
    for (Uint var=0; var<vec.size(); ++var)
      const_cast<vector_type&>(vec)[var] = 0;
  }

  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_plus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    increase_elementary_operations;
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] += m_N[pt] * access(from,pt,var);
  }

  /// @note to is marked as const, but constness is casted away inside,
  ///       according to Eigen documentation http://eigen.tuxfamily.org/dox/TopicFunctionTakingEigenTypes.html
  template <typename matrix_type, typename vector_type>
  void contribute_minus(const matrix_type& from, const vector_type& to,const Uint pt) const
  {
    increase_elementary_operations;
    for (Uint var=0; var<nb_vars(from); ++var)
      const_cast<vector_type&>(to)[var] -= m_N[pt] * access(from,pt,var);
  }

  const Real& coeff(const Uint pt) const
  {
    return m_N[pt];
  }

  const std::vector<Uint>& used_points() const
  {
    return m_pts;
  }

  static Uint elementary_operations;

protected:
  Handle<mesh::ShapeFunction const> m_sf;
  RealVector m_coord;
  RealRowVector m_N;
  std::vector<Uint> m_pts;
};
Uint ReconstructBase::elementary_operations=0u;

struct ReconstructPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const vector_type& coord, const Handle<mesh::ShapeFunction const>& sf)
  {
    m_coord = coord;
    m_pts.clear();
    m_N.resize(sf->nb_nodes());
    sf->compute_value(coord,m_N);

    // Save indexes of non-zero values to speed up reconstruction
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }
};

struct DerivativeReconstructPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint derivative, const vector_type& coord, const Handle<mesh::ShapeFunction const>& sf)
  {
    m_derivative = derivative;
    m_coord = coord;
    m_N = sf->gradient(coord).col(derivative);

    // Save indexes of non-zero values to speed up reconstruction
    m_pts.clear();
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }

  Uint derivative() const
  {
    return m_derivative;
  }

private:
  Uint m_derivative;
};

struct ReconstructToFluxPoints
{
  void build_coefficients(const Handle<SFDM::ShapeFunction const>& sf)
  {
    m_reconstruct.resize(sf->nb_flx_pts());
    for (Uint flx_pt=0; flx_pt<sf->nb_flx_pts(); ++flx_pt)
    {
      m_reconstruct[flx_pt].build_coefficients(sf->flx_pts().row(flx_pt),sf);
    }
  }

  const ReconstructPoint& operator[](const Uint flx_pt) const
  {
    return m_reconstruct[flx_pt];
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_reconstruct.size()==to.size());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to[r]);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_reconstruct.size()==to.rows());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to.row(r));
  }

private:
  std::vector<ReconstructPoint> m_reconstruct;
};

struct DerivativeReconstructFromFluxPoint : ReconstructBase
{
  /// Build coefficients for reconstruction in a given coordinate
  template <typename vector_type>
  void build_coefficients(const Uint derivative, const vector_type& coord, const Handle<SFDM::ShapeFunction const>& sf)
  {
    m_derivative = derivative;
    m_coord = coord;
    RealVector tmp(sf->nb_flx_pts());
    sf->compute_flux_derivative(derivative,coord,tmp);
    m_N = tmp;
    // Save indexes of non-zero values to speed up reconstruction
    m_pts.clear();
    for (Uint pt=0; pt<m_N.size(); ++pt)
    {
      if (std::abs(m_N[pt])>math::Consts::eps())
      {
        m_pts.push_back(pt);
      }
    }
  }

  Uint derivative() const
  {
    return m_derivative;
  }

private:
  Uint m_derivative;
};

struct GradientReconstructFromFluxPoints
{
  void build_coefficients(const Handle<SFDM::ShapeFunction const>& from_sf,Handle<mesh::ShapeFunction const> to_sf = Handle<mesh::ShapeFunction const>())
  {
    if ( is_null(to_sf) )
      to_sf=from_sf;
    m_derivative_reconstruct_from_flx_pt.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_derivative_reconstruct_from_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
        m_derivative_reconstruct_from_flx_pt[pt][d].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
    }
  }


  /// Double Operator [pt][derivative](from,to)
  /// @return derivative reconstruction in a given point
  const std::vector<DerivativeReconstructFromFluxPoint>& operator[](const Uint pt) const
  {
    return m_derivative_reconstruct_from_flx_pt[pt];
  }

private:
  std::vector< std::vector<DerivativeReconstructFromFluxPoint> > m_derivative_reconstruct_from_flx_pt;
};

struct DivergenceReconstructFromFluxPoints
{
  void build_coefficients(const Handle<SFDM::ShapeFunction const>& from_sf,Handle<mesh::ShapeFunction const> to_sf = Handle<mesh::ShapeFunction const>())
  {
    m_ndims = from_sf->dimensionality();
    if ( is_null(to_sf) )
      to_sf=from_sf;
    m_derivative_reconstruct_from_flx_pt.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_derivative_reconstruct_from_flx_pt[pt].resize(to_sf->dimensionality());
      for (Uint d=0; d<to_sf->dimensionality(); ++d)
        m_derivative_reconstruct_from_flx_pt[pt][d].build_coefficients(d,to_sf->local_coordinates().row(pt),from_sf);
    }
  }

  template <typename matrix_type>
  void set_zero(matrix_type& m) const
  {
    for (Uint i=0; i<m.size(); ++i) {
      for (Uint j=0; j<m[i].size(); ++j) {
        m[i][j]=0.;
      }
    }
  }
  void set_zero(RealMatrix& m) const
  {
    m.setZero();
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.size());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
         m_derivative_reconstruct_from_flx_pt[r][d].add(from,to[r]);
      }
    }
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_derivative_reconstruct_from_flx_pt.size()==to.rows());
    set_zero(to);
    for (Uint r=0; r<m_derivative_reconstruct_from_flx_pt.size(); ++r) {
      for (Uint d=0; d<ndims(); ++d) {
        m_derivative_reconstruct_from_flx_pt[r][d].add(from,to.row(r));
      }
    }
  }

  template <typename matrix_type_from>
  RealMatrix operator()(const matrix_type_from& from) const
  {
    RealMatrix to(m_derivative_reconstruct_from_flx_pt.size(),ReconstructBase::nb_vars(from));
    operator()(from,to);
    return to;
  }

  Uint ndims() const { return m_ndims; }

private:
  Uint m_ndims;
  std::vector< std::vector<DerivativeReconstructFromFluxPoint> > m_derivative_reconstruct_from_flx_pt;
};

struct Reconstruct
{
  void build_coefficients(const Handle<mesh::ShapeFunction const>& from_sf,
                          const Handle<mesh::ShapeFunction const>& to_sf)
  {
    m_from_sf=from_sf;
    m_to_sf=to_sf;
    m_reconstruct.resize(to_sf->nb_nodes());
    for (Uint pt=0; pt<to_sf->nb_nodes(); ++pt)
    {
      m_reconstruct[pt].build_coefficients(to_sf->local_coordinates().row(pt),from_sf);
    }
  }

  const ReconstructPoint& operator[](const Uint pt) const
  {
    return m_reconstruct[pt];
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from, typename matrix_type_to>
  void operator()(const matrix_type_from& from, matrix_type_to& to) const
  {
    cf3_assert(m_reconstruct.size()==to.size());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to[r]);
  }

  /// Reconstruct values from matrix with values in row-vectors to matrix with values in row-vectors
  template <typename matrix_type_from>
  void operator()(const matrix_type_from& from, RealMatrix& to) const
  {
    cf3_assert(m_reconstruct.size()==to.rows());
    for (Uint r=0; r<m_reconstruct.size(); ++r)
      m_reconstruct[r](from,to.row(r));
  }

private:
  Handle<mesh::ShapeFunction const> m_from_sf;
  Handle<mesh::ShapeFunction const> m_to_sf;
  std::vector<ReconstructPoint> m_reconstruct;
};



/// ElementData base class is a component. It is counted on not many of these objects will be created
/// using a caching/locking mechanism
class ElementDataBase //: public common::Component
{
public:

  ElementDataBase(const std::string& name) //: common::Component(name)
  {
    idx = math::Consts::uint_max();
    std::cout<<"Create new " << name;
    unlock();
  }

  virtual void configure(const Entities& entities_comp)
  {
    std::cout<<" for " << entities_comp.uri() <<std::endl;
    // reset and reconfigure for this element type
    unlock();
    if(entities)
    {
      if (&entities_comp==entities.get())
      {
        return;
      }
    }
    // compute if it was not configured yet
    idx = math::Consts::uint_max();
    entities = entities_comp.handle<Entities>();
    compute_fixed_data();
  }

  // mark this object as locked, must unlock manualy
  void compute_element(const Uint elem_idx)
  {
    lock();
    if (idx != elem_idx)
    {
      idx = elem_idx;
      compute_variable_data();
    }
  }

  Handle< mesh::Entities const      > entities;
  Uint idx;

  // locking mechanism
  bool locked() const { return m_locked; }
  void lock() { m_locked=true; }
  void unlock() { m_locked=false; }

private:

  virtual void compute_fixed_data() = 0;
  virtual void compute_variable_data() = 0;

private:
  bool m_locked;
};

struct GeometryElementData : ElementDataBase
{
  static std::string type_name() { return "GeometryElementData"; }
  GeometryElementData (const std::string& name=type_name()) : ElementDataBase(name) {}
private:
  virtual void compute_fixed_data()
  {
    cf3_assert(entities);
    space = entities->geometry_space().handle<mesh::Space>();
    sf = space->shape_function().handle<mesh::ShapeFunction>();
    entities->allocate_coordinates(nodes);
  }

  virtual void compute_variable_data()
  {
    entities->put_coordinates(nodes,idx);
  }

public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< mesh::ShapeFunction const > sf;

  // extrinsic state (changed for every computation)
  RealMatrix nodes;
};

template <Uint NEQS=1>
struct SpectralElementData : ElementDataBase
{
  static std::string type_name() { return "SpectralElementData"; }
  SpectralElementData (const std::string& name=type_name()) : ElementDataBase(name) {}

private:
  virtual void compute_fixed_data()
  {
    geo.configure(*entities);
    space = entities->space("sfd_space").handle<Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();

    reconstruct_to_flux_points.build_coefficients(sf);
    compute_divergence_from_flux_points.build_coefficients(sf);

    sol_in_flx_pts.resize(sf->nb_flx_pts());
    flx_in_flx_pts.resize(sf->nb_flx_pts());
    plane_jacobian_normal.resize(sf->nb_flx_pts(),RealVector(entities->element_type().dimension()));
  }

  virtual void compute_variable_data()
  {
    geo.compute_element(idx); // computes geo.nodes, for use of plane_jacobian normals

    // compute plane-jacobian normals
    const RealMatrix& flx_pts = sf->flx_pts();
    for (Uint f=0; f<sf->nb_flx_pts(); ++f)
    {
      cf3_assert(sf->flx_pt_dirs(f).size() == 1);
      CoordRef dir = static_cast<CoordRef>(sf->flx_pt_dirs(f)[0]);
      geo.entities->element_type().
          compute_plane_jacobian_normal(flx_pts.row(f),geo.nodes,dir,
                                        plane_jacobian_normal[f]);
    }
  }
public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  GeometryElementData geo;
  std::vector<RealVector> plane_jacobian_normal;
  ReconstructToFluxPoints reconstruct_to_flux_points;
  DivergenceReconstructFromFluxPoints compute_divergence_from_flux_points;
  typedef Eigen::Matrix<Real, NEQS, 1> state_t;
  std::vector<state_t> sol_in_flx_pts;
  std::vector<state_t> flx_in_flx_pts;
};

template <Uint NEQS=1>
struct SpectralNeighbourElementData : ElementDataBase
{
  static std::string type_name() { return "SpectralNeighbourElementData"; }
  SpectralNeighbourElementData (const std::string& name=type_name()) : ElementDataBase(name) {}

private:
  virtual void compute_fixed_data()
  {
    space = entities->space("sfd_space").handle<Space>();
    sf = space->shape_function().handle<SFDM::ShapeFunction>();
    reconstruct_to_flux_points.build_coefficients(sf);
    sol_in_flx_pts.resize(sf->nb_flx_pts());
  }
  virtual void compute_variable_data() {}
public:
  // intrinsic state (not supposed to change)
  Handle< mesh::Space const         > space;
  Handle< SFDM::ShapeFunction const > sf;
  ReconstructToFluxPoints reconstruct_to_flux_points;
  typedef Eigen::Matrix<Real, NEQS, 1> state_t;
  std::vector<state_t> sol_in_flx_pts;
};

struct DummyElementData : ElementDataBase
{
  static std::string type_name() { return "DummyElementData"; }
  DummyElementData (const std::string& name=type_name()) : ElementDataBase(name) {}
private:
  virtual void compute_fixed_data() {}
  virtual void compute_variable_data() {}
};

#define ELEMENTDATA_MAX_CACHE_SIZE 2
//#define ElementDataHandle boost::shared_ptr

template <typename ElementData>
ElementDataHandle<ElementData>::~ElementDataHandle()
{
  if (is_not_null(m_element_data))
    m_element_data->unlock();
}
template <typename ElementData>
ElementDataHandle<ElementData>::ElementDataHandle(boost::shared_ptr<ElementData>& element_data) :
  m_element_data(element_data.get())
{
  m_element_data->lock();
}
template <typename ElementData>
ElementDataHandle<ElementData>::ElementDataHandle() :
  m_element_data(nullptr)
{
}

template <typename ElementData>
class ElementDataFactory : public common::Component
{
public:
  typedef Entities const* KEY;

  ElementDataFactory(const std::string& name) :
    common::Component(name),
    m_max_cache_size(ELEMENTDATA_MAX_CACHE_SIZE)
  {
  }

  static std::string type_name() { return "ElementDataFactory"; }

  void set_max_cache_size(const Uint max_cache_size)
  {
    m_max_cache_size=max_cache_size;
  }

  /// destructor
  virtual ~ElementDataFactory()
  {
    while(!m_element_datas.empty())
    {
      typename std::map< KEY, std::vector<boost::shared_ptr<ElementData> > >::iterator it = m_element_datas.begin();
//      std::vector<boost::shared_ptr<ElementData> >& cached_ElementDatas = it->second;
//      boost_foreach(ElementData* t, cached_ElementDatas)
//      {
//        delete t;
//      }
      m_element_datas.erase(it);
    }

  }

  /// Create ElementData if non-existant, else get the ElementData and lock it through ElementDataHandle constructor
  ElementDataHandle<ElementData> get_element_data(const Entities& entities, int idx=-1)
  {
    typename std::map< KEY, std::vector<boost::shared_ptr<ElementData> > >::iterator it = m_element_datas.find(&entities);
    if(it != m_element_datas.end())
    {
      boost_foreach(boost::shared_ptr<ElementData>& t, it->second)
      {
        if(static_cast<int>(t->idx)==idx) // safe to use
        {
          std::cout << "ElementData is shared for " << entities.uri() << "["<<idx<<"]" << std::endl;
          return ElementDataHandle<ElementData>(t);
        }
        if(t->locked()==false) // safe to use
        {
          std::cout << "Use available ElementData for " << entities.uri() << std::endl;
          return ElementDataHandle<ElementData>(t);
        }

      }
      if (it->second.size() == m_max_cache_size) // Check for caching
        throw InvalidStructure(FromHere(),"All ElementDatas are locked for "+entities.uri().string()+" created. (max_cache_size="+to_str(m_max_cache_size)+")");
    }

//    boost::shared_ptr<ElementData> ElementData = allocate_component<ElementData>(ElementData::type_name());
    boost::shared_ptr<ElementData> element_data ( new ElementData );
    element_data->configure(entities);

    if(it != m_element_datas.end())
      it->second.push_back(element_data);
    else
      m_element_datas[&entities]=std::vector< boost::shared_ptr<ElementData> >(1,element_data);

    return ElementDataHandle<ElementData>(element_data);
  }

private:
  Uint m_max_cache_size;
  std::map< KEY, std::vector<boost::shared_ptr<ElementData> > > m_element_datas;
};

class ElementDataFactoryMother : public Component
{
public:
  static std::string type_name() { return "ElementDataFactoryMother"; }
  ElementDataFactoryMother(const std::string& name) : Component(name) {}
  virtual ~ElementDataFactoryMother() {}
  template <typename ElementData>
  Handle< ElementDataFactory<ElementData> > factory()
  {
    Handle< ElementDataFactory<ElementData> > fac = Handle< ElementDataFactory<ElementData> >(get_child(ElementData::type_name()));
    if (!fac) // if not available, generate it
    {
      std::cout << "Creating ElementDataFactory for " << ElementData::type_name() << std::endl;
      fac = create_component< ElementDataFactory<ElementData> >(ElementData::type_name());
    }
    return fac;
  }
};


// UBER shared almost global struct
class DomainDiscretizer : public Component
{
public:
  static std::string type_name() { return "DomainDiscretizer"; }
  DomainDiscretizer(const std::string& name) :
    Component(name)
  {
    factory_mother = create_static_component<ElementDataFactoryMother>("factory_mother");
    terms = create_static_component<Group>("terms");
  }
  virtual ~DomainDiscretizer() {}

  template <typename TermT>
  Handle<TermT> create_term(const std::string& name)
  {
    Handle<TermT> term = terms->create_component<TermT>(name);
    term->set_discretizer(*this);
    cf3_assert(term->discretizer);
    return term;
  }

  void set_element(const Entities& entities_c, const Uint idx)
  {
    entities = entities_c.handle<Entities>();
    elem=idx;
  }

  void execute();

  Handle<Entities const> entities;
  Uint elem;

  Handle<ElementDataFactoryMother> factory_mother;
  Handle<Group> terms;

  Handle<Field> solution_field;
  Handle<Field> residual;
  Handle<Field> jacobian_determinant;
  Handle<Field> wave_speed;
};

class Term : public Component
{
public:
  static std::string type_name() { return "Term"; }
  Term(const std::string& name) : Component(name), store_term_in_field(true) {}
  virtual ~Term() {}
  virtual void execute(const Entities& entities, const Uint elem_idx) = 0;

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

  template <typename ElementData>
  void set_element(const Entities& cell, const Uint cell_idx, ElementDataHandle<ElementData>& elem)
  {
    if (is_not_null(elem.get()))
      elem->unlock();
    cf3_assert(discretizer);
    elem = discretizer->factory_mother->factory<ElementData>()->get_element_data(cell,cell_idx);
    elem->compute_element(cell_idx);
    precompute();
  }

  virtual void precompute() {};

  template <typename ElementData, typename NeighbourElementData>
  void set_neighbour(const ElementDataHandle<ElementData>& elem, const Uint face_nb, ElementDataHandle<NeighbourElementData>& neighbour_elem)
  {
    ElementConnectivity const& face_connectivity = *elem->entities->get_child("face_connectivity")->handle<ElementConnectivity>();
    Entity face = face_connectivity[elem->idx][face_nb];
    FaceCellConnectivity const& cell_connectivity = *face.comp->get_child("cell_connectivity")->handle<FaceCellConnectivity>();
    if (cell_connectivity.is_bdry_face()[face.idx])
    {
      neighbour_elem = ElementDataHandle<NeighbourElementData>();
    }
    else
    {
      Entity neighbour;
      if (cell_connectivity.connectivity()[face.idx][LEFT].comp == elem->entities.get() &&
          cell_connectivity.connectivity()[face.idx][LEFT].idx == elem->idx)
        neighbour = cell_connectivity.connectivity()[face.idx][LEFT];
      else
        neighbour = cell_connectivity.connectivity()[face.idx][RIGHT];
      neighbour_elem = discretizer->factory_mother->factory<NeighbourElementData>()->get_element_data(*neighbour.comp,neighbour.idx);
      neighbour_elem->compute_element(neighbour.idx);
    }
  }

  bool store_term_in_field;
  Handle<Field> term_field;
};

void DomainDiscretizer::execute()
{
//    element_data->compute_element_data(elem);
//    Field::View sfd_sol_in_sol_pts = sfd_coords.view(element_data->space->indexes_for_element(elem));
//    reconstruct_to_flx_pts(sfd_sol_in_sol_pts,sfd_sol_in_flx_pts);
//    std::cout << sfd_sol_in_flx_pts.transpose() << std::endl;

  // convective terms:
  // Per term, create a field (good for visualization)
  // So 1 term needs: analytical-flux-function,  numerical-flux-function,  boundary-condition
//      Field::View sfd_div_flx = sfd_coord_grad.view(sfd_elem->space->indexes_for_element(elem_idx));
//      divergence_reconstruct_from_flx_pts(convection->flx_in_flx_pts,sfd_div_flx);
  // transform term to physical space

  // diffusive terms:
  // ...

  // source terms:
  // ...

    boost_foreach(Term& term, find_components<Term>(*terms))
    {
      term.execute(*entities,elem);
    }
}


template <Uint NEQS>
class ConvectiveTerm : public Term
{
public:
  static std::string type_name() { return "ConvectiveTerm"; }
  ConvectiveTerm(const std::string& name) : Term(name) {}
  virtual ~ConvectiveTerm() {}

  Uint nb_eqs() const { return NEQS; }
  Uint flx_pt;

  // Convective term execution
  // -------------------------

  void execute(const Entities& entities, const Uint elem_idx)
  {
    set_element(entities,elem_idx,elem);
    elem->reconstruct_to_flux_points(discretizer->solution_field->view(elem->space->indexes_for_element(elem->idx)),elem->sol_in_flx_pts);

    boost_foreach(flx_pt, elem->sf->interior_flx_pts())
    {
      std::cout << "compute analytical flux in flx_pt["<<flx_pt<<"]"<<std::endl;
      compute_analytical_flux();
    }
    for(Uint f=0; f<elem->sf->nb_faces(); ++f)
    {
      set_neighbour(elem,f,neighbour_elem);
      if ( is_not_null(neighbour_elem.get()) )
      {
        // 2) solve riemann problem on interior-faces  ----> Flux   ( linked with (1) )
        //     Save in face (yes/no)
        boost_foreach(flx_pt, elem->sf->face_flx_pts(f))
        {
          std::cout << "compute numerical flux in flx_pt["<<flx_pt<<"]" << std::endl;
          neighbour_elem->reconstruct_to_flux_points(discretizer->solution_field->view(neighbour_elem->space->indexes_for_element(neighbour_elem->idx)),neighbour_elem->sol_in_flx_pts);
          std::cout << "neighbour sol_in_flx_pt = " << neighbour_elem->sol_in_flx_pts[f] << std::endl;
          compute_numerical_flux();
        }
        neighbour_elem->unlock();
      }
      else
      {
        // 3) solve boundary condition on outer-faces  ----> Flux
        // - see which face this is on, and apply correct BC, depending on 2 parameters:
        //   --> term and location
        boost_foreach(flx_pt, elem->sf->face_flx_pts(f))
        {
          compute_boundary_flux();
        }
      }
    }
    // compute divergence in solution points
    Field::View term = term_field->view(elem->space->indexes_for_element(elem->idx));
    elem->compute_divergence_from_flux_points(elem->flx_in_flx_pts,term);
    std::cout << "div_flx = " << to_str(term) << std::endl; //elem->divergence_from_flux_points(elem->flx_in_flx_pts).transpose() << std::endl;

    Field::View residual = discretizer->residual->view(elem->space->indexes_for_element(elem->idx));
    for (Uint sol_pt=0; sol_pt<elem->sf->nb_sol_pts(); ++sol_pt) {
      for (Uint v=0; v<NEQS; ++v) {
        residual[sol_pt][v] -= term[sol_pt][v];
      }
    }
  }

  // Flux evaluations
  // ----------------
  virtual void compute_analytical_flux() = 0;
  virtual void compute_numerical_flux() = 0;
  virtual void compute_boundary_flux() = 0;

  // Data
  ElementDataHandle< SpectralElementData<NEQS>          > elem;
  ElementDataHandle< SpectralNeighbourElementData<NEQS> > neighbour_elem;
};

class LinearAdvection : public ConvectiveTerm<1u>
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
    analytical_flux.evaluate(elem->sol_in_flx_pts[flx_pt],elem->flx_in_flx_pts[flx_pt]);
  }
  virtual void compute_numerical_flux()
  {
    analytical_flux.evaluate(elem->sol_in_flx_pts[flx_pt],elem->flx_in_flx_pts[flx_pt]);
  }
  virtual void compute_boundary_flux()
  {
    analytical_flux.evaluate(elem->sol_in_flx_pts[flx_pt],elem->flx_in_flx_pts[flx_pt]);
  }

private:
  math::VectorialFunction analytical_flux;
};


BOOST_AUTO_TEST_CASE( sandbox )
{
  Fly willem("willem",5);
  Fly bart("bart",10);
  Fly another_willem("willem",12);

  std::cout << willem.age << " " << bart.age << " " << another_willem.age << std::endl;
  std::cout << &willem.first_name.get() << " " << &bart.first_name.get() << " " << &another_willem.first_name.get() << std::endl;


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

  SpaceFields& sfd_space_fields = mesh->create_space_and_field_group("sfd_space",SpaceFields::Basis::CELL_BASED,"cf3.SFDM.P0");
  Field& solution = sfd_space_fields.create_field("solution","solution[vector]");
  Field& residual = sfd_space_fields.create_field("residual","residual[vector]");

  Handle<DomainDiscretizer> discretizer = sandbox->create_component<DomainDiscretizer>("discretizer");
  discretizer->solution_field = solution.handle<Field>();
  discretizer->residual = residual.handle<Field>();
  Handle<LinearAdvection> advection = discretizer->create_term<LinearAdvection>("advection");
  Handle<LinearAdvection> convection = discretizer->create_term<LinearAdvection>("convection");

  boost_foreach(const Cells& elements, find_components_recursively<Cells>(*mesh))
  {
    // Initial condition
    ElementDataHandle<SpectralElementData<1> > sfd_elem = discretizer->factory_mother->factory<SpectralElementData<1> >()->get_element_data(elements,0);

    Reconstruct reconstruct_to_sfd;
    reconstruct_to_sfd.build_coefficients(sfd_elem->geo.sf,sfd_elem->sf);

    for (Uint elem=0; elem<elements.size(); ++elem)
    {
      sfd_elem->compute_element(elem);
      Field::View sfd_sol_in_sol_pts = solution.view(sfd_elem->space->indexes_for_element(elem));
      reconstruct_to_sfd(sfd_elem->geo.nodes,sfd_sol_in_sol_pts);
    }
  }
  // Domain discretization
  boost_foreach(const Cells& elements, find_components_recursively<Cells>(*mesh))
  {
    for (Uint elem_idx=0; elem_idx<elements.size(); ++elem_idx)
    {
      // Actions shared before for all terms are computed
      // ------------------------------------------------
      discretizer->set_element(elements,elem_idx);

      // Computation of terms
      // --------------------
      discretizer->execute();

      // Actions shared after all terms are computed
      // -------------------------------------------
      // - Add all terms to residual
    }
  }

  std::cout << "operations = " << ReconstructBase::elementary_operations << std::endl;
  std::vector<URI> fields;
  fields.push_back(solution.uri());
  fields.push_back(residual.uri());
  fields.push_back(advection->term_field->uri());
  fields.push_back(convection->term_field->uri());
  mesh->write_mesh("sandbox.plt",fields);

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
  solver.domain_discretization().create_ElementData("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementData& dirichlet = solver.domain_discretization().create_ElementData("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set ElementDatainal png\n";
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
  solver.domain_discretization().create_ElementData("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementData& dirichlet = solver.domain_discretization().create_ElementData("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set ElementDatainal png\n";
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
  solver.domain_discretization().create_ElementData("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

//  // Boundary condition
//  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementData& dirichlet = solver.domain_discretization().create_ElementData("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set ElementDatainal png\n";
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
  solver.domain_discretization().create_ElementData("cf3.SFDM.Convection","convection",std::vector<URI>(1,mesh.topology().uri()));

  // Boundary condition
  std::vector<URI> bc_regions;
//  bc_regions.push_back(mesh.topology().uri()/"xneg");
//  bc_regions.push_back(mesh.topology().uri()/"xpos");
//  ElementData& dirichlet = solver.domain_discretization().create_ElementData("cf3.SFDM.BCDirichlet","dirichlet",bc_regions);
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
  gp << "set ElementDatainal png\n";
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
