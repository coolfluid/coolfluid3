import coolfluid as cf
from math import pi
import numpy as np
import pylab as pl

# Flow properties
h = 1.
nu = 0.0001
re_tau = 180.
u_tau = re_tau * nu / h
a_tau = re_tau**2*nu**2/h**3

y_segs = 32
x_size = 4.*pi*h
x_segs = 32
ungraded_h = float(y_segs)

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the yplus computation
yplus = solver.add_unsteady_solver('cf3.solver.actions.YPlus')
# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the Spalat-Allmaras turbulence model
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., ungraded_h]
points[3]  = [x_size, ungraded_h]
points[4]  = [0.,2.*ungraded_h]
points[5]  = [x_size, 2.*ungraded_h]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]
block_subdivs[1] = block_subdivs[0]

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

#blocks.partition_blocks(nb_partitions = 4, direction = 0)

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

coordmap = {}
b = 0.9544
xi = np.linspace(-h, h, y_segs*2+1)
y_graded = h/b * np.tanh(xi*np.arctanh(b))

coords = mesh.geometry.coordinates
for i in range(len(coords)):
  y_key = int(coords[i][1])
  coords[i][1] = y_graded[y_key]

# Make the boundary global, to allow wall distance and periodics to work correctly
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

link_horizontal = domain.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
link_horizontal.mesh = mesh
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-x_size, 0.]
link_horizontal.execute()

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

yplus.mesh = mesh

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
yplus.regions = [mesh.topology.bottom.uri(), mesh.topology.top.uri()]
nstokes.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

u_wall = [0., 0.]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.spalart_allmaras_solution.SAViscosity = 0.1
solver.InitialConditions.density_ratio.density_ratio = 1.

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0']

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Compute the wall distance
make_boundary_global.execute()
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall

bc = satm.children.BoundaryConditions
bc.add_constant_bc(region_name = 'bottom', variable_name = 'SAViscosity').value = 0.
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').value = 0.

# Setup a time series write
write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 10
writer = write_manager.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
writer.mesh = mesh
writer.file = cf.URI('atest-channel-spalart-allmaras-{iteration}.vtm')

# Time setup
time = model.create_time()
time.time_step = 10.
time.end_time = 3000.

# Run the simulation
model.simulate()

# Plot simulation velocity
coords = np.array(mesh.geometry.coordinates)
ns_sol = np.array(mesh.geometry.navier_stokes_solution)
line = np.abs(coords[:,0])<1e-6
pl.plot(coords[line, 1], ns_sol[line, 0])

# Plot MKM velocity
y_mkm = np.array([0.0, 0.00030118, 0.0012045, 0.0027095, 0.0048153, 0.0075205, 0.010823, 0.014722, 0.019215, 0.024298, 0.029969, 0.036224, 0.04306, 0.050472, 0.058456, 0.067007, 0.07612, 0.08579, 0.096011, 0.10678, 0.11808, 0.12991, 0.14227, 0.15515, 0.16853, 0.18242, 0.19679, 0.21165, 0.22699, 0.24279, 0.25905, 0.27575, 0.29289, 0.31046, 0.32844, 0.34683, 0.36561, 0.38477, 0.4043, 0.42419, 0.44443, 0.465, 0.4859, 0.5071, 0.5286, 0.55039, 0.57244, 0.59476, 0.61732, 0.6401, 0.66311, 0.68632, 0.70972, 0.73329, 0.75702, 0.7809, 0.80491, 0.82904, 0.85327, 0.87759, 0.90198, 0.92644, 0.95093, 0.97546, 1.0])
u_mkm = np.array([0.0, 0.053639, 0.21443, 0.48197, 0.85555, 1.3339, 1.9148, 2.5939, 3.3632, 4.2095, 5.1133, 6.0493, 6.9892, 7.9052, 8.7741, 9.579, 10.311, 10.967, 11.55, 12.066, 12.52, 12.921, 13.276, 13.59, 13.87, 14.121, 14.349, 14.557, 14.75, 14.931, 15.101, 15.264, 15.419, 15.569, 15.714, 15.855, 15.993, 16.128, 16.26, 16.389, 16.515, 16.637, 16.756, 16.872, 16.985, 17.094, 17.2, 17.302, 17.4, 17.494, 17.585, 17.672, 17.756, 17.835, 17.911, 17.981, 18.045, 18.103, 18.154, 18.198, 18.235, 18.264, 18.285, 18.297, 18.301])
pl.plot(y_mkm-1., u_mkm*u_tau)

pl.show()

model.print_timing_tree()
