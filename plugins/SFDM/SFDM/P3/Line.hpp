// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_SFDM_P3_Line_hpp
#define cf3_SFDM_P3_Line_hpp

#include "SFDM/ShapeFunction.hpp"

namespace cf3 {
namespace SFDM {
namespace P3 {

class SFDM_API Line : public ShapeFunction {
public:

  typedef boost::shared_ptr<Line>       Ptr;
  typedef boost::shared_ptr<Line const> ConstPtr;

  static const Mesh::GeoShape::Type shape          = Mesh::GeoShape::LINE;
  static const Uint                 nb_nodes       = 4;
  static const Uint                 dimensionality = 1;
  static const Uint                 order          = 3;

  enum FaceNumbering { KSI_NEG = 0, KSI_POS = 1 };

public:

  /// Constructor
  Line(const std::string& name = type_name());

  /// Type name
  static std::string type_name() { return "Line"; }

  virtual const ShapeFunction& line() const;
  virtual const ShapeFunction& flux_line() const;

  virtual const RealMatrix& local_coordinates() const;

  virtual void compute_value(const RealVector& local_coordinate, RealRowVector& value) const;
  virtual void compute_gradient(const RealVector& local_coordinate, RealMatrix& gradient) const;

};

} // P3
} // SFDM
} // cf3

#endif // cf3_SFDM_P3_Line_hpp
