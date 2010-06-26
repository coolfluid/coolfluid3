#include <string>
#include <fstream>

#include <boost/assign/list_of.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/foreach.hpp>

#include "Common/Exception.hpp"
#include "Common/Log.hpp"

#include "Mesh/OpenFOAM/BlockMeshReader.hpp"
#include "Mesh/CArray.hpp"

BOOST_FUSION_ADAPT_STRUCT(
  CF::Mesh::OpenFOAM::BlockData,
  (CF::Real, scalingFactor)
  (std::vector<CF::Mesh::OpenFOAM::BlockData::PointT>, points)
  (std::vector<CF::Mesh::OpenFOAM::BlockData::IndicesT>, blockPoints)
  (std::vector<CF::Mesh::OpenFOAM::BlockData::CountsT>, blockSubdivisions)
  (std::vector<CF::Mesh::OpenFOAM::BlockData::GradingT>, blockGradings)
  (std::vector<std::string>, patchTypes)
  (std::vector<std::string>, patchNames)
  (std::vector<CF::Mesh::OpenFOAM::BlockData::IndicesT>, patchPoints)
)

namespace CF {
namespace Mesh {
namespace OpenFOAM {

using namespace CF::Common;

/// True if the given block facet is part if a boundary patch
template<typename FaceT, typename PatchesT>
bool isPatch(const FaceT& facePoints, const PatchesT& patchPoints) {
  FaceT rotatingPoints = facePoints; // copy that we can rotate, since block and patch index ordering may not match
  const Uint facePointCount = facePoints.size();

  for(Uint rot = 0; rot != facePoints.size(); ++rot) { // for all possible orderings of the block face
    std::rotate(rotatingPoints.begin(), rotatingPoints.begin()+rot, rotatingPoints.end());
    for(Uint patch = 0; patch != patchPoints.size(); ++patch) { // for all boundary patches
      const Uint patchNbFaces = patchPoints[patch]/4;
      for(Uint patchFace = 0; patchFace != patchNbFaces; ++patchFace) { // for all "faces" on the boundary patch
        if(std::equal(rotatingPoints.begin(), rotatingPoints.end(), patchPoints[patch].begin()+4*patchFace))
          return true;
      }
    }
  }
  return false;
}

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
          >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> double_[push_back(_val, _1)] >> ')';
    grading %= simpleGrading | edgeGrading;

    patchTypes %= lit("cyclic") | lit("wall");
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
void expandSimpleGradings(std::vector<BlockData::GradingT>& Gradings)
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

void readBlockMeshFile(std::fstream& file, BlockData& blockData)
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
    CFinfo << "Error parsing blockMeshDict file\n";
  else {
    expandSimpleGradings(blockData.blockGradings);
    CFinfo << "scaling factor: " << blockData.scalingFactor << "\n";
    CFinfo << "points: ";
    for(Uint i = 0; i != blockData.points.size(); ++i)
      CFinfo << "\n" << blockData.points[i][0] << ", " << blockData.points[i][1] << ", " << blockData.points[i][2];
    CFinfo << "\npoint indices: ";
    for(Uint i = 0; i != blockData.blockPoints.size(); ++i)
    {
      CFinfo << "\n";
      for(Uint j = 0; j != blockData.blockPoints[i].size(); ++j)
        CFinfo << " " << blockData.blockPoints[i][j];
    }
    CFinfo << "\nblockSubdivisions: ";
    for(Uint i = 0; i != blockData.blockSubdivisions.size(); ++i)
    {
      CFinfo << "\n";
      for(Uint j = 0; j != blockData.blockSubdivisions[i].size(); ++j)
        CFinfo << " " << blockData.blockSubdivisions[i][j];
    }
    CFinfo << "\nblockGradings: ";
    for(Uint i = 0; i != blockData.blockGradings.size(); ++i)
    {
      CFinfo << "\n";
      for(Uint j = 0; j != blockData.blockGradings[i].size(); ++j)
        CFinfo << " " << blockData.blockGradings[i][j];
    }
    CFinfo << "\npatchTypes:";
    for(Uint i = 0; i !=  blockData.patchTypes.size(); ++i)
    {
      CFinfo << " " << blockData.patchTypes[i];
    }
    CFinfo << "\npatchNames:";
    for(Uint i = 0; i !=  blockData.patchNames.size(); ++i)
    {
      CFinfo << " " << blockData.patchNames[i];
    }
    CFinfo << "\npatch point indices: ";
    for(Uint i = 0; i != blockData.patchPoints.size(); ++i)
    {
      CFinfo << "\n";
      for(Uint j = 0; j != blockData.patchPoints[i].size(); ++j)
        CFinfo << " " << blockData.patchPoints[i][j];
    }
  }
}

void buildMesh(const BlockData& blockData, CMesh& mesh)
{
  // Create the coordinate table
  CArray& coordinates = *mesh.create_array("coordinates");
  const Uint dim = computeDimensionality(blockData);
  coordinates.initialize(dim);
  CArray::Buffer coordinatesBuffer = coordinates.create_buffer();
  const Uint blocks_begin = 0;
  const Uint blocks_end = blockData.blockPoints.size();
  for(Uint block = blocks_begin; block != blocks_end; ++block)
  {
    //const Uint x_segments = blockData.blockSubdivisions[block][XX];
    //const Uint y_segments = blockData.blockSubdivisions[block][YY];
    const Uint z_segments = blockData.blockSubdivisions[block][ZZ];
    for(Uint k = 0; k != z_segments; ++k)
    {

    }
  }
}

} // namespace OpenFOAM
} // namespace Mesh
} // namespace CF
