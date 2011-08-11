// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CInterpolator_hpp
#define CF_Mesh_CInterpolator_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

  class CMesh;
  class Field;
  class CStencilComputer;
  
////////////////////////////////////////////////////////////////////////////////

/// CInterpolator component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CInterpolator : public Common::Component {

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CInterpolator> Ptr;
  typedef boost::shared_ptr<CInterpolator const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CInterpolator ( const std::string& name );

  /// Virtual destructor
  virtual ~CInterpolator();

  /// Get the class name
  static std::string type_name () { return "CInterpolator"; }

  // --------- Signals ---------

  void signal_interpolate( Common::SignalArgs& node  );
  
  // --------- Direct access ---------

  void interpolate();

  virtual void construct_internal_storage(const CMesh& source) = 0;
  
  virtual void interpolate_field_from_to(const Field& source, Field& target) = 0;

private: // functions

  void configure_stencil_computer();
  
  void configure_interpolator_function();

private: // data
  
  /// source field
  boost::weak_ptr<Field> m_source;
  
  /// target field
  boost::weak_ptr<Field> m_target;
  
  /// The strategy to compute the stencil
  boost::shared_ptr<CStencilComputer>       m_stencil_computer;
  
  /// The strategy to interpolate.
  boost::shared_ptr<Component>       m_interpolator_function;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CInterpolator_hpp
