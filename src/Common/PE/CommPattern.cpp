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

#include "Common/PE/Comm.hpp"
#include "Common/PE/CommPattern.hpp"
#include "Common/PE/CommWrapper.hpp"

#include "Common/PE/debug.hpp"

/*
TODO:
1.: remove boost.mpi deps when pe ready
2.: if 1. is done, remove &xxx[0] accessses
*/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common  {
namespace PE {

////////////////////////////////////////////////////////////////////////////////
// Provider
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CommPattern, Component, LibCommon > CommPattern_Provider;

////////////////////////////////////////////////////////////////////////////////
// Constructor & destructor
////////////////////////////////////////////////////////////////////////////////

CommPattern::CommPattern(const std::string& name): Component(name), m_gid(allocate_component< CommWrapperPtr<int> >("dummy")),
  m_isUpdatable(0),
  m_add_buffer(0),
  m_mov_buffer(0),
  m_rem_buffer(0),
  m_free_lids(1,0),
  m_sendCount(PE::Comm::instance().size(),0),
  m_sendMap(0),
  m_recvCount(PE::Comm::instance().size(),0),
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
  // if performance issues, replace for(...) add_global(...) with direct push_back
  if (gid->size()!=0)
  {
    m_isUpToDate=false;
    std::vector<int> map(gid->size());
    for(int i=0; i<(const int)map.size(); i++) map[i]=i;
    PE::CommWrapperView<Uint> cwv_gid(m_gid);
    std::vector<Uint>::iterator irank=rank.begin();
    for (Uint* iigid=cwv_gid();irank!=rank.end();irank++,iigid++)
      add_global(*iigid,*irank);
/*
PECheckPoint(100,"-- Setup comission: --");
PEProcessSortedExecute(-1,
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) std::cout << "("<< i.gid << "|" << i.rank << "|" << i.option << ")";
  std::cout << "\n" << std::flush;
  )
*/
    gid->resize(0);
    m_isUpdatable.resize(0);
    m_free_lids.resize(1);
    setup();
    m_free_lids[0]=m_isUpdatable.size();
    m_isUpToDate=true;
  }
}

////////////////////////////////////////////////////////////////////////////////

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
  // if performance issues, replace for(...) add_global(...) with direct push_back
  if (gid->size()!=0) {
    m_isUpToDate=false;
    std::vector<int> map(gid->size());
    for(int i=0; i<(int)map.size(); i++) map[i]=i;
    PE::CommWrapperView<Uint> cwv_gid(m_gid);
    boost::multi_array<Uint,1>::iterator irank=rank.begin();
    for (Uint* iigid=cwv_gid();irank!=rank.end();irank++,iigid++)
      add_global(*iigid,*irank);
/*
PECheckPoint(100,"-- Setup comission: --");
PEProcessSortedExecute(-1,
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) std::cout << "("<< i.gid << "|" << i.rank << "|" << i.option << ")";
  std::cout << "\n" << std::flush;
)
*/
    gid->resize(0);
    m_isUpdatable.resize(0);
    m_free_lids.resize(1);
    setup();
    m_free_lids[0]=m_isUpdatable.size();
    m_isUpToDate=true;
  }
}

