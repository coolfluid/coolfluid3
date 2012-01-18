// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_mesh_Manipulations_hpp
#define cf3_mesh_Manipulations_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/PE/Buffer.hpp"

#include "common/List.hpp"
#include "common/Table.hpp"
#include "common/DynTable.hpp"

namespace cf3 {
namespace mesh {

  class SpaceFields;
  
  class Elements;

  ////////////////////////////////////////////////////////////////////////////////

struct RemoveNodes
{
  RemoveNodes(SpaceFields& nodes);

  void operator() (const Uint idx);

  void flush();

  common::List<Uint>::Buffer       glb_idx;
  common::List<Uint>::Buffer       rank;
  common::Table<Real>::Buffer      coordinates;
  common::DynTable<Uint>::Buffer   connected_elements;
};

struct RemoveElements
{
  RemoveElements(Elements& elements);

  void operator() (const Uint idx);

  void flush();

  common::List<Uint>::Buffer       glb_idx;
  common::List<Uint>::Buffer       rank;
  common::Table<Uint>::Buffer      connected_nodes;
};


struct PackUnpackElements: common::PE::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackElements(Elements& elements);

  PackUnpackElements& operator() (const Uint idx,const bool remove_after_pack = false);

  void remove(const Uint idx);

  virtual void pack(common::PE::Buffer& buf);

  virtual void unpack(common::PE::Buffer& buf);

  void flush();

  Elements& m_elements;
  Uint m_idx;
  bool m_remove_after_pack;
  common::List<Uint>::Buffer       glb_idx;
  common::List<Uint>::Buffer       rank;
  common::Table<Uint>::Buffer      connected_nodes;
};


struct PackUnpackNodes: common::PE::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackNodes(SpaceFields& nodes);

  PackUnpackNodes& operator() (const Uint idx,const bool remove_after_pack = false);

  void remove(const Uint idx);

  virtual void pack(common::PE::Buffer& buf);

  virtual void unpack(common::PE::Buffer& buf);

  void flush();

  SpaceFields& m_nodes;
  Uint m_idx;
  bool m_remove_after_pack;
  common::List<Uint>::Buffer       glb_idx;
  common::List<Uint>::Buffer       rank;
  common::Table<Real>::Buffer      coordinates;
  common::DynTable<Uint>::Buffer   connected_elements;
};

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_mesh_Manipulations_hpp
