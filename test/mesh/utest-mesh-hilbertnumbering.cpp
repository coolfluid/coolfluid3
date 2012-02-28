// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "Test module for cf3::mesh::HilbertNumbering"


#include <boost/test/unit_test.hpp>

#include <boost/cstdint.hpp>
#include "common/Log.hpp"

#include "common/Core.hpp"
#include "common/Environment.hpp"

#include "mesh/BoundingBox.hpp"
#include "math/Consts.hpp"

using namespace cf3;
using namespace cf3::mesh;
using namespace cf3::math::Consts;
using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////////

struct HilbertNumberingTests_Fixture
{
  /// common setup for each test case
  HilbertNumberingTests_Fixture()
  {
    m_argc = boost::unit_test::framework::master_test_suite().argc;
    m_argv = boost::unit_test::framework::master_test_suite().argv;
  }

  /// common tear-down for each test case
  ~HilbertNumberingTests_Fixture()
  {
  }
  /// possibly common functions used on the tests below


  /// common values accessed by all tests goes here
  int    m_argc;
  char** m_argv;

};

////////////////////////////////////////////////////////////////////////////////

/// @brief Class to compute a global index given a coordinate
///
/// This algorithm is based on:
/// - John J. Bartholdi and Paul Goldsman "Vertex-Labeling Algorithms for the Hilbert Spacefilling Curve"
/// It is adapted to return contiguous numbers of the boost::uint64_t type.
///
/// Given a bounding box and number of hilbert levels, the bounding box can be divided in
/// 2^(dim*levels) equally spaced cells. A given coordinate falling inside one of these cells, is assigned
/// the 1-dimensional Hilbert-index of this cell. To make sure that 1 coordinate corresponds to only 1
/// Hilbert index, the number of levels have to be increased.
/// In 1D, the levels cannot be higher than 32, if you want the indices to fit in "unsigned int" type of 32bit.
/// In 2D, the levels cannot be higher than 15, if you want the indices to fit in "unsigned int" type of 32bit.
/// In 3D, the levels cannot be higher than 10, if you want the indices to fit in "unsigned int" type of 32bit.
///
///
/// No attempt is made to provide the most efficient algorithm. There exist other open-source
/// libraries with more efficient computation, such as libhilbert, but its GPL license
/// is not compatible with the LGPL license.
///
/// @author Willem Deconinck
class Hilbert
{
public:

  /// Constructor
  /// Initializes the hilbert space filling curve with a given "space" and "levels"
  Hilbert(const Handle<BoundingBox const>& bounding_box, Uint levels)
  {
    m_max_level = levels;
    m_bounding_box = bounding_box;
    m_dim = bounding_box->dim();

    switch (m_dim)
    {
    case DIM_2D:
      m_box2.resize(4);
      m_tmp_box2.resize(4);
      break;
    case DIM_3D:
      m_box3.resize(8);
      m_tmp_box3.resize(8);
      break;
    case DIM_1D:
      m_box1.resize(2);
      break;
    }
    m_nb_keys = (boost::uint64_t) std::ldexp(1,m_dim*levels);
  }

   boost::uint64_t operator() (const RealVector& point)
  {
    switch (m_dim)
    {
    case DIM_2D:
      return operator()( static_cast<RealVector2>(point) );
    case DIM_3D:
      return operator()( static_cast<RealVector3>(point) );
    case DIM_1D:
      return operator()( static_cast<RealVector1>(point) );
    }
    return 0u;
  }

   boost::uint64_t operator() (const RealVector& point, Real& relative_tolerance)
  {
    switch (m_dim)
    {
    case DIM_2D:
      return operator()( static_cast<RealVector2>(point),relative_tolerance);
    case DIM_3D:
      return operator()( static_cast<RealVector3>(point),relative_tolerance);
    case DIM_1D:
      return operator()( static_cast<RealVector1>(point),relative_tolerance);
    }
    return 0u;
  }

   boost::uint64_t operator() (const RealVector1& point)
  {
    cf3_assert(m_dim==1);
    m_key = 0;
    m_level = 0;
    m_box1[A] = m_bounding_box->min()[XX];
    m_box1[B] = m_bounding_box->max()[XX];
    recursive_algorithm_1d(point[XX]);
    return m_key;
  }

