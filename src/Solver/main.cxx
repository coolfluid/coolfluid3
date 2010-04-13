#include "Common/Log.hh"
#include "Common/Component.hh"

using namespace CF;
using namespace CF::Common;

int main(int argc, char * argv[])
{
 CFinfo << "Welcome to the COOLFLUID K3 solver!\n" << CFendl;

 Component* root = new Component ( "ROOT" );

 Component* dir1 =  new Component ( "dir1" );
 Component* dir2 =  new Component ( "dir2" );

 root->add_component( dir1 );
 dir1->add_component( dir2 );

 CFinfo << root->path().string() << "\n" << CFendl;
 CFinfo << dir1->path().string() << "\n" << CFendl;
 CFinfo << dir2->path().string() << "\n" << CFendl;

 CFinfo << "\n" << CFendl;

 CFinfo << root->full_path().string() << "\n" << CFendl;
 CFinfo << dir1->full_path().string() << "\n" << CFendl;
 CFinfo << dir2->full_path().string() << "\n" << CFendl;

 delete_ptr ( dir2 );
 delete_ptr ( dir1 );
 delete_ptr ( root );

 return 0;
}
