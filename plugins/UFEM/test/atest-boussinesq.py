import sys
# sys.path.append('/data/scholl/coolfluid3/build/dso')
# sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global confifuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

# setup a model
model = root.create_component('Boussinesq', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Create a component to manage initial conditions
ic = solver.create_initial_conditions()

# Add the Navier-Stokes solver as an unsteady solver
boussinesq = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
boussinesq.options().set('use_boussinesq', True)

boussinesq.Assembly.BoussinesqAssemblyQuads.g = [0., 9.81]

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1.,1.]
points[3]  = [0., 1.]

block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40, 40]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]

# fluid block
left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)
left_patch[0] = [3, 0]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)
right_patch[0] = [1, 2]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [2, 3]

blocks.options().set('block_regions', ['fluid'])

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# For each solver, set the region in which it operates
boussinesq.options().set('regions', [mesh.access_component('topology/fluid').uri()])

u_in = [1., 0.]
u_wall = [0., 0.]
phi_in = 10.
phi_wall = 0.

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
solver.InitialConditions.navier_stokes_solution.Temperature = phi_in

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = 1.e-5
physics.reference_velocity = u_in[0]

# Boundary conditions for Boussinesq
bc = boussinesq.get_child('BoundaryConditions')
bc.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work

bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').options().set('value', u_wall)
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_bc(region_name = 'right', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').options().set('value', u_wall)

bc.add_constant_bc(region_name = 'left', variable_name = 'Temperature').options().set('value', phi_wall)
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Temperature').options().set('value',  phi_in)
bc.add_constant_bc(region_name = 'right', variable_name = 'Temperature').options().set('value',  phi_wall)
bc.add_constant_bc(region_name = 'top', variable_name = 'Temperature').options().set('value', phi_wall)

# Time setup
time = model.create_time()
time.options().set('time_step', 0.01)

# Setup a time series write

final_end_time = 10.
save_interval = 0.01
current_end_time = 0.
iteration = 0.

while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('boussinesq-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
