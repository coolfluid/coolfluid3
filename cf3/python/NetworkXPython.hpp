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
 *   show graph(uri,depth,tree,caption)
 *     uri: starting component
 *     depth: how many levels deep into subtree
 *     tree: set of: 'cosfl'
 *     caption: set of: 'cosfl'
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

  /// signal for listing the components tree
  void signal_get_detailed_info( common::SignalArgs& args );
  /// signature for listing the components tree
  void signature_get_detailed_info( common::SignalArgs& args );

  /// signal for listing the components tree
  void signal_get_component_graph( common::SignalArgs& args );
  /// signature for listing the components tree
  void signature_get_component_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_component_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_component_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the options
  void signal_get_option_graph( common::SignalArgs& args );
  /// signature for listing the options
  void signature_get_option_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_option_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_option_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the signals
  void signal_get_signal_graph( common::SignalArgs& args );
  /// signature for listing the signals
  void signature_get_signal_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_signal_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_signal_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the fields
  void signal_get_field_graph( common::SignalArgs& args );
  /// signature for listing the fields
  void signature_get_field_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_field_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_field_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the links
  void signal_get_link_graph( common::SignalArgs& args );
  /// signature for listing the links
  void signature_get_link_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_link_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth, std::string printroot);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_link_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the properties
  void signal_get_property_graph( common::SignalArgs& args );
  /// signature for listing the properties
  void signature_get_property_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_property_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_property_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  /// signal for listing the tags
  void signal_get_tag_graph( common::SignalArgs& args );
  /// signature for listing the tags
  void signature_get_tag_graph( common::SignalArgs& args );
  /// going recursively on the tree and append to string the command to add the nodes to the graph
  void append_tag_nodes_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);
  /// going recursively on the tree and append to string the command to add the edges to the graph
  void append_tag_edges_recursive(const Component &c, std::string &coll, const int depthlimit, const int depth);

  private:

  /// little helper function for mapping bool to python style string
  std::string pybool(const bool &opt)
  {
    if (opt) return std::string("True");
    return std::string("False");
  }
};

////////////////////////////////////////////////////////////////////////////////

  } // namespace python
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////

#endif // CF3_Python_NetworkXPython_hpp
