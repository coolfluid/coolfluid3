// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef CF_Common_XML_SignalFrame_hpp
#define CF_Common_XML_SignalFrame_hpp

////////////////////////////////////////////////////////////////////////////

#include "Common/URI.hpp"

#include "Common/XML/Map.hpp"
#include "Common/XML/XmlDoc.hpp"

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {
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

  /// Contructor.
  /// Builds a new XML document and adds frame node to it.
  /// @param target The signal name.
  /// @param sender The sender CPATH.
  /// @param receiver The receiver CPATH
  SignalFrame ( const std::string & target, const URI & sender, const URI & receiver );

  /// Destructor.
  ~SignalFrame ();

  /// Sets an option to the main map.
  /// @c #Map::set_single_value() is used to acheive this.
  /// @param name The option name.
  /// @param value The option value.
  /// @see Map::set_value()
  template<typename TYPE>
  XmlNode set_option ( const std::string & name, const TYPE & value );

  /// Sets an option to the main map.
  /// @c #Map::set_array_value() is used to acheive this.
  /// @param name The option name.
  /// @param value The option value.
  /// @param delimiter The delimiter string.
  /// @see Map::set_value()
  template<typename TYPE>
  XmlNode set_array ( const std::string & name, const std::vector<TYPE> & value,
                   const std::string & delimiter );

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

public: // data

  /// The frame node
  XmlNode node;

  /// The main map.
  Map main_map;

  /// The XML document. This pointer is not null only if a document was
  /// created by this class.
  boost::shared_ptr<XmlDoc> xml_doc;

private: // data

  /// Maps contained in this frame.
  std::map<std::string, SignalFrame> m_maps;

}; // SignalFrame

/////////////////////////////////////////////////////////////////////////////

} // XML
} // Common
} // CF

#endif // CF_Common_XML_SignalFrame_hpp
