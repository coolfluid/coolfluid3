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
  typedef boost::program_options::options_description commands_description;
  
private:
  typedef std::pair<std::string,std::vector<boost::shared_ptr<Mesh::CMeshReader> > > extensions_to_readers_pair_t;
	typedef std::pair<std::string,std::vector<boost::shared_ptr<Mesh::CMeshWriter> > > extensions_to_writers_pair_t;
	typedef std::pair<std::string,std::string> transformers_description_t;

public:
  
  Transformer();

  static void help( const std::string& param);
  static void input( const std::vector<std::string>& params );
  static void output( const std::vector<std::string>& params );
  static void transform( const std::vector<std::string>& params );
  
  static commands_description description();
};

////////////////////////////////////////////////////////////////////////////////

} // MeshTransformer
} // Tools
} // CF
