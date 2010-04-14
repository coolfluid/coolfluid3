#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Common/ConnectivityTable.hh"
#include "Common/SwapEmpty.hh"

#include "MathTools/RCM.h"

using namespace std;
using namespace COOLFluiD;
using namespace COOLFluiD::Common;


/////////////////////////////////////////////////////////////////////////////

//.......... RCM_algorithm begins here.............

void RCM::renumber ( ConnectivityTable<CFuint>& cellnode, std::valarray <CFuint>& new_id )
{
  ConnectivityTable<CFuint> nodenode;

  RCM::transformCellNode2NodeNode (cellnode, nodenode);

  const CFuint nbelems = cellnode.nbRows();
  const CFuint nnodes  = nodenode.nbRows();

  CFuint readcount   = 0; // readcount = "main counter" on new[i]
  CFuint writecount  = 0; // counter of nodes filling newV[]..

  std::valarray < CFuint > newV ( ( CFuint ) 0, nnodes );
	std::valarray < CFuint > newV_R ( ( CFuint ) 0, nnodes ); // Used in Reverse CUTHILL MCKEE

	std::valarray<bool> flag ( false , nnodes );


// ############################################################################
// #                            MAIN CYCLE                                    #
// ############################################################################
	std::cout << "Started renumbering ... " << std::endl;


	CFuint counter=0; //just a counter..
	while ( readcount < nnodes )
	{

		if ( readcount == 0 )
		{

//............. SEARCHING FOR THE STARTING NODE ................
			CFuint least_conn = std::numeric_limits < CFuint >::max();
			CFuint ileast=0;

			for ( CFuint i=0; i<nnodes; ++i )
			{
        const CFuint nbcols = nodenode.nbCols ( i );
				if ( (  nbcols < least_conn ) && ( !flag[i] ) )
				{
					least_conn = nbcols;
					ileast=i;
				}
			};

			//..insert into NewV[]
			newV[writecount]=ileast;
			flag[ileast]=true;                  //..so far OK

			writecount++;

// 			std::cout << "ileast  " <<ileast<< std::endl;
//..............................................................

//.... inserting "first-level" nodes into newV[] .... (1)
			CFuint mino=0,minn=0;
			CFuint ccount=0;

			for ( CFuint i=0; i < nodenode.nbCols ( newV[readcount] ); i++ )
			{
				mino = std::numeric_limits < CFuint >::max();
				minn=mino;

				for ( CFuint j=0; j < nodenode.nbCols ( newV[readcount] ); j++ )
				{
					if ( ( nodenode.nbCols ( nodenode ( newV[readcount],j ) ) <= minn ) && ( !flag[nodenode ( newV[readcount],j ) ] ) )
					{
						newV[writecount+i] = nodenode ( newV[readcount],j );
						minn = nodenode.nbCols ( nodenode ( newV[readcount],j ) );
					}
				}
				if ( minn != mino )
				{
					flag[newV[writecount+i]]=true;
					ccount++;
				}

			}

			writecount= writecount + ccount;
			readcount++;
//-----------------------------------------------------


		} // end if ......1ST ITERATION (placing starting node..)



//Now, I have to repeat all the operations done in the first iteration
// for all the other nodes, but this time, nodes have to be taken from newV[i]....


//.... inserting "first-level" nodes into newV[] .... (main)
		CFuint mino=0,minn=0;
		CFuint ccount=0;

		for ( CFuint i=0; i < nodenode.nbCols ( newV[readcount] ); i++ )
		{
			mino = std::numeric_limits < CFuint >::max();
			minn=mino;

			for ( CFuint j=0; j < nodenode.nbCols ( newV[readcount] ); j++ )
			{

				if ( ( nodenode.nbCols ( nodenode ( newV[readcount],j ) ) <= minn ) && ( !flag[nodenode ( newV[readcount],j ) ] ) )
				{
					newV[writecount+i] = nodenode ( newV[readcount],j );
					minn = nodenode.nbCols ( nodenode ( newV[readcount],j ) );
				}
			}
			if ( minn != mino )
			{
				flag[newV[writecount+i]]=true;
				ccount++;
			}
		}

		writecount= writecount + ccount;
		++readcount;
		++counter;
// 		if ( ( counter % 100 ) == 0 ) { std::cout << counter << "\n"; }
//...end cycle on newV[i]......}
	}

//-------------------------------------------------------------



//----------------- REVERSE CUTHILL MCKEE ---------------------
	for ( CFuint i=0; i<nnodes ; ++i )
	{
		newV_R[nnodes-i-1]=newV[i];
	}

	for ( CFuint i=0; i<nnodes; ++i )
	{
		newV[i]=newV_R[i];
	}
//-------------------------------------------------------------


  // make sure that new_id has same number of rows as cellnode
  new_id.resize(nnodes);
  // assign new ids
	for ( CFuint i=0;i<nnodes; ++i )
	{
		new_id [ newV[i] ] = i;
	}

//--------------------------------------------------------------------------
//                      REWRITE THE TABLE nodenode                               -
//--------------------------------------------------------------------------
	for ( CFuint i=0; i<nnodes; ++i )
	{
    const CFuint nbcols = nodenode.nbCols ( i );
		for ( CFuint j=0; j< nbcols; ++j )
		{
			nodenode ( i,j ) = new_id[nodenode ( i,j ) ];
		}
	}
//--------------------------------------------------------------------------



//--------------------------------------------------------------------------
//                      REWRITE THE TABLE cellnode                               -
//--------------------------------------------------------------------------
	for ( CFuint i=0; i< nbelems; ++i )
	{
    const CFuint nbcols = cellnode.nbCols ( i );
		for ( CFuint j=0; j<nbcols; ++j )
		{
			cellnode ( i,j ) = new_id[cellnode ( i,j ) ];
		}
	}
//---------------------------------------------------------------------------

  std::cout << "Finished renumbering" << std::endl;

// function ends
}

