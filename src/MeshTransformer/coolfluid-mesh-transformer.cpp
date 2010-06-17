#include <boost/regex.hpp>

#include "Common/GetPot.hpp"
#include "Common/Factory.hpp"
#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace boost;
using namespace CF::Common;
using namespace CF::Mesh;

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 mesh transformer!\n" << CFflush;

  //CoreEnv::getInstance().initiate(argc, argv);


  // map file extensions to readers and writers

  typedef std::pair<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers_pair_t;
  typedef std::pair<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers_pair_t;
  std::map<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers;
  std::map<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers;
  std::vector<std::string> input_formats;
  std::vector<std::string> output_formats;

  std::vector<CMeshReader::PROVIDER*> allmeshreaders = Factory<CMeshReader>::getInstance().getAllConcreteProviders();
  BOOST_FOREACH(CMeshReader::PROVIDER* prov, allmeshreaders)
  {
    CMeshReader::Ptr reader = dynamic_pointer_cast<CMeshReader>(prov->create("reader"));
    input_formats.push_back(reader->get_format());
    BOOST_FOREACH(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  std::vector<CMeshWriter::PROVIDER*> allmeshwriters = Factory<CMeshWriter>::getInstance().getAllConcreteProviders();
  BOOST_FOREACH(CMeshWriter::PROVIDER* prov, allmeshwriters)
  {
    CMeshWriter::Ptr writer = dynamic_pointer_cast<CMeshWriter>(prov->create("writer"));
    output_formats.push_back(writer->get_format());
    BOOST_FOREACH(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  // options, formats and transformations
  GetPot o(argc,argv);

  // help (options and descriptions)
   if (o.search(2,"-h","--help") || o.size()<=1) {

     // set keys and descriptions
     std::vector< std::string > vk, vt;
     vk.push_back("");                      vt.push_back("");
     vk.push_back("General options:");      vt.push_back("");
     vk.push_back("  -h [ --help ]  ");      vt.push_back("this help");
     vk.push_back("  -i [ --input ] arg  ");  vt.push_back("input file(s)");
     vk.push_back("  -o [ --output ] arg  ");  vt.push_back("output file(s)");
     vk.push_back("  -f [ --format ] arg  ");  vt.push_back("format of the file specified in previous arg");
     vk.push_back("  -t[...]  ");             vt.push_back("transformations");
     vk.push_back("  -v [ --version ]  ");        vt.push_back("display version");
     vk.push_back("");                      vt.push_back("");
     vk.push_back("Input formats:");        vt.push_back("");
     BOOST_FOREACH(const extensions_to_readers_pair_t& p, extensions_to_readers)
     BOOST_FOREACH(const CMeshReader::Ptr& reader, p.second)
     { vk.push_back("  "+p.first);   vt.push_back(reader->get_format());}
     vk.push_back("");                      vt.push_back("");
     vk.push_back("Output formats:");       vt.push_back("");
     BOOST_FOREACH(const extensions_to_writers_pair_t& p, extensions_to_writers)
     BOOST_FOREACH(const CMeshWriter::Ptr& writer, p.second)
     { vk.push_back("  "+p.first);   vt.push_back(writer->get_format());}
     vk.push_back("");                      vt.push_back("");
     vk.push_back("Transformations:");      vt.push_back("");
     //ft->getkeys(vk);                     ft->gettxts(vt);

     // output with keys strings adjusted to the same length
     unsigned l = 0;
     for (unsigned i=0; i<vk.size(); ++i)
       l = (l>vk[i].length()? l:vk[i].length());
     l+=2;
     for (unsigned i=0; i<vk.size(); ++i)
       vk[i].insert(vk[i].end(),l-vk[i].length(),' ');

     CFinfo << CFendl << "Usage: " << o[0] << " [options]" << CFendl;
     for (unsigned i=0; i<vk.size(); ++i)
       CFinfo << vk[i] << vt[i] << CFendl;
     exit(0);

   }
   else if (o.search(2,"-v","--version")) {

     CFinfo << CFendl
          << "Version " << 1.0 << CFendl;

     CFinfo << "mfinput keys:";
     std::vector< std::string > vk;
     BOOST_FOREACH(const extensions_to_readers_pair_t& p, extensions_to_readers)
       vk.push_back(p.first);
     //fi->getkeys(vk);
     for (unsigned i=0; i<vk.size(); ++i)
       if (vk[i].length())
         CFinfo << ' ' << vk[i];
     CFinfo << CFendl;

     CFinfo << "mfoutput keys:";
     vk.clear();
     BOOST_FOREACH(const extensions_to_writers_pair_t& p, extensions_to_writers)
       vk.push_back(p.first);
     for (unsigned i=0; i<vk.size(); ++i)
       if (vk[i].length())
         CFinfo << ' ' << vk[i];
     CFinfo << CFendl;

     CFinfo << "mtransform keys:";
     
     vk.clear();
     //ft->getkeys(vk);
     for (unsigned i=0; i<vk.size(); ++i)
       if (vk[i].length())
         CFinfo << ' ' << vk[i];
     CFinfo << CFendl;

     if (o.search("--version"))
       exit(0);

   }

   // create mesh object
  CRoot::Ptr root = CRoot::create("root");
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");

   // process command line arguments in order
   for (unsigned i=1; i<o.size(); ++i) {
     const std::string arg(o[i]);
     const std::string val(o[i+1]);
     o.set_cursor(i);
     if (arg=="-i") {

       // multiple input merges another mesh into current
       filesystem::path inputfile (val);
       const std::string key = inputfile.extension();
       CMeshReader& reader = (*extensions_to_readers[key][0]);
       CFinfo << "Reading file with " << reader.get_format() << CFendl;

       reader.read_from_to(inputfile,mesh);
//       if (fi->search(key.c_str())) {
//         CFinfo << "::read \"" << val << "\"..." << CFendl;
//         mfinput* p = fi->Create(key);
//         if (m.v()) {
//           mmesh m2;
//           p->read(o,m2);
//           CFinfo << "::merge [d/n/e]: " << m.d() << "+" << m2.d()
//                              << " / " << m.n() << "+" << m2.n()
//                              << " / " << m.e() << "+" << m2.e() << "..." << CFendl;
//           m.merge(m2);
//           CFinfo << "::merge [d/n/e]: " << m.d()
//                              << " / " << m.n()
//                              << " / " << m.e() << "." << CFendl;
//         }
//         else {
//           p->read(o,m);
//         }
//         delete p;
//         CFinfo << "::read \"" << val << "\"." << CFendl;
//       }

     }
     else if (arg.find("-t")==0) {

//       const string key(arg);
//       if (ft->search(key.c_str())) {
//         CFinfo << "::transform \"" << key << "\"..." << CFendl;
//         mtransform* p = ft->Create(key);
//         p->transform(o,m);
//         delete p;
//         CFinfo << "::transform \"" << key << "\"." << CFendl;
//       }

     }
     else if (arg=="-o") {

       // write current mesh
       filesystem::path outputfile (val);
       const std::string key = outputfile.extension();

       CMeshWriter& writer = (*extensions_to_writers[key][0]);
       CFinfo << "Writing file with " << writer.get_format() << CFendl;

       writer.write_from_to(mesh,outputfile);

       //       if (fo->search(key.c_str())) {
//         CFinfo << "::write \"" << val << "\"..." << CFendl;
//         mfoutput* p = fo->Create(key);
//         p->write(o,m);
//         delete p;
//         CFinfo << "::write \"" << val << "\"." << CFendl;
//       }

     }
   }


  //CoreEnv::getInstance().terminate();

}
