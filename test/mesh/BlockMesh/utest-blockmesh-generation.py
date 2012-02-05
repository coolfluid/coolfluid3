import coolfluid as cf

root = cf.Core.root()

block_arrays = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')

length = 5.
half_height = 0.5
width = 3.

points  = block_arrays.create_points(dimensions = 3, nb_points = 12)
points[ 0] = [0.    ,-half_height,0.   ]
points[ 1] = [length,-half_height,0.   ]
points[ 2] = [0.    , 0.         ,0.   ]
points[ 3] = [length, 0.         ,0.   ]
points[ 4] = [0.    , half_height,0.   ]
points[ 5] = [length, half_height,0.   ]
points[ 6] = [0.    ,-half_height,width]
points[ 7] = [length,-half_height,width]
points[ 8] = [0.    , 0.         ,width]
points[ 9] = [length, 0.         ,width]
points[10] = [0.    , half_height,width]
points[11] = [length, half_height,width]

blocks = block_arrays.create_blocks(2)
blocks[0] = [0, 1, 3, 2, 6, 7, 9, 8]
blocks[1] = [2, 3, 5, 4, 8, 9, 11, 10]

subdivs = block_arrays.create_block_subdivisions()
subdivs[0] = [10, 5, 5]
subdivs[1] = [10, 5, 5]

ratio = 0.2
gradings = block_arrays.create_block_gradings()
gradings[0] = [1,1,1,1,1./ratio,1./ratio,1./ratio,1./ratio,1,1,1,1]
gradings[1] = [1,1,1,1,ratio,ratio,ratio,ratio,1,1,1,1]

inner_blocks = block_arrays.create_block_mesh()
inner_blocks.write_mesh('block_mesh01.msh')
block_arrays.create_patch_face_list(name = 'in', face_list = [4, 9])
block_arrays.create_patch_face_list(name = 'out', face_list = [3, 7])
block_arrays.create_patch_face_list(name = 'top', face_list = [8])
block_arrays.create_patch_face_list(name = 'bottom', face_list = [2])
block_arrays.create_patch_face_list(name = 'front', face_list = [0, 5])
block_arrays.create_patch_face_list(name = 'back', face_list = [1, 6])
patched_blocks = block_arrays.create_block_mesh()
patched_blocks.write_mesh('block_mesh02.msh')

block_arrays.partition_blocks(nb_partitions = 5, direction = 0)
patched_blocks = block_arrays.create_block_mesh()
patched_blocks.write_mesh('block_mesh03.msh')

block_arrays.options().configure_option("block_distribution", [0, 10])

mesh = root.create_component('OutputMesh', 'cf3.mesh.Mesh')
block_arrays.create_mesh(mesh.uri())
mesh.write_mesh('meshed.msh')