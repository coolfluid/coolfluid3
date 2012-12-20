import sys
# sys.path.append('/data/scholl/coolfluid3/build/dso')
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
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Create a component to manage initial conditions
ic = solver.create_initial_conditions()

# Add the scalar advection solver as an unsteady solver
scalaradv = solver.add_iteration_solver('cf3.UFEM.ScalarAdvection')
scalaradv.options().set('scalar_name', 'Temperature')

# Add the heat conduction solver for the solid
heatcond = solver.add_iteration_solver('cf3.UFEM.HeatConductionSteady')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 14)
points[0]  = [0, 0.]
points[1]  = [1, 0.]
points[2]  = [0.,0.2]
points[3]  = [1, 0.2]
points[4]  = [0.,2.1]
points[5]  = [1, 2.2]

points[6]  = [2.,0.]
points[7]  = [2, 0.2]
points[8]  = [2, 2.3]

points[9]  = [-1.,0.]
points[10]  = [-1, 0.2]
points[11]  = [-1, 2.]

points[12] = [0, -1.]
points[13] = [1., -1]

block_nodes = blocks.create_blocks(7)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_nodes[2] = [1, 6, 7, 3]
block_nodes[3] = [3, 7, 8, 5]
block_nodes[4] = [9, 0, 2, 10]
block_nodes[5] = [10, 2, 4, 11]

block_nodes[6] = [12, 13, 1, 0]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40, 20]
block_subdivs[1] = [40, 20]

block_subdivs[2] = [40, 20]
block_subdivs[3] = [40, 20]
block_subdivs[4] = [40, 20]
block_subdivs[5] = [40, 20]
block_subdivs[6] = [40, 20]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 5., 5.]
gradings[1] = [1., 1., 10., 10.]
gradings[2] = [1., 1., 5., 5.]
gradings[3] = [1., 1., 10., 10.]
gradings[4] = [1., 1., 5., 5.]
gradings[5] = [1., 1., 10., 10.]
gradings[6] = [1., 1., 1., 1.]

# fluid block
inlet_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 2)
inlet_patch[0] = [10, 9]
inlet_patch[1] = [11, 10]

bottom_patch1 = blocks.create_patch_nb_faces(name = 'solid_bottom', nb_faces = 1)
bottom_patch1[0] = [12, 13]

bottom_patch1 = blocks.create_patch_nb_faces(name = 'solid_left', nb_faces = 1)
bottom_patch1[0] = [0, 12]

bottom_patch1 = blocks.create_patch_nb_faces(name = 'solid_right', nb_faces = 1)
bottom_patch1[0] = [13, 1]

bottom_patch2 = blocks.create_patch_nb_faces(name = 'bottom2', nb_faces = 1)
bottom_patch2[0] = [1, 6]

bottom_patch3 = blocks.create_patch_nb_faces(name = 'bottom3', nb_faces = 1)
bottom_patch3[0] = [9, 0]

outlet_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 2)
outlet_patch[0] = [6, 7]
outlet_patch[1] = [7, 8]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 3)
top_patch[0] = [5, 4]
top_patch[1] = [8, 5]
top_patch[2] = [4, 11]

blocks.options().set('block_regions', ['fluid', 'fluid', 'fluid', 'fluid', 'fluid', 'fluid', 'solid'])

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# rename the temperature variables, so each temperature has a unique name (needed for mesh writing)
variables = physics.get_child('VariableManager')
variables.get_child('scalar_advection_solution').options().set('Temperature_variable_name', 'Tadv')
variables.get_child('heat_conduction_solution').options().set('Temperature_variable_name', 'Tcond')

# For each solver, set the region in which it operates
nstokes.options().set('regions', [mesh.access_component('topology/fluid').uri()])
scalaradv.options().set('regions', [mesh.access_component('topology/fluid').uri()])
heatcond.options().set('regions', [mesh.access_component('topology/solid').uri()])

u_in = [1., 0.]
u_wall = [0., 0.]
phi_in = 10.
phi_wall = 0.

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
solver.InitialConditions.scalar_advection_solution.Scalar = phi_wall
solver.InitialConditions.heat_conduction_solution.Temperature = phi_wall

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = 1.e-5
physics.reference_velocity = u_in[0]
scalaradv.scalar_coefficient = 1.

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').options().set('value', u_in)
bc.add_constant_bc(region_name = 'region_bnd_fluid_solid', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_component_bc(region_name = 'bottom3', variable_name = 'Velocity', component = 1).options().set('value',  0.)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').options().set('value', 1.)
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').options().set('value', u_in)

# Boundary conditions for ScalarAdvection
bc = scalaradv.get_child('BoundaryConditions')
bc.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Temperature').options().set('value', phi_in)
bc_wall_temp = bc.create_bc_action(region_name = 'region_bnd_fluid_solid', builder_name = 'cf3.UFEM.BCHoldValue')
bc_wall_temp.set_tags(from_field_tag = 'heat_conduction_solution', to_field_tag = 'scalar_advection_solution', from_variable = 'Temperature', to_variable = 'Temperature')
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Temperature').options().set('value',  phi_in)
bc.add_constant_bc(region_name = 'bottom3', variable_name = 'Temperature').options().set('value',  phi_in)
bc.add_constant_bc(region_name = 'top', variable_name = 'Temperature').options().set('value', phi_in)

# Boundary conditions for HeatConduction
bc = heatcond.get_child('BoundaryConditions')
bc.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work
heat_coupling = bc.create_bc_action(region_name = 'region_bnd_fluid_solid', builder_name = 'cf3.UFEM.HeatCouplingFlux')
heat_coupling.options().set('gradient_region', mesh.access_component('topology/fluid'))
heat_coupling.options().set('temperature_field_tag', 'scalar_advection_solution')
bc.add_constant_bc(region_name = 'solid_bottom', variable_name = 'Temperature').options().set('value',  phi_wall)

# Time setup
time = model.create_time()
time.options().set('time_step', 0.01)

# Setup a time series write

final_end_time = 0.02
save_interval = 0.01
current_end_time = 0.
iteration = 0.
solver.TimeLoop.CouplingIteration.options.max_iter = 10
solver.create_fields()
solver.InitialConditions.execute()
domain.write_mesh(cf.URI('atest-conjugate-heat-flatplate_output-initial.pvtu'))
while current_end_time < final_end_time:
  current_end_time += save_interval
  time.options().set('end_time', current_end_time)
  model.simulate()
  domain.write_mesh(cf.URI('atest-cht-flatplate_10-iterations-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
