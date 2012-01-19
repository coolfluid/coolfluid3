from cStringIO import StringIO
import os
import sys
import string
import coolfluid as cf

def show_graph(starturi):
  root = cf.Core.root()
  nx = root.create_component('NetworkXPython','cf3.python.NetworkXPython')
  old_stdout = sys.stdout
  sys.stdout = mystdout = StringIO()
  nx.print_component_graph( uri = starturi )
  sys.stdout = old_stdout
  print "006 I AM NETWORKXPYTHON.PY"
  print mystdout.getvalue()