   boost::uint64_t operator() (const RealVector1& point, Real& relative_tolerance)
  {
    operator()(point);
    relative_tolerance = std::abs(m_box1[A]-m_box1[B]) / std::abs(m_bounding_box->max()[XX]-m_bounding_box->min()[XX]);
    return m_key;
  }


  boost::uint64_t operator() (const RealVector2& point)
  {
    cf3_assert(m_dim==2);
    m_key = 0;
    m_level = 0;
    m_box2[A] = m_bounding_box->min();
    m_box2[C] = m_bounding_box->max();
    m_box2[D][XX] = m_box2[C][XX];
    m_box2[D][YY] = m_box2[A][YY];
    m_box2[B][XX] = m_box2[A][XX];
    m_box2[B][YY] = m_box2[C][YY];
    recursive_algorithm_2d(point);
    return m_key;
  }

  boost::uint64_t operator() (const RealVector2& point, Real& relative_tolerance)
  {
    operator()(point);
    relative_tolerance = (m_box2[A]-m_box2[C]).norm() / (m_bounding_box->max()-m_bounding_box->min()).norm();
    return m_key;
  }

  boost::uint64_t operator() (const RealVector3& point)
  {
    cf3_assert(m_dim==3);
    m_key = 0;
    m_level = 0;
    m_box3[A] = m_bounding_box->min();
    m_box3[F] = m_bounding_box->max();
    m_box3[B] <<  m_box3[A][XX]  ,  m_box3[A][YY]  ,  m_box3[F][ZZ];
    m_box3[C] <<  m_box3[F][XX]  ,  m_box3[A][YY]  ,  m_box3[F][ZZ];
    m_box3[D] <<  m_box3[F][XX]  ,  m_box3[A][YY]  ,  m_box3[A][ZZ];
    m_box3[E] <<  m_box3[F][XX]  ,  m_box3[F][YY]  ,  m_box3[A][ZZ];
    m_box3[G] <<  m_box3[A][XX]  ,  m_box3[F][YY]  ,  m_box3[F][ZZ];
    m_box3[H] <<  m_box3[A][XX]  ,  m_box3[F][YY]  ,  m_box3[A][ZZ];
    recursive_algorithm_3d(point);
    return m_key;
  }

  boost::uint64_t operator() (const RealVector3& point, Real& relative_tolerance)
  {
    operator()(point);
    relative_tolerance = (m_box3[A]-m_box3[F]).norm() / (m_bounding_box->max()-m_bounding_box->min()).norm();
    return m_key;
  }

  boost::uint64_t max_key() const { return m_nb_keys-1; }

private:

  void recursive_algorithm_1d(const Real& p)
  {
    m_min_distance=real_max();
    for (m_idx=0; m_idx<2; ++m_idx)
    {
      m_distance = std::abs(p-m_box1[m_idx]);
      if (m_distance < m_min_distance)
      {
        m_quadrant=m_idx;
        m_min_distance = m_distance;
      }
    }
    if(m_quadrant == A)
    {
      m_box1[B]=0.5*(m_box1[A]+m_box1[B]);
    }
    else
    {
      m_box1[A]=0.5*(m_box1[A]+m_box1[B]);
    }

    m_key += (boost::uint64_t) std::ldexp((long double) 0.5*m_quadrant,(m_max_level-m_level));

    ++m_level;
    if (m_level < m_max_level)
      recursive_algorithm_1d(p);

  }

