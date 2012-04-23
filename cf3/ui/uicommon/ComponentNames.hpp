// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef cf3_ui_uiCommon_ComponentNames_hpp
#define cf3_ui_uiCommon_ComponentNames_hpp

//////////////////////////////////////////////////////////////////////////////

#define CLIENT_UI_DIR        "UI"
#define CLIENT_LOG           "Log"
#define CLIENT_TREE          "Tree"
#define CLIENT_BROWSERS      "Browsers"
#define CLIENT_JOURNAL       "Local Journal"
#define CLIENT_HISTORY       "History"
#define CLIENT_PLUGINS       "Plugins"
#define CLIENT_NETWORK_QUEUE "NetworkQueue"

#define CLIENT_SCRIPT_ENGINE "ScriptEngine"
#define CLIENT_PYTHON_PRE_COMPILER "PythonPreCompiler"

#define CLIENT_ROOT_PATH          "/"
#define CLIENT_UI_DIR_PATH        "/" CLIENT_UI_DIR
#define CLIENT_LOG_PATH           CLIENT_UI_DIR_PATH "/" CLIENT_LOG
#define CLIENT_TREE_PATH          CLIENT_UI_DIR_PATH "/" CLIENT_TREE
#define CLIENT_BROWSERS_PATH      CLIENT_UI_DIR_PATH "/" CLIENT_BROWSERS
#define CLIENT_JOURNAL_PATH       CLIENT_UI_DIR_PATH "/" CLIENT_JOURNAL
#define CLIENT_PLUGINS_PATH       CLIENT_UI_DIR_PATH "/" CLIENT_PLUGINS
#define CLIENT_NETWORK_QUEUE_PATH CLIENT_UI_DIR_PATH "/" CLIENT_NETWORK_QUEUE

#define CLIENT_SCRIPT_ENGINE_PATH CLIENT_UI_DIR_PATH "/" CLIENT_SCRIPT_ENGINE
#define CLIENT_PYTHON_PRE_COMPILER_PATH CLIENT_UI_DIR_PATH "/" CLIENT_PYTHON_PRE_COMPILER

//===========================================================================

#define SERVER_CORE     "Core"
#define SERVER_JOURNAL  "Journal"
#define SERVER_HISTORY  "History"

#define SERVER_ROOT_PATH     "/"
#define SERVER_CORE_PATH     "/" SERVER_CORE
#define SERVER_JOURNAL_PATH  "/Tools/" SERVER_JOURNAL

/////////////////////////////////////////////////////////////////////////////

#endif // cf3_ui_uiCommon_ComponentNames_hpp
