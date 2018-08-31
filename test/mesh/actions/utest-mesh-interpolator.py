import sys
import coolfluid as cf
from math import pi

env = cf.Core.environment()
env.log_level = 4
#env.only_cpu0_writes = False

root = cf.Core.root()
source_domain = root.create_component('SourceDomain', 'cf3.mesh.Domain')
target_domain = root.create_component('TargetDomain', 'cf3.mesh.Domain')
source_mesh = source_domain.create_component('sourcemesh','cf3.mesh.Mesh')

h = 1.

div = 2

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
grading = 20.
y_segs = 64/div
x_size = 4.*pi*h
z_size = 2.*pi*h
x_segs = 32/div
z_segs = 32/div
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., h]
points[3]  = [x_size, h]
points[4]  = [0.,2.*h]
points[5]  = [x_size, 2.*h]
block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]
block_subdivs[1] = block_subdivs[0]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., grading, grading]
gradings[1] = [1., 1., 1./grading, 1./grading]
left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]
bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]
top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]
right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]
blocks.extrude_blocks(positions=[z_size], nb_segments=[z_segs], gradings=[1.])



blocks.create_mesh(source_mesh.uri())

source_coords = source_mesh.geometry.coordinates
testfield = source_mesh.geometry.create_field(name = 'test', variables = 'testx,testy')
for i in range(len(source_coords)):
  testfield[i][0] = 1.+2.*source_coords[i][0]
  testfield[i][1] = 2.+3.*source_coords[i][1]
  
target_mesh = target_domain.create_component('targetmesh','cf3.mesh.Mesh')
blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
grading = 80.
y_segs = 128/div
x_size = 4.*pi*h
z_size = 2.*pi*h
x_segs = 32/div
z_segs = 32/div
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., h]
points[3]  = [x_size, h]
points[4]  = [0.,2.*h]
points[5]  = [x_size, 2.*h]
block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]
block_subdivs[1] = block_subdivs[0]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., grading, grading]
gradings[1] = [1., 1., 1./grading, 1./grading]
left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]
bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]
top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]
right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]
blocks.extrude_blocks(positions=[z_size], nb_segments=[z_segs], gradings=[1.])



blocks.create_mesh(target_mesh.uri())

interpolator = root.create_component('Interpolator', 'cf3.mesh.actions.MeshInterpolator')
interpolator.source_mesh = source_mesh
interpolator.target_mesh = target_mesh
interpolator.execute()

source_domain.write_mesh(cf.URI('interpolator-source.pvtu'))
target_domain.write_mesh(cf.URI('interpolator-target.pvtu'))