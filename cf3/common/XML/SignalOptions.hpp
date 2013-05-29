// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_XML_SignalOptions_hpp
#define cf3_common_XML_SignalOptions_hpp

//////////////////////////////////////////////////////////////////////////////

#include "common/OptionList.hpp"
#include "common/URI.hpp"

#include "common/XML/Map.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {
namespace XML {

//////////////////////////////////////////////////////////////////////////////

class SignalFrame;

/// Abstracts the use of XML when adding options to a signal frame.

/// @author Quentin Gasper.

class Common_API SignalOptions : public OptionList
{

public:

  /// Default constructor.

  /// No map is managed when using this constructor.
  SignalOptions();

  /// Constructor.

  /// Parses the XML data to options.
  /// @param frame The frame to parse. Must be valid.
  SignalOptions( SignalFrame & frame, const std::string & name = std::string() );

  /// Constructor.
  /// Creates a copy of the provided list.
  /// @param list List to copy
  SignalOptions( const OptionList & list );

  /// Desctructor.
  /// Flushes the options.
  ~SignalOptions();

  /// Creates a frame and puts the options in an "options" map.

  /// This method does not need a valid managed map. It is ignored if it exists.
  /// @param name Frame name. Can be empty.
  /// @param sender Sender path. Can be empty.
  /// @param receiver Receiver path. Can be empty.
  /// @return Returns the created frame.
  SignalFrame create_frame( const std::string & name = std::string(),
                            const URI & sender = URI(),
                            const URI & receiver = URI() ) const;

  /// Creates a reply to a specified frame and puts options to it.
  /// This method does not need a valid managed map. It is ignored if it exists.
  /// @param frame The frame to add the reply to. Must be valid.
  /// @param sender The reply sender. Can be empty.
  /// @return Returns the created frame.
  SignalFrame create_reply_to( SignalFrame & frame, const URI & sender = URI() ) const;

  /// Converts an XML node to an option (if possible).

  /// This function handles all kind of options supported by COOLFluiD :
  /// @li single values or arrays
  /// @li advanced or basic options
  /// @li with or without a restricted list of values
  /// @param node The node to convert. Must be valid.
  /// @return Returns the converted.
  /// @throw ProtocolError if the "key" attribute is missing
  /// @throw CastingFailed if the value could not be cast to the destination type
  /// @throw ShouldNotBeHere if the option type is not recognized
  static boost::shared_ptr<Option> xml_to_option( const XmlNode & node );

  /// Adds options to a provided map.
  /// @param map Map options will be added to. Must be valid.
  static void add_to_map( Map & main_map, const OptionList & list ) ;

  /// Writes option to the managed map.
  /// If the managed map is not valid, nothing is done.
  void flush();

public: // data

  /// The managed map
  Map main_map;

}; // SignalOptionList

//////////////////////////////////////////////////////////////////////////////

} // XML
} // common
} // cf3
//////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_XML_SignalOptions_hpp
