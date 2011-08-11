// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
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

#include "Common/BasicExceptions.hpp"
#include "Mesh/BlockMesh/BlockData.hpp"

#include "Parser.hpp"


BOOST_FUSION_ADAPT_STRUCT(
  CF::Mesh::BlockMesh::BlockData,
  (CF::Real, scaling_factor)
  (std::vector<CF::Mesh::BlockMesh::BlockData::PointT>, points)
  (std::vector<CF::Mesh::BlockMesh::BlockData::IndicesT>, block_points)
  (std::vector<CF::Mesh::BlockMesh::BlockData::CountsT>, block_subdivisions)
  (std::vector<CF::Mesh::BlockMesh::BlockData::GradingT>, block_gradings)
  (std::vector<std::string>, patch_types)
  (std::vector<std::string>, patch_names)
  (std::vector<CF::Mesh::BlockMesh::BlockData::IndicesT>, patch_points)
)

namespace CF {
namespace BlockMeshReader {

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Mesh::BlockMesh;

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

void parse_blockmesh_dict(std::istream& file, BlockData& blockData)
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

  std::string::const_iterator iter = storage.begin();
  std::string::const_iterator end = storage.end();
  bool r = phrase_parse(iter, end, blockMesh, ws, blockData);
  if(!r)
    throw ParsingFailed(FromHere(), "Error parsing blockMeshDict file");
  expand_simple_gradings(blockData.block_gradings);
  
  blockData.block_distribution.push_back(0);
  blockData.block_distribution.push_back(blockData.block_points.size());
}

} // BlockMeshReader
} // CF
