// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Mesh_Manipulations_hpp
#define cf3_Mesh_Manipulations_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/PE/Buffer.hpp"

#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CDynTable.hpp"

namespace cf3 {
namespace Mesh {

  class Geometry;
  class CElements;

  ////////////////////////////////////////////////////////////////////////////////

struct RemoveNodes
{
  RemoveNodes(Geometry& nodes);

  void operator() (const Uint idx);

  void flush();

  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Real>::Buffer      coordinates;
  CDynTable<Uint>::Buffer   connected_elements;
};

struct RemoveElements
{
  RemoveElements(CElements& elements);

  void operator() (const Uint idx);

  void flush();

  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Uint>::Buffer      connected_nodes;
};


struct PackUnpackElements: common::PE::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackElements(CElements& elements);

  PackUnpackElements& operator() (const Uint idx,const bool remove_after_pack = false);

  void remove(const Uint idx);

  virtual void pack(common::PE::Buffer& buf);

  virtual void unpack(common::PE::Buffer& buf);

  void flush();

  CElements& m_elements;
  Uint m_idx;
  bool m_remove_after_pack;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Uint>::Buffer      connected_nodes;
};


struct PackUnpackNodes: common::PE::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackNodes(Geometry& nodes);

  PackUnpackNodes& operator() (const Uint idx,const bool remove_after_pack = false);

  void remove(const Uint idx);

  virtual void pack(common::PE::Buffer& buf);

  virtual void unpack(common::PE::Buffer& buf);

  void flush();

  Geometry& m_nodes;
  Uint m_idx;
  bool m_remove_after_pack;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Real>::Buffer      coordinates;
  CDynTable<Uint>::Buffer   connected_elements;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Mesh_Manipulations_hpp
