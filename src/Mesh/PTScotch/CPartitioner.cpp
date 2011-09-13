// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

// coolfluid
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/MPI/PE.hpp"
#include "Mesh/PTScotch/CPartitioner.hpp"

namespace CF {
namespace Mesh {
namespace PTScotch {

  using namespace Common;
  using namespace Common::Comm;

////////////////////////////////////////////////////////////////////////////////

CF::Common::ComponentBuilder < CPartitioner, CMeshPartitioner, LibPTScotch > ptscotch_partitioner_builder;

//////////////////////////////////////////////////////////////////////////////

CPartitioner::CPartitioner ( const std::string& name ) :
    CMeshPartitioner(name)
{
  // initialize the graph
  if (SCOTCH_dgraphInit(&graph, PE::instance().communicator()))
    throw BadValue(FromHere(),"ptscotch error");
}

//////////////////////////////////////////////////////////////////////////////

CPartitioner::~CPartitioner ( )
{
  // delete the graph
  SCOTCH_dgraphExit(&graph);
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::build_graph()
{
  CF_DEBUG_POINT;

  // resize vertloctab to the number of owned objects
  // +1 because of compact form without holes in global numbering
  vertloctab.resize(nb_objects_owned_by_part(PE::instance().rank())+1,0);

  // copy number of outgoing edges per object in vertloctab
  nb_connected_objects_in_part(PE::instance().rank(),vertloctab);

  // baseval is C style
  baseval = 0;

  // set local number of objects
  vertlocnbr = nb_objects_owned_by_part(PE::instance().rank());

  // set maximum local number of objects
  vertlocmax = vertlocnbr;

  // Convert vertloctab to pt-scotch format
  Uint nb_edges;
  Uint total_nb_edges = 0;
  for (int i=0; i<vertlocnbr; ++i)
  {
    nb_edges=vertloctab[i];
    vertloctab[i]=total_nb_edges;
    total_nb_edges+=nb_edges;
  }
  vertloctab[vertlocnbr] = total_nb_edges;

  // total number of outgoing edges
  edgelocnbr = total_nb_edges;
  edgelocsiz = total_nb_edges;
  edgegsttab.resize(total_nb_edges);
  edgeloctab.resize(total_nb_edges);

  cf_assert(edgelocsiz >= vertloctab[vertlocnbr]);

  list_of_connected_objects_in_part(PE::instance().rank(),edgeloctab);

  if (SCOTCH_dgraphBuild(&graph,
                         baseval,
                         vertlocnbr,      // number of local vertices (for creation of proccnttab)
                         vertlocmax,          // max number of local vertices to be created (for creation of procvrttab)
                         &vertloctab[0],  // local adjacency index array (size = vertlocnbr+1 if vendloctab matches or is null)
                         &vertloctab[1],  //   (optional) local adjacency end index array
                         NULL, //veloloctab,  //   (optional) local vertex load array
                         NULL,  //vlblocltab,  //   (optional) local vertex label array (size = vertlocnbr+1)
                         edgelocnbr,      // total number of arcs (twice number of edges)
                         edgelocsiz,      // minimum size of the edge array required to encompass all used adjacency values (at least equal to the max of vendloctab entries)
                         &edgeloctab[0],  // edgeloctab,  local adjacency array which stores global indices
                         &edgegsttab[0],  // edgegsttab,  //   (optional) if passed it is assumed an empty array that will be filled by SCOTHC_dgraphGhst if required
                         NULL))   //edloloctab)) //   (optional) arc load array of size edgelocsiz
    throw BadValue(FromHere(),"Could not build PT-scotch graph");


    SCOTCH_dgraphSize(&graph,
                      &vertglbnbr,
                      &vertlocnbr,
                      &edgeglbnbr,
                      &edgelocnbr);

    proccnttab.resize(PE::instance().size());
    procvrttab.resize(PE::instance().size()+1);

    CF_DEBUG_POINT;
    //boost::mpi::communicator world;
    //boost::mpi::all_gather(world, vertlocnbr, proccnttab);
    PE::instance().all_gather(vertlocnbr, proccnttab);

    CF_DEBUG_POINT;

    Uint cnt=0;
    for (Uint p=0; p<proccnttab.size(); ++p)
    {
      procvrttab[p] = cnt;
      cnt += proccnttab[p];
    }
    procvrttab[PE::instance().size()] = cnt;

  // CFinfo << "\n" << CFendl;
  // CFinfo << "global graph info" << CFendl;
  // CFinfo << "-----------------" << CFendl;
  // CFLogVar(vertglbnbr);
  // CFLogVar(edgeglbnbr);
  // CFinfo << "proccnttab = [ ";
  // for (Uint i=0; i<PE::instance().size(); ++i)
  //   CFinfo << proccnttab[i] << " ";
  // CFinfo << "]" << CFendl;
  // CFinfo << "procvrttab = [ ";
  // for (Uint i=0; i<PE::instance().size()+1; ++i)
  //   CFinfo << procvrttab[i] << " ";
  // CFinfo << "]" << CFendl;
  //
  // CFinfo << CFendl << CFendl;
  //
  if (SCOTCH_dgraphCheck(&graph))
   throw BadValue(FromHere(),"There is an error in the PT-scotch graph");
}

//////////////////////////////////////////////////////////////////////////////

void CPartitioner::partition_graph()
{
  //PECheckPoint(1,"begin partition_graph()");
  CF_DEBUG_POINT;

  SCOTCH_Strat stradat;
  if(SCOTCH_stratInit(&stradat))
    throw BadValue (FromHere(), "Could not initialze a PT-scotch strategy");

  partloctab.resize(vertlocmax);
  CF_DEBUG_POINT;

  //PECheckPoint(1,"  begin SCOTCH_dgraphPart()");
  if (SCOTCH_dgraphPart(&graph,
                       option("nb_parts").value<Uint>(),
                       &stradat,
                       &partloctab[0]))
    throw BadValue (FromHere(), "Could not partition PT-scotch graph");
  //PECheckPoint(1,"  end SCOTCH_dgraphPart()");
  SCOTCH_stratExit(&stradat);
  CF_DEBUG_POINT;

  std::vector<Uint> owned_objects(vertlocnbr);
  list_of_objects_owned_by_part(PE::instance().rank(),owned_objects);

//  Uint nb_changes = 0;
//  for (int i=0; i<vertlocnbr; ++i)
//  {
//    if ((Uint) partloctab[i] != PE::instance().rank())
//    {
//      ++nb_changes;
//    }
//  }

//  m_changes->reserve(nb_changes);
//  for (int i=0; i<vertlocnbr; ++i)
//  {
//    if ((Uint)partloctab[i] != PE::instance().rank())
//    {
//      m_changes->insert_blindly(owned_objects[i],partloctab[i]);
//    }
//  }

//  m_changes->sort_keys();
  CF_DEBUG_POINT;


  Uint comp; Uint loc_idx; bool found;
  for (int i=0; i<vertlocnbr; ++i)
  {
    if ((Uint)partloctab[i] != PE::instance().rank())
    {
      boost::tie(comp,loc_idx) = location_idx(owned_objects[i]);
      if (comp == 0) // if is node
        m_nodes_to_export[partloctab[i]].push_back(loc_idx);
      else
        m_elements_to_export[comp-1][partloctab[i]].push_back(loc_idx);
    }
  }

  // PECheckPoint(1,"end partition_graph()");
}

//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////

} // PTScotch
} // Mesh
} // CF
