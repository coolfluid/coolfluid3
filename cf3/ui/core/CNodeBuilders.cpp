// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "ui/core/CNodeBuilders.hpp"

using namespace cf3::common;

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

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

bool CNodeBuilders::has_builder(const QString & component_type) const
{
  return m_builders.contains( component_type );
}

////////////////////////////////////////////////////////////////////////////

CNode::Ptr CNodeBuilders::build_cnode( const QString & component_type,
                                       const std::string & name ) const
{
  cf3_assert( m_builders.contains( component_type ) );

  Builder & builder = m_builders[component_type]->as_type<Builder>();

  return builder.build( name )->as_ptr_checked<CNode>();
}

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3
