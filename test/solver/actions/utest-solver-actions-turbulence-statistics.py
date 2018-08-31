import sys
import coolfluid as cf
import os

#Centerline velocity
Uc = 3.

env = cf.Core.environment()
env.log_level = 1
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
block_subdivs[0] = [40,40]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]

blocks.create_mesh(mesh.uri())

# Create a field
velocity = mesh.geometry.create_field(name = 'velocity', variables='Velocity[vector]')
pressure = mesh.geometry.create_field(name = 'pressure', variables='Pressure')

# Action to initialize it
init_field = domain.create_component('InitField', 'cf3.mesh.actions.InitFieldFunction')
init_field.field = velocity
init_field.functions = ['4.*{Uc}*y*(1.-y)'.format(Uc=Uc), '0']
  
# Randomize
randomizer = domain.create_component('Randomizer', 'cf3.solver.actions.RandomizeField')
randomizer.field = velocity
randomizer.variable_name = 'Velocity'
randomizer.maximum_variations = [0.3, 0.3]
randomizer.maximum_values = [Uc*1.3, Uc/3.]
randomizer.minimum_values = [-Uc, -Uc/3.]

stats = domain.create_component('Statistics', 'cf3.solver.actions.TurbulenceStatistics')
stats.region = mesh.topology
stats.file = cf.URI('turbulence-statistics.txt')
stats.rolling_window = 10
stats.add_probe([1., 0.5 ])
stats.add_probe([1., 0.15])
stats.setup()

avg = domain.create_component('Average', 'cf3.solver.actions.FieldTimeAverage')
avg.field = velocity

dir_avg = domain.create_component('DirectionalAverage', 'cf3.solver.actions.DirectionalAverage')
dir_avg.direction = 1
dir_avg.field = mesh.geometry.turbulence_statistics
dir_avg.file = cf.URI('turbulence-statistics-profile.txt')

for i in range(1000):
  init_field.execute()
  randomizer.options.seed = i
  randomizer.execute()
  stats.execute()
  avg.execute()

dir_avg.execute()

writer = domain.create_component('PVWriter', 'cf3.mesh.VTKXML.Writer')
writer.fields = [velocity.uri(), mesh.geometry.coordinates.uri(), mesh.geometry.turbulence_statistics.uri(), mesh.geometry.average_velocity.uri()]
writer.mesh = mesh
writer.file = cf.URI('turbulence-statistics.pvtu')
writer.execute()