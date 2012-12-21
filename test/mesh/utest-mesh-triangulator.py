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

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.extrude_blocks(positions=[1., 2.], nb_segments=[5, 5], gradings=[1., 1.])
blocks.create_mesh(mesh3d.uri())

mesh3d.write_mesh(cf.URI('hexa3d.msh'))

triangulator.options().set('mesh', mesh3d)
triangulator.execute()

mesh3d.write_mesh(cf.URI('tetrahedralized.msh'))