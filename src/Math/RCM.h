#ifndef COOLFluiD_MathTools_RCM_h
#define COOLFluiD_MathTools_RCM_h

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Common/ConnectivityTable.hh"
#include "Common/SwapEmpty.hh"
#include "MathTools/MathTools.hh"

using namespace std;
using namespace COOLFluiD;
using namespace COOLFluiD::Common;

class MathTools_API RCM
{
	/// Transforms a connectivity from Cell to Node to Node to Node
	static void transformCellNode2NodeNode ( const ConnectivityTable<CFuint>& cellnode, ConnectivityTable<CFuint>& nodenode );

	/// Applies the Reverse Cuthill-McKee algorithm to the graph of the connectivity passed
	/// @param new_id is a vector with the new id numbers
	static void renumber ( ConnectivityTable<CFuint>& tb, std::valarray <CFuint>& new_id );
	
	/// reads the a cell to node connectivity from the file
	static int read_input ( const std::string& filename, ConnectivityTable<CFuint>& cellnode );

	/// prints the connectivity to a file
	static void print_table ( const std::string& filename, const ConnectivityTable<CFuint>& tb );
	
};

#endif // COOLFluiD_MathTools_RCM_h
