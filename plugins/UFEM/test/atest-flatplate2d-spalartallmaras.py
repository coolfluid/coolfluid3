import sys
#sys.path.append('/data/scholl/coolfluid3/build/dso')
#sys.path.append('/home/sebastian/coolfluid3/build/dso')
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global configuration
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
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the SpalartAllmaras turbulence model solver(satm)
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

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
inlet_patch[0] = [10, 9]
inlet_patch[1] = [11, 10]

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
top_patch[1] = [8, 5]
top_patch[2] = [4, 11]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

u_in = [1., 0.]
u_wall = [0., 0.]
NU_in = 0.001
NU_wall = 0.

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.spalart_allmaras_solution.SAViscosity = NU_in

#properties for Navier-Stokes
physics.density = 1.2
physics.dynamic_viscosity = 1.7894e-5

# Compute the wall distance
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom1, mesh.topology.bottom2]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').options().set('value', u_in)
bc.add_constant_bc(region_name = 'bottom1', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'Velocity').options().set('value',  u_wall)
bc.add_constant_component_bc(region_name = 'bottom3', variable_name = 'Velocity', component = 1).options().set('value',  0.)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').options().set('value', 1.)
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').options().set('value', u_in)

# Boundary conditions for Spalart-Allmaras
bc = satm.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'SAViscosity').options().set('value', NU_in)
bc.add_constant_bc(region_name = 'bottom1', variable_name = 'SAViscosity').options().set('value',  NU_wall)
bc.add_constant_bc(region_name = 'bottom2', variable_name = 'SAViscosity').options().set('value',  NU_wall)
bc.add_constant_bc(region_name = 'bottom3', variable_name = 'SAViscosity').options().set('value',  NU_in)
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').options().set('value', NU_in)

write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 1
writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/spalart_allmaras_solution')]
writer.file = cf.URI('atest-flatplate2d-spalartallmaras-{iteration}.pvtu')

# Time setup
time = model.create_time()
time.time_step = 0.1
time.end_time = 1.0

model.simulate()

# print timings
model.print_timing_tree()