////////////////////////////////////////////////////////////////////////////////
/*
void CommPattern::setup()
{

#define COMPUTE_IRANK(inode,nproc,nnode) ((((unsigned long long)(inode))*((unsigned long long)(nproc)))/((unsigned long long)(nnode)))
#define COMPUTE_INODE(irank,nproc,nnode) (((unsigned long long)(nnode))>((unsigned long long)(nproc)) ? ((((unsigned long long)(irank))*((unsigned long long)(nnode)))%((unsigned long long)(nproc))==0?(((unsigned long long)(irank))*((unsigned long long)(nnode)))/((unsigned long long)(nproc)):((((unsigned long long)(irank))*((unsigned long long)(nnode)))/((unsigned long long)(nproc)))+1ul) : ((unsigned long long)(irank)))

  if (m_gid==nullptr) throw Common::BadValue(FromHere(),name() + ": There is no gid associated to this CommPattern.");

PECheckPoint(100,"001");

  // exit on obvious
  int changes[3]={0,0,0}; // order: add,mov,rem
  changes[0]=m_add_buffer.size();
  changes[1]=m_mov_buffer.size();
  changes[2]=m_rem_buffer.size();

PECheckPoint(1000,"002");

  PE::Comm::instance().all_reduce(PE::plus(),changes,3,changes);
  if (changes[0]+changes[1]+changes[2]==0) return;
//  if (changes[1]+changes[2]==0) GOTO OPTIMIZED ADD-ONLY FUNCTION FOR PERFORMANCE

PECheckPoint(1000,"003");

  // get stuff
  const CPint irank=(CPint)PE::Comm::instance().rank();
  const CPint nproc=(CPint)PE::Comm::instance().size();
  if (m_gid.get()==nullptr) throw CF::Common::BadValue(FromHere(),"Gid is not registered for for commpattern: " + name());
  if (m_gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Gid is not of stride==1 for commpattern: " + name());
  if (m_gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Gid is not of type Uint for commpattern: " + name());

PECheckPoint(1000,"004");

  // filling buffer to be inverted into a global, over-all-ranks array
  { // brackets necessary for gid view to live short
    PE::CommWrapperView<Uint> cwv_gid(m_gid);
    Uint* gid=cwv_gid();

    // filling a local vector with the existing nodes
    std::vector<dist_struct> l(0);
      for(int i=0; i<(const int)m_gid->size(); i++)
      {
        if (m_isUpdatable[i]) l.push_back(dist_struct(gid[i],irank,i,NOFLAGS));
        else l.push_back(dist_struct(gid[i],irank,i,GHOST));
      }
    }

    // extending the local vector with remove info
    BOOST_FOREACH(temp_buffer_item i, m_rem_buffer)
    {
      if (i.option) l.push_back(dist_struct(gid[i.lid],i.rank,i.lid,ALLDELETE|DELETED));
      else l.push_back(dist_struct(gid[i.lid],i.rank,i.lid,DELETED));
    }

    // extending the local vector with inter-process move info
    BOOST_FOREACH(temp_buffer_item i, m_mov_buffer)
    {
      l.push_back(dist_struct(gid[i.lid],i.rank,i.lid,NOFLAGS));
      if (!i.option) l.push_back(dist_struct(gid[i.lid],i.rank,i.lid,DELETED));
    }

    // extending the local vector with add info
    // 1. count the local add-s to compute needed gid counts
    // 2. organize locally distributed gids not to hook-up with global adds, its probalby a linearized loop over processes
    // 3. push to array
  } // end the scope of gid view

// OK, when the list is made:
// 1. make the global inversion,  into a receive buffer
// 2. probably here communicate the data registered to this commwrapper
// 3. the global array should be 3 arrays:
//    - counter of dist_struct items associated to that gid
//    - pointer to hold the arrays
//    - pointer to hold the data
// 4. collect everything under the backbone structure designed at 3.
// 5. do the comparisons/steps...etc associated with the flags
// 6. distribute back the results
// 7. delete interprocess leaks, add moves (will need another extra gid distribution)

// remark 1.: make sure that the holes in the lids (can happen due to global adds and interpocess deletes) are set to ghosts and and kept in m_free_lids
// remark 2.: need a faster version when no ghost present -> just needs to move data but no interprocess communication
// remark 3.: COMPUTE_IRANK (which rank a gid is in a global array) and COMPUTE_INODE (which are the starting indices on each processors) is useful for a unique, closely equally distributed elements accross all processors in a global array

#undef COMPUTE_IRANK
#undef COMPUTE_INODE

}

/*/

