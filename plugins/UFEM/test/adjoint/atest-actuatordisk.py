import sys
import coolfluid as cf

# to make the CF3 python module known to python, first execute:
# export PYTHONPATH=$HOME/coolfluid/build/cf3-barche/dso

# inlet velocity
u_in = [15., 0., 0.]
u_act = [10., 0., 0.]
initial_velocity = [0., 0., 0.]
rho = 1.225
mu = 0.00001

tstep = 5.
num_steps = 2.

env = cf.Core.environment()
env.log_level = 4

# Basic model setup (container of all the sumilation setup)
model = cf.root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')

# The domain contains the mesh
domain = model.create_domain()

# Physics contains constants such as viscosity
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')

# Solver contains all solvers for the problem (conduction, flow, ...)
solver = model.create_solver('cf3.UFEM.Solver')

# Add a concrete Navier-Stokes finite element solver
disk = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDisk')
disk.constant = 0.7

ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
ns_solver.enable_body_force = True

mesh = domain.load_mesh(file = cf.URI('Actuator.msh'), name = 'Mesh')

# actve region
disk.regions = [mesh.topology.actuator.uri(), mesh.topology.left.uri()]
ns_solver.regions = [mesh.topology.uri()]

# initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = initial_velocity

# set physical constants
physics.density = rho
physics.dynamic_viscosity = mu

# Boundary conditions
bc = ns_solver.BoundaryConditions
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = [0., 0., 0.]
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Velocity').value = [0., 0., 0.]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0., 0.]
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0., 0.]
bc.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.
bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = u_in

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

# time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# run the simulation
model.simulate()

# run paraview output.pvtu to see the result
#writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
#writer.mesh = mesh
#writer.file = cf.URI('output.vtm')
#writer.execute()
domain.write_mesh(cf.URI('outputactutest.pvtu'))
#model.print_timing_tree()
