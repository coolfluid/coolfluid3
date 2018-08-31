import coolfluid as cf
from math import pi
import numpy as np

# cf.Core.wait_for_debugger(0)

# Flow properties
h = 1.
nu = 0.0001
re_tau = 395.
u_tau = re_tau * nu / h
a_tau = u_tau**2 / h
Uc = u_tau**2*h/nu

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
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the yplus computation
yplus = solver.add_unsteady_solver('cf3.solver.actions.YPlus')
# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the k-epsilon turbulence model solver(ke)
ke = solver.add_unsteady_solver('cf3.UFEM.ChienKEpsilon')
ke.options.theta = 1.
ke.options.l_max = 500.*h

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

blocks.periodic_x = ["left", "right"]

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

# Make the boundary global, to allow wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
create_point_region.coordinates = [x_size/2., 0.]  #[x_size/2., h, z_size/2.]
create_point_region.region_name = 'center'
create_point_region.mesh = mesh
create_point_region.execute()

yplus.mesh = mesh

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
yplus.regions = [mesh.topology.bottom.uri(), mesh.topology.top.uri()]
nstokes.regions = [mesh.topology.uri()]
ke.regions = [mesh.topology.uri()]

for lss in [nstokes.LSS, ke.LSS.LSS]:
    lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 2
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.density_ratio.density_ratio = 1.
# ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
# ic_u.variable_name = 'Velocity'
# ic_u.regions = [mesh.topology.uri()]
# ic_u.value = ['{Uc}/({h}*{h})*({h} - y)*({h} + y)'.format(h = h, Uc = Uc), '0']

ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_k.variable_name = 'k'
ic_k.value = [str(k_init)]
ic_k.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
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

# Compute the wall distance
make_boundary_global.execute()
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Needed for yplus to work
conn = domain.create_component("SurfaceToVolumeConnectivity", "cf3.mesh.actions.SurfaceToVolumeConnectivity")
conn.mesh = mesh
conn.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 0.

# Boundary conditions for k
bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'bottom', variable_name = 'k').value =  k_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'k').value =  k_wall

# Boundary conditions for epsilon
bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'bottom', variable_name = 'epsilon').value =  e_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'epsilon').value =  e_wall

# Setup a time series write
# write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
# write_manager.interval = 1
# writer = write_manager.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('atest-channel-kepsilon-{iteration}.vtm')

# Time setup
time = model.create_time()
time.time_step = 10.
time.end_time = 3000.

probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'EffectiveViscosity']
probe0.coordinate = [x_size/2., 0.]
probe0.History.file = cf.URI('atest-channel-kepsilon-probe.tsv')

# set up coprocessing
# coproc = solver.add_unsteady_solver('cf3.vtk.LiveCoProcessor')
# coproc.cf3_to_vtk = coproc.create_component('CF3ToVTK', 'cf3.vtk.CF3ToVTK')
# coproc.cf3_to_vtk.mesh = mesh

# Run the simulation
model.simulate()
model.print_timing_tree()

# Plot simulation velocity
try:
    import pylab as pl
    import os
    if cf.Core.rank() == 0 and os.environ.get('NOPLOT', '0') == '0':
        coords = np.array(mesh.geometry.coordinates)
        ns_sol = np.array(mesh.geometry.navier_stokes_solution)
        yplus_arr = np.array(mesh.geometry.yplus)
        eff_visc = np.array(mesh.geometry.navier_stokes_viscosity)
        line = np.abs(coords[:,0])<1e-6
        u = ns_sol[line, 0]
        y = coords[line, 1]
        pl.figure()
        pl.plot(y, u, 'o')
        u_tau_num = np.sqrt(nu*(u[1]-u[0])/(y[1]-y[0]))
        yplus_cen = u_tau_num*h/nu


        # Plot MKM velocity
        y_mkm = np.array([0.0, 7.5298e-05, 0.00030118, 0.00067762, 0.0012045, 0.0018819, 0.0027095, 0.0036874, 0.0048153, 0.006093, 0.0075205, 0.0090974, 0.010823, 0.012699, 0.014722, 0.016895, 0.019215, 0.021683, 0.024298, 0.02706, 0.029969, 0.033024, 0.036224, 0.039569, 0.04306, 0.046694, 0.050472, 0.054393, 0.058456, 0.062661, 0.067007, 0.071494, 0.07612, 0.080886, 0.08579, 0.090832, 0.096011, 0.10133, 0.10678, 0.11236, 0.11808, 0.12393, 0.12991, 0.13603, 0.14227, 0.14864, 0.15515, 0.16178, 0.16853, 0.17541, 0.18242, 0.18954, 0.19679, 0.20416, 0.21165, 0.21926, 0.22699, 0.23483, 0.24279, 0.25086, 0.25905, 0.26735, 0.27575, 0.28427, 0.29289, 0.30162, 0.31046, 0.3194, 0.32844, 0.33758, 0.34683, 0.35617, 0.36561, 0.37514, 0.38477, 0.39449, 0.4043, 0.4142, 0.42419, 0.43427, 0.44443, 0.45467, 0.465, 0.47541, 0.4859, 0.49646, 0.5071, 0.51782, 0.5286, 0.53946, 0.55039, 0.56138, 0.57244, 0.58357, 0.59476, 0.60601, 0.61732, 0.62868, 0.6401, 0.65158, 0.66311, 0.67469, 0.68632, 0.69799, 0.70972, 0.72148, 0.73329, 0.74513, 0.75702, 0.76894, 0.7809, 0.79289, 0.80491, 0.81696, 0.82904, 0.84114, 0.85327, 0.86542, 0.87759, 0.88978, 0.90198, 0.9142, 0.92644, 0.93868, 0.95093, 0.96319, 0.97546, 0.98773, 1.0])
        u_mkm = np.array([0.0, 0.029538, 0.11811, 0.26562, 0.47198, 0.73701, 1.0605, 1.4417, 1.8799, 2.3729, 2.9177, 3.5093, 4.1409, 4.8032, 5.4854, 6.1754, 6.8611, 7.5309, 8.1754, 8.787, 9.3607, 9.8937, 10.385, 10.836, 11.248, 11.624, 11.966, 12.278, 12.563, 12.822, 13.06, 13.278, 13.479, 13.664, 13.837, 13.998, 14.148, 14.29, 14.425, 14.552, 14.673, 14.79, 14.902, 15.011, 15.117, 15.221, 15.322, 15.421, 15.518, 15.614, 15.707, 15.799, 15.89, 15.979, 16.067, 16.153, 16.239, 16.324, 16.409, 16.493, 16.576, 16.659, 16.741, 16.823, 16.903, 16.984, 17.063, 17.141, 17.218, 17.294, 17.369, 17.443, 17.517, 17.59, 17.664, 17.738, 17.812, 17.886, 17.96, 18.034, 18.108, 18.182, 18.254, 18.326, 18.396, 18.466, 18.535, 18.603, 18.669, 18.734, 18.797, 18.859, 18.919, 18.978, 19.035, 19.092, 19.148, 19.202, 19.256, 19.308, 19.359, 19.408, 19.456, 19.503, 19.548, 19.593, 19.636, 19.678, 19.719, 19.758, 19.796, 19.832, 19.865, 19.897, 19.927, 19.955, 19.981, 20.004, 20.026, 20.046, 20.064, 20.08, 20.094, 20.106, 20.116, 20.123, 20.129, 20.132, 20.133])
        pl.plot(y_mkm-1., u_mkm*u_tau)

        pl.show()
except:
    print('Skipping plot due to python errors')
