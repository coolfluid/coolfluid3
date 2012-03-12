#!/usr/bin/env perl

# use warnings;
use strict;

#==========================================================================
# Modules
#==========================================================================
use Term::ANSIColor;
use Getopt::Long;
use File::Path;
use File::Copy;
use Switch;

#==========================================================================
# Constants
#==========================================================================
my $ERRORCOLOR="bold red";
my $OKCOLOR="bold green";
my $DEBUGCOLOR = "yellow";
my $HIGHLIGHTCOLOR = "bold yellow";

#==========================================================================
# Global Variables
#==========================================================================

my $home = $ENV{HOME};
my $user = $ENV{USER};
my $arch = get_arch();

my $opt_help          = 0;
my $opt_verbose       = 0;
my $opt_list          = 0;
my $opt_dryrun        = 0;
my $opt_nocolor       = 0;
my $opt_debug         = 0;
my $opt_nompi         = 0;
my $opt_mpi           = "openmpi";
my $opt_mpi_dir       = "";
my $opt_fetchonly     = 0;
my $opt_many_mpi      = 0;
my $opt_no_fortran    = 0;
my $opt_install_dir   = "$home/local/$arch";
my $opt_install_mpi_dir = "";
my $opt_cmake_dir     = "";
my $opt_tmp_dir       = "$home/tmp";
my $opt_dwnldsrc      = "http://coolfluidsrv.vki.ac.be/webfiles/coolfluid/packages";
my $opt_wgetprog      = "wget -nc -nd";
my $opt_curlprog      = "curl -O -nc -nv --progress-bar";
my $opt_dwnldprog     = $opt_wgetprog;
my $opt_makeopts      = "-j2";
my @opt_install_list = ();

# list of packages, and their associated values
# [$vrs] : default version to install
# [$dft] : install by default ?
# [$ins] : should it be installed ?
# [$pri] : installation priority
# [$fnc] : function that implements the installation

my $vrs = 0;
my $dft = 1;
my $ins = 2;
my $pri = 3;
my $fnc = 4;

my $priority = 0;

# these packages are listed by priority
my %packages = (  #  version   default install priority      function
    "cmake"      => [ "2.8.5",    'on' ,   'off', $priority++,  \&install_cmake ],
    "wget"       => [ "1.12",     'off',   'off', $priority++,  \&install_wgetprog],
    "blas"       => [ "3.0.3",    'off',   'off', $priority++,  \&install_blas ],
    "lapack"     => [ "3.0.3",    'off',   'off', $priority++,  \&install_lapack ],
    "zlib"       => [ "1.2.5",    'off',   'off', $priority++,  sub { install_gnu("zlib") } ],
    "curl"       => [ "7.21.4",   'off',   'off', $priority++,  \&install_curl ],
    "lam"        => [ "7.1.4",    'off',   'off', $priority++,  \&install_lam ],
    "openmpi"    => [ "1.5.3",    'on' ,   'off', $priority++,  \&install_openmpi ],
    "mpich"      => [ "1.2.7p1",  'off',   'off', $priority++,  \&install_mpich ],
    "mpich2"     => [ "1.3.2p1",  'off',   'off', $priority++,  \&install_mpich2 ],
    "boost-jam"  => [ "3.1.18",   'off',   'off', $priority++,  \&install_boost_jam ],
    "boost"      => [ "1_48_0",   'on' ,   'off', $priority++,  \&install_boost ],
    "parmetis"   => [ "3.2.0",    'off',   'off', $priority++,  \&install_parmetis ],
    "qt"         => [ "4.7.4",    'off',   'off', $priority++,  \&install_qt ],
    "paraview"   => [ "3.10.1",   'off',   'off', $priority++,  \&install_paraview ], # must be installed *BEFORE* hdf5
    "hdf5"       => [ "1.8.7",    'off',   'off', $priority++,  \&install_hdf5 ],
    "trilinos"   => [ "10.8.2",   'off',   'off', $priority++,  \&install_trilinos ],
    "petsc"      => [ "3.1-p8",   'off',   'off', $priority++,  \&install_petsc3 ],
    "cgns"       => [ "3.1.3-2",  'off',   'off', $priority++,  \&install_cgns ],
    "google-perftools" => [ "1.7",'off',   'off', $priority++,  \&install_google_perftools ],
    "cgal"       => [ "3.8",      'off',   'off', $priority++,  \&install_cgal ],
    "superlu"    => [ "4.1",      'off',   'off', $priority++,  \&install_superlu ],
);

# supported extensions for downloading and uncompressing
my @extensions = ( "tar.gz" , "tar.bz2" , "tgz" , "zip" );

#==========================================================================
# Command Line
#==========================================================================

sub parse_commandline() # Parse command line
{
    $opt_help=1 unless GetOptions (
        'help'                  => \$opt_help,
        'verbose'               => \$opt_verbose,
        'list'                  => \$opt_list,
        'nocolor'               => \$opt_nocolor,
        'debug'                 => \$opt_debug,
        'nompi'                 => \$opt_nompi,
        'no-fortran'            => \$opt_no_fortran,
        'many-mpi'              => \$opt_many_mpi,
        'mpi=s'                 => \$opt_mpi,
        'mpi-dir=s'             => \$opt_mpi_dir,
        'fetchonly'             => \$opt_fetchonly,
        'dry-run'               => \$opt_dryrun,
        'install-dir=s'         => \$opt_install_dir,
        'install-mpi-dir=s'     => \$opt_install_mpi_dir,
        'cmake-dir=s'           => \$opt_cmake_dir,
        'tmp-dir=s'             => \$opt_tmp_dir,
        'dwnldsrc=s'            => \$opt_dwnldsrc,
        'makeopts=s'            => \$opt_makeopts,
        'install=s'             => \@opt_install_list,
    );

    # show help if required
    if ($opt_help != 0)
    {
      print <<ZZZ;
install-deps.pl : Install software dependencies for coolfluid

usage: install-deps.pl [options]

By default will install the 'basic' set of dependencies: 
 [cmake,boost,openmpi,parmetis]

options:
        --help             Show this help.
        --verbose          Print every comand before executing.
        --nocolor          Don't color output
        --list             List packages that this script can install
        --fetchonly        Just download the sources. Do not install anything.
        --dry-run          Don't actually install, just output what you would do.

        --debug            Compile some dependencies with debug symbols [petsc]
        --no-fortran       Dont compile any fortran bindings (on mpi, etc...)
        --nompi            Don't compile with mpi support. This is only active for some packages.

        --mpi=             MPI implementation [$opt_mpi]
        --many-mpi=        Install all mpi related packages in a separate directory
                           therefore allowing multiple mpi environments to coexist [$opt_many_mpi]

        --install-dir=     Install location of the packages [$opt_install_dir]
        --install-mpi-dir= Install location for the mpi dependent installations [$opt_install_mpi_dir]
        
        --cmake-dir=       Location for the cmake installation []
        --mpi-dir=         Location for the MPI implementation []
        --tmp-dir=         Location of the temporary directory for complation [$opt_tmp_dir]

        --dwnldsrc=        Download server [$opt_dwnldsrc]
        --makeopts=        Options to pass to make [$opt_makeopts]

        --install          Comma separated list of packages to install. Example: --install=basic,hdf5,lam
                           Special meta keywords for installation: [ basic | recommended | all ]

ZZZ
    exit(0);
    }

	if($opt_list != 0)
	{
		print my_colored("install-deps.pl - can install the following packages:\n",$OKCOLOR);
		
		foreach my $pname (keys %packages) 
		{
			print "Package $pname\t[$packages{$pname}[$vrs]]\n";
	  	}
		exit(0);
	}

    @opt_install_list = split(/,/,join(',',@opt_install_list));
}

