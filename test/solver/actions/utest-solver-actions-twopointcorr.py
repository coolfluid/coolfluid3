import coolfluid as cf

env = cf.Core.environment()
env.log_level = 3

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('OriginalMesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [16,16]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.extrude_blocks(positions=[1.], nb_segments=[4], gradings=[1.])

blocks.create_mesh(mesh.uri())

coords = mesh.geometry.coordinates
for i in range(len(coords)):
  coords[i][0] += 1
  coords[i][1] += 1
  coords[i][2] += 1

corr = domain.create_component('TwoPointCorrelation', 'cf3.solver.actions.TwoPointCorrelation')
corr.normal = 0
corr.field = coords
corr.coordinate = 1.75
corr.file = cf.URI('two-point-correlation01-{iteration}.txt')
corr.interval = 5

corr2 = domain.create_component('TwoPointCorrelation', 'cf3.solver.actions.TwoPointCorrelation')
corr2.normal = 1
corr2.field = coords
corr2.coordinate = 1.75
corr2.file = cf.URI('two-point-correlation02-{iteration}.txt')
corr2.interval = 10

for i in range(20):
  corr.execute()
  corr2.execute()