// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_common_Journal_hpp
#define cf3_common_Journal_hpp

////////////////////////////////////////////////////////////////////////////////

#include "common/BoostFilesystem.hpp"
#include "common/Component.hpp"

#include "common/XML/Map.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace common {

namespace XML { class XmlDoc; }

////////////////////////////////////////////////////////////////////////////////

  /// Component that maintains a journal of signals.
  /// @author Quentin Gasper
  class Common_API Journal : public Component
  {
  public: // methods

    /// Contructor
    /// @param name of the journal
    /// @throw InvalidPath if @c name is not a valid component name.
    Journal (const std::string & name);

    /// Virtual destructor.
    virtual ~Journal();

    /// Creates a Journal and loads its contents from a file.
    /// @param name Journal name.
    /// @param file_path File with the journal contents.
    /// @return Returns the built journal.
    /// @throw FileSystemError if the file does not exist.
    /// @throw InvalidPath if @c name is not a valid component name.
    static boost::shared_ptr<Journal> create_from_file ( const std::string & name,
                                  const URI& file_path );

    /// Get the class name
    static std::string type_name() { return "Journal"; }

    /// Loads journal contents from a file.
    /// The journal is cleared before loading.
    /// @param file_path The file with the new journal contents.
    void load_journal_file ( const URI& file_path );

    /// Dumps the journal contents to a file.
    /// If the file already exists, it is overwritten.
    /// @param file_path The file to where the journal is saved.
    /// @throw FileSystemError if the file can not be open with write access.
    void dump_journal_to ( const URI& file_path ) const;

    /// Adds a signal to the journal.
    /// @param signal_node Signal to add.
    void add_signal ( const SignalArgs & signal_node );

    void execute_signals (const URI& filename);

    /// @name SIGNALS
    // @{

    void list_journal ( SignalArgs & node );

    void load_journal ( SignalArgs & node );

    void save_journal ( SignalArgs & node );

    // @} END SIGNALS

  private: // data

    /// The journal XML document.
    boost::shared_ptr<XML::XmlDoc> m_xmldoc;

    XML::Map m_info_node;

    XML::Map m_signals_map;

    /// The pointers to signals.
    std::vector<XML::XmlNode> m_signals;

    /// Existing child nodes under @c out are not deleted and its name is not
    /// modified.
    /// @param in Node to copy.
    /// @param out Node where the copy has to be appended to.
    /// @return Returns the added node.
    /// @todo This method should be removed when the new Xml layer is in place.
    XML::XmlNode copy_node(const XML::XmlNode & in, XML::XmlNode & out) const;

  }; // Journal

////////////////////////////////////////////////////////////////////////////////

} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif // cf3_common_Journal_hpp
