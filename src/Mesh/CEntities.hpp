// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_CEntities_hpp
#define CF_Mesh_CEntities_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/EnumT.hpp"

#include "Math/MatrixTypes.hpp"
#include "Mesh/LibMesh.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Common { class CLink; class CGroup;}
namespace Mesh {

  template <typename T> class CList;
  class CNodes;
  class ElementType;
  class CSpace;

////////////////////////////////////////////////////////////////////////////////

/// CEntities component class
/// This class stores information about a set of elements of the same type
/// @author Willem Deconinck, Tiago Quintino, Bart Janssens
class Mesh_API CEntities : public Common::Component {

public: // typedefs

  typedef boost::shared_ptr<CEntities> Ptr;
  typedef boost::shared_ptr<CEntities const> ConstPtr;

  class Mesh_API MeshSpaces
  {
  public:

    /// Enumeration of the default created Spaces in CF
    /// @todo SPACE0 to be renamed to MESH_NODES, and decrease number by 1
    enum Type { INVALID=-1, SPACE0=0, MESH_NODES=1 };

    struct Mesh_API Convert : public Common::EnumT< MeshSpaces >
    {
      /// constructor where all the converting maps are built
      Convert();

      /// get the unique instance of the converter class
      static Convert& instance() { static Convert instance; return instance; }
    };

    static std::string to_str(Type type)         { return Convert::instance().to_str(type); }

    static Type to_enum(const std::string& type) { return Convert::instance().to_enum(type); }
  };

public: // functions

  /// Contructor
  /// @param name of the component
  CEntities ( const std::string& name );

  /// Initialize the CEntities using the given type
  virtual void initialize(const std::string& element_type_name);

  /// Initialize the CEntities using the given type, also setting the nodes in one go
  virtual void initialize(const std::string& element_type_name, CNodes& nodes);

  /// Set the nodes
  virtual void set_nodes(CNodes& nodes);

  /// Virtual destructor
  virtual ~CEntities();

  /// Get the class name
  static std::string type_name () { return "CEntities"; }

  /// set the element type
  void configure_element_type();

  /// return the elementType
  const ElementType& element_type() const;

  /// Mutable access to the nodes
  CNodes& nodes();

  /// Const access to the coordinates
  const CNodes& nodes() const;

  /// Mutable access to the list of nodes
  CList<Uint>& glb_idx() { return *m_global_numbering; }

  /// Const access to the list of nodes
  const CList<Uint>& glb_idx() const { return *m_global_numbering; }

  CList<Uint>& rank() { return *m_rank; }
  const CList<Uint>& rank() const { return *m_rank; }

  bool is_ghost(const Uint idx) const;

  /// return the number of elements
  virtual Uint size() const;

  static CList<Uint>& used_nodes(Component& parent, const bool rebuild=false);

  virtual CTable<Uint>::ConstRow get_nodes(const Uint elem_idx) const;

  CSpace& space (const Uint space_idx) { return *m_spaces[space_idx]; }

  const CSpace& space (const Uint space_idx) const;

  CSpace& space (const std::string& space_name) const;

  CSpace& create_space(const std::string& space_name, const std::string& shape_function_builder_name);

  bool exists_space(const Uint space_idx) const;

  bool exists_space(const std::string& space_name) const;

  Uint space_idx(const std::string& space_name) const;

  virtual RealMatrix get_coordinates(const Uint elem_idx) const;

  virtual void put_coordinates(RealMatrix& coordinates, const Uint elem_idx) const;

  void allocate_coordinates(RealMatrix& coords) const;

  void signal_create_space ( Common::SignalArgs& node );

  void signature_create_space ( Common::SignalArgs& node);

protected: // data

  boost::shared_ptr<ElementType> m_element_type;

  boost::shared_ptr<Common::CLink> m_nodes;

  boost::shared_ptr<CList<Uint> > m_global_numbering;

  std::vector<boost::shared_ptr<CSpace> > m_spaces;

  boost::shared_ptr<Common::CGroup> m_spaces_group;

  boost::shared_ptr<CList<Uint> > m_rank;

};

////////////////////////////////////////////////////////////////////////////////

class IsElementsVolume
{
public:
  IsElementsVolume () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

class IsElementsSurface
{
public:
  IsElementsSurface () {}

  bool operator()(const CEntities::Ptr& component);
  bool operator()(const CEntities& component);
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CEntities_hpp
