#include "Common/PEInterface.hh"

namespace CF {
   namespace Common  {

//////////////////////////////////////////////////////////////////////////////

PEInterfaceBase::~PEInterfaceBase () {}

unsigned int
PEInterfaceBase::GetProcessorCount () const
{ throw Common::NotImplementedException (FromHere(),"GetProcessorCount()"); return 0; }

unsigned int
PEInterfaceBase::GetRank () const
{ throw Common::NotImplementedException (FromHere(),"PEInterface()::GetRank()"); return 0; }

void
PEInterfaceBase::setBarrier()
{ throw Common::NotImplementedException (FromHere(),"PEInterface()::setBarrier()"); }

bool
PEInterfaceBase::IsParallel () const
{ throw Common::NotImplementedException (FromHere(),"PEInterface()::IsParallel()"); return false; }

std::string
PEInterfaceBase::GetName () const
{ throw Common::NotImplementedException (FromHere(),"PEInterface::Getname");  return ""; }

bool
PEInterfaceBase::IsParallelCapable () const
{ throw Common::NotImplementedException (FromHere(),"IsParallelCapable"); return false;}

void
PEInterfaceBase::AdvanceCommunication ()
{ throw Common::NotImplementedException (FromHere(),"AdvanceCommunication"); }

//////////////////////////////////////////////////////////////////////////////

   }
}
