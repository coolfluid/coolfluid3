import coolfluid as cf
from math import pi,sqrt
import numpy as np
import pylab as pl

# Flow properties
h = 1.
delta_in = 0.015
nu = 0.0001
re_tau = 587.19
u_tau = re_tau * nu / h
a_tau = u_tau**2 / (2.*h)
Uc = u_tau**2*h/(2.*nu)

# Boundary and initial conditions
u_wall = [0., 0.]
k_init = 0.
e_init = 0.

# Mesh properties
y_segs = 32
x_size = 4.*pi*h
x_segs = 4
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

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
nstokes.enable_body_force = True
nstokes.options.theta = 1.

# Add the k-epsilon turbulence model solver(ke)
ke = solver.add_unsteady_solver('cf3.UFEM.StandardKEpsilon')
ke.options.theta = 1.

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
xi = np.linspace(-(h-delta_in), (h-delta_in), y_segs*2+1)
y_graded = (h-delta_in)/b * np.tanh(xi*np.arctanh(b))

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

create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
create_point_region.coordinates = [x_size/2., 0.]  #[x_size/2., h, z_size/2.]
create_point_region.region_name = 'center'
create_point_region.mesh = mesh
create_point_region.execute()

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
ke.regions = [mesh.topology.uri()]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
# ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
# ic_u.variable_name = 'Velocity'
# ic_u.regions = [mesh.topology.uri()]
# ic_u.value = ['{Uc}/({h}*{h})*({h} - y)*({h} + y)'.format(h = h, Uc = Uc), '0']

ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_k')
ic_k.variable_name = 'k'
ic_k.value = [str(k_init)]
ic_k.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_epsilon')
ic_epsilon.variable_name = 'epsilon'
ic_epsilon.value = [str(e_init)]
ic_epsilon.regions = [mesh.topology.uri()]

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0']

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
# bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
# bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall
bc.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
bc.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 0.

#
# bc = ke.K.BoundaryConditions
# bc.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.BCWallK')
# bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallK')

# Boundary conditions for epsilon
bc = ke.Epsilon.BoundaryConditions
bc.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.BCWallEpsilon')
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallEpsilon')

# Setup a time series write
# write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
# write_manager.interval = 10
# writer = write_manager.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('atest-channel-standardkepsilon-{iteration}.vtm')

# Time setup
time = model.create_time()
time.time_step = 1.
time.end_time = 3000.

probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'EffectiveViscosity']
probe0.coordinate = [x_size/2., 0.]
probe0.History.file = cf.URI('atest-channel-standardkepsilon-probe.tsv')

# Run the simulation
model.simulate()

# Plot simulation velocity
coords = np.array(mesh.geometry.coordinates)
ns_sol = np.array(mesh.geometry.navier_stokes_solution)
line = np.abs(coords[:,0])<1e-6
u = ns_sol[line, 0]
y = coords[line, 1]
pl.figure()
pl.plot(y, u)