#==========================================================================
# Helper funcions
#==========================================================================

sub my_colored ($$)
{
  return ($opt_nocolor ? shift : colored($_[0], $_[1]));
}

#==========================================================================

sub rm_file ($)
{
  my ($file) = @_;
  unlink($file) || warn "warn: not deleting $file: $!";
}

#==========================================================================

sub get_command_status($)
{
    my ($args)=@_;
    if ($opt_verbose) { print my_colored("Executing   : $args\n",$OKCOLOR) };
    unless ($opt_dryrun) {
        my $status = system($args);
        return $status;
    }
    return 0;
}

#==========================================================================

sub run_command_or_die($)
{
    my ($args)=@_;
    if ($opt_verbose) { print my_colored("Executing   : $args\n",$OKCOLOR) };
    unless ($opt_dryrun) {
        my $status = system($args);
        print my_colored("Exit Status : $status\n",$OKCOLOR);
        die "$args exited with error" unless $status == 0;
    }
}

#==========================================================================

sub run_command($)
{
    my ($args)=@_;
    if ($opt_verbose)  { print my_colored("Executing   : $args\n",$OKCOLOR) };
    my $output;
    my $command = join("",$args,"|");
    my $pid=open READER, $command or die "Can't run the program: $args $!\n";
    while(<READER>){
       $output.=$_;
    }
    close READER;
    # print my_colored($output,$OKCOLOR);
    return $output;
}

#==========================================================================

sub safe_chdir($)
{
    my ($dir)=@_;
    print my_colored("Changing to dir $dir\n",$DEBUGCOLOR);
    chdir($dir) or die "Cannot chdir to $dir ($!)";
}

#==========================================================================

sub safe_copy($$)
{
    my ($orig,$targ)=@_;
    copy ($orig,$targ) or die "Cannot copy $orig to $targ ($!)";
}

#==========================================================================

sub safe_delete($)
{
    unlink("$_") or die "Failed to delete file $_\n";
}

#==========================================================================

sub get_arch() # returns the current architecture
{
    my $args="uname -m";
    my $arch = run_command($args);
    chomp($arch);
    return $arch;
}

#==========================================================================

sub is_mac()
{
    my $args="uname -s";
    my $arch = run_command($args);
    chomp($arch);
    if ( $arch =~ m/Darwin/ ) {
        return 1;
    } else {
        return 0;
    }
}

#==========================================================================
# Local functions
#==========================================================================

sub domkdir ()
{
    mkpath "$opt_install_dir";
    mkpath "$opt_install_dir/bin";
    mkpath "$opt_install_dir/local";
    mkpath "$opt_install_dir/lib";
    mkpath "$opt_install_dir/include";
    mkpath "$opt_install_dir/share";
    mkpath "$opt_install_mpi_dir";
    mkpath "$opt_cmake_dir";

    mkpath "$opt_tmp_dir";
}

#==========================================================================

sub prepare ()
{
    # set the default mpi
    $packages{$opt_mpi}[$dft] = 'on';

    # set the mpi install dir if the user did not set
    if ($opt_install_mpi_dir eq "")
    {
      if ($opt_many_mpi)
      {
        my $version = $packages{"$opt_mpi"}[$vrs];
        $opt_install_mpi_dir = "$opt_install_dir/mpi/$opt_mpi-$version";
      } else {
        $opt_install_mpi_dir = $opt_install_dir;
      }
    }

    # set the mpi dir if the user did not set
    if ($opt_mpi_dir eq "")
    {
        $opt_mpi_dir = $opt_install_mpi_dir;	  
    }


    # set the cmake dir if the user did not set
    if ($opt_cmake_dir eq "")
    {
      $opt_cmake_dir = $opt_install_dir;
    }

    # make directories for installation
    unless ($opt_dryrun) { domkdir(); }

    # normal paths
    $ENV{PATH} = "$opt_install_dir/bin:" . $ENV{PATH};
    $ENV{LD_LIBRARY_PATH} = "$opt_install_dir/lib:" . $ENV{LD_LIBRARY_PATH};

    # mpi specific paths
    $ENV{PATH} = "$opt_mpi_dir/bin:" . $ENV{PATH};
    $ENV{LD_LIBRARY_PATH} = "$opt_mpi_dir/lib:" . $ENV{LD_LIBRARY_PATH};

    $ENV{CFLAGS}   = "-O2" . $ENV{CFLAGS};
    $ENV{CXXFLAGS} = "-O2" . $ENV{CXXFLAGS};
    $ENV{FFLAGS}   = "-O2" . $ENV{FFLAGS};
    $ENV{F77FLAGS} = $ENV{FFLAGS};
    $ENV{F90FLAGS} = $ENV{FFLAGS};

    if ($arch eq "x86_64" )
    {
        $ENV{CFLAGS}   = "-fPIC " . $ENV{CFLAGS};
        $ENV{CXXFLAGS} = "-fPIC " . $ENV{CXXFLAGS};
        $ENV{FFLAGS}   = "-fPIC " . $ENV{FFLAGS};
        $ENV{F77FLAGS}  = "-fPIC " . $ENV{F77FLAGS};
        $ENV{F90FLAGS}  = "-fPIC " . $ENV{F90FLAGS};
    }

    if ( !(exists $ENV{CC}) )
    {
      $ENV{CC} = "gcc";
      print "Setting C compiler to \"".$ENV{CC}."\". Overide this with environment variable \"CC\"\n" if ($opt_verbose);
    }

    if ( !(exists $ENV{CXX}) )
    {
      $ENV{CXX} = "g++";
      print "Setting C++ compiler to \"".$ENV{CXX}."\". Overide this with environment variable \"CXX\"\n" if ($opt_verbose);
    }

    unless ($opt_no_fortran)
    {
	    if (!((exists $ENV{FC}) or (exists $ENV{F77})))
	    {
	      $ENV{FC} = "gfortran";
	      print "Setting Fortran compiler to \"".$ENV{FC}."\". Overide this with environment variable \"FC\".\n" if ($opt_verbose);
	    }

	    # makes sure the both compiler variable F77 and FC always exist
	    if ( !(exists $ENV{FC}) )
	    {
	      print "Setting FC equal to F77\n" if ($opt_verbose);
	      $ENV{FC} = $ENV{F77};
	    }
	    if ( !(exists $ENV{F77}) )
	    {
	      print "Setting F77 equal to FC\n" if ($opt_verbose);
	      $ENV{F77} = $ENV{FC};
	    }
	}
}

