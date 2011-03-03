// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"
#include "Common/CAction.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Tools/CommandLineInterpreter/BasicCommands.hpp"
#include "Tools/CommandLineInterpreter/CommandLineInterpreter.hpp"

namespace CF {
namespace Tools {
namespace CommandLineInterpreter {

  using namespace boost;
  using namespace boost::program_options;

  using namespace CF::Common;

////////////////////////////////////////////////////////////////////////////////

BasicCommands::BasicCommands()
{	
}

////////////////////////////////////////////////////////////////////////////////

Component::Ptr BasicCommands::current_component = Core::instance().root();

BasicCommands::commands_description BasicCommands::description()
{
  commands_description desc("Basic Commands");
  desc.add_options()
  ("version,v",   value< std::vector<std::string> >()->zero_tokens()->notifier(boost::bind(&version,_1)),       "show version")
  ("exit,q",      value< std::vector<std::string> >()->zero_tokens()->notifier(boost::bind(&exit,_1)),          "exit program")
  ("pwd",         value< std::vector<std::string> >()->zero_tokens()->notifier(boost::bind(&pwd,_1)),           "print current component")
  ("configure",   value< std::vector<std::string> >()->notifier(&configure)->multitoken(),                      "configure options")
  ("create",      value< std::vector<std::string> >()->notifier(boost::bind(&make,_1))->multitoken(),           "create component_name builder_name")
  ("ls",          value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&ls,_1)),         "list subcomponents")
  ("cd",          value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&cd,_1)),         "change current_component")
  ("rm",          value< std::string >()->notifier(boost::bind(&rm,_1)),                                        "remove component")
  ("mv",          value< std::vector<std::string> >()->notifier(&mv)->multitoken(),                             "move/rename component")
  ("execute",     value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&execute,_1)),    "execute this component or given path")
  ("tree",        value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&tree,_1)),       "print tree")
  ("options",     value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&option_list,_1)),"list options")
  ("call",        value< std::vector<std::string> >()->notifier(boost::bind(&call,_1)),                         "call signal options")
  ;
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::call(const std::vector<std::string>& params)
{
  if (params.size() == 0)
    throw SetupError(FromHere(),"signal name needed + possible options");
  
  std::string name = params[0];
  
//  XmlNode node;
//  current_component->call_signal(name,node)
  
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::exit(const std::vector<std::string> &)
{
  Core::instance().terminate();
  ::exit(0);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::pwd(const std::vector<std::string> &)
{
  CFinfo << current_component->full_path().path() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::ls(const std::string& cpath)
{
  if (!cpath.empty())
  {
    boost_foreach(Component& sub_comp, find_components(current_component->access_component(URI(cpath))))
      CFinfo << sub_comp.name() << CFendl;
  }
  else
  {
    boost_foreach(Component& sub_comp, find_components(*current_component) )
      CFinfo << sub_comp.name() << CFendl;    
  }
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::rm(const std::string& cpath)
{
  Component::Ptr to_delete = current_component->access_component_ptr(URI(cpath));
  to_delete->parent()->remove_component(to_delete->name());
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::cd(const std::string& cpath)
{
  if (cpath.empty())
    current_component = Core::instance().root();
  else
    current_component = current_component->access_component_ptr(URI(cpath));
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::execute(const std::string& cpath)
{
  if (!cpath.empty())
    current_component->access_component(URI(cpath)).as_type<CAction>().execute();
  else
    current_component->as_type<CAction>().execute();
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::tree(const std::string& cpath)
{
  if (!cpath.empty())
    CFinfo << current_component->access_component(URI(cpath)).tree() << CFendl;
  else
    CFinfo << current_component->tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::option_list(const std::string& cpath)
{
  if (!cpath.empty())
    CFinfo << current_component->access_component(URI(cpath)).option_list() << CFendl;
  else
    CFinfo << current_component->option_list() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::configure(const std::vector<std::string>& params)
{
  current_component->configure(params);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::version(const std::vector<std::string>&)
{
  CFinfo << Core::instance().build_info()->version_header() << CFendl;  
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::make(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command [make name builder_path]");
  const std::string& name = params[0];
  const std::string& builder_cpath = params[1];
  Component::Ptr built_component = current_component->access_component(URI(builder_cpath)).follow()->as_type<CBuilder>().build(name);
  current_component->add_component(built_component);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::mv(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command [make cpath1 cpath2]");
  const URI cpath1(params[0]);
  const URI cpath2(params[1]);
  Component& component_1 = current_component->access_component(cpath1);
  Component& parent_2 = current_component->access_component(cpath2.base_path());
  component_1.move_to(parent_2.self());
  component_1.rename(cpath2.name());
}

////////////////////////////////////////////////////////////////////////////////

} // CommandLineInterpreter
} // Tools
} // CF
