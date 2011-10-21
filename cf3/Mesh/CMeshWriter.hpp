// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_CMeshWriter_hpp
#define cf3_mesh_CMeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/FindComponents.hpp"
#include "common/CAction.hpp"

#include "mesh/LibMesh.hpp"
#include "mesh/CMesh.hpp"
#include "mesh/CElements.hpp"
#include "mesh/Geometry.hpp"

namespace cf3 {
namespace common {  class URI;  }
namespace mesh {

  class Geometry;
  class Field;

////////////////////////////////////////////////////////////////////////////////

/// CMeshWriter component class
/// This class serves as a component that that will write
/// the mesh to a file
/// @author Willem Deconinck
class Mesh_API CMeshWriter : public common::CAction {

public: // typedefs

  /// pointer to this type
  typedef boost::shared_ptr<CMeshWriter> Ptr;
  typedef boost::shared_ptr<CMeshWriter const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshWriter ( const std::string& name );

  /// Virtual destructor
  virtual ~CMeshWriter();

  /// Get the class name
  static std::string type_name () { return "CMeshWriter"; }

  // --------- Signals ---------

  void signal_write( common::SignalArgs& node  );

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual std::vector<std::string> get_extensions() = 0;

  virtual void write_from_to(const CMesh& mesh, const common::URI& filepath) = 0;

  virtual void execute();

  void set_fields(const std::vector<boost::shared_ptr<Field> >& fields);

private: // functions

  void config_fields();

protected: // classes

  class IsGroup
  {
   public:
     IsGroup () {}

     bool operator()(const Component& component)
     {
       return count(common::find_components<CEntities>(component));
     }

  }; // IsGroup

protected:

  // TODO: remove this
  const CMesh* m_mesh;

  std::vector<boost::weak_ptr<Field> > m_fields;

};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_CMeshWriter_hpp
