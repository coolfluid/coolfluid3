##############################################################################
# include cmake macros
##############################################################################

include(CheckIncludeFile)
include(CheckIncludeFileCXX)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckTypeSize)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckCCompilerFlag)
include(CheckCXXCompilerFlag)


##############################################################################
# include coolfluid macros
##############################################################################

include(macros/CFVariables)
include(macros/CFSearchPaths)
include(macros/CFFindProjectFiles)
include(macros/CFLogToFile)
include(macros/CFCheckFileLength)
include(macros/CFSeparateSources)
include(macros/CFListOperations)
include(macros/CFAddLibrary)
include(macros/CFAddApp)
include(macros/CFAddCompilationFlags)