void CommPattern::setup()
{
#define COMPUTE_IRANK(inode,nproc,nnode) ((((unsigned long long)(inode))*((unsigned long long)(nproc)))/((unsigned long long)(nnode)))
#define COMPUTE_INODE(irank,nproc,nnode) (((unsigned long long)(nnode))>((unsigned long long)(nproc)) ? ((((unsigned long long)(irank))*((unsigned long long)(nnode)))%((unsigned long long)(nproc))==0?(((unsigned long long)(irank))*((unsigned long long)(nnode)))/((unsigned long long)(nproc)):((((unsigned long long)(irank))*((unsigned long long)(nnode)))/((unsigned long long)(nproc)))+1ul) : ((unsigned long long)(irank)))

  // get stuff
  const CPint irank=(CPint)PE::Comm::instance().rank();
  const CPint nproc=(CPint)PE::Comm::instance().size();
  if (m_gid.get()==nullptr) throw CF::Common::BadValue(FromHere(),"Gid is not registered for for commpattern: " + name());
  if (m_gid->stride()!=1) throw CF::Common::BadValue(FromHere(),"Gid is not of stride==1 for commpattern: " + name());
  if (m_gid->is_data_type_Uint()!=true) throw CF::Common::CastingFailed(FromHere(),"Gid is not of type Uint for commpattern: " + name());

  // reset
  m_sendCount.clear();
  m_sendMap.clear();
  m_recvCount.clear();
  m_recvMap.clear();

  // look around for max gid for the global array's size
  int nglobalarray=-1;
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer) nglobalarray=i.gid>nglobalarray?i.gid:nglobalarray;

PEProcessSortedExecute(-1,std::cout << "nglobalarray= " << nglobalarray << "\n" << std::flush);

  PE::Comm::instance().all_reduce(PE::max(),&nglobalarray,1,&nglobalarray);
  nglobalarray++; // zero based indexing!

PEProcessSortedExecute(-1,std::cout << "nglobalarray= " << nglobalarray << "\n" << std::flush);
PEProcessSortedExecute(-1,
BOOST_FOREACH(temp_buffer_item& i, m_add_buffer)
  std::cout <<  i.lid << "(" << i.gid << ") " << std::flush;
std::cout << "\n" << std::flush;
);

  // fill local and its mpi communication accessories
  std::vector<dist_struct> local(0);
  std::vector<int> sendcnt(nproc,0);
  std::vector<int> sendmap(m_add_buffer.size());
  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer)
    sendcnt[COMPUTE_IRANK(i.gid,nproc,nglobalarray)]++;
  std::vector<int> sendstarts(nproc,0);
  for (int i=1; i<(const int)nproc; i++) sendstarts[i]=sendstarts[i-1]+sendcnt[i-1];

  BOOST_FOREACH(temp_buffer_item& i, m_add_buffer)
  {
    if (i.rank==irank) local.push_back(dist_struct(i.gid,irank,-i.lid,UPDATABLE));
    else local.push_back(dist_struct(i.gid,irank,-i.lid,GHOST));
    sendmap[sendcnt[COMPUTE_IRANK(i.gid,nproc,nglobalarray)]++]=i.lid;
  }



#undef COMPUTE_IRANK
#undef COMPUTE_INODE

}



/**/

