// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_GUI_Network_ComponentNames_hpp
#define CF_GUI_Network_ComponentNames_hpp

//////////////////////////////////////////////////////////////////////////////

#define CLIENT_ROOT      "Root"
#define CLIENT_LOG       "Log"
#define CLIENT_TREE      "Tree"
#define CLIENT_CORE      "Core"
#define CLIENT_BROWSERS  "Browsers"
#define CLIENT_JOURNAL   "Local Journal"
#define CLIENT_HISTORY    "History"

#define CLIENT_ROOT_PATH      "//" CLIENT_ROOT
#define CLIENT_LOG_PATH       CLIENT_ROOT_PATH "/" CLIENT_LOG
#define CLIENT_TREE_PATH      CLIENT_ROOT_PATH "/" CLIENT_TREE
#define CLIENT_CORE_PATH      CLIENT_ROOT_PATH "/" CLIENT_CORE
#define CLIENT_BROWSERS_PATH  CLIENT_ROOT_PATH "/" CLIENT_BROWSERS
#define CLIENT_JOURNAL_PATH   CLIENT_ROOT_PATH "/" CLIENT_JOURNAL
#define CLIENT_HISTORY_PATH   CLIENT_ROOT_PATH "/" CLIENT_HISTORY

//===========================================================================

#define SERVER_ROOT     CLIENT_ROOT
#define SERVER_CORE     "Core"
#define SERVER_JOURNAL  "Journal"
#define SERVER_HISTORY  "History"

#define SERVER_ROOT_PATH     "//" SERVER_ROOT
#define SERVER_CORE_PATH     SERVER_ROOT_PATH "/" SERVER_CORE
#define SERVER_JOURNAL_PATH  SERVER_ROOT_PATH "/Tools/" SERVER_JOURNAL
#define SERVER_HISTORY_PATH  SERVER_ROOT_PATH "/Tools/" SERVER_JOURNAL

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_ComponentNames_hpp
