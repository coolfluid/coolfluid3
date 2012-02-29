// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/PE/Comm.hpp"

#include "math/Consts.hpp"
#include "math/BoundingBox.hpp"
#include "math/MatrixTypesConversion.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {

  using namespace common;
  using namespace math::Consts;

//////////////////////////////////////////////////////////////////////////////

BoundingBox::BoundingBox()
{
}

//////////////////////////////////////////////////////////////////////////////

BoundingBox::BoundingBox(const RealVector& min, const RealVector& max)
{
  m_bounding_min.resize(min.size());
  m_bounding_max.resize(max.size());
  m_bounding_min = min;
  m_bounding_max = max;
}

//////////////////////////////////////////////////////////////////////////////

BoundingBox::BoundingBox(const std::vector<Real>& min, const std::vector<Real>& max)
{
  m_bounding_min.resize(min.size());
  m_bounding_max.resize(max.size());
  math::copy(min,m_bounding_min);
  math::copy(max,m_bounding_max);
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::define(const RealVector& min, const RealVector& max)
{
  m_bounding_min.resize(min.size());
  m_bounding_max.resize(max.size());
  m_bounding_min = min;
  m_bounding_max = max;
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::define(const std::vector<Real>& min, const std::vector<Real>& max)
{
  m_bounding_min.resize(min.size());
  m_bounding_max.resize(max.size());
  math::copy(min,m_bounding_min);
  math::copy(max,m_bounding_max);
}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::extend(const RealVector &point)
{
  if (!dim())
  {
    m_bounding_min.resize(point.size());  m_bounding_min.setConstant(real_max());
    m_bounding_max.resize(point.size());  m_bounding_max.setConstant(real_min());
  }
  cf3_assert(point.size() == dim());
  for (Uint d=0; d<dim(); ++d)
  {
    m_bounding_min[d] = std::min(m_bounding_min[d],  point[d]);
    m_bounding_max[d] = std::max(m_bounding_max[d],  point[d]);
  }

}

//////////////////////////////////////////////////////////////////////////////

void BoundingBox::make_global()
{
  if (PE::Comm::instance().is_active())
  {
    // Find global minimum and global maximum
    std::vector<Real> bounding_min(dim());
    std::vector<Real> bounding_max(dim());
    math::copy(min(),bounding_min);
    math::copy(max(),bounding_max);
    PE::Comm::instance().all_reduce(PE::min(),bounding_min,bounding_min);
    PE::Comm::instance().all_reduce(PE::max(),bounding_max,bounding_max);

    // Copy global minimum and global maximum in bounding box
    math::copy(bounding_min,min());
    math::copy(bounding_max,max());
  }
}

//////////////////////////////////////////////////////////////////////////////

bool BoundingBox::contains(const RealVector& coordinate) const
{
  bool inside=true;
  for (Uint d=0; d<coordinate.size(); ++d)
  {
    inside = inside && (coordinate[d]>=m_bounding_min[d] && coordinate[d]<=m_bounding_max[d]);
  }
  return inside;
}

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3
