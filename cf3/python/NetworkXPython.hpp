// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

/**
 * @file NetworkXPython.hpp Header containing support for displying the component system with NetworkX graph exploration package.
 * @author Tamas Banyai
 * Inside coolfluid the code is standalone, but there is a few package required to work:
 * System-wide: graphviz graphviz-devel graphviz-python (see yum, apt-get, ...)
 * For python: graphviz pygraphviz pydot networkx.
 * Usage:
 *   import cf3-networkx *
 *   cf3-networkx-twopi()
**/

#ifndef CF3_Python_NetworkXPython_hpp
#define CF3_Python_NetworkXPython_hpp

#include "string"
#include "common/Component.hpp"


////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
  namespace python {

////////////////////////////////////////////////////////////////////////////////

class  NetworkXPython : public common::Component {

  public:

  /// Contructor
  /// @param name of the component
  NetworkXPython ( const std::string& name );

  /// Virtual destructor
  virtual ~NetworkXPython();

  /// Get the class name
  static std::string type_name () { return "NetworkXPython"; }

  /// signal for listing the tree
  void signal_print_component_graph( common::SignalArgs& args );
  void signal_get_component_graph( common::SignalArgs& args );
  void signature_print_component_graph( common::SignalArgs& args );

  /// writing to python's stream
  /// @warning max 1000 characters at a time!
  void print_to_python_stdout(std::string what);

  /// going recursively on the tree and append to string the command to add the nodes and edges to the graph
  void append_components_recursive(const Component &c, std::string &coll, int depth);

};

////////////////////////////////////////////////////////////////////////////////

  } // namespace python
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Python_NetworkXPython_hpp