/////////////////////////////////////////////////////////////////////////////

void RCM::transformCellNode2NodeNode ( const ConnectivityTable<CFuint>& cellnode, ConnectivityTable<CFuint>& nodenode )
{
  CFuint maxN = 0;
  const CFuint nbrows = cellnode.nbRows();


//........counting number of nodes stored in table cellnode .........
  for ( CFuint i = 0; i < nbrows; ++i )
  {
    const CFuint nbcols = cellnode.nbCols ( i );
    for ( CFuint j = 0; j<nbcols; ++j )
    {
      const CFuint nn = cellnode ( i,j );
      if ( nn >= maxN ) { maxN = nn; }
    }
  }

  const CFuint nnodes = maxN+1;

//.............................................................

// construct array with nb of neighbours for each node
// this is over calculated, because nodes shared between elements get accounted more then once..
  std::valarray <CFuint> maxnb_neighb ( nnodes );

  for ( CFuint i = 0; i < nbrows; ++i )
  {
    const CFuint nbcols = cellnode.nbCols ( i );
    cf_assert ( nbcols > 0 );
    const CFuint nnei = nbcols;
    for ( CFuint j = 0; j < nbcols; ++j )
    {
      const CFuint nn = cellnode ( i,j );
      maxnb_neighb[nn] = maxnb_neighb[nn] + nnei;
    }
  }
//.............................................................


//.... data structure to store the true nb of neighbours .......
  std::vector < std::vector<CFuint> > true_neigh ( nnodes );
  for ( CFuint i = 0; i < nnodes; ++i )
  {
    true_neigh[i].reserve ( maxnb_neighb[i] );
  }

  for ( CFuint i = 0; i < nbrows; i++ )
  {
    const CFuint nbcols = cellnode.nbCols ( i );
    for ( CFuint j = 0; j < nbcols; j++ )
    {
      for ( CFuint k = 0; k < nbcols; k++ )
      {
        true_neigh[cellnode ( i,j ) ].push_back ( cellnode ( i,k ) );
      }
    }
  }

  for ( CFuint i = 0; i < nnodes; ++i )
  {
    // sort the vector so we can then remove duplicated ids
    std::sort ( true_neigh[i].begin(), true_neigh[i].end(), std::less<CFuint>() );
    // place duplicated ids in end of vector
    std::vector<CFuint>::iterator last_id = std::unique ( true_neigh[i].begin(),true_neigh[i].end() );
    // remove duplicated id
    true_neigh[i].erase ( last_id,true_neigh[i].end() );
  }
//.............................................................

//...here I create the table nodenode to store State to State connectivity.....
//......the pattern first......

  std::valarray <CFuint> pat_node ( nnodes );
  for ( CFuint n = 0; n < nnodes; ++n )
  {
    pat_node[n] = true_neigh[n].size();
  }
  //... and then the table itself..........
  nodenode.resize ( pat_node );
  for ( CFuint n = 0; n < nnodes; n++ )
  {
    const CFuint nneig = nodenode.nbCols ( n );
    for ( CFuint j = 0; j < nneig; j++ )
    {
      nodenode ( n,j ) = true_neigh[n][j];
    }
  }

  // enforce deallocation of memory in std::vector
  for ( CFuint i = 0; i < nnodes; ++i )
  {
    Common::SwapEmpty ( true_neigh[i] );
  }
  Common::SwapEmpty ( true_neigh );

}

/////////////////////////////////////////////////////////////////////////////

int RCM::read_input ( const std::string& filename, ConnectivityTable<CFuint>& cellnode )
{
	CFuint nb_elems;

	std::ifstream file_in ( filename.c_str() );
	if ( ! file_in.is_open() )
	{
		std::cout << "Could not open file " <<  filename << std::endl;
		throw std::string ("Could not open file " +  filename);
	}

	file_in >> nb_elems;
	std::cout << "Nb Elms: " << nb_elems << std::endl;

	std::valarray <CFuint> pattern ( nb_elems );
	for ( CFuint i = 0; i < nb_elems; ++i ) // read pattern
	{
		file_in >> pattern[i];
	}

	cellnode.resize ( pattern );
	file_in >> cellnode;

  return 0;
}

/////////////////////////////////////////////////////////////////////////////

void RCM::print_table ( const std::string& filename, const ConnectivityTable<CFuint>& cellnode )
{

  ConnectivityTable<CFuint> nodenode;

  transformCellNode2NodeNode (cellnode, nodenode);

  const CFuint nnodes  = nodenode.nbRows();

//-------- write nodes connectivity in "filename"-------------
	ofstream inputTEC ( filename.c_str() );
	if ( !inputTEC.is_open() )
	{
    throw std::string ("Unable to open file '" +  filename + "' to write data");
  }

		inputTEC << "x " <<"y " << "\n";

		for ( CFuint n = 0; n < nnodes; n++ )
		{
			const CFuint nneig = nodenode.nbCols ( n );
			for ( CFuint j=0; j<nneig; ++j )
			{
          inputTEC <<  n << " " << nodenode ( n,j )  <<"\n";
			}
		}
		inputTEC.close();

	std::cout << "Wrote file " << filename << std::endl;
}

/////////////////////////////////////////////////////////////////////////////


