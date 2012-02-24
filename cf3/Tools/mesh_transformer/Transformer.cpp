// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>
#include <boost/regex.hpp>

#include "common/Log.hpp"
#include "common/Core.hpp"
#include "common/BuildInfo.hpp"
#include "common/Factory.hpp"
#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "mesh/Mesh.hpp"
#include "mesh/MeshReader.hpp"
#include "mesh/MeshWriter.hpp"
#include "mesh/MeshTransformer.hpp"
#include "mesh/Field.hpp"

#include "Tools/Shell/BasicCommands.hpp"
#include "Tools/mesh_transformer/Transformer.hpp"

namespace cf3 {
namespace Tools {
namespace mesh_transformer {

  using namespace boost;
  using namespace boost::program_options;

  using namespace cf3;
  using namespace cf3::common;
  using namespace cf3::mesh;

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
    transformers_description[bdt.builder_concrete_type_name()] = transformer->options().option("brief").value<std::string>();
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

void Transformer::input( const std::vector<std::string>& params )
{
  bool dryrun = false;

  Handle< Mesh > mesh(Core::instance().root().get_child("mesh"));

  std::map<std::string,std::vector<boost::shared_ptr<mesh::MeshReader> > > extensions_to_readers;
  std::vector<boost::shared_ptr<mesh::MeshReader> > readers;

  Handle< Factory > meshreader_fac = Core::instance().factories().get_factory<MeshReader>();
  boost_foreach(Builder& bdr, find_components_recursively<Builder>( *meshreader_fac ) )
  {
    boost::shared_ptr< MeshReader > reader = boost::dynamic_pointer_cast<MeshReader>(bdr.build("reader"));
    readers.push_back(reader);
    boost_foreach(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  boost_foreach(const std::string& value, params)
  {
    URI inputfile (value);
    const std::string ext = inputfile.extension();
    Handle< MeshReader > reader;
    if (!extensions_to_readers.count(ext))
    {
      Uint selection = 0;
      CFinfo << inputfile.path() << " has ambiguous extension " << ext << CFendl;
      boost_foreach(const boost::shared_ptr< MeshReader > selectreader , readers)
      CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
      CFinfo << "Select the correct reader: " << CFflush;
      std::cin >> selection;
      reader = Handle<MeshReader>(readers[--selection]);
    }
    else
    {
      Uint selection = 0;
      if (extensions_to_readers[ext].size()>1)
      {
        CFinfo << inputfile.path() << " with extension " << ext << " has multiple readers: " << CFendl;
        boost_foreach(const boost::shared_ptr< MeshReader > selectreader , extensions_to_readers[ext])
        CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
        CFinfo << "Select the correct reader: " << CFflush;
        std::cin >> selection;
        --selection;
      }
      reader = Handle<MeshReader>(extensions_to_readers[ext][selection]);
    }

    CFinfo << "\nReading " << inputfile.path() << " with " << reader->get_format() << CFendl;
    if (!dryrun) reader->read_mesh_into(inputfile,*mesh);
  }
}

////////////////////////////////////////////////////////////////////////////////

void Transformer::output( const std::vector<std::string>& params )
{
  bool dryrun = false;

  Handle< Mesh > mesh(Core::instance().root().get_child("mesh"));

  std::map<std::string,std::vector<boost::shared_ptr<mesh::MeshWriter> > > extensions_to_writers;
  std::vector<boost::shared_ptr<mesh::MeshWriter> > writers;

  Handle< Factory > meshwriter_fac = Core::instance().factories().get_factory<MeshWriter>();

  boost_foreach(Builder& bdw, find_components_recursively<Builder>( *meshwriter_fac ) )
  {
    boost::shared_ptr< MeshWriter > writer = boost::dynamic_pointer_cast<MeshWriter>(bdw.build("writer"));
    writers.push_back(writer);
    boost_foreach(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  boost_foreach(const std::string& value, params)
  {
    URI outputfile (value);
    const std::string ext = outputfile.extension();

    Handle< MeshWriter > writer;
    if (!extensions_to_writers.count(ext))
    {
      Uint selection = 0;
      CFinfo << outputfile.path() << " has ambiguous extension " << ext << CFendl;
      boost_foreach(const boost::shared_ptr< MeshWriter >& selectwriter , writers)
      CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
      CFinfo << "Select the correct writer: " << CFflush;
      std::cin >> selection;
      writer = Handle<MeshWriter>(writers[--selection]);
    }
    else
    {
      Uint selection = 0;
      if (extensions_to_writers[ext].size()>1)
      {
        CFinfo << outputfile.path() << " with extension " << ext << " has multiple writers: " << CFendl;
        boost_foreach(const boost::shared_ptr< MeshWriter >& selectwriter , extensions_to_writers[ext])
        CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
        CFinfo << "Select the correct writer: " << CFflush;
        std::cin >> selection;
        --selection;
      }
      writer = Handle<MeshWriter>(extensions_to_writers[ext][selection]);
    }

    CFinfo << "\nWriting " << outputfile.path() << " with " << writer->get_format() << CFendl;

    std::vector<URI> fields;
    boost_foreach ( Field& field, find_components<Field>(*mesh) )
      fields.push_back(field.uri());
    if (!dryrun) writer->options().configure_option("fields",fields);
    if (!dryrun) writer->options().configure_option("mesh",mesh);
    if (!dryrun) writer->options().configure_option("file",outputfile);
    if (!dryrun) writer->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////

void Transformer::transform( const std::vector<std::string>& params )
{
  bool dryrun = false;

  Handle< Mesh > mesh(Core::instance().root().get_child("mesh"));

  std::map<std::string,std::string> transformers_description;
  std::map<std::string,boost::shared_ptr<mesh::MeshTransformer> > name_to_transformers;

  Handle< Factory > meshtrans_fac = Core::instance().factories().get_factory<MeshTransformer>();

  boost_foreach(Builder& bdt, find_components_recursively<Builder>( *meshtrans_fac ))
  {
    boost::shared_ptr< MeshTransformer > transformer = boost::dynamic_pointer_cast<MeshTransformer>(bdt.build("transformer"));
    transformers_description[bdt.builder_concrete_type_name()] = transformer->options().option("brief").value<std::string>();
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
    Handle< MeshTransformer > transformer(name_to_transformers[transformer_name]);
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

} // mesh_transformer
} // Tools
} // cf3
