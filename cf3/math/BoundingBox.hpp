// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_math_BoundingBox_hpp
#define cf3_math_BoundingBox_hpp

////////////////////////////////////////////////////////////////////////////////

#include "math/MatrixTypes.hpp"
#include "math/LibMath.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace math {
  
//////////////////////////////////////////////////////////////////////////////

/// @brief Bounding box defined by minimum and maximum coordinates
/// @author Willem Deconinck
class Math_API BoundingBox
{
public: // functions

  /// Gets the Class name
  static std::string type_name() { return "BoundingBox"; }

  BoundingBox();

  /// Constructor using RealVector
  BoundingBox(const RealVector& min, const RealVector& max);

  /// Constructor using std::vector<Real>
  BoundingBox(const std::vector<Real>& min, const std::vector<Real>& max);

  /// @brief Define bounding box with RealVectors
  void define(const RealVector& min, const RealVector& max);

  /// @brief Define bounding box with std::vector<Real>
  void define(const std::vector<Real>& min, const std::vector<Real>& max);

  /// @brief Extend BoundingBox, given a point
  void extend(const RealVector& point);

  /// @brief Expand bounding box to encompass all processors
  /// @note This function must be called on all processors
  void make_global();

public: // functions

  /// @brief Check if coordinate falls inside the bounding box
  bool contains(const RealVector& coordinate) const;

  /// @brief minimum coordinates, defining one corner of the bounding box
  const RealVector& min() const { return m_bounding_min; }

  /// @brief maximum coordinates, defining one corner of the bounding box  
  const RealVector& max() const { return m_bounding_max; }  

  /// @brief minimum coordinates, defining one corner of the bounding box
  RealVector& min() { return m_bounding_min; }

  /// @brief maximum coordinates, defining one corner of the bounding box  
  RealVector& max() { return m_bounding_max; }  
  
  /// @brief dimension of the bounding box
  Uint dim() const { return m_bounding_min.size(); }
  
private: // data

  RealVector m_bounding_min; ///< minimum coordinates
  RealVector m_bounding_max; ///< maximum coordinates

}; // end BoundingBox

////////////////////////////////////////////////////////////////////////////////

} // math
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_math_BoundingBox_hpp
