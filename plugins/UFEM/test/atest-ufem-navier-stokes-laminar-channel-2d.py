import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

ic_visc = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_viscosity')
ic_visc.variable_name = 'EffectiveViscosity'

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [10., 0.]
points[2]  = [0., 0.5]
points[3]  = [10., 0.5]
points[4]  = [0.,1.]
points[5]  = [10., 1.]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40, 20]
block_subdivs[1] = [40, 20]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]

blocks.partition_blocks(nb_partitions = cf.Core.nb_procs(), direction = 0)

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

ns_solver.regions = [mesh.topology.uri()]

u_in = [2., 0.]

solver.create_fields()

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
ic_visc.value = ['10. + 2*sin(2/pi*x)']
ic_visc.regions = [mesh.topology.uri()]
ic_visc.execute()
domain.write_mesh(cf.URI('laminar-channel-2d_output-init.pvtu'))

# Physical constants
physics.options().set('density', 1000.)
physics.options().set('dynamic_viscosity', 10.)
physics.options().set('reference_velocity', u_in[0])

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.

pressure_integral = solver.add_unsteady_solver('cf3.UFEM.SurfaceIntegral')
pressure_integral.variable_name = 'Pressure'
pressure_integral.field_tag = 'navier_stokes_solution'
pressure_integral.regions = [mesh.topology.access_component('bottom').uri()]
pressure_integral.history = solver.create_component('ForceHistory', 'cf3.solver.History')
pressure_integral.history.file = cf.URI('force-implicit.tsv')
pressure_integral.history.dimension = 2

# Time setup
time = model.create_time()
time.options().set('time_step', 0.1)

#ns_solver.options().set('disabled_actions', ['SolveLSS'])

# Setup a time series write
final_end_time = 10.
save_interval = 1.
current_end_time = 0.
iteration = 0

writer = root.create_component('Writer', 'cf3.mesh.gmsh.Writer')
writer.fields = [mesh.geometry.navier_stokes_solution.uri()]
writer.enable_overlap = True
writer.mesh = mesh

while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  writer.file = cf.URI('laminar-channel-2d_output-' +str(iteration) + '.msh')
  writer.execute()
  domain.write_mesh(cf.URI('laminar-channel-2d_output-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
