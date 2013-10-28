import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
source_mesh = root.create_component('sourcemesh','cf3.mesh.Mesh')

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
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(source_mesh.uri())

source_coords = source_mesh.geometry.coordinates
testfield = source_mesh.geometry.create_field(name = 'test', variables = 'testx,testy')
for i in range(len(source_coords)):
  testfield[i][0] = 2.*source_coords[i][0]
  testfield[i][1] = 3.*source_coords[i][1]
  
target_mesh = domain.create_component('targetmesh','cf3.mesh.Mesh')
blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40,40]
gradings = blocks.create_block_gradings()
gradings[0] = [0.2, 0.2, 10., 10.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(target_mesh.uri())

interpolator = root.create_component('Interpolator', 'cf3.mesh.actions.MeshInterpolator')
interpolator.source_mesh = source_mesh
interpolator.target_mesh = target_mesh
interpolator.execute()

domain.write_mesh(cf.URI('interpolator.pvtu'))