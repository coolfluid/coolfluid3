// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <string>
#include <fstream>

#define FUSION_MAX_VECTOR_SIZE 12 // For edgeGrading

#include <boost/assign/list_of.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/multi_array.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include "common/BasicExceptions.hpp"
#include "common/Table.hpp"
#include "mesh/BlockMesh/BlockData.hpp"

#include "Parser.hpp"

namespace cf3 {
namespace BlockMeshReader {

/// Helper struct to read the data into vectors
struct BlockData
{
  /// Type to store indices into another vector
  typedef std::vector<Uint> IndicesT;
  /// Data type for counts of data, i.e. number of points
  typedef std::vector<Uint> CountsT;
  /// Storage for a single point coordinate (STL vector for ease of use with boost::spirit)
  typedef std::vector<Real> PointT;
  /// Storage for a grading corresponding to a single block
  typedef std::vector<Real> GradingT;
  /// Storage for true/false flags
  typedef std::vector<bool> BooleansT;

  Real scaling_factor;

  /// The coordinates for all the nodes
  std::vector<PointT> points;

  /// Points for each block, in terms of node indices
  std::vector<IndicesT> block_points;
  /// Subdivisions for each block, along X, Y and Z
  std::vector<CountsT> block_subdivisions;
  /// edgeGrading for each block
  std::vector<GradingT> block_gradings;

  std::vector<std::string> patch_types;
  /// Name for each patch
  std::vector<std::string> patch_names;
  /// Point indices for each patch (grouped per 4)
  std::vector<IndicesT> patch_points;
};


}
}

BOOST_FUSION_ADAPT_STRUCT(
  cf3::BlockMeshReader::BlockData,
  (cf3::Real, scaling_factor)
  (std::vector<cf3::BlockMeshReader::BlockData::PointT>, points)
  (std::vector<cf3::BlockMeshReader::BlockData::IndicesT>, block_points)
  (std::vector<cf3::BlockMeshReader::BlockData::CountsT>, block_subdivisions)
  (std::vector<cf3::BlockMeshReader::BlockData::GradingT>, block_gradings)
  (std::vector<std::string>, patch_types)
  (std::vector<std::string>, patch_names)
  (std::vector<cf3::BlockMeshReader::BlockData::IndicesT>, patch_points)
)

namespace cf3 {
namespace BlockMeshReader {
  
using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::mesh::BlockMesh;

namespace fusion = boost::fusion;
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;



/// Grammar for white-space and comment skipping
template<typename Iterator>
struct WhiteSpace: qi::grammar<Iterator> {
  WhiteSpace() :
    WhiteSpace::base_type(start) {
    using boost::spirit::ascii::char_;

    start = ascii::space // tab/space/cr/lf
        | "/*" >> *(char_ - "*/") >> "*/" // C-style comments
        | "//" >> *(char_ - boost::spirit::eol) >> boost::spirit::eol // C++-style comments
    ;
  }

  qi::rule<Iterator> start;
};

/// Grammar for OpenFOAM blockMeshDict files
template <typename Iterator>
struct BlockMeshGrammar : qi::grammar<Iterator, BlockData(), WhiteSpace<Iterator> > {
  typedef WhiteSpace<Iterator> WhiteSpaceT;

