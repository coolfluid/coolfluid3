#include "Common/PEInterface.hh"

namespace CF {
  namespace Common  {

//////////////////////////////////////////////////////////////////////////////

PEInterfaceBase::~PEInterfaceBase () {}

unsigned int
PEInterfaceBase::GetProcessorCount () const
{ throw Common::NotImplemented (FromHere(),"GetProcessorCount()"); return 0; }

unsigned int
PEInterfaceBase::GetRank () const
{ throw Common::NotImplemented (FromHere(),"PEInterfaceBase()::GetRank()"); return 0; }

void
PEInterfaceBase::setBarrier()
{ throw Common::NotImplemented (FromHere(),"PEInterfaceBase()::setBarrier()"); }

bool
PEInterfaceBase::IsParallel () const
{ throw Common::NotImplemented (FromHere(),"PEInterfaceBase()::IsParallel()"); return false; }

std::string
PEInterfaceBase::GetName () const
{ throw Common::NotImplemented (FromHere(),"PEInterfaceBase::Getname");  return ""; }

bool
PEInterfaceBase::IsParallelCapable () const
{ throw Common::NotImplemented (FromHere(),"IsParallelCapable"); return false;}

void
PEInterfaceBase::AdvanceCommunication ()
{ throw Common::NotImplemented (FromHere(),"AdvanceCommunication"); }

//////////////////////////////////////////////////////////////////////////////

  }
}
