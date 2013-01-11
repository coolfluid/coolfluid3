import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')

# 2D case
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 9)
points[0] = [0., 0.]
points[1] = [1., 0.]
points[2] = [1., 1.]
points[3] = [0., 1.]
points[4] = [0.5, 0.5]
points[5] = [0.5, 1.]
points[6] = [1., 0.5]
points[7] = [0.5, 0.]
points[8] = [0., 0.5]
block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 7, 4, 8]
block_nodes[1] = [8, 4, 5, 3]
block_nodes[2] = [4, 6, 2, 5]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
block_subdivs[1] = [10,10]
block_subdivs[2] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 7]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [6, 2]
top = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top[0] = [2, 5]
top[1] = [5, 3]
left = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left[0] = [3, 8]
left[1] = [8, 0]
step = blocks.create_patch_nb_faces(name = 'step', nb_faces = 2)
step[0] = [7, 4]
step[1] = [4, 6]
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(mesh.uri())

make_boundary_global = root.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

wall_distance = root.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.step]
wall_distance.execute()

domain.write_mesh(cf.URI('wall-distance-2dstep.pvtu'))

mesh.delete_component()

# 3D, triangle surface elements
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'mesh')
make_boundary_global.mesh = mesh
make_boundary_global.execute()
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.inner]
wall_distance.execute()
domain.write_mesh(cf.URI('wall-distance-sphere.pvtu'))

mesh.delete_component()

# 3D, quad surface elements
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 9)
points[0] = [0., 0.]
points[1] = [1., 0.]
points[2] = [1., 1.]
points[3] = [0., 1.]
points[4] = [0.5, 0.5]
points[5] = [0.5, 1.]
points[6] = [1., 0.5]
points[7] = [0.5, 0.]
points[8] = [0., 0.5]
block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 7, 4, 8]
block_nodes[1] = [8, 4, 5, 3]
block_nodes[2] = [4, 6, 2, 5]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
block_subdivs[1] = [10,10]
block_subdivs[2] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 7]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [6, 2]
top = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top[0] = [2, 5]
top[1] = [5, 3]
left = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left[0] = [3, 8]
left[1] = [8, 0]
step = blocks.create_patch_nb_faces(name = 'step', nb_faces = 2)
step[0] = [7, 4]
step[1] = [4, 6]
blocks.extrude_blocks(positions=[1.], nb_segments=[10], gradings=[1.])
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(mesh.uri())

make_boundary_global.mesh = mesh
make_boundary_global.execute()
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.step, mesh.topology.back]
wall_distance.execute()
domain.write_mesh(cf.URI('wall-distance-3dstep.pvtu'))