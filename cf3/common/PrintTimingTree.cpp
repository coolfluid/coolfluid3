// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/TimedComponent.hpp"

#include "PrintTimingTree.hpp"

namespace cf3 {
namespace common {

ComponentBuilder < PrintTimingTree, Action, LibCommon > PrintTimingTree_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

PrintTimingTree::PrintTimingTree(const std::string& name): Action(name)
{
  options().add("root", m_root)
    .description("Root component to time")
    .pretty_name("Root")
    .link_to(&m_root)
    .mark_basic();
}

void PrintTimingTree::execute()
{
  if(is_not_null(m_root))
    print_timing_tree(*m_root);
}



////////////////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3
