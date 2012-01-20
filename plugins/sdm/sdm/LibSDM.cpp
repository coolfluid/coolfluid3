// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/RegistLibrary.hpp"

#include "sdm/LibSDM.hpp"
#include "common/Link.hpp"

namespace cf3 {
namespace sdm {

using namespace common;

cf3::common::RegistLibrary<LibSDM> LibSDM;

void LibSDM::initiate()
{
  if(m_is_initiated)
    return;

  Handle<Component> lib = Core::instance().libraries().get_child("cf3.sdm");
  cf3_assert(lib);

  m_is_initiated = true;
}


} // sdm
} // cf3
