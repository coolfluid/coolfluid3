import sys
import coolfluid as cf
import os

def copy_and_reset(source, domain):
  nb_items = len(source)
  destination = domain.create_component(source.name(), 'cf3.mesh.Field')
  destination.set_row_size(1)
  destination.resize(nb_items)

  for i in range(nb_items):
    destination[i][0] = source[i][0]
    source[i][0] = 0
    
  return destination

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

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

blocks.create_mesh(mesh.uri())

time = domain.create_component('Time', 'cf3.solver.Time')
time.time_step = 0.1
time.end_time = 1.

series_writer = domain.create_component('SeriesWriter', 'cf3.solver.actions.TimeSeriesWriter')
series_writer.time = time
series_writer.interval = 5

writer = series_writer.create_component('Writer', 'cf3.mesh.cf3mesh.Writer')
file_template = 'timeseries-{iteration}-{time}.cf3mesh'
writer.file = cf.URI(file_template)
writer.mesh = mesh

advance_time = domain.create_component('AdvanceTime', 'cf3.solver.actions.AdvanceTime')
advance_time.time = time

# Write the time series
for i in range(10):
  series_writer.execute()
  advance_time.execute()

# Check that only the expected files exist, and if they do read and compare to reference mesh
reader = domain.create_component('Reader', 'cf3.mesh.cf3mesh.Reader')
meshdiff = domain.create_component('MeshDiff', 'cf3.mesh.actions.MeshDiff')
meshdiff.left = mesh
for i in range(10):
  if i != 0:
    filename = file_template.format(time = float(i)*time.time_step, iteration = i)
  else:
    filename = file_template.format(time = 0, iteration = 0)
  if i % series_writer.interval != 0:
    if os.path.isfile(filename):
      raise Exception('File ' + filename + ' is not supposed to exist')
  else:
    reader.mesh = domain.create_component('ReadMesh', 'cf3.mesh.Mesh')
    reader.file = cf.URI(filename)
    reader.execute()
    meshdiff.right = reader.mesh
    meshdiff.execute()
    if not meshdiff.properties()['mesh_equal']:
      raise Exception('Error in read mesh')