  /// @brief compute recursively the hilbert key for a given point
  void recursive_algorithm_2d(const RealVector2& p)
  {
    m_min_distance=real_max();
    for (m_idx=0; m_idx<4; ++m_idx)
    {
      m_distance = (p-m_box2[m_idx]).squaredNorm();
      if (m_distance < m_min_distance)
      {
        m_quadrant=m_idx;
        m_min_distance = m_distance;
      }
    }

    switch (m_quadrant)
    {
    case A:
      m_tmp_box2[A] = m_box2[A];
      m_tmp_box2[B] = 0.5*(m_box2[A]+m_box2[D]);
      m_tmp_box2[C] = 0.5*(m_box2[A]+m_box2[C]);
      m_tmp_box2[D] = 0.5*(m_box2[A]+m_box2[B]);
      break;
    case B:
      m_tmp_box2[A] = 0.5*(m_box2[A]+m_box2[B]);
      m_tmp_box2[B] = m_box2[B];
      m_tmp_box2[C] = 0.5*(m_box2[B]+m_box2[C]);
      m_tmp_box2[D] = 0.5*(m_box2[B]+m_box2[D]);
      break;
    case C:
      m_tmp_box2[A] = 0.5*(m_box2[A]+m_box2[C]);
      m_tmp_box2[B] = 0.5*(m_box2[B]+m_box2[C]);
      m_tmp_box2[C] = m_box2[C];
      m_tmp_box2[D] = 0.5*(m_box2[C]+m_box2[D]);
      break;
    case D:
      m_tmp_box2[A] = 0.5*(m_box2[C]+m_box2[D]);
      m_tmp_box2[B] = 0.5*(m_box2[B]+m_box2[D]);
      m_tmp_box2[C] = 0.5*(m_box2[A]+m_box2[D]);
      m_tmp_box2[D] = m_box2[D];
      break;
    }

    m_box2 = m_tmp_box2;

    m_key += (boost::uint64_t) std::ldexp((long double) 0.25*m_quadrant,2*(m_max_level-m_level));

    cf3_assert( (p-m_box2[A]).norm() <= (m_box2[A]-m_box2[C]).norm() );
    cf3_assert( (p-m_box2[B]).norm() <= (m_box2[A]-m_box2[C]).norm() );
    cf3_assert( (p-m_box2[C]).norm() <= (m_box2[A]-m_box2[C]).norm() );
    cf3_assert( (p-m_box2[D]).norm() <= (m_box2[A]-m_box2[C]).norm() );

    ++m_level;
    if (m_level < m_max_level)
      recursive_algorithm_2d(p);
  }


