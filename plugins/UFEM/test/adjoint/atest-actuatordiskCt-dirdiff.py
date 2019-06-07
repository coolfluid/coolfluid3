import coolfluid as cf
import math
from math import pi
from math import sqrt
import numpy as np 
import multiprocessing

#Flow properties
Ct1 = 2.4
Ct2 = 3.4
rho = 1.225
mu = 0.0000178
D = 126.0
area = D
u_in = [15.0, 0.0]
initial_velocity = [15.0, 0.0]
thickness = 10.0

tstep = 10.0
nstep = 3

# Shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

# Setup a model
model = root.create_component('NavierSotkes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

gradient1 = solver.add_unsteady_solver('cf3.UFEM.PressureGradient')
gradient1.options.gradient_tag = 'pressure_gradient'
gradient1.options.pressure_variable = 'Pressure'
gradient1.options.pressure_tag = 'navier_stokes_solution'
gradient1.options.gradient_name = 'p'

gradient2 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient2.options.gradient_tag = 'velocity_gradient'
gradient2.options.velocity_variable = 'Velocity'
gradient2.options.velocity_tag = 'navier_stokes_solution'
gradient2.options.gradient_name = 'u'

gradient3 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient3.options.gradient_tag = 'pressure_hessian'
gradient3.options.velocity_variable = 'grad_p'
gradient3.options.velocity_tag = 'pressure_gradient'
gradient3.options.gradient_name = 'p2'

gradient4 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient4.options.gradient_tag = 'ux_hessian'
gradient4.options.velocity_variable = 'grad_ux'
gradient4.options.velocity_tag = 'velocity_gradient'
gradient4.options.gradient_name = 'ux2'

gradient5 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient5.options.gradient_tag = 'uy_hessian'
gradient5.options.velocity_variable = 'grad_uy'
gradient5.options.velocity_tag = 'velocity_gradient'
gradient5.options.gradient_name = 'uy2'

dirdiff_solver = solver.add_unsteady_solver('cf3.UFEM.adjoint.DirectDifferentiationCt')
dirdiff_solver.options.theta = 1.0
dirdiff_solver.area = area
dirdiff_solver.th = thickness

dirdiff_disk1 = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDiskCtDirdiff')
dirdiff_disk1.ct = Ct1
dirdiff_disk1.th = thickness
dirdiff_disk1.area = area

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
reader = domain.create_component('CF3MeshReader', 'cf3.mesh.cf3mesh.Reader')
reader.mesh = mesh
# reader.file = cf.URI('out-fluid.cf3mesh')
reader.file = cf.URI('out-fluid.cf3mesh')
reader.execute()

u_wall = [0.0, 0.0]

gradient1.regions = [mesh.topology.uri()]
gradient2.regions = [mesh.topology.uri()]
gradient3.regions = [mesh.topology.uri()]
gradient4.regions = [mesh.topology.uri()]
gradient5.regions = [mesh.topology.uri()]
dirdiff_solver.regions = [mesh.topology.uri()]

dirdiff_disk1.regions = [mesh.topology.actuator1.uri(), mesh.topology.actuator2.uri()]  # First disk: disk for which the sensitivity will be computed

lss = dirdiff_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
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



# Initial conditions
solver.InitialConditions.sensitivity_solution.SensU = initial_velocity

# properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = mu

# Boundary conditions for Direct Differentiation
bcd = dirdiff_solver.BoundaryConditions
bcd.add_constant_bc(region_name = 'inlet', variable_name = 'SensU').value = [0.0, 0.0]
bcd.add_constant_bc(region_name = 'outlet', variable_name = 'SensP').value = 0.0

#    bcd.add_constant_bc(region_name = 'top', variable_name = 'SensU').value = [0.0, 0.0]
#    bcd.add_constant_bc(region_name = 'bottom', variable_name = 'SensU').value = [0.0, 0.0]
    #normals_field = mesh.geometry.NodalNormal

probe0 = solver.add_probe(name = 'Probe', parent = dirdiff_solver, dict = mesh.geometry)
probe0.Log.variables = ['SensU[0]', 'SensP']
probe0.coordinate = [10.0, 0.0]
probe0.History.file = cf.URI('out-dirdiff-probe.tsv')


time = model.create_time()
time.time_step = tstep

time.end_time = nstep * time.time_step
model.simulate()
domain.write_mesh(cf.URI('out-dirdiff.pvtu'))
domain.write_mesh(cf.URI('out-dirdiff.cf3mesh'))
