// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CUnifiedData_hpp
#define CF_Mesh_CUnifiedData_hpp

#include <boost/tuple/tuple.hpp>

#include "Common/Foreach.hpp"
#include "Common/Component.hpp"

#include "Mesh/CList.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

////////////////////////////////////////////////////////////////////////////////

/// This class allows to access data spread over multiple components
/// with a continuous index
/// @pre the data components must be of the same type and must have
///      a member function "Uint size() const" defined.
template <typename DATA>
class Mesh_API CUnifiedData : public Common::Component
{
public:
    
  typedef boost::shared_ptr<CUnifiedData> Ptr;
  typedef boost::shared_ptr<CUnifiedData const> ConstPtr;
  
  typedef DATA data_type;
  
  typedef boost::tuple<boost::shared_ptr<data_type const>, Uint> const_data_location_type;
  typedef boost::tuple<boost::shared_ptr<data_type>, Uint>       data_location_type;
  
  /// Contructor
  /// @param name of the component
  CUnifiedData ( const std::string& name );

  /// Virtual destructor
  virtual ~CUnifiedData() {}

  /// Get the class name
  static std::string type_name () { return "CUnifiedData<"+data_type::type_name()+">"; }
  
  /// set the data given a range of components obtained for instance
  /// using the find_components_recursively<data_type>() function
  /// @param [in] range  The range of data components to be unified
  template <typename DataRangeT>
    void set_data(const DataRangeT& range );
  
  /// Get the component and local index in the component
  /// given a continuous index spanning multiple components
  /// @param [in] data_glb_idx continuous index covering multiple components
  /// @return boost::tuple<data_type::Ptr component, Uint idx_in_component> 
  data_location_type data_location(const Uint data_glb_idx);

  /// Get the const component and local index in the component
  /// given a continuous index spanning multiple components
  /// @param [in] data_glb_idx continuous index covering multiple components
  /// @return boost::tuple<data_type::ConstPtr component, Uint idx_in_component>   
  const_data_location_type data_location(const Uint data_glb_idx) const;
  
  /// Get the total number of data spanning multiple components
  /// @return the size
  Uint size() const;
  
  /// non-const access to the unified data components
  /// @return vector of data components
  std::vector<boost::shared_ptr<data_type> >& data_components();
  
  /// const access to the unified data components
  /// @return vector of data components
  const std::vector<boost::shared_ptr<data_type> >& data_components() const;
  
private: // data

  /// vector of components to view as continuous
  std::vector<boost::shared_ptr<data_type> > m_data_vector; 
  
  /// start index for each component in the continous view
  boost::shared_ptr<CList<Uint> > m_data_indices;
  
  /// total number of indices spanning all components
  Uint m_size;
  
}; // CUnifiedData

////////////////////////////////////////////////////////////////////////////////

template <typename DATA>
inline CUnifiedData<DATA>::CUnifiedData ( const std::string& name ) : Component(name)
{
  m_data_indices = create_static_component<CList<Uint> >("data_indices");
}

////////////////////////////////////////////////////////////////////////////////

/// set the data given a range of components obtained for instance
/// using the find_components_recursively<data_type>() function
/// @param [in] range  The range of data components to be unified
template <typename DATA>
template <typename DataRangeT>
inline void CUnifiedData<DATA>::set_data(const DataRangeT& range )
{
  m_data_vector = range_to_vector(range);
  m_data_indices->resize(m_data_vector.size()+1);
  Uint sum = 0;
  (*m_data_indices)[0] = sum;
  index_foreach(i,const typename CUnifiedData<DATA>::data_type& data_val, range)
  {
    sum += data_val.size();
    (*m_data_indices)[i+1] = sum;
  }
  m_size = sum;
}

////////////////////////////////////////////////////////////////////////////////

/// Get the component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::Ptr component, Uint idx_in_component> 
template <typename DATA>
inline typename CUnifiedData<DATA>::data_location_type CUnifiedData<DATA>::data_location(const Uint data_glb_idx)
{
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 - m_data_indices->array().begin();
  return boost::make_tuple(m_data_vector[data_vector_idx], data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the const component and local index in the component
/// given a continuous index spanning multiple components
/// @param [in] data_glb_idx continuous index covering multiple components
/// @return boost::tuple<data_type::ConstPtr component, Uint idx_in_component>
template <typename DATA>
inline typename CUnifiedData<DATA>::const_data_location_type CUnifiedData<DATA>::data_location(const Uint data_glb_idx) const
{
  const Uint data_vector_idx = std::upper_bound(m_data_indices->array().begin(), m_data_indices->array().end(), data_glb_idx) - 1 - m_data_indices->array().begin();
  return boost::make_tuple(m_data_vector[data_vector_idx].as_const_type<CUnifiedData<DATA>::data_type>(), data_glb_idx - m_data_indices->array()[data_vector_idx]);
}

////////////////////////////////////////////////////////////////////////////////

/// Get the total number of data spanning multiple components
/// @return the size
template <typename DATA>
inline Uint CUnifiedData<DATA>::size() const
{
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

/// non-const access to the unified data components
/// @return vector of data components
template <typename DATA>
inline std::vector<boost::shared_ptr<typename CUnifiedData<DATA>::data_type> >& CUnifiedData<DATA>::data_components()
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

/// const access to the unified data components
/// @return vector of data components
template <typename DATA>
inline const std::vector<boost::shared_ptr<typename CUnifiedData<DATA>::data_type> >& CUnifiedData<DATA>::data_components() const
{
  return m_data_vector;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CUnifiedData_hpp
