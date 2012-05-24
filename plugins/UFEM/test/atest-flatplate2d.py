import sys
#sys.path.append('/data/scholl/coolfluid3/build/dso')
# sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global confifuration
env.options().set('assertion_throws', False)
env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('log_level', 4)

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Create a component to manage initial conditions
ic = solver.create_initial_conditions()

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the scalar advection solver as an unsteady solver
scalaradv = solver.add_unsteady_solver('cf3.UFEM.ScalarAdvection')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 12)
points[0]  = [0, 0.]
points[1]  = [1, 0.]
points[2]  = [0.,0.2]
points[3]  = [1, 0.2]
points[4]  = [0.,1.1]
points[5]  = [1, 1.2]

points[6]  = [2.,0.]
points[7]  = [2, 0.2]
points[8]  = [2, 1.3]

points[9]  = [-1.,0.]
points[10]  = [-1, 0.2]
points[11]  = [-1, 1.]

block_nodes = blocks.create_blocks(6)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_nodes[2] = [1, 6, 7, 3]
block_nodes[3] = [3, 7, 8, 5]
block_nodes[4] = [9, 0, 2, 10]
block_nodes[5] = [10, 2, 4, 11]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [80, 40]
block_subdivs[1] = [80, 40]
block_subdivs[2] = [80, 40]
block_subdivs[3] = [80, 40]
block_subdivs[4] = [80, 40]
block_subdivs[5] = [80, 40]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 5., 5.]
gradings[1] = [1., 1., 10., 10.]
gradings[2] = [1., 1., 5., 5.]
gradings[3] = [1., 1., 10., 10.]
gradings[4] = [1., 1., 5., 5.]
gradings[5] = [1., 1., 10., 10.]

# fluid block
inlet_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 2)
inlet_patch[0] = [9, 10]
inlet_patch[1] = [10, 11]

bottom_patch1 = blocks.create_patch_nb_faces(name = 'bottom1', nb_faces = 1)
bottom_patch1[0] = [0, 1]

bottom_patch2 = blocks.create_patch_nb_faces(name = 'bottom2', nb_faces = 1)
bottom_patch2[0] = [1, 6]

bottom_patch3 = blocks.create_patch_nb_faces(name = 'bottom3', nb_faces = 1)
bottom_patch3[0] = [9, 0]

outlet_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 2)
outlet_patch[0] = [6, 7]
outlet_patch[1] = [7, 8]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 3)
top_patch[0] = [5, 4]
top_patch[1] = [5, 8]
top_patch[2] = [11, 4]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())
nstokes.options().set('regions', [mesh.access_component('topology').uri()])
scalaradv.options().set('regions', [mesh.access_component('topology').uri()])

# LSS for Navier-Stokes
ns_lss = nstokes.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
ns_lss.get_child('Matrix').options().set('settings_file', sys.argv[1])
#LSS for scalar advection
sa_lss = scalaradv.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
sa_lss.get_child('Matrix').options().set('settings_file', sys.argv[1])

u_in = [0.5, 0.]
u_wall = [0., 0.]
phi_in = 100
phi_wall = 200

# Add initial conditions for the Navier-Stokes solver, which uses 'navier_stokes_solution' as a tag for its solution fields
ic_ns = ic.create_initial_condition('navier_stokes_solution')
# Initial advection velocity and its previous values, using linearized_velocity as tag
ic_linearized_vel = ic.create_initial_condition('linearized_velocity')
# Initial conditions for the scalar advection solver
ic_phi = ic.create_initial_condition('scalar_advection_solution')

#initial conditions
ic_ns.options().set('Velocity', u_in)
ic_linearized_vel.options().set('AdvectionVelocity', u_in)
ic_linearized_vel.options().set('AdvectionVelocity1', u_in)
ic_linearized_vel.options().set('AdvectionVelocity2', u_in)
ic_linearized_vel.options().set('AdvectionVelocity3', u_in)
ic_phi.options().set('Scalar', phi_in)

#properties for Navier-Stokes
physics.options().set('density', 1.2)
physics.options().set('dynamic_viscosity', 1.7894e-5)
physics.options().set('reference_velocity', u_in[0])
scalaradv.options().set('scalar_coefficient', 1.)

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').options().set('value', u_in)
bc.add_constant_bc(region_name = 'bottom1', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_component_bc(region_name = 'bottom3', variable_name = 'Velocity', component = 1).options().set('value',  0.)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').options().set('value', 1.)
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').options().set('value', u_in)

# Boundary conditions for ScalarAdvection
bc = scalaradv.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Scalar').options().set('value', phi_in)
bc.add_constant_bc(region_name = 'bottom1', variable_name = 'Scalar').options().set('value',  phi_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Scalar').options().set('value',  phi_in)
bc.add_constant_bc(region_name = 'bottom3', variable_name = 'Scalar').options().set('value',  phi_in)
bc.add_constant_bc(region_name = 'top', variable_name = 'Scalar').options().set('value', phi_in)

# Time setup
time = model.create_time()
time.options().set('time_step', 0.01)

# Setup a time series write
final_end_time = 0.1
save_interval = 0.1
current_end_time = 0.
iteration = 0
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-flatplate2d_output_b-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
