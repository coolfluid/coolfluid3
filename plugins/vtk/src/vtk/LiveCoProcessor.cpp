// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/thread/thread.hpp>

#include <vtkCPCxxHelper.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkCPPipeline.h>
#include <vtkCPProcessor.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkProcessModule.h>
#include <vtkLiveInsituLink.h>
#include <vtkSessionIterator.h>
#include <vtkSMProxyManager.h>
#include <vtkSMSession.h>
#include <vtkSMSessionProxyManager.h>

#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Log.hpp"
#include "common/List.hpp"
#include "common/OptionList.hpp"

#include "solver/Tags.hpp"

#include "vtk/LiveCoProcessor.hpp"
#include "vtk/SourcePipeline.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace cf3;

////////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace vtk {


////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LiveCoProcessor, common::Action, LibVTK> LiveCoProcessor_Builder;

////////////////////////////////////////////////////////////////////////////////

LiveCoProcessor::LiveCoProcessor ( const std::string& name ) : common::Action ( name )
{
  options().add("cf3_to_vtk", m_cf3_to_vtk)
    .description("Converter to VTK")
    .pretty_name("CF3 To VTK")
    .link_to(&m_cf3_to_vtk)
    .attach_trigger(boost::bind(&LiveCoProcessor::finalize, this))
    .mark_basic();

  options().add("hostname", "localhost")
    .pretty_name("Hostname")
    .description("Hostname of the live vizualisation server")
    .attach_trigger(boost::bind(&LiveCoProcessor::finalize, this))
    .mark_basic();

  options().add(solver::Tags::time(), m_time)
    .pretty_name("Time")
    .description("Component that keeps track of time for this simulation")
    .link_to(&m_time)
    .mark_basic();
}

LiveCoProcessor::~LiveCoProcessor()
{
  finalize();
}

void LiveCoProcessor::execute()
{
  if(m_cf3_to_vtk == nullptr)
  {
    throw common::SetupError(FromHere(), "No CF3ToVTK component set for LiveCoProcessor");
  }

  initialize();

  if(m_processor == nullptr)
  {
    throw common::SetupError(FromHere(), "CoProcessor is not initialized");
  }

  if(m_time == nullptr)
  {
    throw common::SetupError(FromHere(), "Time component is not set for LiveCoProcessor");
  }

  m_cf3_to_vtk->execute();

  m_data_description->GetInputDescriptionByName("input")->SetGrid(m_cf3_to_vtk->vtk_multiblock_set());
  vtkSMProxyManager* proxy_manager = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* session_proxy_manager = proxy_manager->GetActiveSessionProxyManager();

  int time_step = m_time->iter();
  double time = m_time->current_time();
  m_data_description->SetTimeData(time, time_step);

  while(true)
  {
    m_link->InsituUpdate(time, time_step);
    m_processor->CoProcess(m_data_description);
    session_proxy_manager->UpdateRegisteredProxiesInOrder();
    m_link->InsituPostProcess(time, time_step);

    if(m_link->GetSimulationPaused())
    {
      CFinfo << "Simulation is pauzed" << CFendl;
      if(!m_link->WaitForLiveChange())
      {
        CFwarn << "disconnect during pause wait" << CFendl;
      }
    }
    else
    {
      break;
    }
  }
}

void LiveCoProcessor::initialize()
{
  if(m_sm_helper != nullptr)
    return;

  m_sm_helper = vtkCPCxxHelper::New();

  // Initialize the link
  m_link = vtkSmartPointer<vtkLiveInsituLink>::New();
  m_link->SetHostname(options().value<std::string>("hostname").c_str());
  m_link->SetInsituPort(22222);
  m_link->Initialize(vtkSMProxyManager::GetProxyManager()->GetActiveSessionProxyManager());

  // Create the pipeline processor
  m_processor = vtkSmartPointer<vtkCPProcessor>::New();

  // Add a pipeline
  auto pipeline = vtkSmartPointer<SourcePipeline>::New();
  pipeline->Initialize(1);
  m_processor->AddPipeline(pipeline);

  m_data_description = vtkSmartPointer<vtkCPDataDescription>::New();
  m_data_description->AddInput("input");
}

void LiveCoProcessor::finalize()
{
  if(m_sm_helper == nullptr)
    return;

  m_sm_helper->Delete();
  m_sm_helper = nullptr;
}

////////////////////////////////////////////////////////////////////////////////

} // namespace vtk
} // namespace cf3

////////////////////////////////////////////////////////////////////////////////
