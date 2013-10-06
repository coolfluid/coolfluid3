// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file
/// @brief Translation from dynamic to static API
///
/// This file deals with the translation from the dynamic API of
/// Quadrature to static implementations of quadrature.
/// Implementations of quadratures only implement static functions, have
/// a default constructor, and inherit from QuadratureBase.\n
/// The concrete dynamic implementation
/// is created as QuadratureT<STATIC_QUADRATURE>, wrapping the static
/// implementation in a component inheriting from Quadrature.
/// @author Willem Deconinck

#ifndef cf3_mesh_QuadratureT_hpp
#define cf3_mesh_QuadratureT_hpp

////////////////////////////////////////////////////////////////////////////////

#include "mesh/Quadrature.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {

////////////////////////////////////////////////////////////////////////////////


/// @brief Translation class to link concrete static implementations to the dynamic API
template <typename Concrete, typename Abstract=Quadrature>
class QuadratureT : public Abstract
{
public: // typedefs

  typedef Concrete QDR;

public:
  /// @name Constructor / Destructor / Type name
  //  ------------------------------------------
  //@{

  /// Default constructor without arguments
  QuadratureT( const std::string& name = type_name() ) : Abstract(name)
  {
    Abstract::m_shape = QDR::shape;

    Abstract::m_order = QDR::order;

    Abstract::m_nb_nodes = QDR::nb_nodes;

    Abstract::m_dimensionality = QDR::dimensionality;

    Abstract::m_local_coordinates.resize(QDR::nb_nodes,QDR::dimensionality);
    Abstract::m_local_coordinates = QDR::local_coordinates();

    Abstract::m_weights.resize(QDR::nb_nodes);
    Abstract::m_weights = QDR::weights();
  }

  /// Default destructor
  virtual ~QuadratureT() {}

  /// Type name: Quadrature
  static std::string type_name() { return QDR::type_name(); }

  //@}

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_QuadratureT_hpp
