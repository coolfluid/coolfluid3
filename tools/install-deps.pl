#!/usr/bin/env perl

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
my $HEADINGCOLOR = "bold";
my $DEBUGCOLOR = "yellow";
my $WARNCOLOR = "bold yellow";

#==========================================================================
# Global Variables
#==========================================================================

my $home = $ENV{HOME};
my $user = $ENV{USER};
my $arch = get_arch();

my $opt_help          = 0;
my $opt_dryrun        = 0;
my $opt_nocolor       = 0;
my $opt_envars        = 0;
my $opt_genconf       = 0;
my $opt_debug         = 0;
my $opt_nompi         = 0;
my $opt_mpi           = "openmpi";
my $opt_mpi_dir       = "";
my $opt_dependencies  = 0;
my $opt_sources       = 0;
my $opt_compile       = 0;
my $opt_fetchonly     = 0;
my $opt_many_mpi      = 0;
my $opt_install_dir   = "$home/local/$arch";
my $opt_install_mpi_dir = "";
my $opt_cmake_dir     = "";
my $opt_confile       = "coolfluid.conf";
my $opt_tmp_dir       = "$home/tmp";
my $opt_build         = "optim";
my $opt_dwnldsrc      = "http://coolfluidsrv.vki.ac.be/webfiles/coolfluid/packages";
my $opt_wgetprog      = "wget -nc -nd";
my $opt_curlprog      = "curl -O -nc -nv --progress-bar";
my $opt_dwnldprog     = $opt_wgetprog;
my $opt_makeopts      = "-j2";
my $opt_svnrevision       = 0;
my @opt_install = ();

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

$priority = 0;

# these packages are listed by priority
my %packages = (  #  version   default install priority      function
    "coreutils"  => [ "6.7",    'off',  'off', $priority++,  sub { install_gnu("coreutils") } ],
    "make"       => [ "3.81",   'off',  'off', $priority++,  sub { install_gnu("make") } ],
    "cmake"      => [ "2.8.1",  'on' ,  'off', $priority++,  \&install_cmake ],
    "wget"       => [ "1.11.4", 'off',  'off', $priority++,  \&install_wgetprog],
    "binutils"   => [ "2.16.1", 'off',  'off', $priority++,  sub { install_gnu("binutils") } ],
    "m4"         => [ "1.4.4",  'off',  'off', $priority++,  sub { install_gnu("m4") } ],
    "tar"        => [ "1.15",   'off',  'off', $priority++,  sub { install_gnu("tar") } ],
    "gcc4"       => [ "4.1.1",  'off',  'off', $priority++,  \&install_gcc4 ],
    "gcc3"       => [ "3.4.5",  'off',  'off', $priority++,  \&install_gcc3 ],
    "autoconf"   => [ "2.59",   'off',  'off', $priority++,  sub { install_gnu("autoconf") } ],
    "automake"   => [ "1.9.5",  'off',  'off', $priority++,  sub { install_gnu("automake") } ],
    "libtool"    => [ "1.5.14", 'off',  'off', $priority++,  sub { install_gnu("libtool") } ],
    "openssl"    => [ "0.9.7i", 'off',  'off', $priority++,  \&install_openssl ],
    "blas"       => [ "3.0.3",  'off',  'off', $priority++,  \&install_blas ],
    "clapack"    => [ "",       'off',  'off', $priority++,  \&install_clapack ],
    "lapack"     => [ "3.0.3",  'off',  'off', $priority++,  \&install_lapack ],
    "log4cpp"    => [ "0.3.4b", 'off',  'off', $priority++,  sub { install_gnu("log4cpp") } ],
    "zlib"       => [ "1.2.3",  'off',  'off', $priority++,  sub { install_gnu("zlib") } ],
    "cppunit"    => [ "1.10.2", 'off',  'off', $priority++,  sub { install_gnu("cppunit") } ],
    "dateshift"  => [ "1.0",    'off',  'off', $priority++,  sub { install_gnu("dateshift") } ],
    "curl"       => [ "7.19.7", 'on' ,  'off', $priority++,  \&install_curl ],
    "libfaketime"=> [ "0.8",    'off',  'off', $priority++,  \&install_libfaketime ],
    "boost"      => [ "1_42_0", 'on' ,  'off', $priority++,  \&install_boost ],
    "lam"        => [ "7.1.4",  'off',  'off', $priority++,  \&install_lam ],
    "openmpi"    => [ "1.4.1",  'off',  'off', $priority++,  \&install_openmpi ],
    "mpich"      => [ "1.2.7p1",'off',  'off', $priority++,  \&install_mpich ],
    "mpich2"     => [ "1.2.1",  'off',  'off', $priority++,  \&install_mpich2 ],
    "parmetis"   => [ "3.1.1",    'on' ,  'off', $priority++,  \&install_parmetis ],
    "hdf5"       => [ "1.6.4",  'off',  'off', $priority++,  \&install_hdf5 ],
    "subversion" => [ "1.4.3",  'off',  'off', $priority++,  \&install_subversion ],
    "trilinos"   => [ "10.2.0", 'off',  'off', $priority++,  \&install_trilinos ],
    "petsc2"     => [ "2.2.0",  'off',  'off', $priority++,  \&install_petsc2 ],
    "petsc"      => [ "3.1-p2", 'on',   'off', $priority++,  \&install_petsc3 ],
    "gmsh"       => [ "1.60.1", 'off',  'off', $priority++,  sub { install_gnu("gmsh") } ],
    "ccache"     => [ "2.4",    'off',  'off', $priority++,  sub { install_gnu("ccache") } ],
    "distcc"     => [ "2.18.3", 'off',  'off', $priority++,  sub { install_gnu("distcc") } ],
    "cgnslib"    => [ "2.5-4",  'off',  'off', $priority++,  \&install_cgnslib ],
    "cgnstools"  => [ "2-5-4",  'off',  'off', $priority++,  \&install_cgnstools ],
    "google-perftools" => [ "1.5",'off','off', $priority++, \&install_google_perftools ],
);

