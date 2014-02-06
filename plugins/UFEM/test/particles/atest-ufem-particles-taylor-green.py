import sys
import coolfluid as cf
import numpy as np
import os
from optparse import OptionParser

env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

dt = 0.1
segs = 64
D = 0.5
tau_p = 0.1

numsteps = 10
write_interval = 1

nu = 1./5000.

def create_mesh(domain, segments):
  blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
  points = blocks.create_points(dimensions = 2, nb_points = 4)
  points[0]  = [0., 0.]
  points[1]  = [1., 0.]
  points[2]  = [1., 1.]
  points[3]  = [0., 1.]

  block_nodes = blocks.create_blocks(1)
  block_nodes[0] = [0, 1, 2, 3]

  block_subdivs = blocks.create_block_subdivisions()
  block_subdivs[0] = [segments, segments]

  gradings = blocks.create_block_gradings()
  gradings[0] = [1., 1., 1., 1.]

  left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)
  left_patch[0] = [3, 0]

  bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
  bottom_patch[0] = [0, 1]

  top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
  top_patch[0] = [2, 3]

  right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)
  right_patch[0] = [1, 2]
  
  blocks.partition_blocks(nb_partitions = cf.Core.nb_procs(), direction = 0)
  
  mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
  
  blocks.create_mesh(mesh.uri())
  
  create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
  create_point_region.coordinates = [0., 0.]
  create_point_region.region_name = 'center'
  create_point_region.mesh = mesh
  create_point_region.execute()
  
  partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
  partitioner.mesh = mesh

  link_horizontal = partitioner.create_link_periodic_nodes()
  link_horizontal.source_region = mesh.topology.right
  link_horizontal.destination_region = mesh.topology.left
  link_horizontal.translation_vector = [-1., 0.]

  link_vertical = partitioner.create_link_periodic_nodes()
  link_vertical.source_region = mesh.topology.top
  link_vertical.destination_region = mesh.topology.bottom
  link_vertical.translation_vector = [0., -1.]

  partitioner.execute()
  
  return mesh

# Create the model and solvers
model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
tg_solver = solver.add_unsteady_solver('cf3.UFEM.particles.TaylorGreen')
tg_solver.options.tau_p = tau_p

# Set up the physical constants
physics.density = 1.
physics.dynamic_viscosity = nu

# Create the mesh
mesh = create_mesh(domain, segs)
tg_solver.regions = [mesh.topology.interior.uri()]
    
series_writer = solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
writer.file = cf.URI('taylor-green-{iteration}.pvtu')
writer.mesh = mesh
series_writer.interval = write_interval

solver.create_fields()
writer.fields = [mesh.geometry.taylor_green.uri()]
    
# Time setup
time = model.create_time()
time.time_step = dt
time.end_time = numsteps*dt

model.simulate()
