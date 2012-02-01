import coolfluid as cf
import sys

root = cf.Core.root()

netwx = root.create_component('netwx', 'cf3.python.NetworkXPython')
print netwx.get_component_graph(root.uri())
