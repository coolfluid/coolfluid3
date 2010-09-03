#ifndef CF_GUI_Client_NMeshReader_hpp
#define CF_GUI_Client_NMeshReader_hpp

////////////////////////////////////////////////////////////////////////////

#include <QObject>

#include "GUI/Client/CNode.hpp"

class QString;

////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace GUI {
namespace Client {

  //////////////////////////////////////////////////////////////////////////

  class RemoteOpenFile;

  class NMeshReader : public QObject, public CNode
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

    boost::shared_ptr<RemoteOpenFile> m_openFile;

    /// regists all the signals declared in this class
    static void regist_signals ( Component* self ) {}

  }; // class NMeshReader

  //////////////////////////////////////////////////////////////////////////

} // namespace Client
} // namespace GUI
} // namespace CF

////////////////////////////////////////////////////////////////////////////

#endif // NMESHREADER_HPP
