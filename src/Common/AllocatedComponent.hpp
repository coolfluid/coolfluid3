// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3.
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/// @file Component.hpp
/// @brief Holds the Component class, as well as the ComponentIterator class
///        plus some functions related to component creation

#ifndef CF_Common_AllocatedComponent_hpp
#define CF_Common_AllocatedComponent_hpp

////////////////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_base_of.hpp>

#include "Common/CF.hpp"
#include "Common/TypeInfo.hpp"

namespace CF {
namespace Common {

/// Wraps a component that has been allocated using Common::allocate_component.
/// This class implements derived_type_name, making it transparent to the portable typing system
/// and ensuring at compile time that components must be properly allocated
/// This class also provides some possibilities to wrap certain component functions, allowing transparent
/// addition of features like timing
/// This class should never be used directly, use the component interface instead
template<typename ComponentT>
class AllocatedComponent : public ComponentT
{
public:
  AllocatedComponent(const std::string& name) : ComponentT(name)
  {
  }

  /// Component::derived_type_name implementation
  std::string derived_type_name() const
  {
    return TypeInfo::instance().portable_types[ typeid(ComponentT).name() ];
  }
};

////////////////////////////////////////////////////////////////////////////////////////////

class CAction;
struct TimedActionImpl
{
  TimedActionImpl(CAction& action);
  
  void start_timing();
  void stop_timing();
  
  CAction& m_timed_component;
};

/// Alternative wrapper that adds timing functionality around the execute() function for CAction
template<typename ComponentT>
class TimedAction : public ComponentT
{
public:
  TimedAction(const std::string& name) : ComponentT(name), m_impl(*this)
  {
  }

  static const bool is_action = boost::is_base_of<CAction, ComponentT>::value;

  /// Component::derived_type_name implementation
  std::string derived_type_name() const
  {
    return TypeInfo::instance().portable_types[ typeid(ComponentT).name() ];
  }
  
  inline void execute()
  {
    m_impl.start_timing();
    ComponentT::execute();
    m_impl.stop_timing();
  }
  
  TimedActionImpl m_impl;
};

/// Helper struct to select the correct wrapper for a component
template<typename ComponentT>
struct SelectComponentWrapper
{
  typedef typename boost::mpl::if_
  <
    boost::is_base_of<CAction, ComponentT>,
    TimedAction<ComponentT>,
    AllocatedComponent<ComponentT>
  >::type type;
};

} // Common
} // CF

#endif // CF_Common_AllocatedComponent_hpp
