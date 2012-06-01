// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include<sstream>

#include "common/BasicExceptions.hpp"
#include "common/OptionFactory.hpp"

using namespace cf3::common;
using namespace cf3::common::XML;

///////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

///////////////////////////////////////////////////////////////////////////////

OptionFactory& OptionFactory::instance()
{
  static OptionFactory factory;
  return factory;
}

boost::shared_ptr< Option > OptionFactory::create_option ( const std::string& name, const std::string& type, const boost::any& default_value )
{
  boost::shared_ptr<OptionBuilder> builder = m_builders[type];
  if(is_null(builder))
    throw common::ValueNotFound(FromHere(), "No option builder for type " + type );

  return builder->create_option(name, default_value);
}

void OptionFactory::register_builder ( const std::string& type, const boost::shared_ptr< OptionBuilder >& builder )
{
  m_builders[type] = builder;
}


OptionFactory::OptionFactory()
{
}

RegisterOptionBuilder::RegisterOptionBuilder ( const std::string& type, OptionBuilder* builder )
{
  OptionFactory::instance().register_builder(type, boost::shared_ptr<OptionBuilder>(builder));
}


//////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

