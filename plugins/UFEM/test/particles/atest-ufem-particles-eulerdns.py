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

dt = 0.04
segs = 32
theta = 0.5
D = 0.5

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
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
eulerdns = solver.add_unsteady_solver('cf3.UFEM.particles.EulerDNS')

# Set up the physical constants
physics.density = 1.
physics.dynamic_viscosity = nu
physics.reference_velocity = 1.

# Create the mesh
mesh = create_mesh(domain, segs)

# Set the boundary condition
bc = ns_solver.BoundaryConditions
bc.regions = [mesh.topology.uri()]
nu = model.NavierStokesPhysics.kinematic_viscosity
bc.add_function_bc(region_name = 'center', variable_name = 'Pressure').value = ['-0.25 * (cos(2*pi/{D}*x) + cos(2*pi/{D}*y)) * exp(-4*{nu}*pi^2/{D}^2*(t+{dt})) '.format(D = D, nu = nu, dt = dt)]

# Set the initial conditions
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.interior.uri()]
ic_u.value = ['-cos(pi/{D}*x)*sin(pi/{D}*y)'.format(D = D), 'sin(pi/{D}*x)*cos(pi/{D}*y)'.format(D = D)]

ic_p = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_p.regions = [mesh.topology.interior.uri()]
ic_p.variable_name = 'Pressure'
ic_p.value = ['-0.25*(cos(2*pi/{D}*x) + cos(2*pi/{D}*y))'.format(D = D)]

ns_solver.regions = [mesh.topology.interior.uri()]
ns_solver.options.theta = theta
eulerdns.regions = [mesh.topology.interior.uri()]
    
series_writer = solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
writer.file = cf.URI('eulerdns-{iteration}.pvtu')
writer.mesh = mesh
series_writer.interval = write_interval

solver.create_fields()
writer.fields = [mesh.geometry.navier_stokes_solution.uri(), mesh.geometry.particle_concentration.uri()]
ns_solver.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
eulerdns.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'

for i in range(len(mesh.geometry.coordinates)):
  mesh.geometry.particle_concentration[i][0] = 1.
  # x = tg_impl.mesh.geometry.coordinates[i][0]
  # y = tg_impl.mesh.geometry.coordinates[i][1]
  # tg_impl.mesh.geometry.particle_concentration[i][0] = np.exp(-((x-0.25)**2 + y**2)/0.1**2) + np.exp(-((x+0.25)**2 + y**2)/0.1**2)
  #if abs(x) < 0.0001:
    #tg_impl.mesh.geometry.particle_concentration[i][0] = 1.

eulerdns.options.tau_p = 0.25
    
# Time setup
time = model.create_time()
time.time_step = dt
time.end_time = numsteps*dt

model.simulate()
