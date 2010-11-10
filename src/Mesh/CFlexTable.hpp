// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CFlexTable_hpp
#define CF_Mesh_CFlexTable_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"

#include "Mesh/ArrayBase.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/ArrayBufferT.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// Component holding a connectivity table with variable row-size per row
/// @author Willem Deconinck
class Mesh_API CFlexTable : public Common::Component {

public:
  typedef boost::shared_ptr<CFlexTable> Ptr;
  typedef boost::shared_ptr<CFlexTable const> ConstPtr;

  class Row
  {
  public:
    Row(Uint* loc, Uint size)  : m_loc(loc), m_size(size) {}
    
    Uint& operator[](const Uint idx) { return m_loc[idx]; }
    const Uint& operator[](const Uint idx) const { return m_loc[idx]; }
    
    Uint size() const { return m_size; }
    
  private:
    Uint* m_loc;
    Uint m_size;
  };
  
  class ConstRow
  {
  public:
    ConstRow(const Uint* loc, const Uint size)  : m_loc(loc), m_size(size) {}
    
    const Uint operator[](const Uint idx) const { return m_loc[idx]; }
    
    const Uint size() const { return m_size; }
    
  private:
    const Uint* m_loc;
    const Uint m_size;
  };
  
  /// Contructor
  /// @param name of the component
  CFlexTable ( const CName& name );

  /// Get the class name
  static std::string type_name () { return "CFlexTable"; }

  /// Configuration Options
  static void defineConfigProperties ( Common::PropertyList& options ) {}

  Uint size() const {return m_idx.size()-1;}
  
  Uint row_size(const Uint i) const {return m_idx[i+1]-m_idx[i];}
  
  template <typename VectorT>
  Uint add_row(const VectorT& row)
  {
    Uint last_idx = m_array.size();
    m_array.resize(m_array.size()+row.size());
    for (Uint i=0; i<row.size(); ++i)
      m_array[last_idx+i] = row[i];

    m_idx.push_back(m_array.size());
    
    return m_idx.size()-1;
  }
  
  void add_rows(std::vector< std::set< Uint > > rows)
  {
    Uint next_idx_idx = m_idx.size();
    Uint next_array_idx = m_array.size();
    
    m_idx.resize(m_idx.size()+rows.size());
    
    Uint grow_size = 0;
  	BOOST_FOREACH(std::set<Uint> row, rows)
  	{
  	  grow_size += row.size();
      m_idx[next_idx_idx] = m_idx[next_idx_idx-1] + row.size();
      ++next_idx_idx;
  	}
    
    m_array.resize(m_array.size()+grow_size);
    BOOST_FOREACH(std::set<Uint> row, rows)
  	{
      BOOST_FOREACH(Uint row_elem, row)
        m_array[next_array_idx++] = row_elem;
  	}
  }
  
  template<typename VectorT>
  void set_row(const Uint array_idx, const VectorT& row)
  {
    cf_assert(row.size() == row_size(array_idx));
    Uint start_idx = m_idx[array_idx];
    for(Uint j=0; j<row.size(); ++j)
      m_array[start_idx+j] = row[j];
  }
  
  Row operator[](const Uint idx)
  {
    return Row(&m_array[m_idx[idx]],row_size(idx));
  } 
  
  ConstRow operator[] (const Uint idx) const
  {
    return ConstRow(&m_array[m_idx[idx]],row_size(idx));
  } 
  
  
private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}
  
  
private: // data

  std::vector<Uint> m_array;
  std::vector<Uint> m_idx;
};

//std::ostream& operator<<(std::ostream& os, const CFlexTable::ConstRow& row);

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CFlexTable_hpp
