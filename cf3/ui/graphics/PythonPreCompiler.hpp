// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_graphics_PythonPreCompiler_hpp
#define cf3_ui_graphics_PythonPreCompiler_hpp

//////////////////////////////////////////////////////////////////////////////

#include "ui/core/CNode.hpp"
#include "ui/graphics/LibGraphics.hpp"
#include <QString>
#include <QMap>

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace ui {
namespace graphics {

using namespace core;

/////////////////////////////////////////////////////////////////////////////

  /// @brief Try to compile a python line to detect simple errors
  /// @author Bolsee Vivian

  class Graphics_API PythonPreCompiler :
      public CNode
  {
  public:

    /// @brief Constructor.
    PythonPreCompiler();

    ~PythonPreCompiler();

    /// @brief try to compile the given line
    const QString try_compile(const QString & line) const;
  protected:
    QString tool_tip() const {return "";}
    void disable_local_signals(QMap<QString, bool> &local_signals) const {}
  private:
    static int python_init;
  }; // class PythonPreCompiler

  ///////////////////////////////////////////////////////////////////////////

} // Graphics
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_graphics_PythonPreCompiler_hpp
