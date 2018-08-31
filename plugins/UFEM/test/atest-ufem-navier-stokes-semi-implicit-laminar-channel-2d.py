import sys
import coolfluid as cf
import math

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

# Copy the pressure once, to get the history
p_copy = solver.add_unsteady_solver('cf3.solver.actions.CopyScalar')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
ns_solver.options.theta = 0.5
ns_solver.options.nb_iterations = 2
ns_solver.enable_body_force = True
# ns_solver.options.pressure_rcg_solve = True
ns_solver.options.alpha_su = 1.0
ns_solver.PressureLSS.solution_strategy = 'cf3.math.LSS.DirectStrategy'
refinement_level = 1

supg_terms = solver.add_unsteady_solver('cf3.UFEM.SUPGFields')
residuals = solver.add_unsteady_solver('cf3.UFEM.NSResidual')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [10., 0.]
points[2]  = [0., 1.]
points[3]  = [10., 1.]
points[4]  = [0.,2.]
points[5]  = [10., 2.]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [refinement_level*20, refinement_level*16]
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

# create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
# create_point_region.coordinates = [5., 1.]
# create_point_region.region_name = 'center'
# create_point_region.mesh = mesh
# create_point_region.execute()

# Physical constants
physics.options().set('density', 1.)
physics.options().set('dynamic_viscosity', 1.)

tstep = 0.05

ns_solver.regions = [mesh.topology.uri()]
supg_terms.regions = [mesh.topology.uri()]
residuals.regions = [mesh.topology.uri()]
p_copy.regions = [mesh.topology.uri()]

# for strat in [ns_solver.children.FirstPressureStrategy, ns_solver.children.SecondPressureStrategy]:
#   strat.MLParameters.aggregation_type = 'Uncoupled'
#   strat.MLParameters.max_levels = 4
#   strat.MLParameters.smoother_sweeps = 2
#   strat.MLParameters.coarse_type = 'Amesos-KLU'
#   strat.MLParameters.add_parameter(name = 'repartition: start level', value = 0)
#   strat.MLParameters.add_parameter(name = 'repartition: max min ratio', value = 1.1)
#   #strat.SolverParameters.convergence_tolerance = 1e-6

# lss = ns_solver.VelocityLSS.LSS
# lss.SolutionStrategy.preconditioner_reset = 20000000
# lss.SolutionStrategy.Parameters.preconditioner_type = 'Ifpack'
# lss.SolutionStrategy.Parameters.PreconditionerTypes.Ifpack.overlap = 0
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block CG'
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockCG.convergence_tolerance = 1e-6
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockCG.maximum_iterations = 300
#lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 100

# Initial conditions
ic_u = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = ['0', '0']
ic_g = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = ['2', '0']

# Boundary conditions
bc_u = ns_solver.VelocityLSS.BC
bc_u.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
bc_u.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0.]
# Pressure BC
#ns_solver.PressureLSS.BC.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 10.

solver.create_fields()
# supg_avg = solver.add_unsteady_solver('cf3.solver.actions.FieldTimeAverage')
# supg_avg.field = mesh.geometry.supg_terms

# res_avg = solver.add_unsteady_solver('cf3.solver.actions.FieldTimeAverage')
# res_avg.field = mesh.geometry.navier_stokes_residual

# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = 2.*tstep
model.simulate()

domain.write_mesh(cf.URI('semi-implicit-laminar-channel-2d.pvtu'))

# for ((x, y), (u, v)) in zip(mesh.geometry.coordinates, mesh.geometry.navier_stokes_u_solution):
#   u_ref = y*(2-y)
#   if abs(u_ref - u) > 1e-2:
#     raise Exception('Error in u component: {u} != {u_ref} at y = {y}'.format(u = u, u_ref = u_ref, y = y))
#   if abs(v) > 1e-3:
#     raise Exception('Non-zero v-component {v} at y = {y}'.format(v = v, y = y))

# print timings
model.print_timing_tree()
