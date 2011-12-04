// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "SFDM/LibSFDM.hpp"
#include "common/Link.hpp"

namespace cf3 {
namespace SFDM {

using namespace common;
  
cf3::common::RegistLibrary<LibSFDM> LibSFDM;

void LibSFDM::initiate()
{
  if(m_is_initiated)
    return;
  
  Handle<Component> lib = Core::instance().libraries().get_child("cf3.SFDM");
  cf3_assert(lib);
  
  Core::instance().libraries().create_component<Link>("cf3.SFDM.P1")->link_to(*lib);
  Core::instance().libraries().create_component<Link>("cf3.SFDM.P2")->link_to(*lib);
  Core::instance().libraries().create_component<Link>("cf3.SFDM.P3")->link_to(*lib);
  Core::instance().libraries().create_component<Link>("cf3.SFDM.P4")->link_to(*lib);
  
  m_is_initiated = true;
}


} // SFDM
} // cf3
