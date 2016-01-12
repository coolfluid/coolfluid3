import coolfluid as cf
from math import pi,sqrt
import numpy as np
import pylab as pl

# Flow properties
h = 1.
nu = 0.0001
re_tau = 50.
u_tau = re_tau * nu / h
a_tau = u_tau**2 / (2.*h)
Uc = u_tau**2*h/(2.*nu)

# Boundary and initial conditions
u_wall = [0., 0.]
k_init = 0.
k_wall = 0.
e_init = 0.
e_wall = 0.

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

# Add the yplus computation
#yplus = solver.add_unsteady_solver('cf3.solver.actions.YPlus')
# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
nstokes.enable_body_force = True

# Add the k-epsilon turbulence model solver(ke)
# ke = solver.add_unsteady_solver('cf3.UFEM.KEpsilon')
# ke.options.d0 = 0.
# ke.options.l_max = 200000.*h
# ke.options.minimal_viscosity_ratio = 1e-4
# ke.options.theta = 1.
#ke.options.supg_type = 'cf2'
#ke.options.u_ref = 0.3

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
y_graded = xi#h/b * np.tanh(xi*np.arctanh(b))

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

#yplus.mesh = mesh

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
#yplus.regions = [mesh.topology.bottom.uri(), mesh.topology.top.uri()]
nstokes.regions = [mesh.topology.uri()]
# ke.regions = [mesh.topology.uri()]

#initial conditions
# solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = ['{Uc}/({h}*{h})*({h} - y)*({h} + y)'.format(h = h, Uc = Uc), '0']

# ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_k')
# ic_k.variable_name = 'k'
# ic_k.value = [str(k_init)]
# ic_k.regions = [mesh.topology.uri()]

# ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_epsilon')
# ic_epsilon.variable_name = 'epsilon'
# ic_epsilon.value = [str(e_init)]
# ic_epsilon.regions = [mesh.topology.uri()]

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0']

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Compute the wall distance
# make_boundary_global.execute()
# wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
# wall_distance.mesh = mesh
# wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
# wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
# bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
# bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall
bc.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit').tau_wall = 2.*u_tau**2
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit').tau_wall = 2.*u_tau**2
bc.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 0.

# Boundary conditions for k
# bc = ke.K.BoundaryConditions
# bc.add_constant_bc(region_name = 'bottom', variable_name = 'k').value =  k_wall
# bc.add_constant_bc(region_name = 'top', variable_name = 'k').value =  k_wall

# Boundary conditions for epsilon
# bc = ke.Epsilon.BoundaryConditions
# bc.add_constant_bc(region_name = 'bottom', variable_name = 'epsilon').value =  e_wall
# bc.add_constant_bc(region_name = 'top', variable_name = 'epsilon').value =  e_wall

# Setup a time series write
# write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
# write_manager.interval = 10
# writer = write_manager.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('atest-channel-standardkepsilon-{iteration}.vtm')

# Time setup
time = model.create_time()
time.time_step = 1.
time.end_time = 500.

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
pl.plot(y, Uc/(h*h)*(h**2 - y**2))

pl.show()

print Uc
print u_tau**2
print 2.*nu*Uc/h
