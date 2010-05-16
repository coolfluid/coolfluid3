: Ms Windows prepare script
: Creates a directory with the coolfluid build
: Assumes sources are in dir kernel\ parallel to builds\

: no repeated output
@echo off

: check for the build name
if [%1]==[] ECHO Please provide a build name. Usage 'cmake-win32.bat buildname'
if [%1]==[] EXIT /B

: defined the build name
set buildname=%1
set builddir=builds\%buildname%

echo Build Type %buildname%

: ensure build dir exists
md %builddir%

: go to build dir
pushd %builddir%

: execute cmake
cmake ..\..\kernel -G"Visual Studio 10" -DMPI_ROOT="C:\Program Files\MPICH2" -DBOOST_ROOT="C:\Program Files\boost" -DCF_SKIP_FORTRAN:BOOL=OFF -DBoost_COMPILER="-vc100" -DCMAKE_BUILD_TYPE=DEBUG

: go to original directory
popd