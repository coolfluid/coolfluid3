// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>

namespace cf3 {
namespace common { class Component; class Root; class SignalDispatcher; }
namespace Tools {
namespace Shell {

////////////////////////////////////////////////////////////////////////////////

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

  static boost::shared_ptr<common::Component> current_component;

  static boost::shared_ptr<common::Root> tree_root;

  static common::SignalDispatcher * dispatcher;
};

////////////////////////////////////////////////////////////////////////////////

} // Shell
} // Tools
} // cf3
