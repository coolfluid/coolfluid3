#ifndef CF_Math_RCM_hpp
#define CF_Math_RCM_hpp

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include "Common/ConnectivityTable.hpp"
#include "Math/Math.hpp"

using namespace std;
using namespace CF;
using namespace CF::Common;

class Math_API RCM
{
/// Transforms a connectivity from Cell to Node to Node to Node
static void transformCellNode2NodeNode ( const ConnectivityTable<Uint>& cellnode, ConnectivityTable<Uint>& nodenode );

/// Applies the Reverse Cuthill-McKee algorithm to the graph of the connectivity passed
/// @param new_id is a vector with the new id numbers
static void renumber ( ConnectivityTable<Uint>& tb, std::valarray <Uint>& new_id );

/// reads the a cell to node connectivity from the file
static int read_input ( const std::string& filename, ConnectivityTable<Uint>& cellnode );

/// prints the connectivity to a file
static void print_table ( const std::string& filename, const ConnectivityTable<Uint>& tb );

};

#endif // CF_Math_RCM_hpp
