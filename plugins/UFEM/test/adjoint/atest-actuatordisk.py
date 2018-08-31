import sys
import coolfluid as cf

# to make the CF3 python module known to python, first execute:
# export PYTHONPATH=$HOME/coolfluid/build/cf3-barche/dso

# inlet velocity
u_in = [15., 0.]
initial_velocity = [0., 0.]
rho = 1.225
mu = 0.0000178
D=0.5
l0 = 230.60
I=0.06
area = D
Ct0=0.1
k_init = 1.5*(u_in[0]*I)**2
k_wall = 1.5*(u_in[0]*I)**2
e_init = 0.09**(3/4)*k_init**1.5/l0
e_wall = 0.09**(3/4)*k_init**1.5/l0

tstep = 1.
num_steps = 1.

env = cf.Core.environment()
env.log_level = 4

# Basic model setup (container of all the sumilation setup)
model = cf.root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')

# The domain contains the mesh
domain = model.create_domain()

# Physics contains constants such as viscosity
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')
# Solver contains all solvers for the problem (conduction, flow, ...)
solver = model.create_solver('cf3.UFEM.Solver')

# Add a concrete Navier-Stokes finite element solver
disk = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDisk')
disk.area = area
disk.u_in = u_in[0]
disk.th = 0.05
disk.ct = Ct0
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
ad_solver = solver.add_unsteady_solver('cf3.UFEM.adjoint.Adjoint')
ad_solver.ct = [Ct0]
ad_solver.th = 0.05
ad_solver.area = area
ad_solver.turbulence = 1.
#k-epsilon
ke = solver.add_unsteady_solver('cf3.UFEM.StandardKEpsilon')
ke.options.theta = 1.
ke.options.l_max = 1000.
kaea = solver.add_unsteady_solver('cf3.UFEM.adjoint.keAdjoint')
kaea.options.theta = 1.
kaea.options.l_max = 1000.
gradient1 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient1.options.gradient_tag = 'Adjvelocity_gradient'
gradient1.options.velocity_variable = 'AdjVelocity'
gradient1.options.velocity_tag = 'adjoint_solution'
gradient1.options.gradient_name = 'U'

mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# active region
disk.regions = [mesh.topology.actuator.uri(), mesh.topology.actuator.uri()]
ns_solver.regions = [mesh.topology.uri()]
ke.regions = [mesh.topology.uri()]
ad_solver.regions = [mesh.topology.uri(), mesh.topology.actuator.uri()]
kaea.regions = [mesh.topology.uri()]
gradient1.regions = [mesh.topology.uri()]
# initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = initial_velocity
solver.InitialConditions.density_ratio.density_ratio = 1. # This enables the body force
solver.InitialConditions.adjoint_solution.AdjVelocity = [0., 0.]
ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_k.variable_name = 'k'
ic_k.value = [str(k_init)]
ic_k.regions = [mesh.topology.uri()]

ic_ka = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'keAdjoint_solution')
ic_ka.variable_name = 'ka'
ic_ka.value = [str(0.1)]
ic_ka.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_epsilon.variable_name = 'epsilon'
ic_epsilon.value = [str(e_init)]
ic_epsilon.regions = [mesh.topology.uri()]

ic_epsilona = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'keAdjoint_solution')
ic_epsilona.variable_name = 'epsilona'
ic_epsilona.value = [str(0.1)]
ic_epsilona.regions = [mesh.topology.uri()]

ic_wall_distance = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'wall_distance')
ic_wall_distance.variable_name = 'wall_distance'
ic_wall_distance.value = ['100']
ic_wall_distance.regions = [mesh.topology.uri()]

# set physical constants
physics.density = rho
physics.dynamic_viscosity = mu
#boundary condition

bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.

bca = ad_solver.BoundaryConditions
bc_adj_p1 = bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjoint.BCAdjointpressurex')
bc_adj_p1.turbulence = 1
bc_adj_p2 = bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjoint.BCAdjointpressurey')
bc_adj_p2.turbulence = 1
bc_adj_p3 = bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjoint.BCAdjointpressurey')
bc_adj_p3.turbulence = 1
bc_adj_u0 = bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjoint.RobinUt')
bc_adj_u0.u_index1 = 1
bc_adj_u1 = bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjoint.RobinUt')
bc_adj_u1.u_index1 = 0
bc_adj_u2 = bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjoint.RobinUt')
bc_adj_u2.u_index1 = 0
bca.add_constant_component_bc(region_name = 'inlet', variable_name = 'AdjVelocity', component =1).value = 0.
bca.add_constant_component_bc(region_name = 'bottom', variable_name = 'AdjVelocity', component =1).value = 0.
bca.add_constant_component_bc(region_name = 'top', variable_name = 'AdjVelocity', component =1).value = 0.

bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'inlet', variable_name = 'k').value = k_wall
bc.add_constant_bc(region_name = 'inlet', variable_name = 'epsilon').value = e_wall

bca = kaea.LSS.BoundaryConditions
bca.add_constant_bc(region_name = 'inlet', variable_name = 'epsilona').value = 0.
bca.add_constant_bc(region_name = 'inlet', variable_name = 'ka').value = 0.
bca_ks=bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjoint.kaRobinke')
bca_ks=bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjoint.kaRobinke')
bca_ks=bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjoint.kaRobinke')

# Solver setup
lss = ns_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
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

lss = ke.LSS.LSS
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'


lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

lss = ad_solver.LSS
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'


lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

lss = kaea.LSS.LSS
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'


lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

# time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# solver.create_fields()
# solver.InitialConditions.execute()

# run the simulation (forward, NS only)
solver.TimeLoop.options.disabled_actions = ['Adjoint','keAdjoint']
model.simulate()

# run adjoint, starting from converged NS solution
# solver.TimeLoop.options.disabled_actions = ['NavierStokes','StandardKEpsilon'] # NS disabled, adjoint enabled
# solver.options.disabled_actions = ['InitialConditions'] # disable initial conditions
# time.end_time += num_steps*tstep # add again the same number of steps as in the forward solution
#
# model.simulate()

# lss.print_system("adjlss.tec")
# run paraview output.pvtu to see the result
# writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('output.vtm')
# writer.execute()
domain.write_mesh(cf.URI('outputactutest.pvtu'))
model.print_timing_tree()
