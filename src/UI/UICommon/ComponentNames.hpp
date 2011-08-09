// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_UI_UICommon_ComponentNames_hpp
#define CF_UI_UICommon_ComponentNames_hpp

//////////////////////////////////////////////////////////////////////////////

#define CLIENT_ROOT          "Root"
#define CLIENT_UI_DIR        "UI"
#define CLIENT_LOG           "Log"
#define CLIENT_TREE          "Tree"
#define CLIENT_BROWSERS      "Browsers"
#define CLIENT_JOURNAL       "Local Journal"
#define CLIENT_HISTORY       "History"
#define CLIENT_PLUGINS       "Plugins"
#define CLIENT_NETWORK_QUEUE "NetworkQueue"

#define CLIENT_ROOT_PATH          "//" CLIENT_ROOT
#define CLIENT_UI_DIR_PATH        "//" CLIENT_ROOT "/" CLIENT_UI_DIR
#define CLIENT_LOG_PATH           CLIENT_UI_DIR_PATH "/" CLIENT_LOG
#define CLIENT_TREE_PATH          CLIENT_UI_DIR_PATH "/" CLIENT_TREE
#define CLIENT_BROWSERS_PATH      CLIENT_UI_DIR_PATH "/" CLIENT_BROWSERS
#define CLIENT_JOURNAL_PATH       CLIENT_UI_DIR_PATH "/" CLIENT_JOURNAL
#define CLIENT_PLUGINS_PATH       CLIENT_UI_DIR_PATH "/" CLIENT_PLUGINS
#define CLIENT_NETWORK_QUEUE_PATH CLIENT_UI_DIR_PATH "/" CLIENT_NETWORK_QUEUE

//===========================================================================

#define SERVER_ROOT     CLIENT_ROOT
#define SERVER_CORE     "Core"
#define SERVER_JOURNAL  "Journal"
#define SERVER_HISTORY  "History"

#define SERVER_ROOT_PATH     "//" SERVER_ROOT
#define SERVER_CORE_PATH     SERVER_ROOT_PATH "/" SERVER_CORE
#define SERVER_JOURNAL_PATH  SERVER_ROOT_PATH "/Tools/" SERVER_JOURNAL

/////////////////////////////////////////////////////////////////////////////

#endif // CF_UI_UICommon_ComponentNames_hpp
