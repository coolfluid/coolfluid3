import coolfluid as cf
from math import pi,sqrt

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
y_segs = 64
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
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
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

gr = 0.1
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

ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_k.variable_name = 'k'
ic_k.value = [str(k0)]
ic_k.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
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
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit').options.theta = nstokes.options.theta
bc.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.

bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'k').value = k0

bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'epsilon').value = e0
bc.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCWallEpsilon').options.theta = ke.options.theta

# Time setup
time = model.create_time()
time.time_step = 0.001
time.end_time = 1.

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

try:
    import numpy as np
    import pylab as pl
    import os
    if os.environ.get('NOPLOT', '0') != '0':
        raise Exception("no plots wanted")

    coords = np.array(mesh.geometry.coordinates)
    ns_sol = np.array(mesh.geometry.navier_stokes_solution)
    line = np.abs(coords[:,0])>(60*h-1e-6)
    u = ns_sol[line, 0]
    y = coords[line, 1]
    k = np.array(mesh.geometry.ke_solution)[line, 0]
    epsilon = np.array(mesh.geometry.ke_solution)[line, 1]
    eff_visc = np.array(mesh.geometry.navier_stokes_viscosity)[line, 0]
    #Reference for the plots: Stabilized finite element method for heat transfer and turbulent flows inside industrial furnaces, PhD thesis, Elie Hachem: https://tel.archives-ouvertes.fr/tel-00443532/en/
    pl.figure()
    pl.plot(y, u, label='cf3')
    y_ref = [0., 0.010377, 0.020851, 0.028998, 0.037144, 0.043448, 0.049849, 0.054698, 0.059644, 0.063427, 0.067209, 0.070119, 0.073125, 0.075453, 0.07778, 0.079526, 0.081175, 0.082629, 0.083987, 0.085054, 0.086121]
    u_ref = [10.2904, 10.2569, 10.1567, 10.0443, 9.8828, 9.7423, 9.5809, 9.4306, 9.2565, 9.1007, 8.9266, 8.7764, 8.5927, 8.4352, 8.2404, 8.0662, 7.8825, 7.6934, 7.4631, 7.2572, 7.0013]
    pl.plot(y_ref, u_ref, label = 'ref')
    pl.legend(loc='lower left')
    pl.grid()
    pl.xlabel('y (m)')
    pl.ylabel('u (m/s)')

    pl.figure()
    pl.plot(y, k, label='cf3')
    y_ref = [0., 0.010451, 0.020916, 0.029044, 0.037126, 0.043385, 0.049785, 0.054643, 0.059548, 0.063331, 0.067115, 0.070011, 0.073047, 0.075243, 0.077578, 0.0794, 0.081128, 0.082576, 0.08393, 0.08491, 0.086031]
    k_ref = [0.20025, 0.21055, 0.23838, 0.2695, 0.30462, 0.33345, 0.36322, 0.38641, 0.40948, 0.42703, 0.44526, 0.45933, 0.47391, 0.48562, 0.49788, 0.50859, 0.51886, 0.52879, 0.5405, 0.5511, 0.566]
    pl.plot(y_ref, k_ref, label = 'ref')
    pl.legend(loc='upper left')
    pl.grid()
    pl.xlabel('y (m)')
    pl.ylabel('k (m^2/s^2)')


    pl.figure()
    pl.plot(y, epsilon, label='cf3')
    y_ref = [0.0, 0.010447, 0.020895, 0.029042, 0.037188, 0.043514, 0.04984, 0.054728, 0.059425, 0.06345, 0.067188, 0.07016, 0.073035, 0.075335, 0.077636, 0.079457, 0.081182, 0.08262, 0.083866, 0.085016]
    e_ref = [0.747, 0.839, 1.195, 1.551, 2.083, 2.566, 3.184, 3.803, 4.685, 5.523, 6.615, 7.716, 9.301, 10.974, 13.351, 15.704, 19.398, 23.359, 29.521, 36.278]
    pl.plot(y_ref, e_ref, label = 'ref')
    pl.legend(loc='upper left')
    pl.grid()
    pl.xlabel('y (m)')
    pl.ylabel('epsilon (m^2/s^3)')

    pl.show()
except:
    print('Skipping plot due to python errors')
