// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_SFDM_SF_QuadP1_hpp
#define CF_SFDM_SF_QuadP1_hpp

#include "SFDM/ShapeFunction.hpp"
#include "SFDM/SF/LibSF.hpp"

namespace CF {
namespace SFDM {
namespace SF {

//////////////////////////////////////////////////////////////////////////////

class SFDM_SF_API QuadP1  : public ShapeFunction {
public:

  static const GeoShape::Type shape          = Mesh::GeoShape::QUAD;
  static const Uint           nb_nodes       = 4;
  static const Uint           dimensionality = 2;
  static const Uint           order          = 1;

public:

  /// Constructor
  QuadP1(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "QuadP1"; }

  virtual const ShapeFunction& line() const;
  virtual const ShapeFunction& flux_line() const;

  virtual const RealMatrix& solution_coordinates() const;
  virtual const RealMatrix& flux_coordinates() const;

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const;
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const;

};

//////////////////////////////////////////////////////////////////////////////

} // SF
} // SFDM
} // CF

#endif // CF_Mesh_SF_QuadP1
