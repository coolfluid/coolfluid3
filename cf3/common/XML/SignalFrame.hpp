// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_common_XML_SignalFrame_hpp
#define cf3_common_XML_SignalFrame_hpp

////////////////////////////////////////////////////////////////////////////

#include "common/URI.hpp"

#include "common/XML/Map.hpp"
#include "common/XML/XmlDoc.hpp"
#include "common/XML/SignalOptions.hpp"

////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

////////////////////////////////////////////////////////////////////////////

/// Manages a set of maps.

/// @author Quentin Gasper.

class Common_API SignalFrame
{
public:

  /// Contructor.
  /// @param xml The node to manage.
  SignalFrame ( XmlNode xml = XmlNode() );

  /// Constructor
  /// @param doc The document the frame is based on.
  SignalFrame ( boost::shared_ptr<XmlDoc> doc );

  /// Contructor.
  /// Builds a new XML document and adds frame node to it.
  /// @param target The signal name.
  /// @param sender The sender CPATH.
  /// @param receiver The receiver CPATH
  SignalFrame ( const std::string & target, const URI & sender, const URI & receiver);

  /// Destructor.
  ~SignalFrame ();

  /// Sets an option to the main map.
  /// @c #Map::set_single_value() is used to acheive this.
  /// @param name The option name.
  /// @param value The option value.
  /// @see Map::set_value()
  XmlNode set_option ( const std::string& value_key, const std::string type_name, const std::string& value_str,
                       const std::string& descr = std::string());

  /// Sets an option to the main map.
  /// @c #Map::set_array_value() is used to acheive this.
  /// @param name The option name.
  /// @param value The option value.
  /// @param delimiter The delimiter string.
  /// @see Map::set_value()
  XmlNode set_array ( const std::string& value_key, const std::string element_type_name, const std::string& value_str, const std::string& delimiter,
                      const std::string& descr = std::string());

  /// Gets an option from the main map.
  /// @param name The option name.
  /// @return Returns the option value.
  template<typename TYPE>
  TYPE get_option ( const std::string & name ) const;

  /// Gets an array from the main map.
  /// @param name The array name.
  /// @return Returns the array value.
  template<typename TYPE>
  std::vector<TYPE> get_array ( const std::string & name ) const;

  /// Checks wether a frame the a specified name exists.
  /// @param name The frame name
  /// @return Returns @c true if the frame exists, otherwise, returns @c false.
  bool has_map ( const std::string & name ) const;

  /// Checks if an option or array exists with specified name
  /// @param name the name of the option
  bool has_entry ( const std::string& name ) const;

  /// Checks whether a reply for this present as sibling node of this signal.
  /// @return Returns @c true if a reply has been found; otherwise, returns
  /// @c false.
  bool has_reply () const;

  /// Returns a sub-frame with a specified name.
  /// If the frame does not exist yet, it is created.
  /// @param name The frame name.
  /// @return Returns the frame.
  SignalFrame & map ( const std::string & name );

  /// Returns a sub-frame with a specified name.
  /// The frame must exist.
  /// @param name The frame name.
  /// @return Returns the frame.
  const SignalFrame & map ( const std::string & name ) const;

  /// Creates a reply to this signal.
  /// The reply is created as a sibling node and has the same parent as this
  /// frame node.
  /// @param sender The sender CPATH. If empty, or not a CPATH, the receiver
  /// of this frame node is used.
  /// @return Returns the created frame.
  /// @throw XmlError If the sender argument is not valid and the receiver
  /// could not be found in the frame node.
  SignalFrame create_reply (const URI & sender = URI() );

  /// Retrieves the reply to this signal.
  /// @return Returns the reply frame. If no reply was found, the node
  /// of the reply returned is not valid.
  SignalFrame get_reply () const;

  /// Inserts the options, translating them from strings
  void insert( std::vector<std::string>& input );

  /// Converts this signal to a string in the CF script format
  std::string to_script( int indentation = 0 ) const;

  /// Converts this signal to a string in the CF python script format
  std::string to_python_script( int indentation = 0 ) const;

  /// Flushes internal @c SignalOptions maps.
  void flush_maps();

public: // data

  /// The frame node
  XmlNode node;

  /// The main map.
  Map main_map;

  /// The XML document. This pointer is not null only if a document was
  /// created by this class.
  boost::shared_ptr<XmlDoc> xml_doc;

  SignalOptions & options( const std::string & name = std::string() );

  const SignalOptions & options( const std::string & name = std::string() ) const;

private: // data

  /// Maps contained in this frame.
  std::map<std::string, SignalFrame> m_maps;

  std::map<std::string, SignalOptions> m_options;

}; // SignalFrame

/////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3

#endif // cf3_common_XML_SignalFrame_hpp