#==========================================================================

sub download_file ($) {
  my ($url)=@_;
  return get_command_status("$opt_dwnldprog $url");
}

#==========================================================================

sub remote_file_exists($) {
  my ($file)=@_;
  my $status = "";

  if ($opt_dwnldprog eq $opt_curlprog) {
    $status = run_command("curl -sl $opt_dwnldsrc/ | grep --quiet '$file' && echo 1");
  } elsif ($opt_dwnldprog eq $opt_wgetprog) {
    $status = run_command("wget -q --spider $opt_dwnldsrc/$file && echo 1");
  } else {
    print my_colored("could not check for file.\n",$DEBUGCOLOR);
  }
  if ($status eq "") {
    return 0;
  } else {
    return 1;
  }
}

#==========================================================================

sub download_src ($) {
  
  my ($stem)=@_;
  my $gotit = 0;

  foreach my $ext (@extensions) 
  {
	my $cfile = "$stem.$ext";
	if ( not -e $cfile ) # download only if does not exist
	{ 
      if ( remote_file_exists($cfile) ) { 
		my $status = download_file("$opt_dwnldsrc/$cfile"); 
		die "Download of $cfile exited with error" unless $status == 0;
		$gotit = 1;
	  }
    }
    else { print my_colored("file $cfile already exists, not downloading.\n",$OKCOLOR); $gotit = 1; }
  }

  die "Could not get source file '$stem' with neither extension [". join(',',@extensions) ."]" if ( not $gotit );
}

#==========================================================================

sub check_curlprog() {
  my $status = run_command("which curl");
  if ($status eq "") {
    return 0;
  } else {
    return 1;
  }
}

#==========================================================================

sub check_wgetprog() {
  my $status = run_command("which wget");
  if ($status eq "") {
    print my_colored("wget is not installed, checking for curl...\n",$DEBUGCOLOR);
    if(check_curlprog()) {
      $opt_dwnldprog = $opt_curlprog;
      print my_colored("curl found, using curl instead of wget\n",$DEBUGCOLOR);
      return 1;
    } else{
      print my_colored("curl and wget not found... install wget manually\n",$DEBUGCOLOR);
      return 0;
    }
  } else {
    return 1;
  }
}

#==========================================================================

sub install_wgetprog() {
  my $lib = "wget";
  my $version = $packages{$lib}[0];
  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  untar_src("$lib-$version");
  safe_chdir("$opt_tmp_dir/$lib-$version/");
  run_command_or_die("./configure --prefix=$opt_install_dir");
  run_command_or_die("make $opt_makeopts");
  run_command_or_die("make install");
}

#==========================================================================

sub untar_src ($) {
  my ($stem)=@_;
  my $gotit = 0;

  foreach my $ext (@extensions) 
  {
	my $cfile = "$stem.$ext";
	my $status = -1;
	if ( -e $cfile ) 
	{
	    $status = get_command_status("tar zxf $stem.$ext") if( $ext eq "tar.gz" );
	    $status = get_command_status("tar zxf $stem.$ext") if( $ext eq "tgz" );
	    $status = get_command_status("tar jxf $stem.$ext") if( $ext eq "tar.bz2" );
	    $status = get_command_status("unzip   $stem.$ext") if( $ext eq "zip" );

	    print my_colored("Exit Status : $status\n",$OKCOLOR);
	    die "Unpack of $stem exited with error" unless $status == 0;
		$gotit = 1;
    }
  }

  die "Could uncompress source file '$stem' with neither extension [". join(',',@extensions) ."]" if ( not $gotit );
}

#==========================================================================

