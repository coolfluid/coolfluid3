import coolfluid as cf

root = cf.Core.root()

cf.env.assertion_throws = False
cf.env.exception_aborts = True

segs = 10

mesh = root.create_component('mesh','cf3.mesh.Mesh')
blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [-1., -1.]
points[1]  = [1., -1.]
points[2]  = [1., 1.]
points[3]  = [-1., 1.]
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
#blocks.extrude_blocks(positions=[1., 2.], nb_segments=[segs/2, segs/2], gradings=[1., 1.])
blocks.create_mesh(mesh.uri())

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

u = mesh.geometry.create_field(name = 'u', size = 1)
coords = mesh.geometry.coordinates
for (i,(x,y)) in enumerate(coords):
  if y < -0.999:
    print x
  u[i][0] = 1. - x**2

p2space = mesh.create_continuous_space(name = 'TestP2Space', shape_function = 'cf3.mesh.LagrangeP2')
u2 = p2space.create_field(name = 'u2', size = 1)
coords = p2space.coordinates
for (i,(x,y)) in enumerate(coords):
  if y < -0.999:
    print x
  u2[i][0] = 1. - x**2

writer = root.create_component('CF3ToVTK', 'cf3.vtk.MultiblockWriter')
writer.mesh =  mesh
writer.file = cf.URI('multiblockwriter-out.vtm')
writer.execute()

