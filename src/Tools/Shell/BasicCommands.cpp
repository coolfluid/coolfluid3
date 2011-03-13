// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <set>

#include "Common/Log.hpp"
#include "Common/Signal.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/Foreach.hpp"
#include "Common/CAction.hpp"
#include "Common/FindComponents.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/Shell/Interpreter.hpp"

namespace CF {
namespace Tools {
namespace Shell {

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
  ("create",      value< std::vector<std::string> >()->notifier(boost::bind(&create,_1))->multitoken(),           "create component_name builder_name")
  ("ls",          value< std::vector<std::string> >()->multitoken()->zero_tokens()->notifier(boost::bind(&ls,_1)),"list subcomponents")
  ("cd",          value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&cd,_1)),         "change current_component")
  ("find",        value< std::vector<std::string> >()->multitoken()->zero_tokens()->notifier(boost::bind(&find,_1)),"find components recursevely in path")
  ("rm",          value< std::string >()->notifier(boost::bind(&rm,_1)),                                        "remove component")
  ("mv",          value< std::vector<std::string> >()->notifier(&mv)->multitoken(),                             "move/rename component")
  ("tree",        value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&tree,_1)),       "print tree")
  ("options",     value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&option_list,_1)),"list options")
  ("call",        value< std::vector<std::string> >()->notifier(boost::bind(&call,_1))->multitoken(),           "call executable options")
  ;
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::call(const std::vector<std::string>& params)
{
  if (params.size() == 0)
    throw SetupError(FromHere(),"executable name needed + possible options");

  std::string executable_path = params[0];
  
  if ( Component::Ptr executable = current_component->access_component_ptr(executable_path) )
  {
    if ( CAction::Ptr action = executable->as_ptr<CAction>() )
      action->execute();
    else
      throw ValueNotFound (FromHere(), executable_path + " is not an executable." );
  }
  else
  {
    Component::Ptr signaling_component = current_component->access_component_ptr(URI(executable_path).base_path());
    if ( is_null(signaling_component) )
      throw ValueNotFound(FromHere(), "component " + URI(executable_path).base_path().path() + " was not found in " + current_component->full_path().path());
    std::string name = URI(executable_path).name();
    std::vector<std::string> signal_options(params.size()-1);
    for (Uint i=0; i<signal_options.size(); ++i)
      signal_options[i] = params[i+1];

    signaling_component->call_signal(name,signal_options);
    
  }
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::unrecognized(std::vector<std::string>& unrecognized_commands)
{
  Uint idx(0);
  boost_foreach(const std::string& command, unrecognized_commands)
  {
    if ( Component::Ptr executable = current_component->access_component_ptr(command) )
    {
      if ( CAction::Ptr action = executable->as_ptr<CAction>() )
      {
        action->execute();
        unrecognized_commands.erase(unrecognized_commands.begin()+idx);
      }
    }
    ++idx;
  }
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

void BasicCommands::ls(const std::vector<std::string>& params)
{
  std::set<std::string> hidden_signals;
  hidden_signals.insert("create_component");
  hidden_signals.insert("list_tree");
  hidden_signals.insert("list_properties");
  hidden_signals.insert("list_signals");
  hidden_signals.insert("configure");
  hidden_signals.insert("print_info");
  hidden_signals.insert("rename_component");
  hidden_signals.insert("delete_component");
  hidden_signals.insert("move_component");
  hidden_signals.insert("save_tree");
  hidden_signals.insert("list_content");
  hidden_signals.insert("create_component");
  hidden_signals.insert("create_component");
  hidden_signals.insert("signal_signature");
  if (params.size() == 0)
  {
    // ls this_component
    boost_foreach(Component& sub_comp, find_components(*current_component) )
    {
      CFinfo << sub_comp.name() << CFendl;
    }
  }
  else if (params.size() == 1)
  {
    // ls x  or  ls path
    std::string arg = params[0];
    if (arg == "l")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(*current_component) )
      {
        if (current_component->is_child_static(sub_comp.name()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.as_ptr<CAction>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
    }
    else if (arg == "s")
    {
      foreach_container((const std::string& sig_name) (SignalPtr sig), current_component->signals_map())
      {
        if (hidden_signals.find(sig_name)==hidden_signals.end())
        {
          if ( sig->is_hidden == false)
            CFinfo << sig_name << CFendl;
        }
      }
    }
    else if (arg == "a")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(*current_component) )
      {
        if (current_component->is_child_static(sub_comp.name()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.as_ptr<CAction>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
      foreach_container((const std::string& sig_name) (SignalPtr sig), current_component->signals_map())
      {
        if (hidden_signals.find(sig_name)==hidden_signals.end())
        {
          if ( ! sig->is_hidden )
          {
            if ( sig->is_read_only )
              CFinfo << "r-s    " << sig_name << CFendl;
            else
              CFinfo << "rws    " << sig_name << CFendl;
          }
        }
      }
    }
    else
    {
      std::string cpath = params.back();
      if (!cpath.empty())
      {
        Component& parent = current_component->access_component(URI(cpath));
        boost_foreach(Component& sub_comp, find_components(parent))
        {
          CFinfo << sub_comp.name() << CFendl;
        }
      }
    }
  }
  else if (params.size() == 2)
  {
    // ls -x path
    std::string arg = params[0];
    std::string cpath = params.back();
    Component& parent = current_component->access_component(URI(cpath));
    
    if (arg == "l")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(parent) )
      {
        if (parent.is_child_static(sub_comp.name()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.as_ptr<CAction>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
    }
    else if (arg == "s")
    {
      foreach_container((const std::string& sig_name) (SignalPtr sig), parent.signals_map())
      {
        if (hidden_signals.find(sig_name)==hidden_signals.end())
        {
          if ( sig->is_hidden == false)
            CFinfo << sig_name << CFendl;
        }
      }
    }
    else if (arg == "a")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(parent) )
      {
        if (parent.is_child_static(sub_comp.name()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.as_ptr<CAction>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
      foreach_container((const std::string& sig_name) (SignalPtr sig), parent.signals_map())
      {
        if (hidden_signals.find(sig_name)==hidden_signals.end())
        {
          if ( ! sig->is_hidden )
          {
            if ( sig->is_read_only )
              CFinfo << "r-s    " << sig_name << CFendl;
            else
              CFinfo << "rws    " << sig_name << CFendl;
          }
        }
      }
    }
    else
    {
      throw ParsingFailed(FromHere(),"unrecognized ls option "+arg);
    }
  }
  else 
    throw ParsingFailed (FromHere(), "More than 2 arguments for ls not supported");
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::rm(const std::string& cpath)
{
  Component::Ptr to_delete = current_component->access_component_ptr_checked(cpath);
  to_delete->parent()->remove_component(to_delete->name());
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::cd(const std::string& cpath)
{
  if (cpath.empty())
    current_component = Core::instance().root();
  else
    current_component = current_component->access_component_ptr_checked(cpath);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::find(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command : find [path] [regex]");

  const std::string& path     = params[0];
  const std::string& regexstr = params[1];

  boost::regex expression ( regexstr );

  Component& start_comp = current_component->access_component( URI(path) );


  boost_foreach(Component& subcomp, find_components_recursively( start_comp ) )
  {
    if ( boost::regex_match(subcomp.name(),expression) )
      CFinfo << subcomp.full_path().path() << CFendl;
  }
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
  std::string option_list;
  if (!cpath.empty())
    option_list = current_component->access_component(URI(cpath)).option_list();
  else
    option_list = current_component->option_list();
  if (!option_list.empty())
  {
    CFinfo << option_list << CFendl;
  }
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

void BasicCommands::create(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command [create name builder]");
  const std::string& name = params[0];
  
  using namespace boost::algorithm;
  std::string builder = params[1];
  std::string builder_name_space (builder.begin(),find_last(builder,".").begin());
  
  Component::Ptr built_component = Core::instance().root()->access_component(URI("cpath://Root/Libraries/"+builder_name_space+"/"+builder)).follow()->as_type<CBuilder>().build(name);
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

} // Shell
} // Tools
} // CF
