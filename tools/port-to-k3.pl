#!/usr/bin/env perl

# modules
use Getopt::Long;
use Tie::File;

my $separator = "\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/";

# command line options
my $opt_help    = 0;

# Parse command line
$opt_help=1 unless GetOptions ( 'help' => \$opt_help, );

sub show_help ()
{
  print <<ZZZ;
reformat-src-files.pl [files] : Formats source files
options:
        --help            Show this help
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
      
      s/CFLog\./Log\./g;
      s/CFLogLevel\./LogLevel\./g;

      s/CFLogNotice/CFinfo/g;
      s/CFLogDebugMin/CFDebug/g;
      s/CFLogDebugMed/CFDebugVerbose/g;
      s/CFLogDebugMax/CFDebugVerbose/g;
      s/CFLogNotice/CFinfo/g;
      
      s/CFLog(\s*)\((\s*)ERROR(\s*)\,/CFLogError \( /g;
      s/CFLog(\s*)\((\s*)WARN(\s*)\,/CFLogWarn \( /g;
      s/CFLog(\s*)\((\s*)NOTICE(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)INFO(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)VERBOSE(\s*)\,/CFLogInfo \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MIN(\s*)\,/CFLogDebug \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MED(\s*)\,/CFLogDebugVerbose \( /g;
      s/CFLog(\s*)\((\s*)DEBUG_MAX(\s*)\,/CFLogDebugVerbose \( /g;

      s/Config::/Config::/g;
      s/Config\//Common\//g;
      s/Config_API/Common_API/g;
      s/namespace(\s+)Config/namespace Common/;
      
      s/Environment::/Common::/g;
      s/Environment\//Common\//g;
      s/Environment_API/Common_API/g;
      s/namespace(\s+)Environment/namespace Common/;
      
      s/MathTools::/Math::/g;
      s/MathTools\//Math\//g;
      s/MathTools_API\//Math_API\//g;
      s/(CF|COOLFluiD)_MathTools/CF_Math/;
      s/namespace(\s+)MathTools/namespace Math/;
     
      s/FailedCastException/CastingFailed/g;
      s/FileFormatException/FileFormatError/g;
      s/FileSystemException/FileFileSystemError/g;
      s/FloatingException/FloatingPointError/g;
      s/LibLoaderException/LibLoadingError/g;
      s/NullPointerException/NullPointerError/g;
      s/ParallelException/ParallelError/g;
      s/ParserException/ParsingFailed/g;
      s/SetupException/SetupError/g;
      
      s/(\w+)Exception/\1/g;
      
      
      s/CFchar/char/g;
      s/CFfloat/float/g;
      s/CFldouble/long double/g;
      s/CFdouble/double/g;
      s/CFint/int/g;
      s/CFuint/Uint/g;
      s/CFreal/Real/g;
      
      s/(\w+)\.hh/\1\.hpp/;
      s/(\w+)\.cxx/\1\.cpp/;
    }
    untie @lines;

    my $nc = $lc + $li + $ls + $lr + $lss;
    print "$filename changed $nc lines ( $li indentations, $ls trail spaces, $lss separators, $lc comments changes and $lr removed )\n" unless ($nc eq 0 )
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
    process ("$file");
  }
  else
  {
    print "$file either does not exist or is not readable\n";
    exit(1);
  }
}




