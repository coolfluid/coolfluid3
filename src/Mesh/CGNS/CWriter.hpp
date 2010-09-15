// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CGNS_CWriter_hpp
#define CF_Mesh_CGNS_CWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/CMeshWriter.hpp"

#include "Mesh/CGNS/CGNSAPI.hpp"
#include "Mesh/CGNS/Shared.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
  
  class CRegion;
  class CElements;
  
namespace CGNS {

//////////////////////////////////////////////////////////////////////////////

/// This class defines CGNS mesh format writer
/// @author Willem Deconinck
class CGNS_API CWriter : public CMeshWriter, public Shared
{

public: // typedefs

  typedef boost::shared_ptr<CWriter> Ptr;
  typedef boost::shared_ptr<CWriter const> ConstPtr;
  
private : // typedefs
  
  typedef std::vector<boost::shared_ptr<CElements const> > GroupedElements;
  typedef std::map<std::string, GroupedElements > GroupsMapType;

public: // functions
  
  /// constructor
  CWriter( const CName& name );
  
  /// Gets the Class name
  static std::string type_name() { return "CWriter"; }

  static void defineConfigOptions ( CF::Common::OptionList& options ) {}

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path);

  virtual std::string get_format() { return "CGNS"; }

  virtual std::vector<std::string> get_extensions();

private: // functions
  
  void write_base();
  
  void write_zone(const CRegion& region);
  
  void write_section(const GroupedElements& grouped_elements);

//  void write_boco(const GroupedElements& grouped_elements);

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  std::string m_fileBasename;
  
  std::map<const CArray*, Uint> m_global_start_idx;
    
}; // end CWriter


////////////////////////////////////////////////////////////////////////////////

} // namespace CGNS
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CGNS_CWriter_hpp