  void recursive_algorithm_3d(const RealVector3& p)
  {
    m_min_distance=real_max();
    for (m_idx=0; m_idx<8; ++m_idx)
    {
      m_distance = (p-m_box3[m_idx]).squaredNorm();
      if (m_distance < m_min_distance)
      {
        m_octant=m_idx;
        m_min_distance = m_distance;
      }
    }

    switch (m_octant)
    {
    case A:
      m_tmp_box3[A] = m_box3[A];
      m_tmp_box3[B] = 0.5*(m_box3[A]+m_box3[H]);
      m_tmp_box3[C] = 0.5*(m_box3[A]+m_box3[E]);
      m_tmp_box3[D] = 0.5*(m_box3[A]+m_box3[D]);
      m_tmp_box3[E] = 0.5*(m_box3[A]+m_box3[C]);
      m_tmp_box3[F] = 0.5*(m_box3[A]+m_box3[F]);
      m_tmp_box3[G] = 0.5*(m_box3[A]+m_box3[G]);
      m_tmp_box3[H] = 0.5*(m_box3[A]+m_box3[B]);
      break;
    case B:
      m_tmp_box3[A] = 0.5*(m_box3[B]+m_box3[A]);
      m_tmp_box3[B] = m_box3[B];
      m_tmp_box3[C] = 0.5*(m_box3[B]+m_box3[G]);
      m_tmp_box3[D] = 0.5*(m_box3[B]+m_box3[H]);
      m_tmp_box3[E] = 0.5*(m_box3[B]+m_box3[E]);
      m_tmp_box3[F] = 0.5*(m_box3[B]+m_box3[F]);
      m_tmp_box3[G] = 0.5*(m_box3[B]+m_box3[C]);
      m_tmp_box3[H] = 0.5*(m_box3[B]+m_box3[D]);
      break;
    case C:
      m_tmp_box3[A] = 0.5*(m_box3[C]+m_box3[A]);
      m_tmp_box3[B] = 0.5*(m_box3[C]+m_box3[B]);
      m_tmp_box3[C] = m_box3[C];
      m_tmp_box3[D] = 0.5*(m_box3[C]+m_box3[D]);
      m_tmp_box3[E] = 0.5*(m_box3[C]+m_box3[E]);
      m_tmp_box3[F] = 0.5*(m_box3[C]+m_box3[F]);
      m_tmp_box3[G] = 0.5*(m_box3[C]+m_box3[G]);
      m_tmp_box3[H] = 0.5*(m_box3[C]+m_box3[H]);
      break;
    case D:
      m_tmp_box3[A] = 0.5*(m_box3[D]+m_box3[G]);
      m_tmp_box3[B] = 0.5*(m_box3[D]+m_box3[B]);
      m_tmp_box3[C] = 0.5*(m_box3[D]+m_box3[A]);
      m_tmp_box3[D] = 0.5*(m_box3[D]+m_box3[H]);
      m_tmp_box3[E] = 0.5*(m_box3[D]+m_box3[E]);
      m_tmp_box3[F] = m_box3[D];
      m_tmp_box3[G] = 0.5*(m_box3[D]+m_box3[C]);
      m_tmp_box3[H] = 0.5*(m_box3[D]+m_box3[F]);
      break;
    case E:
      m_tmp_box3[A] = 0.5*(m_box3[E]+m_box3[C]);
      m_tmp_box3[B] = 0.5*(m_box3[E]+m_box3[F]);
      m_tmp_box3[C] = m_box3[E];
      m_tmp_box3[D] = 0.5*(m_box3[E]+m_box3[D]);
      m_tmp_box3[E] = 0.5*(m_box3[E]+m_box3[A]);
      m_tmp_box3[F] = 0.5*(m_box3[E]+m_box3[H]);
      m_tmp_box3[G] = 0.5*(m_box3[E]+m_box3[G]);
      m_tmp_box3[H] = 0.5*(m_box3[E]+m_box3[B]);
      break;
    case F:
      m_tmp_box3[A] = 0.5*(m_box3[F]+m_box3[A]);
      m_tmp_box3[B] = 0.5*(m_box3[F]+m_box3[B]);
      m_tmp_box3[C] = 0.5*(m_box3[F]+m_box3[C]);
      m_tmp_box3[D] = 0.5*(m_box3[F]+m_box3[D]);
      m_tmp_box3[E] = 0.5*(m_box3[F]+m_box3[E]);
      m_tmp_box3[F] = m_box3[F];
      m_tmp_box3[G] = 0.5*(m_box3[F]+m_box3[G]);
      m_tmp_box3[H] = 0.5*(m_box3[F]+m_box3[H]);
      break;
    case G:
      m_tmp_box3[A] = 0.5*(m_box3[G]+m_box3[E]);
      m_tmp_box3[B] = 0.5*(m_box3[G]+m_box3[F]);
      m_tmp_box3[C] = 0.5*(m_box3[G]+m_box3[C]);
      m_tmp_box3[D] = 0.5*(m_box3[G]+m_box3[D]);
      m_tmp_box3[E] = 0.5*(m_box3[G]+m_box3[A]);
      m_tmp_box3[F] = 0.5*(m_box3[G]+m_box3[B]);
      m_tmp_box3[G] = m_box3[G];
      m_tmp_box3[H] = 0.5*(m_box3[G]+m_box3[H]);
      break;
    case H:
      m_tmp_box3[A] = 0.5*(m_box3[H]+m_box3[G]);
      m_tmp_box3[B] = 0.5*(m_box3[H]+m_box3[B]);
      m_tmp_box3[C] = 0.5*(m_box3[H]+m_box3[C]);
      m_tmp_box3[D] = 0.5*(m_box3[H]+m_box3[F]);
      m_tmp_box3[E] = 0.5*(m_box3[H]+m_box3[E]);
      m_tmp_box3[F] = 0.5*(m_box3[H]+m_box3[D]);
      m_tmp_box3[G] = 0.5*(m_box3[H]+m_box3[A]);
      m_tmp_box3[H] = m_box3[H];
      break;
    }

    m_box3 = m_tmp_box3;

    m_key += (uint64_t) std::ldexp((long double) 0.125*m_octant,3*(m_max_level-m_level));

    ++m_level;
    if (m_level < m_max_level)
      recursive_algorithm_3d(p);
  }

private:

  enum {A=0, B=1, C=2, D=3, E=4, F=5, G=6, H=7};

