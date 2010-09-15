// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFlexTable_hpp
#define CF_Mesh_CFlexTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/CElements.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////
  
/// Component holding a connectivity table
/// The table has to be filled through a buffer.
/// Before using the table one has to be sure that
/// the buffer is flushed.
/// @author Willem Deconinck Tiago Quintino
class Mesh_API CFlexTable : public Common::Component {

public:
  struct InterRegionConnectivity
  {
    CElements* element_region;
    Uint idx;
    InterRegionConnectivity(CElements* arg_elm_reg, const Uint arg_idx) : element_region(arg_elm_reg), idx(arg_idx) {}
  };
  
public:
  typedef boost::shared_ptr<CFlexTable> Ptr;
  typedef boost::shared_ptr<CFlexTable const> ConstPtr;
  typedef std::map<Uint,std::list<InterRegionConnectivity> > ConnectivityTable;
  typedef std::list<InterRegionConnectivity> Row;

  /// Contructor
  /// @param name of the component
  CFlexTable ( const CName& name );

  /// Virtual destructor
  virtual ~CFlexTable() {};

  /// Get the class name
  static std::string type_name () { return "CFlexTable"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  // functions specific to the CFlexTable component
    
  /// @return A mutable reference to the connectivity table data
  ConnectivityTable& table() { return m_table; }
  
  /// @return A const reference to the connectivity table data
  const ConnectivityTable& table() const { return m_table; }

  /// @return A mutable row of the underlying array
  Row& operator[](const Uint idx) { return m_table[idx]; }

  /// @return The number of lines in the table
  Uint size() const { return m_table.size(); }

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

private: // data
  
  ConnectivityTable m_table;  
  
};
  
////////////////////////////////////////////////////////////////////////////////

  

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFlexTable_hpp
