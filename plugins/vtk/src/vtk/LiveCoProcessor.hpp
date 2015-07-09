// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_vtk_LiveCoProcessor_hpp
#define CF_vtk_LiveCoProcessor_hpp

#include <vtkSmartPointer.h>

#include "common/Action.hpp"
#include "mesh/Mesh.hpp"
#include "solver/Time.hpp"

#include "vtk/LibVTK.hpp"
#include "vtk/CF3ToVTK.hpp"

////////////////////////////////////////////////////////////////////////////////

// ParaView forward declarations
class vtkCPProcessor;
class vtkLiveInsituLink;
class vtkCPCxxHelper;

namespace cf3 {
namespace vtk {

////////////////////////////////////////////////////////////////////////////////

/// Handle live CoProcessing with ParaView
class LiveCoProcessor : public common::Action
{
public:
  LiveCoProcessor ( const std::string& name );
  virtual ~LiveCoProcessor();
  
  static std::string type_name () { return "LiveCoProcessor"; }
  
  virtual void execute();

  void trigger_cf3_to_vtk();

private:
  void initialize();

  Handle<CF3ToVTK> m_cf3_to_vtk;
  Handle<solver::Time> m_time;

  vtkCPCxxHelper* m_sm_helper = nullptr;
  vtkSmartPointer<vtkCPProcessor> m_processor;
  vtkSmartPointer<vtkLiveInsituLink> m_link;
  vtkSMSession* m_session = nullptr;
  vtkSmartPointer<vtkCPDataDescription> m_data_description;
};
  
////////////////////////////////////////////////////////////////////////////////

} //  vtk
} // cf3

////////////////////////////////////////////////////////////////////////////////

#endif /* CF_vtk_LiveCoProcessor_hpp */
