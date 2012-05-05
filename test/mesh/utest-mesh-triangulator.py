import sys
import coolfluid as cf

root = cf.Core.root()

mesh = root.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = root.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().configure_option("mesh",mesh.uri())
mesh_generator.options().configure_option("nb_cells",[100,100])
mesh_generator.options().configure_option("lengths",[1.,1.])
mesh_generator.options().configure_option("offsets",[0.,0.])
mesh_generator.execute()

triangulator = root.create_component('triangulator', 'cf3.mesh.MeshTriangulator')
triangulator.options().configure_option('mesh', mesh)
triangulator.execute()

mesh.write_mesh('triangulated.msh')