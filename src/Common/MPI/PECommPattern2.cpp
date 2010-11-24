// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include <vector>

#include <boost/bind.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"
#include "Common/MPI/PECommPattern2.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common  {

////////////////////////////////////////////////////////////////////////////////
// Provider
////////////////////////////////////////////////////////////////////////////////

Common::ObjectProvider < PECommPattern2, Component, LibCommon, NB_ARGS_1 >
PECommPattern2_Provider ( PECommPattern2::type_name() );

////////////////////////////////////////////////////////////////////////////////
// Consstructor & destructor
////////////////////////////////////////////////////////////////////////////////

PECommPattern2::PECommPattern2(const CName& name): Component(name)
{
  BUILD_COMPONENT;
  m_isUpToDate=false;
  m_isFreeze=false;
  std::vector<Uint> gid(0);
  std::vector<Uint> rank(0);
  setup(gid,rank);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern2::PECommPattern2(const CName& name, std::vector<Uint> gid, std::vector<Uint> rank): Component(name)
{
  BUILD_COMPONENT;
  m_isUpToDate=false;
  m_isFreeze=false;
  setup(gid,rank);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern2::~PECommPattern2()
{

}

////////////////////////////////////////////////////////////////////////////////
// Commpattern handling
////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::setup(std::vector<Uint> gid, std::vector<Uint> rank)
{
  BOOST_ASSERT( gid.size()==rank.size() );
  if (gid.size()!=0) {
    std::vector<Uint>::iterator igid=gid.begin();
    std::vector<Uint>::iterator irank=rank.begin();
    for (;igid!=gid.end();igid++,irank++)
    {
      add(*igid,*irank);
    }
    setup();
  }
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::setup()
{
  // first dealing with add, always goes to the end
  BOOST_FOREACH(temp_buffer_item &iadd, m_add_buffer)
  {

  }
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::update()
{

}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::add(Uint gid, Uint rank)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to add nodes to commpattern '" + name() + "' which is freezed.");
  m_add_buffer.push_back(temp_buffer_item(gid,rank,false));
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::move(Uint gid, Uint rank, bool keep_as_ghost)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to moves nodes of commpattern '" + name() + "' which is freezed.");
  m_mov_buffer.push_back(temp_buffer_item(gid,rank,keep_as_ghost));
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::remove(Uint gid, Uint rank, bool on_all_ranks)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to delete nodes from commpattern '" + name() + "' which is freezed.");
  m_rem_buffer.push_back(temp_buffer_item(gid,rank,on_all_ranks));
}

////////////////////////////////////////////////////////////////////////////////
// Component related
////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::regist_signals ( Component* self )
{
//  self->regist_signal ( "update" , "Executes communication patterns on all the registered data not marked as readonly.", "" )->connect ( boost::bind ( &PECommPattern2::update, self, _1 ) );
}

////////////////////////////////////////////////////////////////////////////////

  }  // Common
} // CF











/*
#include <boost/lambda/lambda.hpp>

#include "Common/MPI/PE.hpp"
#include "Common/MPI/PECommPattern2.hpp"
#include "Common/MPI/all_to_all.hpp"

////////////////////////////////////////////////////////////////////////////////

using namespace boost::lambda;

namespace CF {
  namespace Common  {

////////////////////////////////////////////////////////////////////////////////

PECommPattern2::PECommPattern2()
{
  m_isCommPatternPrepared=false;
  m_sendCount.resize(PE::instance().size(),0);
  m_sendMap.resize(0);
  m_receiveCount.resize(PE::instance().size(),0);
  m_receiveMap.resize(0);
  m_updatable.resize(0);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern2::PECommPattern2(std::vector<Uint> gid, std::vector<Uint> rank)
{
  m_isCommPatternPrepared=false;
  m_sendCount.resize(PE::instance().size(),0);
  m_sendMap.resize(0);
  m_receiveCount.resize(PE::instance().size(),0);
  m_receiveMap.resize(0);
  m_updatable.resize(0);
  setup(gid,rank);
}

////////////////////////////////////////////////////////////////////////////////

PECommPattern2::~PECommPattern2()
{
}

////////////////////////////////////////////////////////////////////////////////

void PECommPattern2::setup(std::vector<Uint> gid, std::vector<Uint> rank)
{

  const Uint irank=PE::instance().rank();
  const Uint nproc=(Uint)PE::instance().size();

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
  boost::mpi::all_to_all(PE::instance(),m_sendCount,m_receiveCount);

  // fill receive map
  Uint total_m_receiveCount=0;
  for(Uint i=0; i<nproc; i++)
    total_m_receiveCount+=m_receiveCount[i];

  m_receiveMap.resize(total_m_receiveCount);
  std::vector< int > rcvdisp(nproc,0);

  for(Uint i=1; i<nproc; i++)
    rcvdisp[i]=rcvdisp[i-1]+m_receiveCount[i-1];

  for(Uint i=0; i<nproc; i++)
    MPI_Gatherv(&(sendmap[i])[0], sendmap[i].size(), MPI_INT, &m_receiveMap[0], &m_receiveCount[0], &rcvdisp[0], MPI_INT, i, PE::instance());

  std::cout << "m_updatable: ";
  std::for_each(m_updatable.begin(), m_updatable.end(), std::cout << _1 << ' ');
  std::cout << "\n" << std::flush;

  std::cout << "m_sendCount: ";
  std::for_each(m_sendCount.begin(), m_sendCount.end(), std::cout << _1 << ' ');
  std::cout << "\n" << std::flush;

  std::cout << "m_sendMap: ";
  std::for_each(m_sendMap.begin(), m_sendMap.end(), std::cout << _1 << ' ');
  std::cout << "\n" << std::flush;

  std::cout << "m_receiveCount: ";
  std::for_each(m_receiveCount.begin(), m_receiveCount.end(), std::cout << _1 << ' ');
  std::cout << "\n" << std::flush;

  std::cout << "m_receiveMap: ";
  std::for_each(m_receiveMap.begin(), m_receiveMap.end(), std::cout << _1 << ' ');
  std::cout << "\n" << std::flush;
}

////////////////////////////////////////////////////////////////////////////////

  }  // Common
} // CF
*/