#==========================================================================
# Command Line
#==========================================================================

sub parse_commandline() # Parse command line
{
    $opt_help=1 unless GetOptions (
        'help'                  => \$opt_help,
        'nocolor'               => \$opt_nocolor,
        'envars'                => \$opt_envars,
        'genconf'               => \$opt_genconf,
        'debug'                 => \$opt_debug,
        'nompi'                 => \$opt_nompi,
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
        'branch=s'              => \$opt_branch,
        'build=s'               => \$opt_build,
        'makeopts=s'            => \$opt_makeopts,
        'install=s'             => \@opt_install,
    );

    # show help if required
    if ($opt_help != 0)
    {
      print <<ZZZ;
install-coolfluid-deps.pl : Install software dependencies for COOLFluiD

usage: install-deps.pl [options]

By default will install a recomended set of dependencies: cmake,curl,boost,openmpi,parmetis,petsc

options:
        --help            Show this help.
        --nocolor         Don't color output
        --envars          Place enviromental variables into .bashrc

        --genconf         Generate a coolfluid.conf based on the installed dependencies

        --nompi           Don't compile with mpi support. This is only active for some packages.
        --mpi=            MPI compiler to use for compilations
                            Default: $opt_mpi.
        --many-mpi=       Install all mpi related packages in a separate directory
                          therefore allowing multiple mpi environments to coexist
                            Default: $opt_many_mpi.

        --debug           Compile dependencies and coolfluid with debug symbols
        --fetchonly       Just download the sources. Do not install anything.
        --dry-run         Don't actually perform the configuration.
                          Just output what you would do.

        --install-dir=    Location of the software installation directory
                            Default: $opt_install_dir
        --install-mpi-dir=        Location for the mpi dependent installations
                            Default: $opt_install_mpi_dir
        --cmake-dir=      Location for the cmake installation
        --tmp-dir=        Location of the temporary directory for complation
                            Default: $opt_tmp_dir

        --dwnldsrc=       URL of download server from where to download sources of dependencies
                            Default: $opt_dwnldsrc

        --makeopts=       Options to pass to make
                            Default: $opt_makeopts

        --install         Comma separated list of packages to install.
                          Every test will be run for on the number of cpus specified here.
                            Example: --install=all,hdf5,lam
                            Default: all
ZZZ
    exit(0);
    }
    @opt_install = split(/,/,join(',',@opt_install));
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
    print my_colored("Executing   : $args\n",$OKCOLOR);
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
    print my_colored("Executing   : $args\n",$OKCOLOR);
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
    my $output;
    # print my_colored("Executing : $args",$OKCOLOR);
    my $command = join("",$args,"|");
    my $pid=open READER, $command or die "Can't run the program: $args $!\n";
    while(<READER>){
       $output.=$_;
    }
    close READER;
    # print my_colored($output,$OK_COLOR);
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
    if ($arch =~ Darwin) {
        return 1;
    } else {
        return 0;
    }
}

#==========================================================================

sub print_var($$) # create a recursive dir path
{
    my ($var,$value)=@_;
    print my_colored($var,$OKCOLOR); print " : ";
    print my_colored($value,$DEBUGCOLOR); print "\n";
}

#==========================================================================

