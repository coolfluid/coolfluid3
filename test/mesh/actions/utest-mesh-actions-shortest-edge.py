import sys
import coolfluid as cf

root = cf.Core.root()

# 2D triangles test
mesh = root.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = root.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("nb_cells",[10])
mesh_generator.options().set("lengths",[10.])
mesh_generator.options().set("offsets",[3.2])
mesh_generator.execute()

short_edge_action = root.create_component('ShortestEdge', 'cf3.mesh.actions.ShortestEdge')
short_edge_action.mesh = mesh
short_edge_action.execute()

if abs(short_edge_action.properties.h_xi - 1.) > 1e-10:
  raise Exception('Incorrect length')