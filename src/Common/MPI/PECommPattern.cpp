#include "Common/MPI/PEInterface.hpp"
#include "Common/MPI/PECommPattern.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common  {

////////////////////////////////////////////////////////////////////////////////

PECommPattern::PECommPattern(){
  m_isCommPatternPrepared=false;
  m_sendCount.resize(PEInterface::getInstance().size(),0);
  m_sendMap.resize(0);
  m_receiveCount.resize(PEInterface::getInstance().size(),0);
  m_receiveMap.resize(0);
  m_updatable.resize(0);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern::PECommPattern(std::vector<Uint> gid, std::vector<Uint> rank){
  PECommPattern::PECommPattern();
  setup(gid,rank);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern::~PECommPattern(){
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern::setup(std::vector<Uint> gid, std::vector<Uint> rank){

  const Uint irank=PEInterface::getInstance().rank();
  const Uint nproc=(Uint)PEInterface::getInstance().size();

  assert(gid.size()==rank.size());

  // count how much to send to each process
  // fill send map directly
  // and also fill updatable
  std::vector< std::vector<Uint> > sendmap;
  m_updatable.resize(gid.size());
  sendmap.resize(nproc);
  for(std::vector<Uint>::iterator i=rank.begin(); i < rank.end(); i++){
    if (*i!=irank){
      m_sendCount[*i]++;
      sendmap[*i].push_back(gid[i-rank.begin()]);
    }
    m_updatable[i-rank.begin()]=(*i==irank);
  }
  for(std::vector< std::vector<Uint> >::iterator i=sendmap.begin(); i < sendmap.end(); i++)
    for(std::vector<Uint>::iterator j=i->begin(); j < i->end(); j++)
      m_sendMap.push_back(*j);

  // fill receive count
  mpi::all_to_all(PEInterface::getInstance(),m_sendCount,m_receiveCount);

  // fill receive map
  Uint total_m_receiveCount=0;
  for(Uint i=0; i<nproc; i++) total_m_receiveCount+=m_receiveCount[i];
  m_receiveMap.resize(total_m_receiveCount);
  std::vector<Uint> rcvdisp(nproc,0);
  for(Uint i=1; i<nproc; i++) rcvdisp[i]=rcvdisp[i-1]+m_receiveCount[i-1];
  for(Uint i=0; i<nproc; i++) MPI_Gatherv(&(sendmap[i])[0],sendmap[i].size(),MPI_INT,&m_receiveMap[0],&m_receiveCount[0],&rcvdisp[0],MPI_INT,i,PEInterface::getInstance());

std::cout << "m_updatable: ";
for(std::vector<bool>::iterator i=m_updatable.begin(); i < m_updatable.end(); i++) std::cout << *i << " ";
std::cout << "\n" << std::flush;

std::cout << "m_sendCount: ";
for(std::vector<Uint>::iterator i=m_sendCount.begin(); i < m_sendCount.end(); i++) std::cout << *i << " ";
std::cout << "\n" << std::flush;

std::cout << "m_sendMap: ";
for(std::vector<Uint>::iterator i=m_sendMap.begin(); i < m_sendMap.end(); i++) std::cout << *i << " ";
std::cout << "\n" << std::flush;

std::cout << "m_receiveCount: ";
for(std::vector<Uint>::iterator i=m_receiveCount.begin(); i < m_receiveCount.end(); i++) std::cout << *i << " ";
std::cout << "\n" << std::flush;

std::cout << "m_receiveMap: ";
for(std::vector<Uint>::iterator i=m_receiveMap.begin(); i < m_receiveMap.end(); i++) std::cout << *i << " ";
std::cout << "\n" << std::flush;

}

////////////////////////////////////////////////////////////////////////////////

template<typename T> void PECommPattern::sync(T vec){
}

////////////////////////////////////////////////////////////////////////////////

template<typename T> void PECommPattern::move(T vec, std::vector<Uint> proc){
}

////////////////////////////////////////////////////////////////////////////////

  }  // namespace Common
} // namespace CF

