// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "UI/Core/CNodeBuilders.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

CNodeBuilders::CNodeBuilders()
{

}

////////////////////////////////////////////////////////////////////////////

CNodeBuilders::~CNodeBuilders()
{

}

////////////////////////////////////////////////////////////////////////////

CNodeBuilders & CNodeBuilders::instance()
{
  static CNodeBuilders inst;
  return inst;
}

////////////////////////////////////////////////////////////////////////////

bool CNodeBuilders::hasBuilder(const QString & componentType) const
{
  return m_builders.contains( componentType );
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr CNodeBuilders::buildCNode( const QString & componentType,
                                      const std::string & name ) const
{
  cf3_assert( m_builders.contains( componentType ) );

  Builder & builder = m_builders[componentType]->as_type<Builder>();

  return builder.build( name )->as_ptr_checked<CNode>();
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // cf3
