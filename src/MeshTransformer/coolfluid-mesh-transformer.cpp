#include <boost/regex.hpp>
#include <boost/program_options.hpp>

#include "Common/Factory.hpp"
#include "Common/Log.hpp"
#include "Common/CoreEnv.hpp"
#include "Common/CRoot.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CMeshReader.hpp"
#include "Mesh/CMeshWriter.hpp"

using namespace boost;
using namespace boost::program_options;
using namespace CF;
using namespace CF::Common;
using namespace CF::Mesh;

int main(int argc, char * argv[])
{
  CFinfo << "Welcome to the COOLFLUID K3 mesh transformer!\n" << CFflush;

  //CoreEnv::instance().initiate(argc, argv);


  // map file extensions to readers and writers

  typedef std::pair<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers_pair_t;
  typedef std::pair<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers_pair_t;
  std::map<std::string,std::vector<CMeshReader::Ptr> > extensions_to_readers;
  std::map<std::string,std::vector<CMeshWriter::Ptr> > extensions_to_writers;
  std::vector<CMeshReader::Ptr> readers;
  std::vector<CMeshWriter::Ptr> writers;

  std::vector<CMeshReader::PROVIDER*> allmeshreaders = Factory<CMeshReader>::instance().getAllConcreteProviders();
  BOOST_FOREACH(CMeshReader::PROVIDER* prov, allmeshreaders)
  {
    CMeshReader::Ptr reader = dynamic_pointer_cast<CMeshReader>(prov->create("reader"));
    readers.push_back(reader);
    BOOST_FOREACH(const std::string& extension, reader->get_extensions())
      extensions_to_readers[extension].push_back(reader);
  }

  std::vector<CMeshWriter::PROVIDER*> allmeshwriters = Factory<CMeshWriter>::instance().getAllConcreteProviders();
  BOOST_FOREACH(CMeshWriter::PROVIDER* prov, allmeshwriters)
  {
    CMeshWriter::Ptr writer = dynamic_pointer_cast<CMeshWriter>(prov->create("writer"));
    writers.push_back(writer);
    BOOST_FOREACH(const std::string& extension, writer->get_extensions())
      extensions_to_writers[extension].push_back(writer);
  }

  options_description desc("General Options");
   desc.add_options()
       ("help,h", "this help")
       ("input,i" , value<std::vector<std::string> >()->multitoken(), "input file(s)")
       ("output,o", value<std::vector<std::string> >()->multitoken(), "output file(s)")
       ("dryrun,d", "dry run")
       ("version,v", "show version")
       ;
   variables_map vm;
   parsed_options parsed = parse_command_line(argc, argv, desc);
   store(parsed, vm);
   notify(vm);



   if (vm.count("help") || vm.size()==0)
   {
     CFinfo << CFendl << "Usage: " << argv[0] << " [options]" << CFendl << CFendl;
     CFinfo << desc << CFendl;
     std::vector< std::string > vk, vt;
     vk.push_back("Input formats:");        vt.push_back("");
     BOOST_FOREACH(const CMeshReader::Ptr& reader, readers)
     {
       vk.push_back("  " + reader->get_format());
       std::string extensions;
       BOOST_FOREACH(const std::string& ext, reader->get_extensions())
         extensions += ext + " ";
       vt.push_back(extensions);
     }
     vk.push_back("");                      vt.push_back("");
     vk.push_back("Output formats:");       vt.push_back("");
     BOOST_FOREACH(const CMeshWriter::Ptr& writer, writers)
     {
       vk.push_back("  " + writer->get_format());
       std::string extensions;
       BOOST_FOREACH(const std::string& ext, writer->get_extensions())
         extensions += ext + " ";
       vt.push_back(extensions);
     }     vk.push_back("");                      vt.push_back("");
     vk.push_back("Transformations:");      vt.push_back("");
     vk.push_back("  no transformations implemented");    vt.push_back("");

     // output with keys strings adjusted to the same length
     unsigned l = 0;
     for (unsigned i=0; i<vk.size(); ++i)
       l = (l>vk[i].length()? l:vk[i].length());
     l+=2;
     for (unsigned i=0; i<vk.size(); ++i)
       vk[i].insert(vk[i].end(),l-vk[i].length(),' ');

     for (unsigned i=0; i<vk.size(); ++i)
       CFinfo << vk[i] << vt[i] << CFendl;
     exit(0);
   }

   bool dryrun=false;
   if (vm.count("dryrun"))
   {
     CFinfo << "\nThis is what would happen without the dryrun option:" << CFendl << CFendl;
     dryrun=true;
   }

   if (vm.count("version"))
   {
     CFinfo << "Version: " << 1 << CFendl;
   }

   // create mesh object
  CRoot::Ptr root = CRoot::create("root");
  CMesh::Ptr mesh = root->create_component_type<CMesh>("mesh");


   typedef basic_option<char> Option;
   typedef std::basic_string<char> OptionValue;

   BOOST_FOREACH(Option option, parsed.options)
   {

     // parse
     if (option.string_key=="input")
     {
       BOOST_FOREACH(OptionValue value, option.value)
       {
         filesystem::path inputfile (value);
         const std::string ext = inputfile.extension();
         CMeshReader::Ptr reader;
         if (!extensions_to_readers.count(ext))
         {
           Uint selection = 0;
           CFinfo << inputfile << " has ambiguous extension " << ext << CFendl;
           BOOST_FOREACH(const CMeshReader::Ptr selectreader , readers)
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
             CFinfo << inputfile << " with extension " << ext << " has multiple readers: " << CFendl;
             BOOST_FOREACH(const CMeshReader::Ptr selectreader , extensions_to_readers[ext])
               CFinfo << "  [" << selection++ +1 << "]  " << selectreader->get_format() << CFendl;
             CFinfo << "Select the correct reader: " << CFflush;
             std::cin >> selection;
             --selection;
           }
           reader = extensions_to_readers[ext][selection];
         }

         CFinfo << "Reading " << inputfile << " with " << reader->get_format() << CFendl;

         if (!dryrun) reader->read_from_to(inputfile,mesh);
       }
     }
     else if (option.string_key=="t")
     {

     }
  else if (option.string_key=="output")
  {
    BOOST_FOREACH(OptionValue value, option.value)
    {
     filesystem::path outputfile (value);
     const std::string ext = outputfile.extension();

     CMeshWriter::Ptr writer;
     if (!extensions_to_writers.count(ext))
     {
       Uint selection = 0;
       CFinfo << outputfile << " has ambiguous extension " << ext << CFendl;
       BOOST_FOREACH(const CMeshWriter::Ptr selectwriter , writers)
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
         CFinfo << outputfile << " with extension " << ext << " has multiple writers: " << CFendl;
         BOOST_FOREACH(const CMeshWriter::Ptr selectwriter , extensions_to_writers[ext])
           CFinfo << "  [" << selection++ +1 << "]  " << selectwriter->get_format() << CFendl;
         CFinfo << "Select the correct writer: " << CFflush;
         std::cin >> selection;
         --selection;
       }
       writer = extensions_to_writers[ext][selection];
     }

     CFinfo << "Writing " << outputfile << " with " << writer->get_format() << CFendl;

     if (!dryrun) writer->write_from_to(mesh,outputfile);
    }
  }

  }

  //CoreEnv::instance().terminate();

}