/*
void CommPattern::setup()
{

  // get stuff
  const CPint irank=(CPint)PE::Comm::instance().rank();
  const CPint nproc=(CPint)PE::Comm::instance().size();
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
  PE::Comm::instance().all_to_all(receive_gids,m_recvCount,m_sendMap,m_sendCount);

//PECheckPoint(100,"-- step 3 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_sendCount,m_sendCount.size()));
//PEProcessSortedExecute(-1,PEDebugVector(m_sendMap,m_sendMap.size()));

  // Reverse GID mapping
  typedef std::map<Uint, Uint> GidMapT;
  GidMapT gid_reverse;
  const int gid_count = m_gid->size();
  for(int i=0; i<gid_count; i++)
    gid_reverse[gid[i]] = i;
  
  BOOST_FOREACH(int& si, m_sendMap)
  {
    bool found = false;
    GidMapT::const_iterator match = gid_reverse.find(si);
    if(match != gid_reverse.end())
      si = match->second;
    else
      throw ValueNotFound(FromHere(), "requested global id " + to_str(si) + " not found in gid list" );
  }

//PECheckPoint(100,"-- step 4 --:");
//PEProcessSortedExecute(-1,PEDebugVector(m_sendMap,m_sendMap.size()));

}
/**/

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize_all()
{
  std::vector<unsigned char> sndbuf(0);
  std::vector<unsigned char> rcvbuf(0);
  BOOST_FOREACH( CommWrapper& pobj, find_components_recursively<CommWrapper>(*this) )
  {
    synchronize_this(pobj,sndbuf,rcvbuf);
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize( const std::string& name )
{
  std::vector<unsigned char> sndbuf(0);
  std::vector<unsigned char> rcvbuf(0);
  CommWrapper& pobj = get_child(name).as_type<CommWrapper>();
  synchronize_this(pobj,sndbuf,rcvbuf);
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::synchronize( const CommWrapper& pobj )
{
  std::vector<unsigned char> sndbuf(0);
  std::vector<unsigned char> rcvbuf(0);
  synchronize_this(pobj,sndbuf,rcvbuf);
}

////////////////////////////////////////////////////////////////////////////////

// having the vectors for the intermediate buf coming from outside allows keeping them and reuse for all synchronize
void CommPattern::synchronize_this( const CommWrapper& pobj, std::vector<unsigned char>& sndbuf, std::vector<unsigned char>& rcvbuf )
{
//  std::cout << PERank << pobj.name() << "\n" << std::flush;
//  std::cout << PERank << pobj.needs_update() << "\n" << std::flush;
  if ( pobj.needs_update() )
  {
    pobj.pack(sndbuf,m_sendMap);
    rcvbuf.resize(m_recvMap.size()*pobj.size_of()*pobj.stride());
    PE::Comm::instance().all_to_all(sndbuf,m_sendCount,rcvbuf,m_recvCount,pobj.size_of()*pobj.stride());
    pobj.unpack(rcvbuf,m_recvMap);
  }
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::add_global(Uint gid, Uint rank)
{
  // later a mechanism could be implemented when commpattern can give gids by calling a "reserve(int num)" beforehand, to optimize performance
  // submits NEGATIVE lid's to distuingish add_global and add_local
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to add nodes to commpattern '" + name() + "' which is freezed.");
  int next_lid=m_free_lids.back();
  if (m_free_lids.size()>1) m_free_lids.pop_back();
  else m_free_lids[0]=next_lid+1;
  m_add_buffer.push_back(temp_buffer_item(-next_lid,gid,rank,(rank==PE::Comm::instance().rank())));
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////

Uint CommPattern::add_local(bool as_ghost)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to add nodes to commpattern '" + name() + "' which is freezed.");
  int next_lid=m_free_lids.back();
  if (m_free_lids.size()>1) m_free_lids.pop_back();
  else m_free_lids[0]=next_lid+1;
  m_add_buffer.push_back(temp_buffer_item(next_lid,std::numeric_limits<Uint>::max(),PE::Comm::instance().rank(),as_ghost));
  m_isUpToDate=false;
  return next_lid;
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::move_local(Uint lid, Uint rank, bool keep_as_ghost)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to moves nodes of commpattern '" + name() + "' which is freezed.");
  m_mov_buffer.push_back(temp_buffer_item(lid,std::numeric_limits<Uint>::max(),rank,keep_as_ghost));
  if (!keep_as_ghost) m_free_lids.push_back(lid);
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////

void CommPattern::remove_local(Uint lid, bool on_all_ranks)
{
  if (m_isFreeze) throw Common::ShouldNotBeHere(FromHere(),"Wanted to delete nodes from commpattern '" + name() + "' which is freezed.");
  m_rem_buffer.push_back(temp_buffer_item(lid,std::numeric_limits<Uint>::max(),PE::Comm::instance().rank(),on_all_ranks));
  m_free_lids.push_back(lid);
  m_isUpToDate=false;
}

////////////////////////////////////////////////////////////////////////////////
// Component related
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

} // PE
} // Common
} // CF
