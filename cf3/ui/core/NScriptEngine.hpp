// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.


#ifndef cf3_ui_core_NScriptEngine_hpp
#define cf3_ui_core_NScriptEngine_hpp


//////////////////////////////////////////////////////////////////////////////

#include <QHash>

#include "ui/core/CNode.hpp"

class QString;

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {

namespace ui {
namespace core {

/////////////////////////////////////////////////////////////////////////////

  /// @brief Log component
  /// @author Bolsee Vivian.

  class Core_API NScriptEngine :
      public QObject,
      public CNode
{
      Q_OBJECT
    public:
        NScriptEngine();


        /// @brief Gives the text to put on a tool tip
        /// @return The name of the class.
        virtual QString tool_tip() const;

        static Handle<NScriptEngine> global();

        void execute_line( const QString & line );

        void get_completion_list();

        /// @brief Boost slot called when python console output are sent from the server
        /// @param node Signal node
        void signal_output(common::SignalArgs & node);

        /// @brief Boost slot called when the server send his completion list
        /// @param node Signal node
        void signal_completion(common::SignalArgs & node);

      signals:

        /// @brief Signal emitted when the server send the new console output.
        /// @param output Output message
        void new_output(const QString & output);

        /// @brief Signal emitted when the server send the new console output.
        /// @param output Output message
        void completion_list_received(const QStringList & word_list);

      protected:

        /// Disables the local signals that need to.
        /// @param localSignals Map of local signals. All values are set to true
        /// by default.
        virtual void disable_local_signals(QMap<QString, bool> & localSignals) const {}

      private:
        int current_code_fragment;
}; // class NScriptEngine

///////////////////////////////////////////////////////////////////////////

} // Core
} // ui
} // cf3

/////////////////////////////////////////////////////////////////////////////


#endif // cf3_ui_core_NScriptEngine_hpp
