import math
import sys
import string
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.options().configure_option('log_level', 4)

# simulation parameters
x_parts = 4
y_parts = 2
z_parts = 2

# Meshing parameters
step_length = 10.e-3
step_height = 4.9e-3
channel_height = 5.2e-3
channel_length = 150.e-3

top_grading = 0.2
bottom_grading = 0.2
step_x_grading = 0.8
channel_x_grading = 1.5

step_x_segs = 20
step_y_segs = 11
channel_x_segs = 180
channel_y_segs = 9

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')

points = blocks.create_points(dimensions = 2, nb_points = 13)
points[0]  = [-step_length,   0.               ]
points[1]  = [-step_length,   channel_height/2.]
points[2]  = [-step_length,   channel_height   ]
points[3]  = [0.,             -step_height     ]
points[4]  = [0.,             -step_height/2.  ]
points[5]  = [0.,             0.               ]
points[6]  = [0.,             channel_height/2.]
points[7]  = [0.,             channel_height   ]
points[8]  = [channel_length, -step_height     ]
points[9]  = [channel_length, -step_height/2.  ]
points[10] = [channel_length, 0.               ]
points[11] = [channel_length, channel_height/2.]
points[12] = [channel_length, channel_height   ]

block_nodes = blocks.create_blocks(6)
block_nodes[0] = [0, 5, 6, 1]
block_nodes[1] = [1, 6, 7, 2]
block_nodes[2] = [3, 8, 9, 4]
block_nodes[3] = [4, 9, 10, 5]
block_nodes[4] = [5, 10, 11, 6]
block_nodes[5] = [6, 11, 12, 7]

# y-segments, ordered top-to-bottom
y_segs = [int(math.ceil(step_y_segs/2.)), int(math.floor(step_y_segs/2.)), int(math.floor(channel_y_segs/2.)), int(math.ceil(channel_y_segs/2.))]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [step_x_segs, y_segs[1]]
block_subdivs[1] = [step_x_segs, y_segs[0]]
block_subdivs[2] = [channel_x_segs, y_segs[3]]
block_subdivs[3] = [channel_x_segs, y_segs[2]]
block_subdivs[4] = [channel_x_segs, y_segs[1]]
block_subdivs[5] = [channel_x_segs, y_segs[0]]

gradings = blocks.create_block_gradings()
gradings[0] = [step_x_grading, step_x_grading, 1./top_grading, 1./top_grading]
gradings[1] = [step_x_grading, step_x_grading, top_grading, top_grading]
gradings[2] = [channel_x_grading, channel_x_grading, 1./bottom_grading, 1./bottom_grading]
gradings[3] = [channel_x_grading, channel_x_grading, bottom_grading, bottom_grading]
gradings[4] = [channel_x_grading, channel_x_grading, 1./top_grading, 1./top_grading]
gradings[5] = [channel_x_grading, channel_x_grading, top_grading, top_grading]

inlet_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 2)
inlet_patch[0] = [1, 0]
inlet_patch[1] = [2, 1]

step_top = blocks.create_patch_nb_faces(name = 'step_top', nb_faces = 1)
step_top[0] = [0, 5]

step_front = blocks.create_patch_nb_faces(name = 'step_front', nb_faces = 2)
step_front[0] = [4, 3]
step_front[1] = [5, 4]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top_patch[0] = [12, 7]
top_patch[1] = [7, 2]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [3, 8]

outlet_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 4)
outlet_patch[0] = [8, 9]
outlet_patch[1] = [9, 10]
outlet_patch[2] = [10, 11]
outlet_patch[3] = [11, 12]

# Generate a channel mesh
mesh = root.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.extrude_blocks(positions=[0.025, 0.05], nb_segments=[10, 10], gradings=[5., 0.2])
blocks.partition_blocks(nb_partitions = x_parts, direction = 0)
blocks.partition_blocks(nb_partitions = y_parts, direction = 1)
blocks.partition_blocks(nb_partitions = z_parts, direction = 2)
blocks.create_mesh(mesh.uri())
mesh.write_mesh('backstep.pvtu')
