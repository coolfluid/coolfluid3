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

inner_blocks = block_arrays.create_block_mesh()
inner_blocks.write_mesh('block_mesh01.msh')
block_arrays.create_patch_face_list(name = 'test01', face_list = [0, 1])
block_arrays.create_patch_face_list(name = 'test01', face_list = [2, 8])
patched_blocks = block_arrays.create_block_mesh()
patched_blocks.write_mesh('block_mesh02.msh')