// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CMeshTransformer_hpp
#define CF_Mesh_CMeshTransformer_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/Component.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {

  class CMesh;
  class CRegion;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshTransformer component class
/// This class serves as a component that that will operate on meshes
/// @author Willem Deconinck
class Mesh_API CMeshTransformer : public Common::Component {

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CMeshTransformer,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CMeshTransformer> Ptr;
  typedef boost::shared_ptr<CMeshTransformer const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshTransformer ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshTransformer();

  /// Get the class name
  static std::string type_name () { return "CMeshTransformer"; }
  
  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void transform( Common::XmlNode& node  );

  // --------- Direct access ---------

  virtual void transform(const CMesh::Ptr& mesh, const std::vector<std::string>& args = std::vector<std::string>())= 0;
  
  /// brief description, typically one line
  virtual std::string brief_description() const = 0;

  /// extended help that user can query
  virtual std::string help() const = 0;

protected: // functions

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshTransformer_hpp