  BlockMeshGrammar() : BlockMeshGrammar::base_type(blockData) {
    using qi::double_;
    using qi::lit;
    using qi::lexeme;
    using qi::uint_;
    using ascii::char_;
    using ascii::string;
    using namespace qi::labels;
    using phoenix::at_c;
    using phoenix::push_back;
    using boost::spirit::_1;

    scaling = "convertToMeters" >>  double_[_val = _1] >>  ';';
    header = lit("FoamFile") >> '{' >> *(char_ - '}') >> '}'; // header is skipped

    point = '(' >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> ')';
    points = lit("vertices") >> '(' >> *point[push_back(_val, _1)] >> lit(')') >> lit(';');

    blockPoints = lit("hex") >> '(' >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)] >> ')';
    cellCounts = '('  >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)]
                      >> uint_[push_back(_val, _1)] >> ')';
    simpleGrading = lit("simpleGrading") >> '('
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> ')';
    edgeGrading = lit("edgeGrading") >> '('
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)]
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)]
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)]
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> ')';
    grading %= simpleGrading | edgeGrading;

    patchTypes %= lit("cyclic") | lit("wall") | lit("empty") | lit("patch") | lit("symmetryPlane") | lit("wedge") | lit("processor");
    patchNames %= +(char_ - '(');
    patchPoints = lit('(') >> +(lit('(') >> uint_[push_back(_val, _1)]
                               >> uint_[push_back(_val, _1)]
                               >> uint_[push_back(_val, _1)]
                               >> uint_[push_back(_val, _1)] >> lit(')')) >> lit(')');

    edges = lit("edges") >> '(' >> *(char_ - ')') >> ')' >> lit(';'); //TODO: Support edges
    mergePatchPairs = lit("mergePatchPairs") >> '(' >> *(char_ - ')') >> ')' >> lit(';'); //TODO: Support mergePatchPairs

    blockData = header ^
                scaling[at_c<0>(_val) = _1] ^
                points[at_c<1>(_val) = _1] ^
                (lit("blocks") >> '(' >> *(blockPoints[push_back(at_c<2>(_val), _1)] >> cellCounts[push_back(at_c<3>(_val), _1)] >> grading[push_back(at_c<4>(_val), _1)]) >> ')' >> lit(';') ) ^
                (lit("patches") >> '(' >> *(patchTypes[push_back(at_c<5>(_val), _1)] >> patchNames[push_back(at_c<6>(_val), _1)] >> patchPoints[push_back(at_c<7>(_val), _1)]) >> ')' >> lit(';') ) ^
                edges ^ mergePatchPairs;

  }

  qi::rule<Iterator, double(), WhiteSpaceT> scaling;
  qi::rule<Iterator, std::string(), WhiteSpaceT> header;

  qi::rule<Iterator, BlockData::PointT(), WhiteSpaceT> point;
  qi::rule<Iterator, std::vector<BlockData::PointT>(), WhiteSpaceT> points;

  qi::rule<Iterator, BlockData::IndicesT(), WhiteSpaceT> blockPoints;
  qi::rule<Iterator, BlockData::CountsT(), WhiteSpaceT> cellCounts;
  qi::rule<Iterator, BlockData::GradingT(), WhiteSpaceT> simpleGrading;
  qi::rule<Iterator, BlockData::GradingT(), WhiteSpaceT> edgeGrading;
  qi::rule<Iterator, BlockData::GradingT(), WhiteSpaceT> grading;

  qi::rule<Iterator, std::string(), WhiteSpaceT> patchTypes;
  qi::rule<Iterator, std::string(), WhiteSpaceT> patchNames;
  qi::rule<Iterator, BlockData::IndicesT(), WhiteSpaceT> patchPoints;

  qi::rule<Iterator, std::string(), WhiteSpaceT> edges;
  qi::rule<Iterator, std::string(), WhiteSpaceT> mergePatchPairs;


  qi::rule<Iterator, BlockData(), WhiteSpaceT> blockData;
};

/// Expands simpleGrading vectors (3 elements) into full 12-element edgeGrading vectors
void expand_simple_gradings(std::vector<BlockData::GradingT>& Gradings)
{
  for(Uint i = 0; i != Gradings.size(); ++i)
  {
    BlockData::GradingT& g = Gradings[i];
    if(g.size() == 3)
    {
      BlockData::GradingT new_grading = boost::assign::list_of(g[0])(g[0])(g[0])(g[0])(g[1])(g[1])(g[1])(g[1])(g[2])(g[2])(g[2])(g[2]);
      Gradings[i] = new_grading;
    }
  }
}

void parse_blockmesh_dict(std::istream& file, BlockArrays& blocks)
{
  std::string storage; // We will read the contents here.
  file.unsetf(std::ios::skipws); // No white space skipping!
  std::copy(
    std::istream_iterator<char>(file),
    std::istream_iterator<char>(),
    std::back_inserter(storage));

  typedef BlockMeshGrammar<std::string::const_iterator> GrammarT;
  GrammarT blockMesh; // Our grammar
  WhiteSpace<std::string::const_iterator> ws; // whitespace skipper

  BlockData block_data;
  std::string::const_iterator iter = storage.begin();
  std::string::const_iterator end = storage.end();
  bool r = phrase_parse(iter, end, blockMesh, ws, block_data);
  if(!r)
    throw ParsingFailed(FromHere(), "Error parsing blockMeshDict file");
  expand_simple_gradings(block_data.block_gradings);
  
  // Translate to the BlockArrays structure
  const Uint nb_points = block_data.points.size();
  Table<Real>& points = *blocks.create_points(3, nb_points);
  for(Uint i = 0; i != nb_points; ++i)
  {
    points.set_row(i, block_data.points[i]);
  }

  const Uint nb_blocks = block_data.block_points.size();
  Table<Uint>& block_nodes = *blocks.create_blocks(nb_blocks);
  Table<Uint>& block_subdivisions = *blocks.create_block_subdivisions();
  Table<Real>& block_gradings = *blocks.create_block_gradings();
  for(Uint i = 0; i != nb_blocks; ++i)
  {
    block_nodes.set_row(i, block_data.block_points[i]);
    block_subdivisions.set_row(i, block_data.block_subdivisions[i]);
    block_gradings.set_row(i, block_data.block_gradings[i]);
  }

  const Uint nb_patches = block_data.patch_names.size();
  for(Uint i = 0; i != nb_patches; ++i)
  {
    const Uint patch_nb_elems = block_data.patch_points[i].size();
    Handle< Table<Uint> > patch_tbl = blocks.create_patch(block_data.patch_names[i], patch_nb_elems);
    patch_tbl->seekp(0);
    for(Uint j = 0; j != patch_nb_elems; ++j)
      (*patch_tbl) << block_data.patch_points[i][j];
    patch_tbl->seekp(0);
  }
}

} // BlockMeshReader
} // cf3
