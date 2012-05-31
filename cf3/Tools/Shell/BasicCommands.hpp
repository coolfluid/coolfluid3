// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_Tools_Shell_BasicCommands_hpp
#define cf3_Tools_Shell_BasicCommands_hpp

#include <boost/program_options.hpp>

#include "common/Handle.hpp"

namespace cf3 {
namespace common { class Component; class SignalDispatcher; }
namespace Tools {
namespace Shell {

////////////////////////////////////////////////////////////////////////////////

/// @brief Defines basic set of commands in the coolfluid shell
///
/// Commands include cd, ls, mv, exit, ...
/// @author Willem Deconinck
class BasicCommands
{
public: // typedefs

  typedef boost::program_options::options_description commands_description;

public: // functions

  BasicCommands();

  static void exit(const std::vector<std::string> &);

  static void pwd(const std::vector<std::string> &);

  static void ls(const std::vector<std::string>& params);

  static void rm(const std::string& cpath);

  static void cd(const std::string& cpath);

  static void find(const std::vector<std::string>& params);

  static void tree(const std::string& cpath);

  static void option_list(const std::string& cpath);

  static void configure(const std::vector<std::string>& params);

  static void version(const std::vector<std::string>&);

  static void export_env(const std::vector<std::string>& params);

  static void create(const std::vector<std::string>& params);

  static void mv(const std::vector<std::string>& params);

  static void call(const std::vector<std::string>& params);

  static void echo(const std::vector<std::string>& params);

  static void unrecognized(std::vector<std::string>& unrecognized_options);

  static commands_description description();

  static std::string env_var(const std::string& var);

  static std::string filter_env_vars(const std::string& line);

public: // data

  static Handle<common::Component> current_component;

  static Handle<common::Component> tree_root;

  static common::SignalDispatcher * dispatcher;
};

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3

#endif // cf3_Tools_Shell_BasicCommands_hpp