sub parse_config_file($) # parse the config file to get the user overiding options
{
    my ($filename)=@_;
    open CONFIG, "<", $filename or die ("Error opening config file $filename!\n");

    while (<CONFIG>) {
        chomp;                  # no newline
        s/#.*//;                # no comments
        s/^\s+//;               # no leading white
        s/\s+$//;               # no trailing white
        next unless length;     # anything left?
        my ($var, $value) = split(/\s*=\s*/, $_, 2);
        $user_pref{$var} = $value;
    }

    close CONFIG;
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
        $version = $packages{"$opt_mpi"}[$vrs];
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
      print "Setting C compiler to \"".$ENV{CC}."\". Overide this with environment variable \"CC\"\n";
    }

    if ( !(exists $ENV{CXX}) )
    {
      $ENV{CXX} = "g++";
      print "Setting C++ compiler to \"".$ENV{CXX}."\". Overide this with environment variable \"CXX\"\n";
    }

    if (!((exists $ENV{FC}) or (exists $ENV{F77})))
    {
      $ENV{FC} = "gfortran";
      print "Setting Fortran compiler to \"".$ENV{FC}."\". Overide this with environment variable \"FC\".\n";
    }

    # makes sure the both compiler variable F77 and FC always exist
    if ( !(exists $ENV{FC}) )
    {
      print "Setting FC equal to F77\n";
      $ENV{FC} = $ENV{F77};
    }
    if ( !(exists $ENV{F77}) )
    {
      print "Setting F77 equal to FC\n";
      $ENV{F77} = $ENV{FC};
    }
}

#==========================================================================

sub generate_conf ()
{
   if (-e $opt_confile)
   {
	print my_colored("\nFile $opt_confile exists, not generating\n",$SECTIONCOLOR);
   }
   else
   {
   print my_colored("\nGenerating $opt_confile\n",$SECTIONCOLOR);

   unless ($opt_dryrun)
   {
   open CONF, ">", "$opt_confile" or die ("Error creating $opt_confile!\n");

 print CONF  <<ZZZ;

	cc     = $opt_mpi_dir/bin/mpicc
	cxx    = $opt_mpi_dir/bin/mpic++
	fc     = $ENV{FC}

	fflags    = -O3
	cflags    = -O3
	cxxflags  = -O3

	mpi_dir       = $opt_mpi_dir
	boost_dir     = $opt_install_dir
	petsc_dir     = $opt_install_mpi_dir
	parmetis_dir  = $opt_install_mpi_dir

	mods-getall = 0

	mod_AnalyticalModel    = on
	mod_CFmesh2THOR        = on
	mod_CFmeshCellSplitter = on
	mod_CFmeshExtruder     = on
	mod_CFmeshFileReader   = on
	mod_CFmeshFileWriter   = on
	mod_CGNS2CFmesh        = on
	mod_Gambit2CFmesh      = on
	mod_Solver             = on
	mod_THOR2CFmesh        = on
	mod_TecplotWriter      = on
	mod_Petsc              = on
	mod_XCFcaseConverter   = on

ZZZ
   close CONF;
   }
 }
}

#==========================================================================

