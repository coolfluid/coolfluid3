#!/usr/bin/env perl

use warnings;
use strict;

# librarys
use Getopt::Long;
use Tie::File;
use File::Copy;

# command line options
my $opt_help      = 0;

# Parse command line
$opt_help=1 unless GetOptions ( 'help'   => \$opt_help );
sub show_help ()
{
  print <<ZZZ;
make-libreary.pl [libname] : creates a subdirectory with an empty library
options:
        --help            Show this help
ZZZ
}

if ($opt_help eq 1 )
{
  show_help();
  exit(0);
}

my $num_args = $#ARGV + 1;
if ($num_args != 1) {
  show_help();
  exit(0);
}

my $libname    = $ARGV[0];
my $dirname    = $libname; # dir is name of library
my $cmakefile  = "$dirname/CMakeLists.txt";
my $libhpp     = "$dirname/Lib$libname.hpp";
my $libcpp     = "$dirname/Lib$libname.cpp";

my $libuppercase = uc($libname);
my $liblowercase = lc($libname);

############################################################################################
# make the sub directory

mkdir "$dirname" or die $!;

############################################################################################
# create the CMakeLists.txt
open  ( CMAKEFILE, ">>$cmakefile" ) || die("Cannot open file $cmakefile") ;
print   CMAKEFILE <<ZZZ;
# coolfluid_$liblowercase

list( APPEND coolfluid_$liblowercase\_files
  Lib$libname.cpp
  Lib$libname.hpp
)

list( APPEND coolfluid_$liblowercase\_cflibs coolfluid_common )

coolfluid_add_library( coolfluid_$liblowercase )

ZZZ
close (CMAKEFILE); 

############################################################################################
# create the LibName.hpp
open  ( LIBHPP, ">>$libhpp" ) || die("Cannot open file $libhpp") ;
print   LIBHPP <<ZZZ;
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#ifndef CF_$libname\_Lib$libname\_hpp
#define CF_$libname\_Lib$libname\_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/CLibrary.hpp"

////////////////////////////////////////////////////////////////////////////////

/// Define the macro $libname\_API
/// \@note build system defines COOLFLUID_$libuppercase\_EXPORTS when compiling $libname files
#ifdef COOLFLUID_$libuppercase\_EXPORTS
#   define $libname\_API      CF_EXPORT_API
#   define TEMPLATE
#else
#   define $libname\_API      CF_IMPORT_API
#   define $libname\_TEMPLATE CF_TEMPLATE_EXTERN
#endif

////////////////////////////////////////////////////////////////////////////////

namespace CF {

/// \@brief %$libname classes
///
/// $libname library
/// \@author 
namespace $libname {

////////////////////////////////////////////////////////////////////////////////

/// Class defines the $libname library
/// \@author 
class $libname\_API Lib$libname : public Common::CLibrary
{
public:

  typedef boost::shared_ptr<Lib$libname> Ptr;
  typedef boost::shared_ptr<Lib$libname const> ConstPtr;

  /// Constructor
  Lib$libname ( const std::string& name) : Common::CLibrary(name) { }

  virtual ~Lib$libname() { }

public: // functions

  /// \@return string of the library namespace
  static std::string library_namespace() { return "CF.$libname"; }

  /// Static function that returns the library name.
  /// Must be implemented for CLibrary registration
  /// \@return name of the library
  static std::string library_name() { return "$libname"; }

  /// Static function that returns the description of the library.
  /// Must be implemented for CLibrary registration
  /// \@return description of the library

  static std::string library_description()
  {
    return "This library implements $libname";
  }

  /// Gets the Class name
  static std::string type_name() { return "Lib$libname"; }

protected:

  /// initiate library
  virtual void initiate_impl();

  /// terminate library
  virtual void terminate_impl();

}; // end Lib$libname

////////////////////////////////////////////////////////////////////////////////

} // $libname
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_$libname\_Lib$libname\_hpp

ZZZ
close (LIBHPP); 

############################################################################################
# create the LibName.cpp
open  ( LIBCPP, ">>$libcpp" ) || die("Cannot open file $libcpp") ;
print   LIBCPP <<ZZZ;
// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "$libname/Lib$libname.hpp"

namespace CF {
namespace $libname {

  using namespace Common;

CF::Common::RegistLibrary<Lib$libname> Lib$libname;

////////////////////////////////////////////////////////////////////////////////

void Lib$libname\::initiate_impl()
{
}

void Lib$libname\::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // $libname
} // CF

ZZZ
close (LIBCPP); 


############################################################################################
# add subdirectory to the end of the current CMakeLists.txt
open  ( CMAKE, ">>CMakeLists.txt" ) || die("Cannot open file CMakeLists.txt") ;
print   CMAKE <<ZZZ;

# library $libname
add_subdirectory( $dirname )

ZZZ
close (CMAKE); 


############################################################################################
# print final remarks

print "Remark: don't forge to edit the doxygen comments in the $libhpp file\n";

