from coolfluid import *
root = Core.root()

netwx = root.create_component('netwx', 'cf3.python.NetworkXPython')
print netwx.get_component_graph(uri=root.uri(), depth=2)
