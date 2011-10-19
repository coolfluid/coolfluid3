#!/usr/bin/env perl

# modules
use Getopt::Long;
use Tie::File;
use File::Copy;

my $separator = "\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/";

# command line options
my $opt_help      = 0;
my $opt_rename    = 0;

# Parse command line
$opt_help=1 unless GetOptions ( 'help'   => \$opt_help,
                                'rename' => \$opt_rename );

sub show_help ()
{
  print <<ZZZ;
port-to-k3.pl [files] : Formats source files
options:
        --help            Show this help
        --rename          Rename the files to new extensions .hpp .cpp
ZZZ
}

sub process ($)
{
    my ($filename)=@_;

    my $lr = 0;
    my $lc = 0;
    my $li = 0;
    my $ls = 0;
    my $lss= 0;

    # process doxygen /** ... */ comments
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    for (my $i = 0; $i < scalar @lines; $i++)
    {
        my $fline = $lines[$i];
        if ($fline =~ m/^(\s*)\/\*\*/)
        {
          # delete lines with /**
          splice @lines, $i, 1;
          $i--; $lr++;

          # loop until finding */
          for (my $j = $i; $j < scalar @lines ; $j++)
          {
            $fline = $lines[$j];
            # stop if */
            last if ($fline =~ m/^(\s*)\*\//);
            # correct * to ///
            if ($fline =~ m/^(\s*)\*(\s*)[^\s]/)
            {
              $lines[$j] =~ s/^(\s*)\*/\1\/\/\//;
              $lc++;
            }
            # delete lines with * only
            if ($fline =~ m/^(\s*)\*(\s*)$/)
            {
              splice @lines, $j, 1;
              $j--; $lr++;
            }
          }
        }

        # delete lines with */
        if ($lines[$i] =~ m/^(\s*)\*\//)
        {
          splice @lines, $i, 1;
          $i--; $lr++;
        }
     }
    untie @lines;

    # correct indentations
    # odd number of spaces is converted to even number
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    foreach ( @lines )
    {
       if (($_ =~ m/^\s[^\s]/) or ($_ =~ m/^\s\s\s[^\s]/) or ($_ =~ m/^\s\s\s\s\s[^\s]/) or ($_ =~ m/^\s\s\s\s\s\s\s[^\s]/) ) { $li++; }
       s/^\s([^\s])/\1/;
       s/^\s\s\s([^\s])/  \1/;
       s/^\s\s\s\s\s([^\s])/    \1/;
       s/^\s\s\s\s\s\s\s([^\s])/      \1/;
    }
    untie @lines;

    # remove trailing spaces
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    foreach ( @lines )
    {
       if ($_ =~ m/\s+$/) { $ls++; }
       s/\s+$//;
    }
    untie @lines;

    # correct separator lines
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    foreach ( @lines )
    {
       if (($_ =~ m/^\/\/\/\/(\/*)$/) and ($_ !~ m/^$separator$/) )
       {
          s/^\/\/\/\/(\/*)$/$separator/;
          $lss++;
       }
    }
    untie @lines;

    # convert tabs to spaces
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    foreach ( @lines )
    {
      s/\t/  /;
    }
    untie @lines;

    # convert COOLFluiD namespace to CF
    tie @lines, 'Tie::File', $filename or die ("Error opening file $filename!\n");
    foreach ( @lines )
    {
      s/COOLFluiD/CF/g;
      s/COOFluiD/CF/g;  # there were some strange mispelings
      
      s/CFLog\./Log\./g;
      s/CFLogLevel\./LogLevel\./g;

      s/CFLogNotice/CFLogInfo/g;
      s/CFLogDebugMin/CFLogDebug/g;
      s/CFLogDebugMed/CFLogDebugVerbose/g;
      s/CFLogDebugMax/CFLogDebugVerbose/g;
      s/CFLogNotice/CFLogInfo/g;
      
      s/CFout/CFinfo/g;
      s/CFlog/CFinfo/g;
      s/CFerr/CFerror/g;
      
      s/CFLog(\s*)\((\s*)ERROR(\s*)\,/CFLogError \( /g;
      s/CFLog(\s*)\((\s*)WARN(\s*)\,/CFLogWarn \( /g;
      s/CFLog(\s*)\((\s*)NOTICE(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)INFO(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)VERBOSE(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MIN(\s*)\,/CFLogDebug \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MED(\s*)\,/CFLogDebugVerbose \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MAX(\s*)\,/CFLogDebugVerbose \( /g;

      s/COOLFluiD_Common/CF3_Common/;
      
      s/Config::/Common::/g;
      s/Config\//Common\//g;
      s/Config_API/Common_API/g;
      s/(CF|COOLFluiD)_Config/CF3_Common/;
      s/namespace(\s+)Config/namespace Common/;
      
      s/Environment::/Common::/g;
      s/Environment::/Common::/g;
      s/Environment\//Common\//g;
      s/Environment_API/Common_API/g;
      s/(CF|COOLFluiD)_Environment/CF3_Common/;
      s/namespace(\s+)Environment/namespace Common/;
      
      s/MathTools::/Math::/g;
      s/MathTools\//Math\//g;
      s/MathTools_API/Math_API/g;
      s/MathTools_TEMPLATE/Math_TEMPLATE/g;
      s/MathTools_EXPORTS/Math_EXPORTS/g;
      s/(CF|COOLFluiD)_MathTools/CF3_Math/i;
      s/MathTools.(hpp|hh)/Math.hpp/;
      s/namespace(\s+)MathTools/namespace Math/;
      s/LTGT//g;
     
      s/FailedCastException/CastingFailed/g;
      s/FileFormatException/FileFormatError/g;
      s/FileSystemException/FileFileSystemError/g;
      s/FloatingException/FloatingPointError/g;
      s/LibLoaderException/LibLoadingError/g;
      s/NullPointerException/NullPointerError/g;
      s/ParallelException/ParallelError/g;
      s/ParserException/ParsingFailed/g;
      s/SetupException/SetupError/g;
      s/StorageExistsException/ValueExists/g;
      s/NoSuchStorageException/ValueNotFound/g;
      s/NoSuchValueException/ValueNotFound/g;
      s/SignalException/SignalError/g;

      s/(\w+)Exception/\1/g;

      # modified types
      s/CFchar/char/g;
      s/CFfloat/float/g;
      s/CFldouble/long double/g;
      s/CFdouble/double/g;
      s/CFint/int/g;
      s/CFuint/Uint/g;
      s/CFreal/Real/g;
     
      # modified global functions
      s/deletePtr/delete_ptr/g;

      # modified extenions
      s/(\w+)\.hh/\1\.hpp/;
      s/(\w+)_hh/\1_hpp/;
      s/(\w+)_HH/\1_HPP/;
      s/(\w+)\.cxx/\1\.cpp/;
      
      s/\#include(\s+)\"((\w|\/)*)SelfRegistPtr\.hpp\"//;

      # class rename
      s/CFEnvVars/CoreVars/g;
      s/CFEnv/CoreEnv/g;
      s/Common::SelfRegistPtr/boost::shared_ptr/g;
      s/SelfRegistPtr/boost::shared_ptr/g;
      s/CFMatrix/MatrixT/g;
      s/CFVector/VectorT/g;
      s/CFSliceMatrix/MatrixSliceT/g;
      s/CFSliceVector/VectorSliceT/g;

      # class and members change
      s/PE::getInstance()\.get_rank()/PEInterface::instance()\.rank()/g;
      s/PE::getInstance()\.get_processor_count()/PEInterface::instance()\.size()/g;

      # getInstance is now instance
      s/getInstance()/instance()/g;

      # modified headers
      s/Environment\.hpp/CommonAPI\.hpp/;
      s/EnvironmentAPI\.hpp/CommonAPI\.hpp/;
      s/Config\.hpp/CommonAPI\.hpp/;
      s/ConfigAPI\.hpp/CommonAPI\.hpp/;
      s/Common\.hpp/CommonAPI\.hpp/;
      s/SetupError\.hpp/BasicExceptions\.hpp/;
      s/PE\.hpp/PEInterface\.hpp/;
      s/PE_MPI\.hpp/PEInterface\.hpp/;
      
    }
    untie @lines;

    # my $nc = $lc + $li + $ls + $lr + $lss;
    # print "$filename changed $nc lines ( $li indentations, $ls trail spaces, $lss separators, $lc comments changes and $lr removed )\n" unless ($nc eq 0 )
}

#==========================================================================

unless ($opt_help eq 0)
{
  show_help();
  exit(0);
}

foreach  $file (@ARGV)
{
  if (( -e $file ) and ( -r $file ))
  {
    # process the file
    print "[$file]";
    process ("$file");

    if ($opt_rename)
    {
      # now copy the file to a name with correct extension
      my $newfile = $file;
      $newfile =~ s/\.hh/\.hpp/;
      $newfile =~ s/\.cxx/\.cpp/;

      # finally remove old file
      unless ( $file eq $newfile )
      {
        print " -> [$newfile]";
        copy($file, $newfile) or die "File $file cannot be copied.";
        unlink ($file);
      }
    }
    print "\n";
  }
  else
  {
    print "$file either does not exist or is not readable\n";
    exit(1);
  }
}




