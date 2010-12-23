// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/program_options.hpp>

#include "Common/Log.hpp"
#include "Common/Core.hpp"
#include "Common/CreateComponent.hpp"
#include "Common/BuildInfo.hpp"

#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/SF/Types.hpp"
#include "Mesh/CTable.hpp"

using namespace boost;
using namespace boost::program_options;

using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

/////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char * argv[])
{
  Core::instance().initiate(argc, argv);

  try
  {
    // parse command line options

    options_description desc("General Options");
    desc.add_options()
    ("help,h",   value<std::string>()->implicit_value(std::string()), "show this help")
    ("scase,s",  value<std::vector<std::string> >()->multitoken(),    "list of scripts to execute in order")
    ("version,v", "show version")
    ;
    variables_map vm;
    parsed_options parsed = parse_command_line(argc, argv, desc);
    store(parsed, vm);
    notify(vm);

    if (vm.count("version"))
      CFinfo << Core::instance().build_info()->version_header() << CFendl;

    // show help if asked or no args given
    if (vm.count("help") || vm.size()==0)
    {
      // Default help
      CFinfo << "CF3 batch command\n" << CFendl;
      CFinfo << "Usage: " << argv[0] << " [options]\n" << CFendl;
      CFinfo << desc << CFendl;

      Core::instance().terminate();
      return 0;
    }

    //-------------------------------------------------------------------------------

    // read the mesh
    CMeshReader::Ptr meshreader = create_component_abstract_type<CMeshReader>("CF.Mesh.Neu.CReader","meshreader");

    // the file to read from
    boost::filesystem::path inputfile ("quadtriag.neu");

    // the mesh to store in
    CRoot::Ptr root = CRoot::create("root");
    CMesh::Ptr mesh = meshreader->create_mesh_from(inputfile);
    root->add_component(mesh);

    mesh->create_field("volumes",1, CField::ELEMENT_BASED);

    std::vector<URI> regions_to_loop = boost::assign::list_of(URI("cpath://root/mesh/quadtriag"));

    CFinfo << CFendl << CFendl;

    std::cout << mesh->tree() << std::endl;

    CFinfo << CFendl << CFendl;

  }
  catch ( std::exception& ex )
  {
    CFerror << "Unhandled exception: " << ex.what() << CFendl;
  }
  catch ( ... )
  {
    CFerror << "Detected unknown exception" << CFendl;
  }

  Core::instance().terminate();

  return 0;
}


///////////////////////////////////////////////////////////////////////////////


