import sys
import coolfluid as cf

# Flow configuration
h = 0.5
re_tau = 50.
nu = 0.0001
a_tau = re_tau**2*nu**2/h**3
Uc = a_tau/nu*(h**2/2.)

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
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
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [10., 0.]
points[2]  = [0., h]
points[3]  = [10., h]
points[4]  = [0.,2.*h]
points[5]  = [10., 2.*h]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10, 10]
block_subdivs[1] = [10, 10]

grading = 1.2
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., grading, grading]
gradings[1] = [1., 1., 1./grading, 1./grading]

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

partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
partitioner.mesh = mesh

link_horizontal = partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-10., 0.]

partitioner.execute()

ns_solver.regions = [mesh.topology.interior.uri()]

u_in = [0., 0.]

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0', '0']

# Physical constants
physics.density =  1.
physics.dynamic_viscosity = nu

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.regions = [mesh.topology.uri()]
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0.]

pressure_integral = solver.add_unsteady_solver('cf3.UFEM.SurfaceIntegral')
pressure_integral.set_field(variable_name = 'Pressure', field_tag = 'navier_stokes_solution')
pressure_integral.regions = [mesh.topology.access_component('bottom').uri()]
pressure_integral.history = solver.create_component('ForceHistory', 'cf3.solver.History')
pressure_integral.history.file = cf.URI('force-implicit.tsv')
pressure_integral.history.dimension = 2

bulk_velocity = solver.add_unsteady_solver('cf3.UFEM.BulkVelocity')
bulk_velocity.set_field(variable_name = 'Velocity', field_tag = 'navier_stokes_solution')
bulk_velocity.regions = [mesh.topology.right.uri()]
bulk_velocity.history = bulk_velocity.create_component('History', 'cf3.solver.History')
bulk_velocity.history.file = cf.URI('bulk-velocity.tsv')
bulk_velocity.history.dimension = 1

lss = ns_solver.LSS
#lss.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
#lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 10)
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.eigen_analysis_type = 'Anorm'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'Chebyshev'
#lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 6

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 10
writer = write_manager.create_component('PVTUWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.file = cf.URI('periodic-channel-2d-{iteration}.pvtu')

# Time setup
time = model.create_time()
time.time_step =  1000.
time.end_time = 20000.

# Create the fields, so we can easily set the writer field URI
solver.create_fields()
writer.fields = [mesh.geometry.navier_stokes_solution.uri()]

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# centerline velocity
u_max = 0
for [u,v,p] in mesh.geometry.navier_stokes_solution:
  if u > u_max:
    u_max = u

print 'found u_max:',u_max

if abs(u_max - Uc) > 0.05:
  raise Exception('Laminar profile not reached')