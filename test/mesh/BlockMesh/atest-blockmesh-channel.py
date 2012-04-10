import math
import sys
import string

sys.path.append('/data/scholl/coolfluid3/build/dso')
#sys.path.append('/home/sebastian/coolfluid3/build/dso')

import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.options().configure_option('log_level', 4)

# simulation parameters
x_parts = 1
y_parts = 1
z_parts = 1

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')

points = blocks.create_points(dimensions = 2, nb_points = 12)
points[0]  = [0, 0.]
points[1]  = [1, 0.]
points[2]  = [0.,0.2]
points[3]  = [1, 0.2]
points[4]  = [0.,2.1]
points[5]  = [1, 2.2]

points[6]  = [2.,0.]
points[7]  = [2, 0.2]
points[8]  = [2, 2.3]

points[9]  = [-1.,0.]
points[10]  = [-1, 0.2]
points[11]  = [-1, 2.]

#points[12]  = [0, -0.1]
#points[13]  = [1, -0.1]

block_nodes = blocks.create_blocks(6)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_nodes[2] = [1, 6, 7, 3]
block_nodes[3] = [3, 7, 8, 5]
block_nodes[4] = [9, 0, 2, 10]
block_nodes[5] = [10, 2, 4, 11]

#block_nodes[6] = [0, 12, 13, 1]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [100, 50]
block_subdivs[1] = [100, 50]

block_subdivs[2] = [100, 50]
block_subdivs[3] = [100, 50]
block_subdivs[4] = [100, 50]
block_subdivs[5] = [100, 50]

#block_subdivs[6] = [20, 100]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 5., 5.]
gradings[1] = [1., 1., 10., 10.]
gradings[2] = [1., 1., 5., 5.]
gradings[3] = [1., 1., 10., 10.]
gradings[4] = [1., 1., 5., 5.]
gradings[5] = [1., 1., 10., 10.]

#gradings[6] = [1., 1., 1., 1.]

# fluid block

inlet_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 2)
inlet_patch[0] = [9, 10]
inlet_patch[1] = [10, 11]

bottom_patch1 = blocks.create_patch_nb_faces(name = 'bottom1', nb_faces = 1)
bottom_patch1[0] = [0, 1]

bottom_patch2 = blocks.create_patch_nb_faces(name = 'bottom2', nb_faces = 1)
bottom_patch2[0] = [1, 6]

bottom_patch3 = blocks.create_patch_nb_faces(name = 'bottom3', nb_faces = 1)
bottom_patch3[0] = [9, 0]

outlet_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 2)
outlet_patch[0] = [6, 7]
outlet_patch[1] = [7, 8]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 3)
top_patch[0] = [5, 4]
top_patch[1] = [5, 8]
top_patch[2] = [11, 4]

# solid block

#solid_left_patch = blocks.create_patch_nb_faces(name = 'solid_left', nb_faces = 1)
#solid_left_patch[0] = [12, 0]

#solid_bottom_patch = blocks.create_patch_nb_faces(name = 'solid_bottom', nb_faces = 1)
#solid_bottom_patch[0] = [12, 13]

#solid_right_patch = blocks.create_patch_nb_faces(name = 'solid_right', nb_faces = 1)
#solid_right_patch[0] = [13, 1]

##solid_top_patch = blocks.create_patch_nb_faces(name = 'solid_top', nb_faces = 1)
##solid_top_patch[0] = [0, 1]


# Generate a channel mesh
mesh = root.create_component('Mesh', 'cf3.mesh.Mesh')
#blocks.extrude_blocks(positions=[0.025, 0.05], nb_segments=[10, 10], gradings=[5., 0.2])
#blocks.partition_blocks(nb_partitions = x_parts, direction = 0)
#blocks.partition_blocks(nb_partitions = y_parts, direction = 1)
#blocks.partition_blocks(nb_partitions = z_parts, direction = 2)
blocks.create_mesh(mesh.uri())
mesh.write_mesh('flatplate.pvtu')
mesh.write_mesh('flatplate.neu')
