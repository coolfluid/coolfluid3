// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Client_Core_NMeshReader_hpp
#define CF_GUI_Client_Core_NMeshReader_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/Core/CNode.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace ClientCore {

  //////////////////////////////////////////////////////////////////////////

  class ClientCore_API NMeshReader : public QObject, public CNode
  {
    Q_OBJECT

  public:

    typedef boost::shared_ptr<NMeshReader> Ptr;
    typedef boost::shared_ptr<NMeshReader const> ConstPtr;

    NMeshReader(const QString & name);

    /// @brief Gives the icon associated to this node
    /// @return Returns the icon associated to this node
    /// @note This method should be reimplemented by all subclasses.
    virtual QIcon getIcon() const;

    /// @brief Gives the text to put on a tool tip
    /// @return The name of the class.
    virtual QString getToolTip() const;

  private slots:

    void readMesh();

  private:

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  }; // class NMeshReader

  //////////////////////////////////////////////////////////////////////////

} // namespace ClientCore
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // NMESHREADER_HPP
