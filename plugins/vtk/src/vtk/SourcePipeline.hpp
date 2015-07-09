#ifndef CF_vtk_SourcePipeline_hpp
#define CF_vtk_SourcePipeline_hpp

#include <vtkCPPipeline.h>
#include <vtkSmartPointer.h>
#include <string>

class vtkCPDataDescription;
class vtkCPPythonHelper;
class vtkSMSourceProxy;

class SourcePipeline : public vtkCPPipeline
{
public:
  static SourcePipeline* New();
  vtkTypeMacro(SourcePipeline,vtkCPPipeline);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual void Initialize(int outputFrequency);

  virtual int RequestDataDescription(vtkCPDataDescription* dataDescription);

  virtual int CoProcess(vtkCPDataDescription* dataDescription);

protected:
  SourcePipeline();
  virtual ~SourcePipeline();

private:
  SourcePipeline(const SourcePipeline&); // Not implemented
  void operator=(const SourcePipeline&); // Not implemented

  int OutputFrequency;
  vtkSmartPointer<vtkSMSourceProxy> m_producer;
};
#endif /* CF_vtk_SourcePipeline_hpp */