  boost::uint64_t m_nb_keys;
  Uint m_max_level;
  Handle<BoundingBox const> m_bounding_box;

  Uint m_dim;
  Uint m_level;
   boost::uint64_t m_key;
  Uint m_quadrant;
  Uint m_octant;
  Real m_distance;
  Real m_min_distance;
  Uint m_idx;
  std::vector<Real> m_box1;
  std::vector<RealVector2,Eigen::aligned_allocator<RealVector2> > m_box2;
  std::vector<RealVector2,Eigen::aligned_allocator<RealVector2> > m_tmp_box2;
  std::vector<RealVector3,Eigen::aligned_allocator<RealVector3> > m_box3;
  std::vector<RealVector3,Eigen::aligned_allocator<RealVector3> > m_tmp_box3;
};


////////////////////////////////////////////////////////////////////////////////

BOOST_FIXTURE_TEST_SUITE( HilbertNumberingTests_TestSuite, HilbertNumberingTests_Fixture )

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( init )
{
  Core::instance().initiate(m_argc,m_argv);
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( test_hilbert_1d )
{
  Handle<BoundingBox> bounding_box = Core::instance().root().create_component<BoundingBox>("bounding_box_1d");
  RealVector1 min; min << 0.;
  RealVector1 max; max << 1.;
  bounding_box->define(min,max);
  RealVector1 point;
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  point << 0.25; BOOST_CHECK_EQUAL(compute_hilbert_level_1(point), 0u);
  point << 0.75; BOOST_CHECK_EQUAL(compute_hilbert_level_1(point), 1u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  point << 0.125; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 0u);
  point << 0.375; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 1u);
  point << 0.625; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 2u);
  point << 0.875; BOOST_CHECK_EQUAL(compute_hilbert_level_2(point), 3u);

  CFinfo << "1D: 32 levels: max_key = " << Hilbert(bounding_box,32).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

BOOST_AUTO_TEST_CASE( test_hilbert_2d )
{
  Handle<BoundingBox> bounding_box = Core::instance().root().create_component<BoundingBox>("bounding_box_2d");
  bounding_box->define(RealVector2(0.,0.),RealVector2(1.,1.));
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.25,0.25)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.25,0.75)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.75,0.75)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector2(0.75,0.25)), 3u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.125)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.125)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.375)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.375)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.625)), 4u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.125,0.875)), 5u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.875)), 6u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector2(0.375,0.625)), 7u);

  CFinfo << "2D: 16 levels: max_key = " << Hilbert(bounding_box,16).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

BOOST_AUTO_TEST_CASE( test_hilbert_3d )
{
  Handle<BoundingBox> bounding_box = Core::instance().root().create_component<BoundingBox>("bounding_box_3d");
  bounding_box->define(RealVector3(0.,0.,0.),RealVector3(1.,1.,1.));
  Hilbert compute_hilbert_level_1(bounding_box, 1);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.25,0.25)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.25,0.75)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.25,0.75)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.25,0.25)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.75,0.25)), 4u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.75,0.75,0.75)), 5u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.75,0.75)), 6u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_1(RealVector3(0.25,0.75,0.25)), 7u);

  Hilbert compute_hilbert_level_2(bounding_box, 2);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.125,0.125)), 0u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.375,0.125)), 1u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.375,0.375,0.125)), 2u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.375,0.125,0.125)), 3u);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.875,0.375,0.125)), 28);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.875,0.875,0.875)), 45);
  BOOST_CHECK_EQUAL(compute_hilbert_level_2(RealVector3(0.125,0.875,0.125)), 63);

  Hilbert compute_hilbert(bounding_box,10);
  CFinfo << "3D: 10 levels: max_key = " << Hilbert(bounding_box,10).max_key() << CFendl;
  CFinfo << "3D: 11 levels: max_key = " << Hilbert(bounding_box,11).max_key() << CFendl;
  CFinfo << "uint_max()             = " << uint_max() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_CASE( terminate )
{
  Core::instance().terminate();
}

////////////////////////////////////////////////////////////////////////////////

BOOST_AUTO_TEST_SUITE_END()

////////////////////////////////////////////////////////////////////////////////

