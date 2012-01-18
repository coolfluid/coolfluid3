// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_core_CNodeBuilders_hpp
#define cf3_ui_core_CNodeBuilders_hpp

#include <QMap>

#include "common/Builder.hpp"

#include "ui/core/CNode.hpp"
#include "ui/core/LibCore.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace ui {
namespace core {

////////////////////////////////////////////////////////////////////////////

class Core_API CNodeBuilders
{
public:

  static CNodeBuilders & instance();

  template<typename TYPE>
  void register_builder(const QString & component_type)
  {
    typedef typename common::BuilderT<CNode, TYPE> BuilderType;

    cf3_assert( !m_builders.contains(component_type) );

    m_builders[component_type] = common::allocate_component< typename common::BuilderT<CNode, TYPE> >( component_type.toStdString() );
  }

  bool has_builder(const QString & component_type) const;

  boost::shared_ptr< CNode > build_cnode( const QString & component_type, const std::string & name ) const;

private:

  CNodeBuilders();

  ~CNodeBuilders();

  QMap<QString, boost::shared_ptr< common::Component > > m_builders;

}; // CNodeBuilders

////////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_core_CNodeBuilders_hpp