sub post () {
  if ($opt_envars)
  {
    run_command_or_die("echo export ARCH=\\\"$arch\\\" >> $home/.bash_profile");
    run_command_or_die("echo export PATH=\\\"$opt_install_dir/bin:\\\$PATH\\\" >> $home/.bash_profile");
    run_command_or_die("echo export LD_LIBRARY_PATH=\\\"$opt_install_dir/lib:\\\$LD_LIBRARY_PATH\\\" >> $home/.bash_profile");
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

sub download_src ($$) {
  my ($lib,$version)=@_;

  my $file1 = "$lib-$version.tar.gz";
  my $file2 = "$lib-$version.tar.bz2";
  my $status = 0;

  if ( not -e $file1 and not -e $file2 )
  {
    if (remote_file_exists($file1)) {
      $status = download_file("$opt_dwnldsrc/$file1");
    } elsif (remote_file_exists($file2)) {
      $status = download_file("$opt_dwnldsrc/$file2");
    } else {
      print my_colored("File $file1 or $file2 does not exist on server. \n",$OKCOLOR);
      $status = 1;
    }
    print my_colored("Exit Status : $status\n",$OKCOLOR);

    if ( $status )
    {
      die "$args exited with error" unless $status == 0;
    }
  }
  else { print my_colored("file already exists, not retrieving.\n",$OK_COLOR); }
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
  download_src($lib,$version);
  untar_src($lib,$version);
  safe_chdir("$opt_tmp_dir/$lib-$version/");
  run_command_or_die("./configure --prefix=$opt_install_dir");
  run_command_or_die("make");
  run_command_or_die("make install");
}

#==========================================================================

sub untar_src ($$) {
  my ($lib,$version)=@_;
  my  $status = get_command_status("tar zxf $lib-$version.tar.gz");
  if ($status) {
    $status = get_command_status("tar jxf $lib-$version.tar.bz2");
    print my_colored("Exit Status : $status\n",$OKCOLOR);
    die "$args exited with error" unless $status == 0;
  }
}

#==========================================================================

sub install_google_perftools ()
{
  my $lib="google-perftools";
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --enable-frame-pointers  --prefix=$opt_install_dir");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_gnu ($)
{
  my ($lib)=@_;
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --prefix=$opt_install_dir");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_gcc4() {
  my $lib = "gcc";
  my $version = $packages{"gcc4"}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  my $objdir = "$opt_tmp_dir/$lib-$version-obj";

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    rmtree "$objdir";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    mkpath $objdir;
    safe_chdir($objdir);
    run_command_or_die("$opt_tmp_dir/$lib-$version/configure --prefix=$opt_install_dir --enable-languages=c,c++,fortran");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_gcc3() {
  my $lib = "gcc";
  my $version = $packages{"gcc3"}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  my $objdir = "$opt_tmp_dir/$lib-$version-obj";

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    rmtree "$objdir";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    mkpath $objdir;
    safe_chdir($objdir);
    run_command_or_die("$opt_tmp_dir/$lib-$version/configure --prefix=$opt_install_dir --enable-languages=c,c++,f77");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_openssl ()
{
  my ($lib)= "openssl";
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./config --prefix=$opt_install_dir --shared --openssldir=$opt_install_dir");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_curl ()
{
  my ($lib)= "curl";
  my $version = $packages{$lib}[0];

  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --prefix=$opt_install_dir --without-ssl --without-libidn --without-gnutls --disable-ipv6 ");
    run_command_or_die("make");
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
    print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

    safe_chdir($opt_tmp_dir);
    download_src($lib,$version);
    unless ($opt_fetchonly) {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src($lib,$version);
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
    print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

    safe_chdir($opt_tmp_dir);
    download_src($lib,$version);

    unless ($opt_fetchonly) {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src($lib,$version);
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
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);

  if ($ENV{CXX} eq "g++" ) { $ENV{CXX} = $ENV{CXX} . " -fpermissive"; }

  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --enable-shared --enable-static --with-threads=posix --enable-long-long --enable-languages=c,c++,f77 --disable-checking --enable-cstdio=stdio --with-system-zlib --prefix=$opt_mpi_dir");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_openmpi() {

  my $lib = "openmpi";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");
    run_command_or_die("./configure --enable-shared --enable-static --with-threads=posix --disable-mpi-f90  --prefix=$opt_mpi_dir");
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_mpich2() {
  my $lib = "mpich2";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src($lib,$version);
      safe_chdir("$opt_tmp_dir/$lib-$version/");
      run_command_or_die("./configure --enable-cxx --enable-f77 --enable-f90 --enable-sharedlibs=osx-gcc --prefix=$opt_mpi_dir");
      run_command_or_die("make");
      run_command_or_die("make install");
  }
}

#==========================================================================

sub install_cgnslib() {
  my $lib = "cgnslib";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    my $loptions = "--enable-gcc --enable-lfs --without-fortran --enable-shared=all --with-zlib --prefix=$opt_install_dir";
    if ($arch eq "x86_64" )
    {
        $loptions  = $loptions . " --enable-64bit";
    }
    run_command_or_die("./configure " . $loptions);
    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_cgnstools() {

  my $lib = "cgnstools";
  my $version = $packages{$lib}[$vrs];
  my $cgnslib = "cgnslib";
  my $cgnsversion = $packages{$cgnslib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  # first install a cgnlslib distribution dir
  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
    run_command_or_die("tar -zxf  $cgnslib-$cgnsversion.tar.gz -C $opt_install_dir/local");
    safe_chdir("$opt_install_dir/local/$cgnslib-$cgnsversion/");

    my $loptions = "--enable-gcc --enable-lfs --without-fortran --enable-shared=all --with-zlib";
    if ($arch eq "x86_64" )
    {
        $loptions  = $loptions . " --enable-64bit";
    }
    run_command_or_die("./configure " . $loptions);
    run_command_or_die("make");
  }

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    my $loptions = "--enable-gcc --with-cgns=$opt_install_dir/local/$cgnslib-$cgnsversion/ --prefix=$opt_install_dir";
    if ($arch eq "x86_64" )
    {
        $loptions  = $loptions . " --enable-64bit";
}
    run_command_or_die("./configure " . $loptions);
    run_command_or_die("make");
    run_command_or_die("make install");
}
}

#==========================================================================

sub install_mpich() {
  my $lib = "mpich";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly)
  {
      rmtree "$opt_tmp_dir/$lib-$version";
      untar_src($lib,$version);
      safe_chdir("$opt_tmp_dir/$lib-$version/");
      run_command_or_die("./configure --prefix=$opt_mpi_dir --enable-f77 --enable-f90");
      run_command_or_die("make");
      run_command_or_die("make install");
  }
}

#==========================================================================

sub install_parmetis () {
  my $lib = "parmetis";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  my $include_dir = "$opt_install_mpi_dir/include/";
  my $lib_dir = "$opt_install_mpi_dir/lib/";

  mkpath $include_dir;
  mkpath $lib_dir;

  safe_chdir($opt_tmp_dir);
  download_src("ParMetis",$version);
  unless ($opt_fetchonly) {

    rmtree "$opt_tmp_dir/ParMetis-$version";
    untar_src("ParMetis",$version);
    safe_chdir("$opt_tmp_dir/ParMetis-$version/");

    if (is_mac()) { # add include for malloc.h
       my $filename = 'Makefile.in';
        safe_copy($filename,"$filename.orig");
        open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
        open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
        while (<IN>) {
        chomp;
        s/(^INCDIR\s=\s?$)/INCDIR = -I\/usr\/include\/malloc\//g;
        #print "$_\n";
        print OUT "$_\n";
        }
        print my_colored("Modified Makefile.in to include malloc for MacOSX\n",$DEBUGCOLOR);
        close IN;
        close OUT;
    }

    if ($opt_nompi) { # substitute mpicc for gcc
        my $filename = 'Makefile.in';
        safe_copy($filename,"$filename.orig");
        open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
        open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
        while (<IN>) {
        chomp;
        s/mpicc/gcc/g;
        print "$_\n";
        print OUT "$_\n";
        }
        close IN;
        close OUT;
    }

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

  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);
    

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
    untar_src($lib,$version);

    safe_chdir("$build_dir");

    my $wdebug = "";
    if ($opt_debug) { $wdebug = "--with-debugging=1" };

    my $wblaslib = "";
    if (is_mac()) { 
      # use built-in optimized blas-lapack lib
      $wblaslib = "--with-blas-lapack-lib=\"-framework vecLib\"";
    } else { 
      # use the downloaded blas sources
      $wblaslib = "--download-f-blas-lapack=\"$fblas_file\"";
    }
      
    run_command_or_die("./config/configure.py --prefix=$install_dir $wdebug COPTFLAGS='-O3' FOPTFLAGS='-O3' --with-mpi-dir=$opt_mpi_dir $wblaslib --with-shared=1 --with-dynamic=1 --with-c++-support --PETSC_ARCH=$petsc_arch");

    run_command_or_die("make $opt_makeopts");

    run_command_or_die("make install PETSC_DIR=$build_dir");

  }
}

#==========================================================================

sub install_petsc2 () {
  my $lib = "petsc";
  my $version = $packages{"petsc2"}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);

  my $mpicomp = $opt_mpi;
  if ($opt_nompi) { $mpicomp = 'nompi'; }

  if (not -e "$lib-$version-lite.tar.gz" ) { download_file("$opt_dwnldsrc/$lib-$version-lite.tar.gz") };
  if (not -e "$lib-$version-patch_gcc-3.4.tar.gz" ) { download_file("$opt_dwnldsrc/$lib-$version-patch_gcc-3.4.tar.gz") };
  if (not -e "$lib-$version-patch_$mpicomp.tar.gz" ) { download_file("$opt_dwnldsrc/$lib-$version-patch_$mpicomp.tar.gz") };


  unless ($opt_fetchonly) {

    run_command_or_die("tar -zxf  $lib-$version-lite.tar.gz -C $opt_install_mpi_dir/local");
    run_command_or_die("tar -zxvf $lib-$version-patch_gcc-3.4.tar.gz -C $opt_install_mpi_dir/local");
    run_command_or_die("tar -zxvf $lib-$version-patch_$mpicomp.tar.gz -C $opt_install_mpi_dir/local");


    $ENV{BOPT} = "O_c++";
    if (is_mac())
    {
      $ENV{PETSC_ARCH} = "darwin";
      # $ENV{BLASLAPACK_LIB} = "\"-framework vecLib\"";
      $ENV{BLASLAPACK_LIB} = "\"/System/Library/Frameworks/vecLib.framework/Versions/A/vecLib\"";
    } else {
    	$ENV{PETSC_ARCH} = "linux";
    	$ENV{BLASLAPACK_LIB} = "-L$opt_install_dir/lib  -llapack  -lblas";
    }
    $ENV{PETSC_DIR} = "$opt_install_dir/local/$lib";

    $ENV{MPI_HOME}    = "$opt_install_mpi_dir";
    $ENV{MPI_LIB}     = "-L$ENV{$MPI_HOME}/lib";
    $ENV{MPI_INCLUDE} = "-I$ENV{$MPI_HOME}/include";

    if ($opt_envars)
    {
      run_command_or_die("echo export PETSC_ARCH=$ENV{PETSC_ARCH} >> $home/.bash_profile");
      run_command_or_die("echo export PETSC_DIR=$ENV{PETSC_DIR}   >> $home/.bash_profile");
      run_command_or_die("echo export BOPT=$ENV{BOPT}             >> $home/.bash_profile");
      run_command_or_die("echo export LD_LIBRARY_PATH=\"$ENV{PETSC_DIR}/lib/lib$ENV{BOPT}/$ENV{PETSC_ARCH}:\\\$LD_LIBRARY_PATH\" >> $home/.bash_profile");
    }

    safe_chdir("$opt_install_mpi_dir/local");
    run_command("ln -sf $lib-$version $lib");
    safe_chdir("$opt_install_mpi_dir/local/$lib");

    if (is_mac())
    {
      print "\n============================configuring PETSC for mac============================\n";
      safe_chdir("$opt_mpi_dir/bin");
      # Mac OSX hack: add alias "mpicxx"
      run_command("ln -sf mpic++ mpicxx");
      safe_chdir("$opt_install_dir/local/petsc/python/PETSc/packages");
      # Mac OSX hack: change MPI.py file to set checkMPILink to success
      my $filename = 'MPI.py';
      safe_copy($filename,"$filename.orig");
      open(OUT, ">$filename") or die ("Error opening config file $filename !\n");
      open(IN,  "<$filename.orig") or die ("Error opening config file $filename.orig !\n");
      while (<IN>) {
      chomp;
      s/(^\s*success\s*=\s0$)/    # success = 0 (modified for Mac OS X) \n    return 1/g;
      #print "$_\n";
      print OUT "$_\n";
      }
      print "Mac OSX hack: Modified $filename to set checkMPILink to success\n";
      close IN;
      close OUT;

      safe_chdir("$opt_install_dir/local/$lib");
      run_command_or_die("./config/configure.py --with-blas-lapack-lib=$ENV{BLASLAPACK_LIB} --with-mpi-dir=$ENV{MPI_HOME} --with-PETSC_DIR=$ENV{PETSC_DIR}");
    } else {
      if ($ENV{FC} =~ /gfortran*/) {
        # if using any gfortran we must substitute the fortran libraries
        run_command_or_die("perl -pi -e 's/\\-lfrtbegin(\\s+)\\-lg2c(\\s+)\\-lc(\\s+)\\-lm/\\-lgfortranbegin\\ \\-lgfortran\\ \\-lm\\ \\-lgcc_s/g' bmake/linux/variables");
        run_command_or_die("perl -pi -e 's/\\-Wno\\-globals//g' bmake/linux/variables");
      }
    }
    run_command_or_die("make BOPT=O_c++ all");
  }
}
#==========================================================================

sub install_trilinos() {
  my $lib = "trilinos";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);

  my $mpiopt;
  unless ($opt_nompi) {
      $mpiopt = " -D TPL_ENABLE_MPI:BOOL=ON \\
-D MPI_BASE_DIR_PATH:PATH=$opt_mpi_dir \\
-D CMAKE_C_COMPILER:FILEPATH=$opt_mpi_dir/bin/mpicc \\
-D CMAKE_CXX_COMPILER:FILEPATH=$opt_mpi_dir/bin/mpic++ " 
 }

  unless ($opt_fetchonly) 
  {
    my $build_dir =  "$opt_tmp_dir/$lib-$version-Source/build"; 

    rmtree "$opt_tmp_dir/$lib-$version-Source";
    untar_src($lib,$version);
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
-D Trilinos_ENABLE_Ifpack:BOOL=ON \\
-D Trilinos_ENABLE_Meros:BOOL=ON \\
-D Trilinos_ENABLE_ML:BOOL=ON \\
-D Trilinos_ENABLE_RTOp:BOOL=ON \\
-D Trilinos_ENABLE_Teuchos:BOOL=ON \\
-D Trilinos_ENABLE_Thyra:BOOL=ON \\
-D Trilinos_ENABLE_Triutils:BOOL=ON \\
-D Trilinos_ENABLE_Stratimikos:BOOL=ON \\
-D Trilinos_ENABLE_Zoltan:BOOL=OFF \\
-D TPL_ENABLE_BLAS:BOOL=ON \\
-D TPL_ENABLE_LAPACK:BOOL=ON \\
$mpiopt \\
-D CMAKE_VERBOSE_MAKEFILE:BOOL=FALSE \\
-D BUILD_SHARED_LIBS:BOOL=ON\\
-D Trilinos_VERBOSE_CONFIGURE:BOOL=FALSE  $opt_tmp_dir/$lib-$version-Source"
);

#-D CMAKE_Fortran_COMPILER:FILEPATH=$opt_install_dir/bin/mpif90 
#-D TPL_ENABLE_PARMETIS:BOOL=ON \\
#-D PARMETIS_LIBRARY_DIRS:PATH=\"$opt_install_dir/lib\" \\
#-D PARMETIS_INCLUDE_DIRS:PATH=\"$opt_install_dir/include\" \\

    run_command_or_die("make");
    run_command_or_die("make install");
  }
}

#==========================================================================

sub install_subversion() {
  my $lib = "subversion";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_file("$opt_dwnldsrc/$lib-$version-linux-x86.tar.gz");
  unless ($opt_fetchonly) {
  run_command_or_die("tar zxvf $lib-$version-linux-x86.tar.gz -C $opt_install_dir");
  }
}

#==========================================================================

sub install_libfaketime() {
  my $lib = "libfaketime";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib-$version";
  print my_colored("Installing $pack\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  if ( not -e "$pack.tar.gz" ) { download_file("$opt_dwnldsrc/$pack.tar.gz"); }

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$pack";
    run_command_or_die("tar zxf $pack.tar.gz");
    safe_chdir("$opt_tmp_dir/$pack/");

    run_command_or_die("make");

    safe_copy("libfaketime.so.1","$opt_install_dir/lib/libfaketime.so.1") or die;
    safe_copy("libfaketimeMT.so.1","$opt_install_dir/lib/libfaketimeMT.so.1") or die;
  }
}
#==========================================================================

sub install_boost()
{
  my $lib = "boost";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib\_$version";
  print my_colored("Installing $pack\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  if ( not -e "$pack.tar.bz2" ) { download_file("$opt_dwnldsrc/$pack.tar.bz2"); }

  unless ($opt_fetchonly)
  {
    rmtree "$opt_tmp_dir/$pack";
    run_command_or_die("tar jxf $lib\_$version.tar.bz2");
    safe_chdir("$opt_tmp_dir/$pack/");


    # build toolset
    safe_chdir("tools/jam/src");
    my $toolset = "gcc";
    if($ENV{CC} eq "icc") { $toolset = "intel-linux"; }
    
    # in case g++ is speaical path
    $ENV{GCC} = $ENV{CC};
    $ENV{GXX} = $ENV{CXX};


    my $boost_arch;
    if($arch eq "x86_64") { $boost_arch = "linuxx86_64" ;  }
    if($arch eq "i686")   { $boost_arch = "linuxx86" ;  }

    if(is_mac())         
    { 
	  $toolset = "darwin";
      $boost_arch = "macosxx86"; 
      
      # If Snow Leopard
      my $capable64 = run_command("sysctl hw | grep 'hw.cpu64bit_capable: [0-9]'");
      my $OSversion = run_command("sw_vers | grep 'ProductVersion:'");
      if ($capable64 =~ /hw.cpu64bit_capable:\s1/ && $OSversion =~ /10\.6\.*/) 
      {
          $boost_arch = "macosxx86_64";    
      }
    }

    # disable compression filters in boost because some systems like ubuntu
    # dont have the zlib-dev installed by default
    $ENV{NO_COMPRESSION} = "1";

    run_command_or_die("sh build.sh $toolset");

    # build boost libs
    safe_chdir("../../..");
    
    my $boostmpiopt=" --without-mpi ";
    unless ($opt_nompi) {
      $boostmpiopt=" --with-mpi cxxflags=-DBOOST_MPI_HOMOGENEOUS ";
      open  (USERCONFIGJAM, '>>~/user-config.jam');
      print  USERCONFIGJAM "using mpi : $opt_mpi_dir/bin/mpiCC";
      close (USERCONFIGJAM); 
    }

    run_command_or_die("./tools/jam/src/bin.$boost_arch/bjam --prefix=$opt_install_dir $opt_makeopts --with-test --with-thread --with-iostreams --with-filesystem --with-system --with-regex --with-date_time --with-program_options $boostmpiopt toolset=$toolset threading=multi variant=release stage install");

  }
}

#==========================================================================

sub install_cmake() {
  my $lib = "cmake";
  my $version = $packages{$lib}[$vrs];
  my $pack = "$lib-$version";
  print my_colored("Installing $pack\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);

  unless ($opt_fetchonly) {

    rmtree "$opt_tmp_dir/$pack";
    run_command_or_die("tar zxf $pack.tar.gz");
    safe_chdir("$opt_tmp_dir/$pack/");

    run_command_or_die("./bootstrap --prefix=$opt_cmake_dir");
    run_command_or_die("make");
    run_command_or_die("make install");

  }
}

#==========================================================================

sub install_hdf5() {
  my $lib = "hdf5";
  my $version = $packages{$lib}[$vrs];
  print my_colored("Installing $lib-$version\n",$HEADINGCOLOR);

  safe_chdir($opt_tmp_dir);
  download_src($lib,$version);
  unless ($opt_fetchonly) {
    rmtree "$opt_tmp_dir/$lib-$version";
    untar_src($lib,$version);
    safe_chdir("$opt_tmp_dir/$lib-$version/");

    my $old_cc  = $ENV{CC};
    my $old_cxx = $ENV{CXX};
    my $mpiopt;
    unless ($opt_nompi) {
        $ENV{CC}   = "mpicc";
        $ENV{CXX}  = "mpic++";
        $mpiopt = "--enable-parallel";
    }

    run_command_or_die("./configure --prefix=$opt_install_mpi_dir $mpiopt --enable-cxx --enable-zlib --enable-linux-lfs --with-gnu-ld --enable-hl");
    run_command_or_die("make");
    run_command_or_die("make install");

    $ENV{CC}   = $old_cc;
    $ENV{CXX}  = $old_cxx;
  }
}

#==========================================================================

sub print_info() # print information about the
{
    print my_colored("Installing COOLFLUID dependencies\n",$HEADINGCOLOR);

    print_var("Install     dir ","$opt_install_dir");
    print_var("Install MPI dir ","$opt_install_mpi_dir");
    print_var("CMake       dir ","$opt_cmake_dir");
    print_var("MPI         dir ","$opt_mpi_dir");
    print_var("Temporary   dir ","$opt_tmp_dir");

# Env vars
    print_var(PATH,$ENV{PATH});
    print_var(LD_LIBRARY_PATH,$ENV{LD_LIBRARY_PATH});
    print_var(CC,$ENV{CC});
    print_var(CXX,$ENV{CXX});
    print_var(FC,$ENV{FC});
    print_var(CFLAGS,$ENV{CFLAGS});
    print_var(CXXFLAGS,$ENV{CXXFLAGS});
    print_var(FFLAGS,$ENV{FFLAGS});
    print_var(F77FLAGS,$ENV{F77FLAGS});
    print_var(F90FLAGS,$ENV{F90FLAGS});

# Options
#     while ( my ($key, $value) = each(%options) ) {
#         print_var($key,get_option($key));
#     }

# User prefs
#     while ( my ($key, $value) = each(%user_pref) ) {
#         print_var($key,$value);
#     }
}

#==========================================================================

sub set_install_all()
{
  foreach $pname (keys %packages) {
    $packages{$pname}[$ins] = $packages{$pname}[$dft];
  }
}

#==========================================================================

sub install_packages()
{
  print_info();
  check_wgetprog();

    # if 'all' exists, copy the [$dft] to [$ins]
    for ($i=0; $i < scalar @opt_install; $i++)
    {
        if ($opt_install[$i] eq 'all') { set_install_all(); }
    }

    # if there is no package selected, then also copy the [$dft] to [$ins]
    if (scalar @opt_install == 0) { set_install_all(); }

    # turn on the manually selected packages
    for ($i=0; $i < scalar @opt_install; $i++)
    {
        my $opt = $opt_install[$i];
        if (exists $packages{$opt})
        {
            $packages{$opt}[$ins] = 'on';
        }
        elsif (!($opt eq 'all')) {
            print my_colored("Package does not exist: $opt\n",$ERRORCOLOR);
        }
    }

    my %install_packages = ();

    # sort the packages to install by priority
    foreach $pname (keys %packages) {
    #       print "$pname\n";
        if ($packages{$pname}[$ins] eq 'on') {
            $install_packages{$packages{$pname}[$pri]} = $pname;
        }
    }

    $actually_installed = "";

    # install the packages by priority
    foreach $p (sort {$a <=> $b} keys %install_packages) {
        my $pname = $install_packages{$p};
        my $pversion = $packages{$pname}[$vrs];
        print my_colored("Package marked for installation: $pname\t[$pversion]\n",$WARNCOLOR);
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

if ($opt_genconf)      { generate_conf(); }

post();