#y_mkm = np.array([0.0, 0.00030118, 0.0012045, 0.0027095, 0.0048153, 0.0075205, 0.010823, 0.014722, 0.019215, 0.024298, 0.029969, 0.036224, 0.04306, 0.050472, 0.058456, 0.067007, 0.07612, 0.08579, 0.096011, 0.10678, 0.11808, 0.12991, 0.14227, 0.15515, 0.16853, 0.18242, 0.19679, 0.21165, 0.22699, 0.24279, 0.25905, 0.27575, 0.29289, 0.31046, 0.32844, 0.34683, 0.36561, 0.38477, 0.4043, 0.42419, 0.44443, 0.465, 0.4859, 0.5071, 0.5286, 0.55039, 0.57244, 0.59476, 0.61732, 0.6401, 0.66311, 0.68632, 0.70972, 0.73329, 0.75702, 0.7809, 0.80491, 0.82904, 0.85327, 0.87759, 0.90198, 0.92644, 0.95093, 0.97546, 1.0])
#u_mkm = np.array([0.0, 0.053639, 0.21443, 0.48197, 0.85555, 1.3339, 1.9148, 2.5939, 3.3632, 4.2095, 5.1133, 6.0493, 6.9892, 7.9052, 8.7741, 9.579, 10.311, 10.967, 11.55, 12.066, 12.52, 12.921, 13.276, 13.59, 13.87, 14.121, 14.349, 14.557, 14.75, 14.931, 15.101, 15.264, 15.419, 15.569, 15.714, 15.855, 15.993, 16.128, 16.26, 16.389, 16.515, 16.637, 16.756, 16.872, 16.985, 17.094, 17.2, 17.302, 17.4, 17.494, 17.585, 17.672, 17.756, 17.835, 17.911, 17.981, 18.045, 18.103, 18.154, 18.198, 18.235, 18.264, 18.285, 18.297, 18.301])
#y_mkm = np.array([0.0, 7.5298e-05, 0.00030118, 0.00067762, 0.0012045, 0.0018819, 0.0027095, 0.0036874, 0.0048153, 0.006093, 0.0075205, 0.0090974, 0.010823, 0.012699, 0.014722, 0.016895, 0.019215, 0.021683, 0.024298, 0.02706, 0.029969, 0.033024, 0.036224, 0.039569, 0.04306, 0.046694, 0.050472, 0.054393, 0.058456, 0.062661, 0.067007, 0.071494, 0.07612, 0.080886, 0.08579, 0.090832, 0.096011, 0.10133, 0.10678, 0.11236, 0.11808, 0.12393, 0.12991, 0.13603, 0.14227, 0.14864, 0.15515, 0.16178, 0.16853, 0.17541, 0.18242, 0.18954, 0.19679, 0.20416, 0.21165, 0.21926, 0.22699, 0.23483, 0.24279, 0.25086, 0.25905, 0.26735, 0.27575, 0.28427, 0.29289, 0.30162, 0.31046, 0.3194, 0.32844, 0.33758, 0.34683, 0.35617, 0.36561, 0.37514, 0.38477, 0.39449, 0.4043, 0.4142, 0.42419, 0.43427, 0.44443, 0.45467, 0.465, 0.47541, 0.4859, 0.49646, 0.5071, 0.51782, 0.5286, 0.53946, 0.55039, 0.56138, 0.57244, 0.58357, 0.59476, 0.60601, 0.61732, 0.62868, 0.6401, 0.65158, 0.66311, 0.67469, 0.68632, 0.69799, 0.70972, 0.72148, 0.73329, 0.74513, 0.75702, 0.76894, 0.7809, 0.79289, 0.80491, 0.81696, 0.82904, 0.84114, 0.85327, 0.86542, 0.87759, 0.88978, 0.90198, 0.9142, 0.92644, 0.93868, 0.95093, 0.96319, 0.97546, 0.98773, 1.0])
#u_mkm = np.array([0.0, 0.029538, 0.11811, 0.26562, 0.47198, 0.73701, 1.0605, 1.4417, 1.8799, 2.3729, 2.9177, 3.5093, 4.1409, 4.8032, 5.4854, 6.1754, 6.8611, 7.5309, 8.1754, 8.787, 9.3607, 9.8937, 10.385, 10.836, 11.248, 11.624, 11.966, 12.278, 12.563, 12.822, 13.06, 13.278, 13.479, 13.664, 13.837, 13.998, 14.148, 14.29, 14.425, 14.552, 14.673, 14.79, 14.902, 15.011, 15.117, 15.221, 15.322, 15.421, 15.518, 15.614, 15.707, 15.799, 15.89, 15.979, 16.067, 16.153, 16.239, 16.324, 16.409, 16.493, 16.576, 16.659, 16.741, 16.823, 16.903, 16.984, 17.063, 17.141, 17.218, 17.294, 17.369, 17.443, 17.517, 17.59, 17.664, 17.738, 17.812, 17.886, 17.96, 18.034, 18.108, 18.182, 18.254, 18.326, 18.396, 18.466, 18.535, 18.603, 18.669, 18.734, 18.797, 18.859, 18.919, 18.978, 19.035, 19.092, 19.148, 19.202, 19.256, 19.308, 19.359, 19.408, 19.456, 19.503, 19.548, 19.593, 19.636, 19.678, 19.719, 19.758, 19.796, 19.832, 19.865, 19.897, 19.927, 19.955, 19.981, 20.004, 20.026, 20.046, 20.064, 20.08, 20.094, 20.106, 20.116, 20.123, 20.129, 20.132, 20.133])
y_mkm = np.array([0.0, 7.5298e-05, 0.00030118, 0.00067762, 0.0012045, 0.0018819, 0.0027095, 0.0036874, 0.0048153, 0.006093, 0.0075205, 0.0090974, 0.010823, 0.012699, 0.014722, 0.016895, 0.019215, 0.021683, 0.024298, 0.02706, 0.029969, 0.033024, 0.036224, 0.039569, 0.04306, 0.046694, 0.050472, 0.054393, 0.058456, 0.062661, 0.067007, 0.071494, 0.07612, 0.080886, 0.08579, 0.090832, 0.096011, 0.10133, 0.10678, 0.11236, 0.11808, 0.12393, 0.12991, 0.13603, 0.14227, 0.14864, 0.15515, 0.16178, 0.16853, 0.17541, 0.18242, 0.18954, 0.19679, 0.20416, 0.21165, 0.21926, 0.22699, 0.23483, 0.24279, 0.25086, 0.25905, 0.26735, 0.27575, 0.28427, 0.29289, 0.30162, 0.31046, 0.3194, 0.32844, 0.33758, 0.34683, 0.35617, 0.36561, 0.37514, 0.38477, 0.39449, 0.4043, 0.4142, 0.42419, 0.43427, 0.44443, 0.45467, 0.465, 0.47541, 0.4859, 0.49646, 0.5071, 0.51782, 0.5286, 0.53946, 0.55039, 0.56138, 0.57244, 0.58357, 0.59476, 0.60601, 0.61732, 0.62868, 0.6401, 0.65158, 0.66311, 0.67469, 0.68632, 0.69799, 0.70972, 0.72148, 0.73329, 0.74513, 0.75702, 0.76894, 0.7809, 0.79289, 0.80491, 0.81696, 0.82904, 0.84114, 0.85327, 0.86542, 0.87759, 0.88978, 0.90198, 0.9142, 0.92644, 0.93868, 0.95093, 0.96319, 0.97546, 0.98773, 1.0])
u_mkm = np.array([0.0, 0.044231, 0.17699, 0.39816, 0.7075, 1.1046, 1.5886, 2.1573, 2.8061, 3.5272, 4.3078, 5.1307, 5.9748, 6.8174, 7.6375, 8.4177, 9.1455, 9.8142, 10.421, 10.968, 11.458, 11.895, 12.287, 12.636, 12.95, 13.233, 13.489, 13.722, 13.935, 14.131, 14.313, 14.482, 14.64, 14.789, 14.931, 15.066, 15.196, 15.321, 15.442, 15.56, 15.674, 15.786, 15.896, 16.003, 16.11, 16.214, 16.317, 16.418, 16.518, 16.616, 16.713, 16.808, 16.902, 16.995, 17.087, 17.179, 17.269, 17.358, 17.446, 17.533, 17.62, 17.705, 17.789, 17.873, 17.956, 18.038, 18.12, 18.202, 18.282, 18.362, 18.441, 18.52, 18.598, 18.676, 18.754, 18.831, 18.907, 18.982, 19.056, 19.128, 19.199, 19.269, 19.338, 19.406, 19.473, 19.539, 19.604, 19.668, 19.731, 19.794, 19.855, 19.916, 19.976, 20.035, 20.093, 20.149, 20.204, 20.258, 20.311, 20.364, 20.415, 20.464, 20.513, 20.561, 20.608, 20.653, 20.698, 20.741, 20.784, 20.826, 20.866, 20.905, 20.943, 20.979, 21.013, 21.046, 21.076, 21.105, 21.131, 21.156, 21.178, 21.198, 21.215, 21.23, 21.242, 21.251, 21.258, 21.262, 21.263])
pl.plot(y_mkm-1., u_mkm*u_tau)

pl.figure()
eff_visc = np.array(mesh.geometry.navier_stokes_viscosity)
pl.plot(y, eff_visc[line, 0])
pl.title('eff_visc')

pl.figure()
k_arr = np.array(mesh.geometry.ke_k)
pl.plot(y, k_arr[line, 0])
pl.title('k')

pl.figure()
eps_arr = np.array(mesh.geometry.ke_epsilon)
pl.plot(y, eps_arr[line, 0])
pl.title('epsilon')

u_tau_sim = u[0] / 11.06
print 'u_tau_sim', u_tau_sim
print 'delta', 11.06*nu/u_tau_sim

pl.show()
