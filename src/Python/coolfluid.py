import sys
if sys.version_info < (2, 5):
  import dl
else:
  import ctypes as dl

# This is needed to get MPI symbols to be visible
flags = sys.getdlopenflags()
sys.setdlopenflags(flags | dl.RTLD_GLOBAL)

# Import the C++ module
from libcoolfluid_python import *

# restore the dlopen flags to default
sys.setdlopenflags(flags)

#initiate the CF3 environment
Core.initiate(sys.argv)
