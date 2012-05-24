import sys
import coolfluid as cf

root = cf.Core.root()

# 2D triangles test
mesh2d = root.create_component('mesh2d','cf3.mesh.Mesh')
mesh_generator = root.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh2d.uri())
mesh_generator.options().set("nb_cells",[100,100])
mesh_generator.options().set("lengths",[1.,1.])
mesh_generator.options().set("offsets",[0.,0.])
mesh_generator.execute()

triangulator = root.create_component('triangulator', 'cf3.mesh.MeshTriangulator')
triangulator.options().set('mesh', mesh2d)
triangulator.execute()

mesh2d.write_mesh(cf.URI('triangulated.msh'))

# 3D tetrahedra test
mesh3d = root.create_component('mesh3d','cf3.mesh.Mesh')
generator = root.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().set('mesh', mesh3d.uri())
generator.options().set('x_segments', 24)
generator.execute()

triangulator.options().set('mesh', mesh3d)
triangulator.execute()

mesh3d.write_mesh(cf.URI('tetrahedralized.msh'))