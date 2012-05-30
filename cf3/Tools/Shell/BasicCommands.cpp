// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string.hpp>
#include <set>
#include <iomanip>

#include "common/Log.hpp"
#include "common/Signal.hpp"
#include "common/Core.hpp"
#include "common/BuildInfo.hpp"
#include "common/Factory.hpp"
#include "common/Builder.hpp"
#include "common/LocalDispatcher.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/XML/SignalOptions.hpp"

#include "common/Foreach.hpp"
#include "common/Action.hpp"
#include "common/FindComponents.hpp"

#include "common/Builder.hpp"
#include "solver/Time.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/Shell/Interpreter.hpp"

namespace cf3 {
namespace Tools {
namespace Shell {

  using namespace boost;
  using namespace boost::program_options;

  using namespace cf3::common;

SignalDispatcher * BasicCommands::dispatcher = new LocalDispatcher();

////////////////////////////////////////////////////////////////////////////////

BasicCommands::BasicCommands()
{
}

////////////////////////////////////////////////////////////////////////////////

Handle< Component > BasicCommands::tree_root = Core::instance().root().handle<Component>();

Handle< Component > BasicCommands::current_component = BasicCommands::tree_root;

Component& environment_component = *Core::instance().root().create_component<Component>("env_vars");

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
  ("find",        value< std::vector<std::string> >()->multitoken()->zero_tokens()->notifier(boost::bind(&find,_1)),"find components recursively in path")
  ("rm",          value< std::string >()->notifier(boost::bind(&rm,_1)),                                        "remove component")
  ("mv",          value< std::vector<std::string> >()->notifier(&mv)->multitoken(),                             "move/rename component")
  ("tree",        value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&tree,_1)),       "print tree")
  ("options",     value< std::string >()->implicit_value(std::string())->notifier(boost::bind(&option_list,_1)),"list options")
  ("call",        value< std::vector<std::string> >()->notifier(boost::bind(&call,_1))->multitoken(),           "call executable options")
  ("export",      value< std::vector<std::string> >()->notifier(boost::bind(&export_env,_1))->multitoken(),     "export a CF environment variable")
  ("echo",        value< std::vector<std::string> >()->notifier(boost::bind(&echo,_1))->multitoken(),     "print to screen")
  ;
  return desc;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::call(const std::vector<std::string>& params)
{
  if (params.size() == 0)
    throw SetupError(FromHere(),"executable name needed + possible options");

  std::string executable_path = params[0];

  if ( Handle< Component > executable = current_component->access_component(executable_path) )
    dispatcher->dispatch_empty_signal( "execute", executable->uri() );
  else
  {
    Handle< Component > signaling_component = current_component->access_component(URI(executable_path).base_path());
    if ( is_null(signaling_component) )
      throw ValueNotFound(FromHere(), "component " + URI(executable_path).base_path().path() + " was not found in " + current_component->uri().path());

    std::string name = URI(executable_path).name();
    std::vector<std::string> signal_options(params.size()-1);
    for (Uint i=0; i<signal_options.size(); ++i)
      signal_options[i] = params[i+1];

    // The following 3 lines are added/adapted to get default signature values
    SignalArgs signal_args;
    (*signaling_component->signal(name)->signature()) (signal_args);
    SignalOptions options(signal_args);

    options.set(signal_options);
    options.flush();


    XML::SignalFrame frame = options.create_frame(name, signaling_component->uri(), signaling_component->uri() );


    dispatcher->dispatch_signal( name, signaling_component->uri(), frame );
  }
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::unrecognized(std::vector<std::string>& unrecognized_commands)
{
  Uint idx(0);
  boost_foreach(const std::string& command, unrecognized_commands)
  {
    if ( Handle< Component > executable = current_component->access_component(command) )
    {
      if ( Handle< Action > action = executable->handle<Action>() )
      {
        action->execute();
        unrecognized_commands.erase(unrecognized_commands.begin()+idx);
      }
    }
    ++idx;
  }
}

////////////////////////////////////////////////////////////////////////////////

std::string env_var(const std::string& var)
{
  return environment_component.properties().value_str(var);
}

////////////////////////////////////////////////////////////////////////////////

std::string BasicCommands::filter_env_vars(const std::string &line)
{
  std::string filtered_line = line;
  // Check for environment variables
  boost::regex re("\\$\\{(\\w+)\\}");
  boost::sregex_iterator i(filtered_line.begin(), filtered_line.end(), re);
  boost::sregex_iterator j;
  for(; i!=j; ++i)
  {
    if (current_component->properties().check((*i)[1]) )
      boost::algorithm::replace_all(filtered_line,std::string((*i)[0]),current_component->properties().value_str((*i)[1]));

    if (environment_component.properties().check((*i)[1]) )
      boost::algorithm::replace_all(filtered_line,std::string((*i)[0]),environment_component.properties().value_str((*i)[1]));
  }
  return filtered_line;
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
  CFinfo << current_component->uri().path() << CFendl;
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
      Uint max_size(0);
      boost_foreach(Component& sub_comp, find_components(*current_component) )
        max_size = std::max(Uint(max_size),Uint(sub_comp.derived_type_name().size()));

      boost_foreach(Component& sub_comp, find_components(*current_component) )
      {
        if(sub_comp.has_tag(common::Tags::static_component()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.handle<Action>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << std::left << std::setw(max_size) << sub_comp.derived_type_name() <<  "    " << sub_comp.name() << CFendl;
      }
    }
    else if (arg == "s")
    {
      boost_foreach( SignalPtr sig, current_component->signal_list() )
      {
        if (hidden_signals.find(sig->name()) == hidden_signals.end())
        {
          if ( sig->is_hidden() == false )
            CFinfo << sig->name() << CFendl;
        }
      }
    }
    else if (arg == "a")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(*current_component) )
      {
        if(sub_comp.has_tag(common::Tags::static_component()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.handle<Action>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
      boost_foreach( SignalPtr sig, current_component->signal_list() )
      {
        if (hidden_signals.find(sig->name()) == hidden_signals.end())
        {
          if ( sig->is_hidden() == false )
          {
            if ( sig->is_read_only() )
              CFinfo << "r-s    " << sig->name() << CFendl;
            else
              CFinfo << "rws    " << sig->name() << CFendl;
          }
        }
      }
    }
    else
    {
      std::string cpath = params.back();
      if (!cpath.empty())
      {
        Handle<Component> parent = current_component->access_component(URI(cpath));
        boost_foreach(Component& sub_comp, find_components(*parent))
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
    Handle<Component> parent = current_component->access_component(URI(cpath));

    if (arg == "l")
    {
      // ls this_component
      Uint max_size(0);
      boost_foreach(Component& sub_comp, find_components(*parent) )
        max_size = std::max(Uint(max_size),Uint(sub_comp.derived_type_name().size()));

      boost_foreach(Component& sub_comp, find_components(*parent) )
      {
        if(sub_comp.has_tag(common::Tags::static_component()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.handle<Action>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << std::left << std::setw(max_size) << sub_comp.derived_type_name() <<  "    " << sub_comp.name() << CFendl;
      }
    }
    else if (arg == "s")
    {
      boost_foreach( SignalPtr sig, current_component->signal_list() )
      {
        if (hidden_signals.find(sig->name())==hidden_signals.end())
        {
          if ( sig->is_hidden() == false)
            CFinfo << sig->name() << CFendl;
        }
      }
    }
    else if (arg == "a")
    {
      // ls this_component
      boost_foreach(Component& sub_comp, find_components(*parent) )
      {
        if(sub_comp.has_tag(common::Tags::static_component()))
          CFinfo << "r-";
        else
          CFinfo << "rw";
        if ( is_null(sub_comp.handle<Action>()) )
          CFinfo << "-";
        else
          CFinfo << "x";
        CFinfo << "    " << sub_comp.name() << CFendl;
      }
      boost_foreach( SignalPtr sig, current_component->signal_list() )
      {
        if (hidden_signals.find(sig->name())==hidden_signals.end())
        {
          if ( ! sig->is_hidden() )
          {
            if ( sig->is_read_only() )
              CFinfo << "r-s    " << sig->name() << CFendl;
            else
              CFinfo << "rws    " << sig->name() << CFendl;
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
  Handle< Component > to_delete = current_component->access_component_checked(cpath);
  dispatcher->dispatch_empty_signal( "delete_component", to_delete->uri() );
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::cd(const std::string& cpath)
{
  if (cpath.empty())
    current_component = tree_root; //Core::instance().root().self();
  else
    current_component = current_component->access_component_checked(cpath);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::find(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command : find [path] [regex]");

  const std::string& path     = params[0];
  const std::string& regexstr = params[1];

  boost::regex expression ( regexstr );

  Handle<Component> start_comp = current_component->access_component( URI(path) );


  boost_foreach(Component& subcomp, find_components_recursively( *start_comp ) )
  {
    if ( boost::regex_match(subcomp.name(),expression) )
      CFinfo << subcomp.uri().path() << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::tree(const std::string& cpath)
{
  if (!cpath.empty())
    CFinfo << current_component->access_component(URI(cpath))->tree() << CFendl;
  else
    CFinfo << current_component->tree() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::option_list(const std::string& cpath)
{
  std::string option_list;
  if (!cpath.empty())
    option_list =
        current_component->access_component(URI(cpath))->options().list_options();
  else
    option_list = current_component->options().list_options();
  if (!option_list.empty())
  {
    CFinfo << option_list << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::configure(const std::vector<std::string>& params)
{
  if (params.size() == 0)
    throw SetupError(FromHere(),"configure needs a component path as first parameter");

  std::string path = params[0];

  if ( Handle< Component > comp = current_component->access_component(path) )
  {
    std::vector<std::string> conf_options(params.size()-1);
    for (Uint i=0; i<conf_options.size(); ++i)
       conf_options[i] = params[i+1];


    XML::SignalOptions options;

    options.set(conf_options);

    XML::SignalFrame frame = options.create_frame("configure", comp->uri(), comp->uri() );

    dispatcher->dispatch_signal( "configure", comp->uri(), frame );
  }
  else
    throw ValueNotFound(FromHere(), "No component found at " + path );
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::export_env(const std::vector<std::string>& params)
{
  if (params.size() != 1)
    throw SetupError(FromHere(),"export takes only 1 parameter:  var:type=value");

  /// @note (QG) this will not work from the GUI
  environment_component.properties().set(params[0]);
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::echo(const std::vector<std::string>& params)
{
  if (params.size() != 1)
    throw SetupError(FromHere(),"echo takes only 1 parameter");

  CFinfo << params[0] << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::version(const std::vector<std::string>&)
{
  CFinfo << Core::instance().build_info().version_header() << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::create(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command [create name builder]");
  const URI new_component_path (params[0]);

  Handle< Component > parent_component = current_component->access_component(URI(new_component_path).base_path());
  if ( is_null(parent_component) )
    throw ValueNotFound(FromHere(), "component " + new_component_path.base_path().path() + " was not found in " + current_component->uri().path());

  XML::SignalOptions options;

//  XML::SignalFrame frame("create_component", parent_component->uri(), parent_component->uri());
  //  XML::SignalOptions & options = frame.options();

  options.add( "name", new_component_path.name() );
  options.add( "type", params[1] );

  XML::SignalFrame frame = options.create_frame("create_component", parent_component->uri(), parent_component->uri());
  dispatcher->dispatch_signal( "create_component", parent_component->uri(), frame );
}

////////////////////////////////////////////////////////////////////////////////

void BasicCommands::mv(const std::vector<std::string>& params)
{
  if (params.size() != 2)
    throw SetupError(FromHere(),"2 parameters needed for command [mv cpath1 cpath2]");
  const URI cpath1(params[0]);
  const URI cpath2(params[1]);
  Handle<Component> component_1 = current_component->access_component(cpath1);
  Handle<Component> parent_2 = current_component->access_component(cpath2.base_path());

  XML::SignalOptions options;
//  XML::SignalFrame frame( "move_component", component_1.uri(), component_1.uri() );

  options.add( "path", parent_2->uri() );

  XML::SignalFrame frame = options.create_frame("move_component", component_1->uri(), component_1->uri() );

  dispatcher->dispatch_signal( "move_component", component_1->uri(), frame );

//  component_1.move_to(parent_2);
//  component_1.rename(cpath2.name());
}

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3
