// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/BoostAssertions.hpp"
#include "Common/LibCommon.hpp"
#include "Common/FindComponents.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"

#include "Common/MPI/PE.hpp"
#include "Common/MPI/CommPattern.hpp"
#include "Common/MPI/CommWrapper.hpp"

#include "Common/MPI/debug.hpp"

/*
TODO:
1.: remove boost.mpi deps when pe ready
2.: if 1. is done, remove &xxx[0] accessses
*/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace Comm {

////////////////////////////////////////////////////////////////////////////////
// Provider
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CommPattern, Component, LibCommon > CommPattern_Provider;

////////////////////////////////////////////////////////////////////////////////
// Constructor & destructor
////////////////////////////////////////////////////////////////////////////////

CommPattern::CommPattern(const std::string& name): Component(name), m_gid(new CommWrapperPtr<int>("dummy")),
  m_isUpdatable(0),
  m_add_buffer(0),
  m_mov_buffer(0),
  m_rem_buffer(0),
  m_sendCount(Comm::PE::instance().size(),0),
  m_sendMap(0),
  m_recvCount(Comm::PE::instance().size(),0),
  m_recvMap(0)
{
  //self->regist_signal ( "update" , "Executes communication patterns on all the registered data.", "" )->connect ( boost::bind ( &CommPattern2::update, self, _1 ) );
  m_isUpToDate=false;
  m_isFreeze=false;
}

////////////////////////////////////////////////////////////////////////////////

CommPattern::~CommPattern()
{
  if (m_gid.get()!=nullptr) m_gid->remove_tag("gid_of_"+this->name());
}

////////////////////////////////////////////////////////////////////////////////
// Commpattern handling
////////////////////////////////////////////////////////////////////////////////

void CommPattern::setup(CommWrapper::Ptr gid, std::vector<Uint>& rank)
{
  // basic check
  BOOST_ASSERT( (Uint)gid->size() == rank.size() );
  if (gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Data to be registered as gid is not of stride=1.");
  if (gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Data to be registered as gid is not of type Uint.");
  m_gid=gid;
  m_gid->add_tag("gid_of_"+this->name());
  if (get_child_ptr(gid->name()).get() == nullptr) add_component(gid);

  // sizesof datas matching
  BOOST_FOREACH( CommWrapper& pobj, find_components_recursively<CommWrapper>(*this) )
    if ((Uint) pobj.size()!=m_isUpdatable.size()+gid->size())
      throw CF::Common::BadValue(FromHere(),"Size does not match commpattern's size.");

  // add to add buffer
  // if performance issues, replace for(...) add(...) with direct push_back
  if (gid->size()!=0)
  {
    m_isUpToDate=false;
    std::vector<int> map(gid->size());
    for(int i=0; i<(int)map.size(); i++) map[i]=i;
    Uint *igid=(Uint*)gid->pack(map);
    std::vector<Uint>::iterator irank=rank.begin();
    for (Uint* iigid=igid;irank!=rank.end();irank++,iigid++)
      add(*iigid,*irank);
    delete[] igid;
    /*
     PECheckPoint(100,"-- Setup comission: --");
     PEProcessSortedExecute(-1,
     BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) std::cout << "("<< i.gid << "|" << i.rank << "|" << i.option << ")";
     std::cout << "\n" << std::flush;
      )
    */
    setup();
  }
}

void CommPattern::setup(CommWrapper::Ptr gid, boost::multi_array<Uint,1>& rank)
{
  // basic check
  BOOST_ASSERT( (Uint)gid->size() == rank.size() );
  if (gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Data to be registered as gid is not of stride=1.");
  if (gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Data to be registered as gid is not of type Uint.");
  m_gid=gid;
  m_gid->add_tag("gid_of_"+this->name());
  if (get_child_ptr(gid->name()).get() == nullptr) add_component(gid);

  // sizesof datas matching
  BOOST_FOREACH( CommWrapper& pobj, find_components_recursively<CommWrapper>(*this) )
    if ((Uint) pobj.size()!=m_isUpdatable.size()+gid->size())
      throw CF::Common::BadValue(FromHere(),"Size does not match commpattern's size.");

  // add to add buffer
  // if performance issues, replace for(...) add(...) with direct push_back
  if (gid->size()!=0) {
    m_isUpToDate=false;
    std::vector<int> map(gid->size());
    for(int i=0; i<(int)map.size(); i++) map[i]=i;
    Uint *igid=(Uint*)gid->pack(map);
    boost::multi_array<Uint,1>::iterator irank=rank.begin();
    for (Uint* iigid=igid;irank!=rank.end();irank++,iigid++)
      add(*iigid,*irank);
    delete[] igid;
/*
PECheckPoint(100,"-- Setup comission: --");
PEProcessSortedExecute(-1,
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) std::cout << "("<< i.gid << "|" << i.rank << "|" << i.option << ")";
  std::cout << "\n" << std::flush;
)
*/
    setup();
  }
}

////////////////////////////////////////////////////////////////////////////////

// OK: how this works:
// 0.: ??? lock and pre-flush data ???
// 1.:

void CommPattern::setup()
{

  {  // begin fast
  // get stuff
  const CPint irank=(CPint)Comm::PE::instance().rank();
  const CPint nproc=(CPint)Comm::PE::instance().size();
  if (m_gid.get()==nullptr) throw CF::Common::BadValue(FromHere(),"Gid is not registered for for commpattern: " + name());
  if (m_gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Gid is not of stride==1 for commpattern: " + name());
  if (m_gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Gid is not of type Uint for commpattern: " + name());
  Uint* gid=(Uint*)m_gid->pack();
  m_isUpdatable.resize(m_gid->size(),true);

  // get counts on receive side
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) m_recvCount[i.rank]++;
  m_recvCount[irank]=0;
  int recvSum=0;
  BOOST_FOREACH(CPint& i, m_recvCount) recvSum+=i;
  std::vector<CPint> recvcounter(nproc,0);
  for (int i=1; i<nproc; i++) recvcounter[i]=m_recvCount[i-1]+recvcounter[i-1];
  m_recvMap.reserve(recvSum);
  m_recvMap.resize(recvSum);

//PECheckPoint(100,"-- step 1 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_recvCount,m_recvCount.size()));
//PEProcessSortedExecute(-1,PEDebugVector(recvcounter,recvcounter.size()));

  // fill receive data and inverse send
  std::vector<CPint> receive_gids(recvSum);
  int lid=0;
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer)
  {
    if (i.rank!=irank)
    {
      receive_gids[recvcounter[i.rank]]=i.gid;
      m_recvMap[recvcounter[i.rank]]=lid;
      m_isUpdatable[lid]=false;
      recvcounter[i.rank]++;
    }
    lid++;
  }

//PECheckPoint(100,"-- step 2 --:");
//PEProcessSortedExecute(-1,PEDebugVector(receive_gids,receive_gids.size()));
//PEProcessSortedExecute(-1,PEDebugVector(m_recvMap,m_recvMap.size()));

  // communicate gids per receive side
  m_sendMap.resize(0);
  m_sendMap.reserve(0);
  m_sendCount.assign(nproc,-1);
  Comm::PE::instance().all_to_all(receive_gids,m_recvCount,m_sendMap,m_sendCount);

//PECheckPoint(100,"-- step 3 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_sendCount,m_sendCount.size()));
//PEProcessSortedExecute(-1,PEDebugVector(m_sendMap,m_sendMap.size()));

  // look up the nodes to on send side (brute force searching for now)
  BOOST_FOREACH(int& si, m_sendMap)
  {
    bool found = false;
    for (int i=0; i<m_gid->size(); i++)
    {
      if (gid[i]==si)
      {
        si=i;
        found = true;
        break;
      }
    }
    if (found == false)
      throw ValueNotFound(FromHere(), "requested global id " + to_str(si) + " not found in gid list" );
  }

//PECheckPoint(100,"-- step 4 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_sendMap,m_sendMap.size()));

  return;
  } // end fast

  ///////////////////////////////////////////////////////////////////////
  // experimental version, supports one single setup call and only add //
  ///////////////////////////////////////////////////////////////////////

  // -- 1 -- data definition

//PECheckPoint(100,"-- step 1 --:");

  // -- 2 -- get environment data and gid
  // get stuff from environment
  const CPint irank=(CPint)Comm::PE::instance().rank();
  const CPint nproc=(CPint)Comm::PE::instance().size();
  // get gid and some tests
  if (m_gid.get()==nullptr) throw CF::Common::BadValue(FromHere(),"Gid is not registered for for commpattern: " + name());
  if (m_gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Gid is not of stride==1 for commpattern: " + name());
  if (m_gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Gid is not of type Uint for commpattern: " + name());
  Uint* gid=(Uint*)m_gid->pack();

//PECheckPoint(100,"-- step 2 --:");

  // -- 3 -- build receive info
  // build receive count and receive map, note that filtering out data from laying on current process
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) m_recvCount[i.rank]++;
  m_recvCount[irank]=0;
  int recvSum=0;
  BOOST_FOREACH(CPint& i, m_recvCount) recvSum+=i;
  m_recvMap.reserve(recvSum);
  m_recvMap.assign(recvSum,std::numeric_limits<CPint>::max());
  std::vector<CPint> recvcounter(nproc,0);
  for (int i=1; i<nproc; i++) recvcounter[i]=m_recvCount[i-1]+recvcounter[i-1];
  CPint recvidx=0;
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer)
  {
    if (i.rank!=irank)
      m_recvMap[recvcounter[i.rank]++]=recvidx;
    recvidx++;
  }
  BOOST_FOREACH(CPint& i, m_recvMap)
    BOOST_ASSERT(i!=std::numeric_limits<CPint>::max());

//PECheckPoint(100,"-- step 3 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_recvMap,m_recvMap.size()));
//PEProcessSortedExecute(-1,PEDebugVector(m_recvCount,m_recvCount.size()));

  // -- 4 -- building a distributed info for looking up communication
  // setting up m_isUpdatable and its counts (distributed over processes)
  // also computing displacements of counts
  m_isUpdatable.resize(m_add_buffer.size(),false);
  m_isUpdatable.reserve(m_add_buffer.size());
  std::vector<int> dist_nupdatable(nproc,0);
  for (int i=0; i<m_add_buffer.size(); i++)
    if (irank==m_add_buffer[i].rank)
    {
      dist_nupdatable[irank]++;
      m_isUpdatable[i]=true;
    }
  std::vector<int> dist_nupdatables(nproc);
  Comm::PE::instance().all_reduce(Comm::plus(),dist_nupdatable,dist_nupdatable);
  std::vector<int> dist_nupdatabledisp(nproc,0);
  for (int i=1; i<nproc; i++) dist_nupdatabledisp[i]=dist_nupdatable[i-1]+dist_nupdatabledisp[i-1];

//PECheckPoint(100,"-- step 4 --:");
//PEProcessSortedExecute(-1,PEDebugVector(dist_nupdatable,dist_nupdatable.size()));
//PEProcessSortedExecute(-1,PEDebugVector(dist_nupdatabledisp,dist_nupdatabledisp.size()));

  // building info and communicating dist
  std::vector<dist_struct> dist(dist_nupdatable[irank]);
  CPint ilid=0;
  BOOST_FOREACH(dist_struct& i, dist)
  {
    i.gid=gid[ilid];
    i.rank=irank;
    i.lid=ilid++;
    i.data=nullptr;
  }

PEProcessSortedExecute(-1, BOOST_FOREACH(dist_struct& i, dist) std::cout << i.lid << " " << i.gid << " " << i.rank << " " << i.data << "\n" << std::flush;);

/*
  std::vector<int> dist_nsend(nproc,0);
  delete[] gid;

PEProcessSortedExecute(-1,BOOST_FOREACH(dist_struct& i, dist) std::cout << i.lid << " " << i.gid << " " << i.rank << " " << i.data << "\n" << std::flush; );

*/
/*
PECheckPoint(100,"XXXXX Distributed data, step 1:");
PEProcessSortedExecute(-1,PEDebugVector(dist_lidupdatable,dist_lidupdatable.size()));
PEProcessSortedExecute(-1,PEDebugVector(dist_rankupdatable,dist_rankupdatable.size()));
*/
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize_all()
{
  BOOST_FOREACH( CommWrapper& pobj, find_components_recursively<CommWrapper>(*this) )
  {
    synchronize_this(pobj);
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize( const std::string& name )
{
  CommWrapper& pobj = get_child(name).as_type<CommWrapper>();
  synchronize_this(pobj);
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize_this( const CommWrapper& pobj )
{

//  std::cout << PERank << pobj.name() << "\n" << std::flush;
//  std::cout << PERank << pobj.needs_update() << "\n" << std::flush;

  if ( pobj.needs_update() )
  {
//      PEProcessSortedExecute(-1,std::cout << PERank << "   sync -> " <<  pobj.name() << "\n" << std::flush; );

      char* snd_data = (char*)pobj.pack(m_sendMap);

      char* rcv_data = Comm::PE::instance().all_to_all(snd_data,
                                                      &m_sendCount[0],
                                                      (char*)0,
                                                      &m_recvCount[0],
                                                      pobj.size_of()*pobj.stride());

      pobj.unpack(rcv_data,m_recvMap);

      /// @todo avoid this allocation and deallocation of buffers
      ///       remember that in explicit synchronize is called frequently
      delete[] snd_data;
      delete[] rcv_data;
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::add(Uint gid, Uint rank)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to add nodes to commpattern '" + name() + "' which is freezed.");
  m_add_buffer.push_back(temp_buffer_item(gid,rank,false));
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::move(Uint gid, Uint rank, bool keep_as_ghost)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to moves nodes of commpattern '" + name() + "' which is freezed.");
  m_mov_buffer.push_back(temp_buffer_item(gid,rank,keep_as_ghost));
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::remove(Uint gid, Uint rank, bool on_all_ranks)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to delete nodes from commpattern '" + name() + "' which is freezed.");
  m_rem_buffer.push_back(temp_buffer_item(gid,rank,on_all_ranks));
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////
// Component related
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

} // MPI
} // Common
} // CF
