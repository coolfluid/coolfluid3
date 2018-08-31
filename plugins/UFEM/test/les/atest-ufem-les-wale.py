import coolfluid as cf
from math import pi
import numpy as np

# Flow properties
h = 1.
nu = 0.0001
re_tau = 180.
u_tau = re_tau * nu / h
a_tau = re_tau**2*nu**2/h**3
Uc = a_tau/nu*(h**2/2.)
u_ref = 0.5*Uc

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4
env.only_cpu0_writes = True

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the turbulence model
wale = ns_solver = solver.add_unsteady_solver('cf3.UFEM.les.WALE')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
ns_solver.options.theta = 0.5
ns_solver.options.nb_iterations = 1
ns_solver.enable_body_force = True
ns_solver.options.pressure_rcg_solve = True
ns_solver.VelocityLSS.Assembly.interval = 100

tstep = 0.025

y_segs = 32

x_size = 4.*pi*h
z_size = 4./3.*pi*h

x_segs = 32
z_segs = 32

ungraded_h = h#float(y_segs)

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
gradings[0] = [1., 1., 20., 20.]
gradings[1] = [1., 1., 1./20., 1./20.]

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

blocks.extrude_blocks(positions=[z_size], nb_segments=[z_segs], gradings=[1.])

nb_procs = cf.Core.nb_procs()





mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# coordmap = {}
# b = 0.955
# xi = np.linspace(-h, h, y_segs*2+1)
# y_graded = h/b * np.tanh(xi*np.arctanh(b))
# 
# for i in range(len(mesh.geometry.coordinates)):
#   if i % 100 == 0:
#   y_key = int(mesh.geometry.coordinates[i][1])
#   mesh.geometry.coordinates[i][1] = y_graded[y_key]

create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
create_point_region.coordinates = [x_size/2., h, z_size/2.]
create_point_region.region_name = 'center'
create_point_region.mesh = mesh
create_point_region.execute()

partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
partitioner.load_balance = False
partitioner.mesh = mesh

link_horizontal = partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-x_size, 0., 0.]

link_spanwise = partitioner.create_link_periodic_nodes()
link_spanwise.source_region = mesh.topology.back
link_spanwise.destination_region = mesh.topology.front
link_spanwise.translation_vector = [0., 0., -z_size]

partitioner.execute()

#domain.write_mesh(cf.URI('chan180-init.cf3mesh'))

# Physical constants
physics.density = 1.
physics.dynamic_viscosity = nu

wale.regions = [mesh.topology.uri()]
ns_solver.regions = [mesh.topology.uri()]

#lss = ns_solver.PressureLSS.LSS
#lss.SolutionStrategy.solver_type = 'Amesos_Mumps'

for strat in [ns_solver.children.FirstPressureStrategy, ns_solver.children.SecondPressureStrategy]:
  strat.MLParameters.aggregation_type = 'Uncoupled'
  strat.MLParameters.max_levels = 4
  strat.MLParameters.smoother_sweeps = 3
  strat.MLParameters.coarse_type = 'Amesos-KLU'
  strat.MLParameters.add_parameter(name = 'repartition: start level', value = 0)
  strat.MLParameters.add_parameter(name = 'repartition: max min ratio', value = 1.1)
  strat.MLParameters.add_parameter(name = 'aggregation: aux: enable', value = True)
  strat.MLParameters.add_parameter(name = 'aggregation: aux: threshold', value = 0.005)

lss = ns_solver.VelocityLSS.LSS
lss.SolutionStrategy.preconditioner_reset = 100
lss.SolutionStrategy.Parameters.preconditioner_type = 'Ifpack'
lss.SolutionStrategy.Parameters.PreconditionerTypes.Ifpack.overlap = 1
#lss.SolutionStrategy.Parameters.PreconditionerTypes.Ifpack.create_parameter_list('Ifpack Settings')
#lss.SolutionStrategy.Parameters.PreconditionerTypes.Ifpack.IfpackSettings.add_parameter(name = 'fact: level-of-fill', value = 1)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'PDE equations', value = 3)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.max_levels = 4
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 2
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.coarse_type = 'Amesos-KLU'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'repartition: start level', value = 0)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'repartition: max min ratio', value = 1.1)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'aggregation: aux: enable', value = True)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'aggregation: aux: threshold', value = 0.0005)

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block CG'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockCG.convergence_tolerance = 1e-6
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockCG.maximum_iterations = 300
#lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 100

# Initial conditions
ic_u = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = ['{Uc}/({h}*{h})*y*(2*{h} - y)'.format(h = h, Uc = Uc), '0', '0']
ic_g = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0', '0']

# Boundary conditions
bc_u = ns_solver.VelocityLSS.BC
bc_u.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0., 0.]
bc_u.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0., 0.]
# Pressure BC
ns_solver.PressureLSS.BC.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 0.

#statistics
stats = solver.add_unsteady_solver('cf3.solver.actions.TurbulenceStatistics')
stats.region = mesh.topology
stats.file = cf.URI('turbulence-statistics.txt')
stats.rolling_window = 1000
stats.add_probe([0., 0., 0. ])

solver.create_fields()
stats.setup()
#domain.write_mesh(cf.URI('chan180-mkmfields.cf3mesh'))

# Restarter
restart_writer = solver.add_restart_writer()
restart_writer.Writer.file = cf.URI('chan180-{iteration}.cf3restart')
restart_writer.interval = 1000

dir_avg = solver.TimeLoop.children.WriteRestartManager.create_component('DirectionalAverage', 'cf3.solver.actions.DirectionalAverage')
dir_avg.direction = 1
dir_avg.field = mesh.geometry.turbulence_statistics
dir_avg.file = cf.URI('turbulence-statistics-profile-{iteration}.txt')

skip_director = solver.add_unsteady_solver('cf3.solver.ActionDirectorWithSkip')
skip_director.interval = 100
print_timings = skip_director.create_component('PrintTimingTree', 'cf3.common.PrintTimingTree')
print_timings.root = model

# Time setup
time = model.create_time()
tstep = 0.02
time.end_time = 5.*tstep
time.time_step = tstep

solver.InitialConditions.execute()

model.simulate()
domain.write_mesh(cf.URI('les-wale.pvtu'))
model.print_timing_tree()
