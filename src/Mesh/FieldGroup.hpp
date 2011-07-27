// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_FieldGroup_hpp
#define CF_Mesh_FieldGroup_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"

#include "Mesh/CNodes.hpp"

namespace CF {
namespace Common
{
  class CLink;
}
namespace Mesh {

  class CRegion;
  template <typename T> class CList;

////////////////////////////////////////////////////////////////////////////////

/// Component that holds CFields of the same type (topology and space)
/// @author Willem Deconinck
/// @todo dont make FieldGroup inherit from CNodes
class Mesh_API FieldGroup : public CNodes {

public: // typedefs

  typedef boost::shared_ptr<FieldGroup> Ptr;
  typedef boost::shared_ptr<FieldGroup const> ConstPtr;

  class Mesh_API Basis
  {
  public:

    /// Enumeration of the Shapes recognized in CF
    enum Type { INVALID=-1, POINT_BASED=0,  ELEMENT_BASED=1, CELL_BASED=2, FACE_BASED=3 };

    typedef Common::EnumT< Basis > ConverterBase;

    struct Mesh_API Convert : public ConverterBase
    {
      /// constructor where all the converting maps are built
      Convert();
      /// get the unique instance of the converter class
      static Convert& instance();
    };

    static std::string to_str(Type type)
    {
      return Convert::instance().to_str(type);
    }

    static Type to_enum(const std::string& type)
    {
      return Convert::instance().to_enum(type);
    }

  };

public: // functions

  /// Contructor
  /// @param name of the component
  FieldGroup ( const std::string& name );

  /// Virtual destructor
  virtual ~FieldGroup();

  /// Get the class name
  static std::string type_name () { return "FieldGroup"; }

  const CRegion& topology() const;

  CRegion& topology();

//  Uint size() const { return m_size; }

//  void resize(const Uint size);

  const std::string& space() const { return m_space; }

//  const CList<Uint>& glb_idx() const { return *m_glb_idx; }

//  CList<Uint>& glb_idx() { return *m_glb_idx; }

//  const CList<Uint>& rank() const { return *m_rank; }

//  CList<Uint>& rank() { return *m_rank; }

private: // functions

  void config_topology();

  void config_type();

protected:

  Basis::Type m_basis;

  std::string m_space;

//  Uint m_size;

  boost::shared_ptr<Common::CLink> m_topology;

//  boost::shared_ptr<CList<Uint> > m_glb_idx;

//  boost::shared_ptr<CList<Uint> > m_rank;

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_FieldGroup_hpp
