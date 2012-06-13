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

# Import unit test module
from check import *

# restore the dlopen flags to default
sys.setdlopenflags(flags)

#initiate the CF3 environment. Note: there is no argv if executed from the ScriptEngine
if sys.__dict__.has_key('argv'):
  Core.initiate(sys.argv)

# shortcut for root
root = Core.root()

# shortcut for environment
environment = Core.environment()

# shortcut for tools
tools = Core.tools()

