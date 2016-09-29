import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.compressible.NavierStokes')

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

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

ns_solver.regions = [mesh.topology.uri()]

u_in = [2., 0.]

solver.create_fields()

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
solver.InitialConditions.navier_stokes_solution.Velocity = u_in

# Example of varying initial effective viscosity
ic_visc = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_viscosity')
ic_visc.variable_name = 'EffectiveViscosity'
ic_visc.value = ['0.1 + 0.01*sin(2/pi*x)']
ic_visc.regions = [mesh.topology.uri()]
ic_visc.execute()

# Check initial conditions
domain.write_mesh(cf.URI('laminar-channel-2d_output-init.pvtu'))

# Physical constants
physics.density = 1000.
physics.dynamic_viscosity = 0.1

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.

# Setup a time series write
write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 1 # Saves every timestep
writer = write_manager.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
writer.mesh = mesh
writer.file = cf.URI('out-channel-{iteration}.vtm')

# Time setup
time = model.create_time()
time.time_step = 0.1
time.end_time = 1.

# Run the simulation
model.simulate()

# print timings
model.print_timing_tree()
