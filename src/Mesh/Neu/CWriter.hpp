// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Neu_CWriter_hpp
#define CF_Mesh_Neu_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CTable.hpp"

#include "Mesh/Neu/LibNeu.hpp"
#include "Mesh/Neu/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  class CElements;

namespace Neu {
  

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neu mesh format writer
/// @author Willem Deconinck
class Neu_API CWriter : public CMeshWriter, public Shared
{

public: // typedefs

  typedef boost::shared_ptr<CWriter> Ptr;
  typedef boost::shared_ptr<CWriter const> ConstPtr;
  
private : // typedefs
  

public: // functions
  
  /// constructor
  CWriter( const std::string& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  static void define_config_properties ( CF::Common::PropertyList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "Neu"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
  
  void write_headerData(std::fstream& file);

  void write_coordinates(std::fstream& file);

  void write_connectivity(std::fstream& file);

  void write_groups(std::fstream& file);

  void write_boundaries(std::fstream& file);

  void create_nodes_to_element_connectivity();

private: // helper functions

  /// regists all the signals declared in this class
  virtual void define_signals () {}

private: // data
  
  /// implementation detail, raw pointers are safe as keys
  std::map<CElements const*,Uint> m_global_start_idx;

  std::string m_fileBasename;
    
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // Neu
} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_CWriter_hpp
