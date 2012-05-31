// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
// #include "common/BuildInfo.hpp"
#include "common/Factory.hpp"
#include "common/Builder.hpp"
// #include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"

#include "Tools/mesh_transformer/Help.hpp"
// #include "mesh/Field.hpp"

namespace cf3 {
namespace Tools {
namespace mesh_transformer {

  using namespace boost;
  using namespace boost::program_options;

  using namespace cf3;
  using namespace cf3::common;
  using namespace cf3::mesh;
  using namespace cf3::Tools::Shell;


////////////////////////////////////////////////////////////////////////////////

Help::Help(Shell::commands_description& commands) :
  Command( "help,h",
           "This help if no arg, or more detailed help of submodule",
           commands)
{
}

////////////////////////////////////////////////////////////////////////////////

void Help::execute( const std::vector<std::string>& params )
{
  std::map<std::string,std::vector<boost::shared_ptr<mesh::MeshReader> > > extensions_to_readers;
  std::map<std::string,std::vector<boost::shared_ptr<mesh::MeshWriter> > > extensions_to_writers;
  std::vector<boost::shared_ptr<mesh::MeshReader> > readers;
  std::vector<boost::shared_ptr<mesh::MeshWriter> > writers;
  std::map<std::string,std::string> transformers_description;
  std::map<std::string,boost::shared_ptr<mesh::MeshTransformer> > name_to_transformers;

  Handle< Factory > meshreader_fac = Core::instance().factories().get_factory<MeshReader>();

  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *meshreader_fac ) )
  {
    boost::shared_ptr< MeshReader > reader = boost::dynamic_pointer_cast<MeshReader>(bdr.build("reader"));
    readers.push_back(reader);
    boost_foreach(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  Handle< Factory > meshwriter_fac = Core::instance().factories().get_factory<MeshWriter>();

  boost_foreach(Builder& bdw, find_components_recursively<Builder>( *meshwriter_fac ) )
  {
    boost::shared_ptr< MeshWriter > writer = boost::dynamic_pointer_cast<MeshWriter>(bdw.build("writer"));
    writers.push_back(writer);
    boost_foreach(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  Handle< Factory > meshtrans_fac = Core::instance().factories().get_factory<MeshTransformer>();

  boost_foreach(Builder& bdt, find_components_recursively<Builder>( *meshtrans_fac ))
  {
    boost::shared_ptr< MeshTransformer > transformer = boost::dynamic_pointer_cast<MeshTransformer>(bdt.build("transformer"));
    transformers_description[bdt.builder_concrete_type_name()] = transformer->properties().value<std::string>("brief");
    name_to_transformers[bdt.builder_concrete_type_name()] = transformer;
  }

  if( params.empty() )
  {
    // Default help
    CFinfo << CFendl << "Usage: coolfluid-mesh-transformer [options]" << CFendl << CFendl;
    CFinfo << m_commands << CFendl;
    std::vector< std::string > vk, vt;
    vk.push_back("Input formats:");        vt.push_back("");
    boost_foreach(const boost::shared_ptr< MeshReader >& reader, readers)
    {
      vk.push_back("  " + reader->get_format());
      std::string extensions;
      boost_foreach(const std::string& ext, reader->get_extensions())
      extensions += ext + " ";
      vt.push_back(extensions);
    }
    vk.push_back("");                      vt.push_back("");
    vk.push_back("Output formats:");       vt.push_back("");
    boost_foreach(const boost::shared_ptr< MeshWriter >& writer, writers)
    {
      vk.push_back("  " + writer->get_format());
      std::string extensions;
      boost_foreach(const std::string& ext, writer->get_extensions())
      extensions += ext + " ";
      vt.push_back(extensions);
    }
    vk.push_back("");                      vt.push_back("");
    vk.push_back("Transformations:");      vt.push_back("(use --help 'transformation' for more info)");
    boost_foreach(transformers_description_t transformer, transformers_description)
    {
      vk.push_back("  " + transformer.first);     vt.push_back(transformer.second);
    }
    vk.push_back("");                      vt.push_back("");

    // output with keys strings adjusted to the same length
    unsigned l = 0;
    for (unsigned i=0; i<vk.size(); ++i)
      l = (l>vk[i].length()? l:vk[i].length());
    l+=2;
    for (unsigned i=0; i<vk.size(); ++i)
      vk[i].insert(vk[i].end(),l-vk[i].length(),' ');

    for (unsigned i=0; i<vk.size(); ++i)
      CFinfo << vk[i] << vt[i] << CFendl;
  }
  else
  {
    std::string submodule = params[0];
    if (name_to_transformers.find(submodule) != name_to_transformers.end())
    {
      CFinfo << "\n" << submodule << ":" << CFendl;
      Handle< MeshTransformer > transformer(name_to_transformers[submodule]);
      CFinfo << transformer->help() << CFendl;
    }
    else
    {
      CFinfo << submodule << " does not exist" << CFendl;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

} // mesh_transformer
} // Tools
} // cf3
