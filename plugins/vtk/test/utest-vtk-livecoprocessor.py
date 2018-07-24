import coolfluid as cf

root = cf.Core.root()

segs = 10

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
block_subdivs[0] = [segs,segs]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 0.1, 0.1]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.extrude_blocks(positions=[1., 2.], nb_segments=[segs/2, segs/2], gradings=[1., 1.])
blocks.create_mesh(mesh3d.uri())

cf3tovtk = root.create_component('CF3ToVTK', 'cf3.vtk.CF3ToVTK')
cf3tovtk.options().set('mesh', mesh3d)

coproc = root.create_component('LiveCoProcessor', 'cf3.vtk.LiveCoProcessor')
coproc.cf3_to_vtk = cf3tovtk
coproc.time = root.create_component('Time', 'cf3.solver.Time')
coproc.execute()

#coproc.delete_component()
#coproc = None

#print('deleted coproc')
