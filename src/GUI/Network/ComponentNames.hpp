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

#define CLIENT_ROOT_PATH      "//" CLIENT_ROOT
#define CLIENT_LOG_PATH       CLIENT_ROOT_PATH "/" CLIENT_LOG
#define CLIENT_TREE_PATH      CLIENT_ROOT_PATH "/" CLIENT_TREE
#define CLIENT_CORE_PATH      CLIENT_ROOT_PATH "/" CLIENT_CORE
#define CLIENT_BROWSERS_PATH  CLIENT_ROOT_PATH "/" CLIENT_BROWSERS

//===========================================================================

#define SERVER_ROOT  CLIENT_ROOT
#define SERVER_CORE  "Core"
#define SERVER_SIM   "Simulation"

#define SERVER_ROOT_PATH  "//" SERVER_ROOT
#define SERVER_CORE_PATH  SERVER_ROOT_PATH "/" SERVER_CORE
#define SERVER_SIM_PATH   SERVER_ROOT_PATH "/" SERVER_SIM

/////////////////////////////////////////////////////////////////////////////

#endif // CF_GUI_Network_ComponentNames_hpp
