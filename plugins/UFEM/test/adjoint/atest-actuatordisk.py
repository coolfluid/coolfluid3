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

tstep = 1.
num_steps = 6.

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
disk.area = D
disk.u_in = u_in[0]
disk.th = 0.05
disk.ct = 0.267
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
ad_solver = solver.add_unsteady_solver('cf3.UFEM.adjoint.Adjoint')
mesh = domain.load_mesh(file = cf.URI('actuator2d.msh'), name = 'Mesh')

# actve region
disk.regions = [mesh.topology.actuator.uri(), mesh.topology.actuator.uri()]
ns_solver.regions = [mesh.topology.uri()]
ad_solver.regions = [mesh.topology.uri(), mesh.topology.actuator.uri()]
# initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = initial_velocity
solver.InitialConditions.density_ratio.density_ratio = 1. # This enables the body force
solver.InitialConditions.adjoint_solution.AdjVelocity = [0., 0.]
# set physical constants
physics.density = rho
physics.dynamic_viscosity = mu

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.
bca = ad_solver.BoundaryConditions
bca.add_constant_bc(region_name = 'inlet', variable_name = 'AdjVelocity').value = [0., 0.]
bca.add_constant_bc(region_name = 'outlet', variable_name = 'AdjPressure').value = 0.

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



# time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# solver.create_fields()
# solver.InitialConditions.execute()

# run the simulation (forward, NS only)
solver.TimeLoop.options.disabled_actions = ['Adjoint']
model.simulate()

# run adjoint, starting from converged NS solution
solver.TimeLoop.options.disabled_actions = ['NavierStokes'] # NS disabled, adjoint enabled
solver.options.disabled_actions = ['InitialConditions'] # disable initial conditions
time.end_time += num_steps*tstep # add again the same number of steps as in the forward solution
model.simulate()

# lss.print_system("adjlss.tec")
# run paraview output.pvtu to see the result
# writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
# writer.mesh = mesh
# writer.file = cf.URI('output.vtm')
# writer.execute()
domain.write_mesh(cf.URI('outputactutest.pvtu'))
model.print_timing_tree()
