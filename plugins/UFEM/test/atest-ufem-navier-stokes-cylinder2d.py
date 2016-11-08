import coolfluid as cf
import sys

# inlet velocity
u_in = [1., 0.]
initial_velocity = [1., 0.]
rho = 100.
mu = 1.

tstep = 0.1
num_steps = 5.

# Basic model setup (container of all the sumilation setup)
model = cf.root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')

# The domain contains the mesh
domain = model.create_domain()
domain.load_balancer_type = 'cf3.zoltan.PHG'

# Physics contains constants such as viscosity
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')

# Solver contains all solvers for the problem (conduction, flow, ...)
solver = model.create_solver('cf3.UFEM.Solver')

# Add a concrete Navier-Stokes finite element solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# load the mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# actve region
ns_solver.regions = [mesh.topology.uri()]

# solver setup
lss = ns_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 10)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 2
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

# initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = initial_velocity

# set physical constants
physics.density = rho
physics.dynamic_viscosity = mu

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure').value = 0.

# time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# run the simulation
model.simulate()

# run paraview output.pvtu to see the result
domain.write_mesh(cf.URI('ns-cylinder2d-output.pvtu'))

model.print_timing_tree()
