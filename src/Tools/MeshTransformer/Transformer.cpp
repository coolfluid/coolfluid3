// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CRoot.hpp"
#include "Common/BuildInfo.hpp"
#include "Common/CFactory.hpp"
#include "Common/CBuilder.hpp"
 
#include "Common/Foreach.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/MeshTransformer/Transformer.hpp"

namespace CF {
namespace Tools {
namespace MeshTransformer {

  using namespace boost;
  using namespace boost::program_options;

  using namespace CF;
  using namespace CF::Common;
  using namespace CF::Mesh;

////////////////////////////////////////////////////////////////////////////////

Transformer::Transformer()
{
}

////////////////////////////////////////////////////////////////////////////////

Transformer::commands_description Transformer::description()
{
  commands_description desc("Mesh Transformer Commands");
  desc.add_options()
  //("help,h",      value< std::string >()->implicit_value(std::string())->notifier(&help     ), "this help if no arg, or more detailed help of submodule")
  ("input",     value< std::vector<std::string> >()->notifier(&input    )->multitoken(), "input file(s)")
  ("output" ,   value< std::vector<std::string> >()->notifier(&output   )->multitoken(), "output file(s)")
  ("transform", value< std::vector<std::string> >()->notifier(&transform)->multitoken(), "transformations")
  ;
  return desc;
}
////////////////////////////////////////////////////////////////////////////////

void Transformer::help( const std::string& param )
{
  std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshReader> > > extensions_to_readers;
  std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshWriter> > > extensions_to_writers;
  std::vector<boost::shared_ptr<Mesh::CMeshReader> > readers;
  std::vector<boost::shared_ptr<Mesh::CMeshWriter> > writers;
  std::map<std::string,std::string> transformers_description;
  std::map<std::string,boost::shared_ptr<Mesh::CMeshTransformer> > name_to_transformers;


  CFactory::Ptr meshreader_fac = Core::instance().factories().get_factory<CMeshReader>();

  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_fac ) )
  {
    CMeshReader::Ptr reader = bdr.build("reader")->as_ptr<CMeshReader>();
    readers.push_back(reader);
    boost_foreach(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  CFactory::Ptr meshwriter_fac = Core::instance().factories().get_factory<CMeshWriter>();

  boost_foreach(CBuilder& bdw, find_components_recursively<CBuilder>( *meshwriter_fac ) )
  {
    CMeshWriter::Ptr writer = bdw.build("writer")->as_ptr<CMeshWriter>();
    writers.push_back(writer);
    boost_foreach(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  CFactory::Ptr meshtrans_fac = Core::instance().factories().get_factory<CMeshTransformer>();

  boost_foreach(CBuilder& bdt, find_components_recursively<CBuilder>( *meshtrans_fac ))
  {
    CMeshTransformer::Ptr transformer = bdt.build("transformer")->as_ptr<CMeshTransformer>();
    transformers_description[bdt.builder_concrete_type_name()] = transformer->option("brief").value<std::string>();
    name_to_transformers[bdt.builder_concrete_type_name()] = transformer;
  }

  std::string submodule = param;

  if( submodule.empty() )
  {
    // Default help
    CFinfo << CFendl << "Usage: coolfluid-mesh-transformer [options]" << CFendl << CFendl;
    CFinfo << Shell::BasicCommands::description() << CFendl;
    CFinfo << description() << CFendl;
    std::vector< std::string > vk, vt;
    vk.push_back("Input formats:");        vt.push_back("");
    boost_foreach(const CMeshReader::Ptr& reader, readers)
    {
      vk.push_back("  " + reader->get_format());
      std::string extensions;
      boost_foreach(const std::string& ext, reader->get_extensions())
      extensions += ext + " ";
      vt.push_back(extensions);
    }
    vk.push_back("");                      vt.push_back("");
    vk.push_back("Output formats:");       vt.push_back("");
    boost_foreach(const CMeshWriter::Ptr& writer, writers)
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
    if (name_to_transformers.find(submodule) != name_to_transformers.end())
    {
      CFinfo << "\n" << submodule << ":" << CFendl;
      CMeshTransformer::Ptr transformer = name_to_transformers[submodule];
      CFinfo << transformer->help() << CFendl;
    }
    else
    {
      CFinfo << submodule << " does not exist" << CFendl;
    }

  }
}

////////////////////////////////////////////////////////////////////////////////

void Transformer::input( const std::vector<std::string>& params )
{
  bool dryrun = false;

  CMesh::Ptr mesh = Core::instance().root().get_child("mesh").as_ptr<CMesh>();

  std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshReader> > > extensions_to_readers;
  std::vector<boost::shared_ptr<Mesh::CMeshReader> > readers;

  CFactory::Ptr meshreader_fac = Core::instance().factories().get_factory<CMeshReader>();
  boost_foreach(CBuilder& bdr, find_components_recursively<CBuilder>( *meshreader_fac ) )
  {
    CMeshReader::Ptr reader = bdr.build("reader")->as_ptr<CMeshReader>();
    readers.push_back(reader);
    boost_foreach(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  boost_foreach(const std::string& value, params)
  {
    URI inputfile (value);
    const std::string ext = inputfile.extension();
    CMeshReader::Ptr reader;
    if (!extensions_to_readers.count(ext))
    {
      Uint selection = 0;
      CFinfo << inputfile.path() << " has ambiguous extension " << ext << CFendl;
      boost_foreach(const CMeshReader::Ptr selectreader , readers)
      CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
      CFinfo << "Select the correct reader: " << CFflush;
      std::cin >> selection;
      reader = readers[--selection];
    }
    else
    {
      Uint selection = 0;
      if (extensions_to_readers[ext].size()>1)
      {
        CFinfo << inputfile.path() << " with extension " << ext << " has multiple readers: " << CFendl;
        boost_foreach(const CMeshReader::Ptr selectreader , extensions_to_readers[ext])
        CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
        CFinfo << "Select the correct reader: " << CFflush;
        std::cin >> selection;
        --selection;
      }
      reader = extensions_to_readers[ext][selection];
    }

    CFinfo << "\nReading " << inputfile.path() << " with " << reader->get_format() << CFendl;
    if (!dryrun) reader->read_mesh_into(inputfile,*mesh);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Transformer::output( const std::vector<std::string>& params )
{
  bool dryrun = false;

  CMesh::Ptr mesh = Core::instance().root().get_child("mesh").as_ptr<CMesh>();

  std::map<std::string,std::vector<boost::shared_ptr<Mesh::CMeshWriter> > > extensions_to_writers;
  std::vector<boost::shared_ptr<Mesh::CMeshWriter> > writers;

  CFactory::Ptr meshwriter_fac = Core::instance().factories().get_factory<CMeshWriter>();

  boost_foreach(CBuilder& bdw, find_components_recursively<CBuilder>( *meshwriter_fac ) )
  {
    CMeshWriter::Ptr writer = bdw.build("writer")->as_ptr<CMeshWriter>();
    writers.push_back(writer);
    boost_foreach(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  boost_foreach(const std::string& value, params)
  {
    URI outputfile (value);
    const std::string ext = outputfile.extension();

    CMeshWriter::Ptr writer;
    if (!extensions_to_writers.count(ext))
    {
      Uint selection = 0;
      CFinfo << outputfile.path() << " has ambiguous extension " << ext << CFendl;
      boost_foreach(const CMeshWriter::Ptr selectwriter , writers)
      CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
      CFinfo << "Select the correct writer: " << CFflush;
      std::cin >> selection;
      writer = writers[--selection];
    }
    else
    {
      Uint selection = 0;
      if (extensions_to_writers[ext].size()>1)
      {
        CFinfo << outputfile.path() << " with extension " << ext << " has multiple writers: " << CFendl;
        boost_foreach(const CMeshWriter::Ptr selectwriter , extensions_to_writers[ext])
        CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
        CFinfo << "Select the correct writer: " << CFflush;
        std::cin >> selection;
        --selection;
      }
      writer = extensions_to_writers[ext][selection];
    }

    CFinfo << "\nWriting " << outputfile.path() << " with " << writer->get_format() << CFendl;

    std::vector<Field::Ptr> fields;
    boost_foreach ( Field& field, find_components<Field>(*mesh) )
      fields.push_back(field.as_ptr<Field>());
    if (!dryrun) writer->set_fields(fields);
    if (!dryrun) writer->write_from_to(*mesh,outputfile);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Transformer::transform( const std::vector<std::string>& params )
{
  bool dryrun = false;

  CMesh::Ptr mesh = Core::instance().root().get_child("mesh").as_ptr<CMesh>();

  std::map<std::string,std::string> transformers_description;
  std::map<std::string,boost::shared_ptr<Mesh::CMeshTransformer> > name_to_transformers;

  CFactory::Ptr meshtrans_fac = Core::instance().factories().get_factory<CMeshTransformer>();

  boost_foreach(CBuilder& bdt, find_components_recursively<CBuilder>( *meshtrans_fac ))
  {
    CMeshTransformer::Ptr transformer = bdt.build("transformer")->as_ptr<CMeshTransformer>();
    transformers_description[bdt.builder_concrete_type_name()] = transformer->option("brief").value<std::string>();
    name_to_transformers[bdt.builder_concrete_type_name()] = transformer;
  }

  const std::string transformer_name = params[0];
  std::string transformer_args;
  std::vector<std::string> parsed_transformer_args;
  for (Uint i=1; i<params.size(); ++i)
  {
    transformer_args += params[i];
    parsed_transformer_args.push_back(params[i]);
    if (i<params.size()-1)
      transformer_args += " ";
  }
  if (name_to_transformers.find(transformer_name) != name_to_transformers.end())
  {
    CMeshTransformer::Ptr transformer = name_to_transformers[transformer_name];
    CFinfo << "\nTransforming mesh with " << transformer_name << " [" << transformer_args << "]" << CFendl;
    if (!dryrun) transformer->set_mesh(mesh);
    if (!dryrun) transformer->configure(parsed_transformer_args);
    if (!dryrun) transformer->execute();
  }
  else
  {
    CFinfo << transformer_name << " does not exist" << CFendl;
  }

}

////////////////////////////////////////////////////////////////////////////////

} // MeshTransformer
} // Tools
} // CF
