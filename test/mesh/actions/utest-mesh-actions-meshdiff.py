import sys
import coolfluid as cf

def build_mesh(mesh, parent):
  blocks = parent.create_component('Blocks', 'cf3.mesh.BlockMesh.BlockArrays')
  points = blocks.create_points(dimensions = 2, nb_points = 4)
  points[0]  = [0., 0.]
  points[1]  = [1., 0.]
  points[2]  = [1., 1.]
  points[3]  = [0., 1.]
  block_nodes = blocks.create_blocks(1)
  block_nodes[0] = [0, 1, 2, 3]
  block_subdivs = blocks.create_block_subdivisions()
  block_subdivs[0] = [40,20]
  gradings = blocks.create_block_gradings()
  gradings[0] = [1., 1., 1., 1.]
  blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
  blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
  blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
  blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
  blocks.autopartition = False

  blocks.create_mesh(mesh.uri())
  blocks.delete_component()

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = False

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
meshdiff = domain.create_component('MeshDiff', 'cf3.mesh.actions.MeshDiff')
meshdiff.left = domain.create_component('mesh1','cf3.mesh.Mesh')
meshdiff.right = domain.create_component('mesh2','cf3.mesh.Mesh')

build_mesh(meshdiff.left, domain)

meshdiff.execute()
if meshdiff.properties()['mesh_equal']:
  raise Exception('Incorrect meshdiff result')

build_mesh(meshdiff.right, domain)

meshdiff.execute()
if not meshdiff.properties()['mesh_equal']:
  raise Exception('Incorrect meshdiff result')

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = meshdiff.right
make_par_data.execute()

meshdiff.execute()
if meshdiff.properties()['mesh_equal']:
  raise Exception('Incorrect meshdiff result')

make_par_data.mesh = meshdiff.left
make_par_data.execute()

meshdiff.execute()
if not meshdiff.properties()['mesh_equal']:
  raise Exception('Incorrect meshdiff result')
