// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Mesh_Manipulations_hpp
#define CF_Mesh_Manipulations_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/MPI/Buffer.hpp"

#include "Mesh/CList.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CDynTable.hpp"

namespace CF {
namespace Mesh {

  class CNodes;
  class CElements;

  ////////////////////////////////////////////////////////////////////////////////

struct RemoveNodes
{
  RemoveNodes(CNodes& nodes);

  void operator() (const Uint idx);

  void flush();

  CList<bool>::Buffer       is_ghost;
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


struct PackUnpackElements: Common::mpi::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackElements(CElements& elements, bool remove_after_pack=false);

  PackUnpackElements& operator() (const Uint idx);

  virtual void pack(Common::mpi::Buffer& buf);

  virtual void unpack(Common::mpi::Buffer& buf);

  void flush();

  CElements& m_elements;
  Uint m_idx;
  bool m_remove_after_pack;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Uint>::Buffer      connected_nodes;
};


struct PackUnpackNodes: Common::mpi::PackedObject
{
  enum CommunicationType {COPY=0, MIGRATE=1};

  PackUnpackNodes(CNodes& nodes, const bool remove_after_pack = 0);

  PackUnpackNodes& operator() (const Uint idx);

  virtual void pack(Common::mpi::Buffer& buf);

  virtual void unpack(Common::mpi::Buffer& buf);

  void flush();

  CNodes& m_nodes;
  Uint m_idx;
  bool m_remove_after_pack;
  CList<bool>::Buffer       is_ghost;
  CList<Uint>::Buffer       glb_idx;
  CList<Uint>::Buffer       rank;
  CTable<Real>::Buffer      coordinates;
  CDynTable<Uint>::Buffer   connected_elements;
};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Manipulations_hpp
