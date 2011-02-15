// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>

namespace CF {
  namespace Mesh {
    class CMeshReader;
    class CMeshWriter;
    class CMeshTransformer;
  }
namespace Tools {
namespace MeshTransformer {
  
////////////////////////////////////////////////////////////////////////////////

class Transformer
{
public:
  
  typedef std::pair<std::string,std::vector<boost::shared_ptr<Mesh::CMeshReader> > > extensions_to_readers_pair_t;
	typedef std::pair<std::string,std::vector<boost::shared_ptr<Mesh::CMeshWriter> > > extensions_to_writers_pair_t;
	typedef std::pair<std::string,std::string> transformers_description_t;


public:
  
  Transformer();

  void command(boost::program_options::parsed_options& parsed);
  
private:
  	
	std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshReader> > > extensions_to_readers;
	std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshWriter> > > extensions_to_writers;
	std::vector<boost::shared_ptr<Mesh::CMeshReader> > readers;
	std::vector<boost::shared_ptr<Mesh::CMeshWriter> > writers;
	std::map<std::string,std::string> transformers_description;
	std::map<std::string,boost::shared_ptr<Mesh::CMeshTransformer> > name_to_transformers;

};

////////////////////////////////////////////////////////////////////////////////

} // MeshTransformer
} // Tools
} // CF
