// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_Core_CNodeBuilders_hpp
#define CF_UI_Core_CNodeBuilders_hpp

#include <QMap>

#include "Common/CBuilder.hpp"

#include "UI/Core/CNode.hpp"
#include "UI/Core/LibCore.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace UI {
namespace Core {

////////////////////////////////////////////////////////////////////////////

class Core_API CNodeBuilders
{
public:

  static CNodeBuilders & instance();

  template<typename TYPE>
  void registerBuilder(const QString & componentType)
  {
    typedef typename Common::CBuilderT<CNode, TYPE> BuilderType;

    cf_assert( !m_builders.contains(componentType) );

    m_builders[componentType] = typename Common::CBuilderT<CNode, TYPE>::Ptr(
          new typename Common::CBuilderT<CNode, TYPE>(componentType.toStdString()) );
  }

  bool hasBuilder(const QString & componentType) const;

  CNode::Ptr buildCNode( const QString & componentType, const std::string & name ) const;

private:

  CNodeBuilders();

  ~CNodeBuilders();

  QMap<QString, Common::Component::Ptr> m_builders;

}; // CNodeBuilders

////////////////////////////////////////////////////////////////////////////

} // Core
} // UI
} // CF

////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_Core_CNodeBuilders_hpp
