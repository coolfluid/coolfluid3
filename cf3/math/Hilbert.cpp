// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#include "math/Hilbert.hpp"
#include "math/Defs.hpp"
#include "math/Consts.hpp"


//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

  using namespace common;
  using namespace math::Consts;

//////////////////////////////////////////////////////////////////////////////

Hilbert::Hilbert(const math::BoundingBox& bounding_box, Uint levels) :
  m_bounding_box(bounding_box),
  m_max_level(levels)
{
  m_dim = m_bounding_box.dim();

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
  m_nb_keys = (boost::uint64_t) std::ldexp(1,m_dim*m_max_level);
}

boost::uint64_t Hilbert::operator() (const RealVector& point)
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

boost::uint64_t Hilbert::operator() (const RealVector& point, Real& relative_tolerance)
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

boost::uint64_t Hilbert::operator() (const RealVector1& point)
{
  cf3_assert(m_dim==1);
  m_key = 0;
  m_level = 0;
  m_box1[A] = m_bounding_box.min()[XX];
  m_box1[B] = m_bounding_box.max()[XX];
  recursive_algorithm_1d(point[XX]);
  return m_key;
}

boost::uint64_t Hilbert::operator() (const RealVector1& point, Real& relative_tolerance)
{
  operator()(point);
  relative_tolerance = std::abs(m_box1[A]-m_box1[B]) / std::abs(m_bounding_box.max()[XX]-m_bounding_box.min()[XX]);
  return m_key;
}


boost::uint64_t Hilbert::operator() (const RealVector2& point)
{
  cf3_assert(m_dim==2);
  m_key = 0;
  m_level = 0;
  m_box2[A] = m_bounding_box.min();
  m_box2[C] = m_bounding_box.max();
  m_box2[D][XX] = m_box2[C][XX];
  m_box2[D][YY] = m_box2[A][YY];
  m_box2[B][XX] = m_box2[A][XX];
  m_box2[B][YY] = m_box2[C][YY];
  recursive_algorithm_2d(point);
  return m_key;
}

boost::uint64_t Hilbert::operator() (const RealVector2& point, Real& relative_tolerance)
{
  operator()(point);
  relative_tolerance = (m_box2[A]-m_box2[C]).norm() / (m_bounding_box.max()-m_bounding_box.min()).norm();
  return m_key;
}

boost::uint64_t Hilbert::operator() (const RealVector3& point)
{
  cf3_assert(m_dim==3);
  m_key = 0;
  m_level = 0;
  m_box3[A] = m_bounding_box.min();
  m_box3[F] = m_bounding_box.max();
  m_box3[B] <<  m_box3[A][XX]  ,  m_box3[A][YY]  ,  m_box3[F][ZZ];
  m_box3[C] <<  m_box3[F][XX]  ,  m_box3[A][YY]  ,  m_box3[F][ZZ];
  m_box3[D] <<  m_box3[F][XX]  ,  m_box3[A][YY]  ,  m_box3[A][ZZ];
  m_box3[E] <<  m_box3[F][XX]  ,  m_box3[F][YY]  ,  m_box3[A][ZZ];
  m_box3[G] <<  m_box3[A][XX]  ,  m_box3[F][YY]  ,  m_box3[F][ZZ];
  m_box3[H] <<  m_box3[A][XX]  ,  m_box3[F][YY]  ,  m_box3[A][ZZ];
  recursive_algorithm_3d(point);
  return m_key;
}

boost::uint64_t Hilbert::operator() (const RealVector3& point, Real& relative_tolerance)
{
  operator()(point);
  relative_tolerance = (m_box3[A]-m_box3[F]).norm() / (m_bounding_box.max()-m_bounding_box.min()).norm();
  return m_key;
}

boost::uint64_t Hilbert::max_key() const { return m_nb_keys-1; }

void Hilbert::recursive_algorithm_1d(const Real& p)
{
  m_min_distance=Consts::real_max();
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


void Hilbert::recursive_algorithm_2d(const RealVector2& p)
{
  m_min_distance=Consts::real_max();
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


void Hilbert::recursive_algorithm_3d(const RealVector3& p)
{
  m_min_distance=Consts::real_max();
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

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3
