import coolfluid as cf
from math import pi,sqrt
import numpy as np
import pylab as pl

# Flow properties
h = 0.09
nu = 1.5e-5
rho = 1.208
u0 = 10.5
u_avg = 9.5
delta = 0.004

k0 = 1.805
e0 = 242.5

velocity_profile_string = '{U0}-3*({U0}-{Uavg})*(y/({H}-{delta}))^2'.format(U0=u0, Uavg=u_avg, H=h, delta = delta)

# Mesh properties
y_segs = 16
x_size = 60.*h
x_segs = 8

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
nstokes.enable_body_force = False
nstokes.options.theta = 1.

# Add the k-epsilon turbulence model solver(ke)
ke = solver.add_unsteady_solver('cf3.UFEM.StandardKEpsilon')
ke.options.theta = 1.

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., h-delta]
points[3]  = [x_size, h-delta]
# points[4]  = [0.,-h+delta]
# points[5]  = [x_size, -h+delta]

block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 3, 2]
# block_nodes[1] = [4, 5, 1, 0]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]
# block_subdivs[1] = [x_segs, y_segs]

gr = 0.25
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., gr, gr]
# gradings[1] = [1., 1., 1./gr, 1./gr]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)
left_patch[0] = [2, 0]
# left_patch[1] = [0, 4]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [3, 2]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)
right_patch[0] = [1, 3]
# right_patch[1] = [5, 1]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# Make the boundary global, to allow wall distance and periodics to work correctly
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.top]
wall_distance.execute()

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
ke.regions = [mesh.topology.uri()]

#initial conditions
# solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = [velocity_profile_string, '0']

ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_k')
ic_k.variable_name = 'k'
ic_k.value = [str(k0)]
ic_k.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_epsilon')
ic_epsilon.variable_name = 'epsilon'
ic_epsilon.value = [str(e0)]
ic_epsilon.regions = [mesh.topology.uri()]

#properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = nu*rho

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_function_bc(region_name = 'left', variable_name = 'Velocity').value = [velocity_profile_string, '0']
bc.add_constant_component_bc(region_name = 'bottom', variable_name = 'Velocity', component = 1).options().set('value',  0.)
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
bc.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.

bc = ke.K.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'k').value = k0

bc = ke.Epsilon.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'epsilon').value = e0
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallEpsilon')

# Time setup
time = model.create_time()
time.time_step = 0.001
time.end_time = 1.5

probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'EffectiveViscosity']
probe0.coordinate = [x_size, 0.]
probe0.History.file = cf.URI('atest-comte-bellot-probe.tsv')

# Run the simulation
model.simulate()

# writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('atest-comte-bellot.vtm')
# writer.execute()

# Plot simulation velocity
coords = np.array(mesh.geometry.coordinates)
ns_sol = np.array(mesh.geometry.navier_stokes_solution)
line = np.abs(coords[:,0])>(60*h-1e-6)
u = ns_sol[line, 0]
y = coords[line, 1]
k = np.array(mesh.geometry.ke_k)[line, 0]
epsilon = np.array(mesh.geometry.ke_epsilon)[line, 0]
eff_visc = np.array(mesh.geometry.navier_stokes_viscosity)[line, 0]

pl.figure()
pl.plot(y, u)
pl.title('u')

pl.figure()
pl.plot(y, k)
pl.plot(y, k, 'ko')
pl.title('k')

pl.figure()
pl.plot(y, epsilon)
pl.title('epsilon')

pl.figure()
pl.plot(y, eff_visc-nu)
pl.title('nu_t')

pl.show()
