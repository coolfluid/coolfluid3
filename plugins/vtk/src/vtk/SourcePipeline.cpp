#include "SourcePipeline.hpp"

#include <vtkCommunicator.h>
#include <vtkCPDataDescription.h>
#include <vtkCPInputDataDescription.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkMultiProcessController.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPVTrivialProducer.h>
#include <vtkSmartPointer.h>
#include <vtkSMDoubleVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMSourceProxy.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMProxyManager.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMWriterProxy.h>

#include <string>

vtkStandardNewMacro(SourcePipeline);

//----------------------------------------------------------------------------
SourcePipeline::SourcePipeline()
{
  this->OutputFrequency = 0;
}

//----------------------------------------------------------------------------
SourcePipeline::~SourcePipeline()
{
}

//----------------------------------------------------------------------------
void SourcePipeline::Initialize(int outputFrequency)
{
  this->OutputFrequency = outputFrequency;
}

//----------------------------------------------------------------------------
int SourcePipeline::RequestDataDescription(
  vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("dataDescription is NULL.");
    return 0;
    }

  if(dataDescription->GetForceOutput() == true ||
     (this->OutputFrequency != 0 &&
      dataDescription->GetTimeStep() % this->OutputFrequency == 0) )
    {
    dataDescription->GetInputDescriptionByName("input")->AllFieldsOn();
    dataDescription->GetInputDescriptionByName("input")->GenerateMeshOn();
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------------
int SourcePipeline::CoProcess(
  vtkCPDataDescription* dataDescription)
{
  if(!dataDescription)
    {
    vtkWarningMacro("DataDescription is NULL");
    return 0;
    }
  vtkMultiBlockDataSet* dataset = vtkMultiBlockDataSet::SafeDownCast(
    dataDescription->GetInputDescriptionByName("input")->GetGrid());
  if(dataset == NULL)
    {
    vtkWarningMacro("DataDescription is missing input unstructured grid.");
    return 0;
    }
  if(this->RequestDataDescription(dataDescription) == 0)
    {
    return 1;
    }



  vtkSMProxyManager* proxyManager = vtkSMProxyManager::GetProxyManager();
  vtkSMSessionProxyManager* sessionProxyManager =
    proxyManager->GetActiveSessionProxyManager();

  // Create a vtkPVTrivialProducer and set its output
  // to be the input grid.
  if(m_producer == nullptr)
  {
    m_producer.TakeReference(
      vtkSMSourceProxy::SafeDownCast(
        sessionProxyManager->NewProxy("sources", "PVTrivialProducer")));
    sessionProxyManager->RegisterProxy("sources", "PVTrivialProducer", m_producer);
  }

  m_producer->UpdateVTKObjects();
  vtkObjectBase* clientSideObject = m_producer->GetClientSideObject();
  vtkPVTrivialProducer* realProducer =
    vtkPVTrivialProducer::SafeDownCast(clientSideObject);
  realProducer->SetOutput(dataset);

  return 1;
}

//----------------------------------------------------------------------------
void SourcePipeline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "OutputFrequency: " << this->OutputFrequency << "\n";
}
