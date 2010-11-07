// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_Common_PECommPattern_hpp
#define CF_Common_PECommPattern_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostArray.hpp"

#include "Common/CF.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {

////////////////////////////////////////////////////////////////////////////////

/// Parallel Communication Pattern
/// @author Tamas Banyai

class Common_API PECommPattern {

public:

  /// constructor
  PECommPattern();

  /// constructor with settting up communication pattern
  /// @see setupCommPattern
  PECommPattern(std::vector<Uint> gid, std::vector<Uint> rank);

  /// destructor
  ~PECommPattern();

  /// build communication pattern
  /// beware: interprocess communication heavy
  /// usage: both lid and rank has the size of the local number of items
  /// gid is the global id on the process the item is updatable
  void setup(std::vector<Uint> gid, std::vector<Uint> rank);

  /// synchronize ghost items
  template<typename T> void sync(T vec) {}

  /// move elements accross processes
  template<typename T> void move(T vec, std::vector<Uint> proc) {}

private:

  /// storing updatable information
  std::vector<bool> m_updatable;

  /// this is the process-wise counter of sending communication pattern
  std::vector< int > m_sendCount;

  /// this is the map of sending communication pattern
  std::vector< int > m_sendMap;

  /// this is the process-wise counter of receiveing communication pattern
  std::vector< int > m_receiveCount;

  /// this is the map of receiveing communication pattern
  std::vector< int > m_receiveMap;

  /// flag telling if communication pattern is up-to-date
  bool m_isCommPatternPrepared;

};


////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Common_PECommPattern_hpp