sub install_google_perftools ()
{
  my $lib="google-perftools";
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --enable-frame-pointers  --prefix=$opt_install_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_gnu ($)
{
  my ($lib)=@_;
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --prefix=$opt_install_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_curl ()
{
  my ($lib)= "curl";
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --prefix=$opt_install_dir --without-ssl --without-libidn --without-gnutls --disable-ipv6 ");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_blas()
{
  if (is_mac()) {
    print "Skipping because MacOSX is smarter and already has it ;) \n"
  } else {
    my $lib = "blas";
    my $version = $packages{$lib}[$vrs];
    print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

    safe_chdir($opt_tmp_dir);
    download_src("$lib-$version");
    unless ($opt_fetchonly) {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src("$lib-$version");
      safe_chdir("$opt_tmp_dir/$lib-$version/");

      # fix Makefile
      my $filename = 'Makefile';
      safe_copy($filename,"$filename.orig");
      open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
      open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
      while (<IN>) {
      chomp;
          s/FFLAGS=(([\w]*)(\S*))*/FFLAGS=$ENV{FFLAGS}/g;
          s/cc -shared/gcc -shared $ENV{LDFLAGS}/g;
          print "$_\n";
          print OUT "$_\n";
      }
      close IN;
      close OUT;


      run_command_or_die("make all");
      safe_copy("libblas.so.$version","$opt_install_dir/lib/libblas.so.$version") or die;
      safe_copy("libblas.a","$opt_install_dir/lib/libblas.a");

      # fix some links
      safe_chdir("$opt_install_dir/lib");
      rm_file("libblas.so");   # if it fails is OK
      rm_file("libblas.so.3"); # if it fails is OK
      run_command_or_die("ln -sf libblas.so.$version libblas.so.3");
      run_command_or_die("ln -sf libblas.so.$version libblas.so");
    }
  }
}

#==========================================================================

sub install_lapack() {
  if (is_mac()) {
    print "Skipping because MacOSX is smarter and already has it ;) \n"
  } else {
    my $lib = "lapack";
    my $version = $packages{$lib}[$vrs];
    print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

    safe_chdir($opt_tmp_dir);
    download_src("$lib-$version");

    unless ($opt_fetchonly) {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src("$lib-$version");
      safe_chdir("$opt_tmp_dir/$lib-$version/SRC");

      # fix Makefile
      my $filename = 'Makefile';
      safe_copy($filename,"$filename.orig");
      open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
      open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
      while (<IN>) {
      chomp;
          s/FFLAGS=(([\w]*)(\S*))*/FFLAGS=$1 $ENV{FFLAGS}/g;
          s/BLAS_PATH=/BLAS_PATH=$opt_install_dir\/lib/g;
          s/INSTALL_PATH=/INSTALL_PATH=$opt_install_dir\/lib/g;
          s/cc -shared/gcc -shared $ENV{LDFLAGS}/g;
          print "$_\n";
          print OUT "$_\n";
      }
      close IN;
      close OUT;

      run_command_or_die("make all");

      safe_copy("liblapack.so.$version","$opt_install_dir/lib/liblapack.so.$version") or die;
      safe_copy("liblapack.a","$opt_install_dir/lib/liblapack.a");

      # fix some links
      safe_chdir("$opt_install_dir/lib");
      rm_file("liblapack.so");   # if it fails is OK
      rm_file("liblapack.so.3"); # if it fails is OK
      run_command_or_die("ln -sf liblapack.so.$version liblapack.so.3");
      run_command_or_die("ln -sf liblapack.so.$version liblapack.so");
    }
  }
}

#==========================================================================

sub install_lam() {
  my $lib = "lam";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");

  if ($ENV{CXX} eq "g++" ) { $ENV{CXX} = $ENV{CXX} . " -fpermissive"; }

  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --enable-shared --enable-static --with-threads=posix --enable-long-long --enable-languages=c,c++,f77 --disable-checking --enable-cstdio=stdio --with-system-zlib --prefix=$opt_mpi_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_openmpi() {

  my $lib = "openmpi";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");

  my $fortran_opts = "";
  if ( $opt_no_fortran )
  {
	$fortran_opts = "--disable-mpi-f77 --disable-mpi-f90" ;
  }
  else
  {
	# support fortran but not f90
	$fortran_opts = "FC=$ENV{F77} --disable-mpi-f90";
  } 

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure CC=$ENV{CC} CXX=$ENV{CXX} --disable-visibility --without-cs-fs --with-threads=posix $fortran_opts --prefix=$opt_mpi_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_mpich2() {
  my $lib = "mpich2";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly)
  {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src("$lib-$version");
      safe_chdir("$opt_tmp_dir/$lib-$version/");
      run_command_or_die("./configure --enable-cxx --enable-f77 --enable-f90 --enable-sharedlibs=osx-gcc --prefix=$opt_mpi_dir");
      run_command_or_die("make $opt_makeopts");
      run_command_or_die("make install");
  }
}

#==========================================================================

sub install_cgns() {
  my $lib = "cgns";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    
    mkpath("build",1);
    safe_chdir("build");
    run_command_or_die("cmake ../ -DCMAKE_C_FLAGS=\"-Wno-return-type\" -DHDF5_LIBRARY_DIR=$opt_install_mpi_dir/lib -DHDF5_INCLUDE_DIR=$opt_install_mpi_dir/include -DHDF5_NEED_MPI=ON -DHDF5_NEED_ZLIB=ON -DHDF5_NEED_SZIP=OFF -DMPI_INCLUDE_DIR=$opt_mpi_dir/include -DMPI_LIBRARY_DIR=$opt_mpi_dir/lib -DCMAKE_INSTALL_PREFIX=$opt_install_mpi_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_cgal() {
  my $lib = "cgal";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    mkpath("build",1);
    safe_chdir("build");
    run_command_or_die("cmake ../ -Wno-dev -DBOOST_ROOT=$opt_install_dir -DCMAKE_INSTALL_PREFIX=$opt_install_dir -DCMAKE_BUILD_TYPE=Release -DWITH_GMP=OFF -DWITH_CGAL_Qt3=OFF" );
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_mpich() {
  my $lib = "mpich";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly)
  {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src("$lib-$version");
      safe_chdir("$opt_tmp_dir/$lib-$version/");
      run_command_or_die("./configure --prefix=$opt_mpi_dir --enable-f77 --enable-f90");
      run_command_or_die("make $opt_makeopts");
      run_command_or_die("make install");
  }
}

#==========================================================================

sub install_parmetis () {
  my $lib = "parmetis";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  my $include_dir = "$opt_install_mpi_dir/include/";
  my $lib_dir = "$opt_install_mpi_dir/lib/";

  mkpath $include_dir;
  mkpath $lib_dir;

  safe_chdir($opt_tmp_dir);
  download_src("ParMetis-$version");
  unless ($opt_fetchonly) {

    rmtree "$opt_tmp_dir/ParMetis-$version";
    untar_src("ParMetis-$version");
    safe_chdir("$opt_tmp_dir/ParMetis-$version/");

       my $filename = 'Makefile.in';
        safe_copy($filename,"$filename.orig");
        open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
        open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
        while (<IN>) 
        {
        chomp;
        my $line = $_;
        # add include for malloc.h
        if (is_mac())   { $line =~ s/(^INCDIR\s=\s?$)/INCDIR = -I\/usr\/include\/malloc\//g; }
        if ($opt_nompi) { $line =~ s/mpicc/gcc/g; }
        $line =~ s/\-O3/$ENV{CFLAGS}/g;
        print OUT "$line\n";
        }
        print my_colored("Modified Makefile.in to include malloc for MacOSX\n",$DEBUGCOLOR);
        close IN;
        close OUT;

    safe_chdir("METISLib");
    run_command_or_die("make $opt_makeopts");
    safe_chdir("..");

    safe_chdir("ParMETISLib");
    run_command_or_die("make $opt_makeopts");
    safe_chdir("..");

    safe_copy("parmetis.h","$include_dir/parmetis.h");
    safe_copy("libmetis.a","$lib_dir/libmetis.a");
    safe_copy("libparmetis.a","$lib_dir/libparmetis.a");
  }
}

#==========================================================================

sub install_petsc3 ()
{
  my $lib = "petsc";
  my $version = $packages{"$lib"}[$vrs];
  my $source_file = "$lib-$version.tar.gz";
  my $fblas_name = "fblaslapack-3.1.1.tar.gz";
  my $fblas_file = "$opt_tmp_dir/$fblas_name";

  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);
    

  safe_chdir($opt_tmp_dir);

  if ( not -e $source_file ) { download_file("$opt_dwnldsrc/$source_file") };
  if ( not -e $fblas_file  ) { download_file("$opt_dwnldsrc/$fblas_name") };

  unless ($opt_fetchonly)
  {
    my $build_dir   = "$opt_tmp_dir/$lib-$version";
    my $install_dir = "$opt_install_mpi_dir/";
    my $petsc_arch  = "arch-$arch";
    if (is_mac()) { $petsc_arch = "arch-darwin"; };

    $ENV{PETSC_DIR}  = "$build_dir";
    $ENV{PETSC_ARCH} = $petsc_arch;

    # extract sources to build dir
    rmtree $build_dir;
    untar_src("$lib-$version");

    safe_chdir("$build_dir");

    my $wdebug = "";
    if ($opt_debug) { $wdebug = "--with-debugging=1" };

    # handle blas/lapack and fortran at the same time (depend on each other)
    my $wblaslib = "";
    my $fortran_opt = "";
    if ($opt_no_fortran)
    {
       $fortran_opt = " --with-fc=0 ";
       if ( not is_mac() ) # use the downloaded c blas sources
       {
          $wblaslib = "--download-c-blas-lapack=1";
       }
    }
    else
    {
       if ( not is_mac() ) # use the downloaded fortran blas sources
       {
          $wblaslib = "--download-f-blas-lapack=\"$fblas_file\"";
       }
    }

    if (is_mac()) {
      # use built-in optimized blas-lapack lib
      $wblaslib = "--with-blas-lapack-lib=\"-framework vecLib\"";
    }

    run_command_or_die("./config/configure.py --prefix=$install_dir $wdebug COPTFLAGS='-O3' FOPTFLAGS='-O3' --with-mpi-dir=$opt_mpi_dir $fortran_opt $wblaslib --with-shared=1 --with-dynamic=1 --with-c++-support --PETSC_ARCH=$petsc_arch");

    run_command_or_die("make $opt_makeopts");

    run_command_or_die("make install PETSC_DIR=$build_dir");

  }
}

#==========================================================================

sub install_trilinos() {

  install_parmetis();

  my $lib = "trilinos";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");

  my $tri_mpi_opt = "";
  unless ($opt_nompi) {

      $tri_mpi_opt = "-D TPL_ENABLE_MPI:BOOL=ON \\
                      -D MPI_BASE_DIR_PATH:PATH=$opt_mpi_dir \\
                      -D CMAKE_C_COMPILER:FILEPATH=$opt_mpi_dir/bin/mpicc \\
                      -D CMAKE_CXX_COMPILER:FILEPATH=$opt_mpi_dir/bin/mpic++ ";

      $tri_mpi_opt .= "-D CMAKE_Fortran_COMPILER:FILEPATH=$opt_mpi_dir/bin/mpif77 " unless ($opt_no_fortran);
  }


  my $tri_parmetis_opt = "-D Zoltan_ENABLE_ParMETIS:BOOL=ON \\
                          -D ParMETIS_INCLUDE_DIRS:FILEPATH=\"$opt_install_mpi_dir/include\" \\
                          -D ParMETIS_LIBRARY_DIRS:FILEPATH=\"$opt_install_mpi_dir/lib\"";
  my $tri_scotch_opt = "-D Zoltan_ENABLE_Scotch:BOOL=ON \\
                        -D Scotch_INCLUDE_DIRS:FILEPATH=\"$opt_install_mpi_dir/include\" \\
                        -D Scotch_LIBRARY_DIRS:FILEPATH=\"$opt_install_mpi_dir/lib\"";
  my $tri_patoh_opt = "-D Zoltan_ENABLE_PaToH:BOOL=ON \\
                       -D PaToH_LIBRARY_DIRS:FILEPATH=\"$opt_install_mpi_dir/include\" \\
                       -D PaToH_INCLUDE_DIRS:FILEPATH=\"$opt_install_mpi_dir/lib\"";

 
  my $tri_fortran_opt = "";
  if ($opt_no_fortran) { $tri_fortran_opt = "-D Trilinos_ENABLE_Fortran:BOOL=OFF " };

  unless ($opt_fetchonly) 
  {
    my $build_dir =  "$opt_tmp_dir/$lib-$version-Source/build"; 

    rmtree "$opt_tmp_dir/$lib-$version-Source";
    untar_src("$lib-$version");
    # make build dir - newer versions dont support in-source builds
    mkpath $build_dir or die ("could not create dir $build_dir\n");
    safe_chdir($build_dir);
    run_command_or_die("$opt_cmake_dir/bin/cmake -G KDevelop3 \\
      -D CMAKE_INSTALL_PREFIX:PATH=$opt_install_mpi_dir -D CMAKE_BUILD_TYPE:STRING=RELEASE \\
      -D Trilinos_ENABLE_DEFAULT_PACKAGES:BOOL=OFF \\
      -D Trilinos_ENABLE_ALL_OPTIONAL_PACKAGES:BOOL=ON \\
      -D Trilinos_ENABLE_TESTS:BOOL=OFF \\
      -D Trilinos_ENABLE_Amesos:BOOL=ON \\
      -D Trilinos_ENABLE_AztecOO:BOOL=ON \\
      -D Trilinos_ENABLE_Belos:BOOL=ON \\
      -D Trilinos_ENABLE_Didasko:BOOL=OFF \\
      -D Didasko_ENABLE_TESTS:BOOL=OFF \\
      -D Didasko_ENABLE_EXAMPLES:BOOL=OFF \\
      -D Trilinos_ENABLE_Epetra:BOOL=ON \\
      -D Trilinos_ENABLE_EpetraExt:BOOL=ON \\
      -D Trilinos_ENABLE_Tpetra:BOOL=ON \\
      -D Trilinos_ENABLE_TpetraExt:BOOL=ON \\
      -D Trilinos_ENABLE_Ifpack:BOOL=ON \\
      -D Trilinos_ENABLE_Meros:BOOL=ON \\
      -D Trilinos_ENABLE_ML:BOOL=ON \\
      -D Trilinos_ENABLE_RTOp:BOOL=ON \\
      -D Trilinos_ENABLE_Teuchos:BOOL=ON \\
      -D Trilinos_ENABLE_Thyra:BOOL=ON \\
      -D Trilinos_ENABLE_ThyraCore:BOOL=ON \\
      -D Trilinos_ENABLE_Triutils:BOOL=ON \\
      -D Trilinos_ENABLE_Stratimikos:BOOL=ON \\
      -D Trilinos_ENABLE_Zoltan:BOOL=ON \\
      -D Zoltan_ENABLE_EXAMPLES:BOOL=ON \\
      -D Zoltan_ENABLE_TESTS:BOOL=ON \\
      -D Zoltan_ENABLE_ULLONG_IDS:Bool=ON \\
      $tri_parmetis_opt \\
      -D TPL_ENABLE_BLAS:BOOL=ON \\
      -D TPL_ENABLE_LAPACK:BOOL=ON \\
      $tri_fortran_opt \\
      $tri_mpi_opt \\
      -D CMAKE_VERBOSE_MAKEFILE:BOOL=FALSE \\
      -D BUILD_SHARED_LIBS:BOOL=ON\\
      -D Trilinos_VERBOSE_CONFIGURE:BOOL=FALSE  $opt_tmp_dir/$lib-$version-Source"
    );

    #-D CMAKE_Fortran_COMPILER:FILEPATH=$opt_install_dir/bin/mpif90
    #-D TPL_ENABLE_PARMETIS:BOOL=ON \\
    #-D PARMETIS_LIBRARY_DIRS:PATH=\"$opt_install_dir/lib\" \\
    #-D PARMETIS_INCLUDE_DIRS:PATH=\"$opt_install_dir/include\" \\

    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub boost_arch()
{
	my $boost_arch;
    
    # linux
	if($arch eq "x86_64") { $boost_arch = "linuxx86_64" ;  }
    if($arch eq "i686")   { $boost_arch = "linuxx86" ;  }

    if(is_mac())         
    { 
        $boost_arch = "macosxx86"; 
      
        # If Snow Leopard
        my $capable64 = run_command("sysctl hw | grep 'hw.cpu64bit_capable: [0-9]'");
        my $OSversion = run_command("sw_vers | grep 'ProductVersion:'");
        if ($capable64 =~ /hw.cpu64bit_capable:\s1/ && ( $OSversion =~ /10\.6\.*/ || $OSversion =~ /10\.7\.*/ ) ) 
        {
           $boost_arch = "macosxx86_64";    
        }
    }
	return $boost_arch;
}

#==========================================================================

sub install_boost_jam()
{
  my $lib = "boost-jam";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib-$version";

  print my_colored("Installing $pack\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$pack");

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$pack";
    untar_src("$pack");
    safe_chdir("$opt_tmp_dir/$pack/");

    # select the toolset
    my $toolset = "gcc";
    if( ( $ENV{CC} =~ m/icc$/   ) or ( $ENV{CXX} =~ m/icpc$/ )      ) { $toolset = "intel-linux"; }
    if( ( $ENV{CC} =~ m/clang$/ ) or ( $ENV{CXX} =~ m/clang\+\+$/ ) ) { $toolset = "cc"; }

    if ($toolset eq 'gcc' ) # in case g++ is special path
    {
      $ENV{GCC} = $ENV{CC};
      $ENV{GXX} = $ENV{CXX};
    }

    if( is_mac() and $toolset eq "gcc" ) { $toolset = "darwin"; }
	
    my $output = run_command("sh build.sh $toolset");

    my $boost_arch = boost_arch();

    move ( "$opt_tmp_dir/$pack/bin.$boost_arch/bjam", "$opt_install_dir/bin" ) or die "Cannot move bjam to $opt_install_dir/bin ($!)";
  }
}

#==========================================================================

sub install_boost()
{
  my $lib = "boost";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib\_$version";
  my $bjamcfg="$opt_tmp_dir/$pack/user-config.jam";

  print my_colored("Installing $pack\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$pack");

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$pack";
    untar_src("$pack");
    safe_chdir("$opt_tmp_dir/$pack/");

	# select the toolset
    my $toolset = "gcc";
    if( ( $ENV{CC} =~ m/icc$/   ) or ( $ENV{CXX} =~ m/icpc$/ )      ) { $toolset = "intel-linux"; }
    if( ( $ENV{CC} =~ m/clang$/ ) or ( $ENV{CXX} =~ m/clang\+\+$/ ) ) { $toolset = "clang"; }

    if ($toolset eq 'gcc' ) # in case g++ is special path
    {
      $ENV{GCC} = $ENV{CC};
      $ENV{GXX} = $ENV{CXX};
    }

    if( is_mac() and $toolset eq "gcc" ) { $toolset = "darwin"; }

    # check if we need to build bjam and build if needed
 
  	my $bjampath = run_command("which bjam");
    chomp $bjampath;

    my $boost_arch = boost_arch();

    if ($bjampath eq "" and -e "tools/build/v2/engine/build.sh" ) # newer builds of boost >= 1.48
    {
      print "building tools/build/v2/engine\n";

      safe_chdir("tools/build/v2/engine");

      run_command_or_die("sh build.sh $toolset");

      $bjampath="$opt_tmp_dir/$pack/tools/build/v2/engine/bin.$boost_arch/bjam";
      if ( not -e $bjampath ) { die "Cannot find bjam in $bjampath" }
    }
    elsif ($bjampath eq "" and -d "tools/build/v2/engine/src" ) # newer builds of boost >= 1.45
    {
      print "building tools/build/v2/engine/src\n";

      safe_chdir("tools/build/v2/engine/src");

      run_command_or_die("sh build.sh $toolset");

      $bjampath="$opt_tmp_dir/$pack/tools/build/v2/engine/src/bin.$boost_arch/bjam";
      if ( not -e $bjampath ) { die "Cannot find bjam in $bjampath" }
    }
	elsif ($bjampath eq "" and -d "tools/jam/src" ) # older builds of boost <= 1.44
    {
      print "building tools/jam/src\n";

      safe_chdir("tools/jam/src");
      
      run_command_or_die("sh build.sh $toolset");

      $bjampath="$opt_tmp_dir/$pack/tools/jam/src/bin.$boost_arch/bjam";
      if ( not -e $bjampath ) { die "Cannot find bjam in $bjampath" }
    }


    if ($bjampath eq "") # still empty so something went wrong
    {
      die ("did not find bjam in path and could not find bjam to build in boost sources\n");
    }

    # build boost libs
    safe_chdir("$opt_tmp_dir/$pack");
   
    if( ( $ENV{CC} =~ m/clang$/ ) or ( $ENV{CXX} =~ m/clang\+\+$/ ) )
    {
      open  ( USERCONFIGJAM, ">>$bjamcfg" ) || die("Cannot open file $bjamcfg") ;
      print   USERCONFIGJAM <<ZZZ;
# ----------------------
# clang configuration.
# ----------------------
using clang ;

ZZZ
      close (USERCONFIGJAM); 
    }

    my $boostmpiopt=" --without-mpi ";
    unless ($opt_nompi) {
      $boostmpiopt=" --with-mpi cxxflags=-DBOOST_MPI_HOMOGENEOUS ";
      open  ( USERCONFIGJAM, ">>$bjamcfg") || die("Cannot open file $bjamcfg") ;
      print   USERCONFIGJAM <<ZZZ;

# ----------------------
# mpi configuration.
# ----------------------
using mpi : $opt_mpi_dir/bin/mpicxx ;

ZZZ
      close (USERCONFIGJAM); 
    }
    run_command_or_die("$bjampath --user-config=$bjamcfg --prefix=$opt_install_dir --with-test --with-thread --with-iostreams --with-filesystem --with-system --with-regex --with-date_time --with-program_options --with-python $boostmpiopt toolset=$toolset threading=multi variant=release stage install $opt_makeopts");
  }
}

#==========================================================================

sub install_cmake() {
  my $lib = "cmake";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib-$version";
  print my_colored("Installing $pack\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");

  unless ($opt_fetchonly) {

    rmtree "$opt_tmp_dir/$pack";
    run_command_or_die("tar zxf $pack.tar.gz");
    safe_chdir("$opt_tmp_dir/$pack/");

    run_command_or_die("./bootstrap --prefix=$opt_cmake_dir");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");

  }
}

#==========================================================================

sub install_hdf5() {
  my $lib = "hdf5";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    my $old_cc  = $ENV{CC};
    my $old_cxx = $ENV{CXX};
    unless ($opt_nompi) {
        $ENV{CC}   = "mpicc";
        $ENV{CXX}  = "mpic++";
    }

    run_command_or_die("./configure --prefix=$opt_install_mpi_dir --enable-zlib --enable-linux-lfs --with-gnu-ld --enable-hl --enable-shared");
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");

    $ENV{CC}   = $old_cc;
    $ENV{CXX}  = $old_cxx;
  }
}

#==========================================================================

sub install_superlu() {
  my $lib = "superlu";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-$version");
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src("$lib-$version");
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    
    mkpath("build",1);
    safe_chdir("build");
    run_command_or_die("cmake ../ -DBOOST_ROOT=$opt_install_dir -DCMAKE_INSTALL_PREFIX=$opt_install_dir -DCMAKE_BUILD_TYPE=Release" );
    run_command_or_die("make $opt_makeopts");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_qt() {
  my $lib = "qt";
  my $version = $packages{$lib}[$vrs];

  print my_colored("Installing $lib-everywhere-opensource-src-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("$lib-everywhere-opensource-src-$version");

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-everywhere-opensource-src-$version";
    untar_src("$lib-everywhere-opensource-src-$version");
    safe_chdir("$opt_tmp_dir/$lib-everywhere-opensource-src-$version/");

    # -make libs => compile libraries
    # -make tools => compile tools (uic, rcc,...)
    # -opensource => compile with LGPL license
    # -fast => configure step is faster (do not generate Makefiles for what will not be compiled)
    #  echo "y" => accept the licence automatically
    run_command_or_die ("echo \"y\" | ./configure -prefix $opt_install_dir -make libs -make tools --no-qt3support -opensource -fast");
    run_command_or_die ("make $opt_makeopts");
    run_command_or_die ("make install");
  }
    
}

#==========================================================================

sub install_paraview() {
  my $lib = "paraview";
  my $version = $packages{$lib}[$vrs];

  print my_colored("Installing ParaView-$version\n",$HIGHLIGHTCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src("ParaView-$version");
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/ParaView-$version";
    untar_src("ParaView-$version");
    safe_chdir("$opt_tmp_dir/ParaView-$version/");
   
    mkpath("build",1);
    safe_chdir("build");
    run_command_or_die("cmake ../ -DCMAKE_INSTALL_PREFIX=$opt_install_dir \\
         -DCMAKE_BUILD_TYPE=Release \\
         -DBUILD_SHARED_LIBS:BOOL=ON \\
         -DPARAVIEW_USE_MPI=ON \\
         -DBUILD_TESTING:BOOL=OFF \\
         -DBUILD_DOCUMENTATION=OFF \\
         -DBUILD_EXAMPLE=OFF \\
         -DPARAVIEW_BUILD_QT_GUI:BOOL=ON \\
         -DPARAVIEW_DISABLE_VTK_TESTING=ON \\
         -DPARAVIEW_ENABLE_PYTHON=OFF \\
         -DPARAVIEW_INSTALL_DEVELOPMENT=ON \\
         -DPARAVIEW_TESTING_WITH_PYTHON=OFF " );
    run_command_or_die("> ParaViewLibraryDepends.cmake");
    run_command_or_die("make $opt_makeopts");

    # Install ParaView.
    print "Installing ParaView. This might take some minutes.";

    # According to ParaView wiki, it's not safe to do 'make install' because
    # it might raise some issues. They advice instead to generate a package and
    # extract it to the installation directory. This operation asks about 1GB
    # free disk space and takes some minutes. The archive contains a main 
    # directory which contains the directories we want, so the process is a 
    # bit trickier that just "extracting".
    run_command_or_die("cpack -G TGZ");
    mkpath("extracting",1);
    run_command("mv IceT-*.tar.gz extracting");
    safe_chdir("extracting");
    # extract the archive and delete it (to gain disk space)
    run_command("tar zvxf IceT-*.tar.gz 2> /dev/null #; rm *.tar.gz");
    # Qt libraries were copied inside the archive. We have our own Qt libraries, 
    # so it's better to remove them in order to gain about 200MB of disk space and
    # avoid potential "duplicate symbols" errors. Note that QtTesting is not
    # removed since it's a ParaView library.
    run_command("rm -f \$(ls IceT-*/lib/paraview-*/libQt* | grep -v 'libQtTesting')");
    # remove copied MPI libraries as well
    run_command("rm -f \$(ls IceT-*/lib/paraview-*/libmpi*");
    # finally, we move the remaining files to their destination.
    run_command("cd IceT-*/ ; mv -f bin/* $opt_install_dir/bin");
    run_command("cd IceT-*/ ; mv -f doc/* $opt_install_dir/doc");
    run_command("cd IceT-*/ ; mv -f include/* $opt_install_dir/include");
    run_command("cd IceT-*/ ; mv -f lib/paraview* $opt_install_dir/lib");
    run_command("cd IceT-*/ ; mv -f share/man/man3* $opt_install_dir/man/man3/");
  }
}


#==========================================================================

sub print_info() # print information about the
{
    print my_colored("Installing coolfluid dependencies\n",$HIGHLIGHTCOLOR);

    print "---------------------------------\n"; 
    
    print "Install     dir : $opt_install_dir\n";
    print "Install MPI dir : $opt_install_mpi_dir\n";
    print "CMake       dir : $opt_cmake_dir\n";
    print "MPI         dir : $opt_mpi_dir\n";
    print "Temporary   dir : $opt_tmp_dir\n";

    print "---------------------------------\n"; 

# Env vars
    print "PATH            : $ENV{PATH}\n";
    print "LD_LIBRARY_PATH : $ENV{LD_LIBRARY_PATH}\n";
    print "CC  : $ENV{CC}\n";
    print "CXX : $ENV{CXX}\n";
    print "FC  : $ENV{FC}\n" unless ($opt_no_fortran);
    print "CFLAGS   : $ENV{CFLAGS}\n";
    print "CXXFLAGS : $ENV{CXXFLAGS}\n";
    print "FFLAGS   : $ENV{FFLAGS}\n"   unless ($opt_no_fortran);
    print "F77FLAGS : $ENV{F77FLAGS}\n" unless ($opt_no_fortran);
    print "F90FLAGS : $ENV{F90FLAGS}\n" unless ($opt_no_fortran);

    print "---------------------------------\n"; 
    
# Options
#     while ( my ($key, $value) = each(%options) ) {
#         print "$key : get_option($key)";
#     }

# User prefs
#     while ( my ($key, $value) = each(%user_pref) ) {
#         print "$key : $value" ;
#     }
}

#==========================================================================

sub set_install_basic()
{
  foreach my $pname (keys %packages) {
    $packages{$pname}[$ins] = $packages{$pname}[$dft];
  }
}

sub set_install_recommended()
{
  set_install_basic();

  $packages{"trilinos"}[$ins] = 'on';
  $packages{"hdf5"}[$ins] = 'on';
  $packages{"cgns"}[$ins] = 'on';
}

sub set_install_all()
{
  foreach my $pname (keys %packages) 
  {
    unless ( $pname eq 'lam' or $pname eq 'openmpi' or $pname eq 'mpich' or $pname eq 'mpich2' )
    {
      $packages{$pname}[$ins] = 'on';
    }
  }
  $packages{$opt_mpi}[$ins] = 'on';
}

#==========================================================================

sub install_packages()
{
  print_info();
  check_wgetprog();

    # check for 'basic' or 'all' keywords
    for ( my $i=0; $i < scalar @opt_install_list; $i++ )
    {
        if ($opt_install_list[$i] eq 'basic') { set_install_basic(); }
        if ($opt_install_list[$i] eq 'all')   { set_install_all(); }
        if ($opt_install_list[$i] eq 'recommended')   { set_install_recommended(); }
    }

    # if there is no package selected, then also copy the [$dft] to [$ins]
    if (scalar @opt_install_list == 0) { set_install_basic(); }

    # turn on the manually selected packages
    for ( my $i=0; $i < scalar @opt_install_list; $i++)
    {
        my $opt = $opt_install_list[$i];
        if (exists $packages{$opt})
        {
            $packages{$opt}[$ins] = 'on';
        }
        elsif (!($opt eq 'all') and !($opt eq 'basic') and !($opt eq 'recommended')) {
            print my_colored("Package does not exist: $opt\n",$ERRORCOLOR);
        }
    }

    my %install_packages = ();

    # sort the packages to install by priority
    foreach my $pname (keys %packages) {
    #       print "$pname\n";
        if ($packages{$pname}[$ins] eq 'on') {
            $install_packages{$packages{$pname}[$pri]} = $pname;
        }
    }

    my $actually_installed = "";

    # install the packages by priority
    foreach my $p (sort {$a <=> $b} keys %install_packages) {
        my $pname = $install_packages{$p};
        my $pversion = $packages{$pname}[$vrs];
        print my_colored("Package marked for installation: $pname\t[$pversion]\n",$OKCOLOR);
        unless ($opt_dryrun)
        {
          $packages{$pname}[$fnc]->();
          $actually_installed .= "$pname ";
        }
    }

    unless ($opt_dryrun)
    {
      print my_colored("\n\nInstalled sucessfully: $actually_installed\n",$OKCOLOR);
      print my_colored("\n!!! FINISHED INSTALLING ALL SELECTED DEPENDENCIES !!!\n\n",$OKCOLOR);
    }
}

#==========================================================================
# Main execution
#==========================================================================

parse_commandline();

prepare();

install_packages();
