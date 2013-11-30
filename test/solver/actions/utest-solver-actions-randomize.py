import sys
import coolfluid as cf
import os

#Centerline velocity
Uc = 3.

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
block_subdivs[0] = [64,64]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.partition_blocks(nb_partitions = cf.Core.nb_procs(), direction = 0)
blocks.create_mesh(mesh.uri())

periodic_partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
periodic_partitioner.mesh = mesh
periodic_partitioner.load_balance = False

link_horizontal = periodic_partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0.]

link_vertical = periodic_partitioner.create_link_periodic_nodes()
link_vertical.source_region = mesh.topology.top
link_vertical.destination_region = mesh.topology.bottom
link_vertical.translation_vector = [0., -1.]

periodic_partitioner.execute()

# Create a field
velocity = mesh.geometry.create_field(name = 'velocity', variables='Velocity[vector]')

# Set the X velocity to a parabolic profile
for i in range(len(velocity)):
  y = mesh.geometry.coordinates[i][1]
  velocity[i][0] = 4.*Uc*y*(1.-y)
  
# Randomize
randomizer = domain.create_component('Randomizer', 'cf3.solver.actions.RandomizeField')
randomizer.field = velocity
randomizer.variable_name = 'Velocity'
randomizer.maximum_variations = [0.3, 0.3]
randomizer.maximum_values = [Uc, Uc/3.]
randomizer.minimum_values = [-Uc, -Uc/3.]
randomizer.options.seed = 1
randomizer.execute()

writer = domain.create_component('PVWriter', 'cf3.mesh.VTKXML.Writer')
writer.fields = [velocity.uri()]
writer.mesh = mesh
writer.file = cf.URI('randomize.pvtu')
writer.execute()