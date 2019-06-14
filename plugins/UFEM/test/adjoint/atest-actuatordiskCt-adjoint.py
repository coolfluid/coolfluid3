import sys 
import coolfluid as cf
from math import pi
import numpy as np 

# Flow properties
Ct1 = 2.4
Ct2 = 3.4
rho = 1.225
mu = 0.0000178
D = 126.0
area = D
u_in = [15.0, 0.0]
initial_velocity = [0.0, 0.0]
thickness = 10.0
max_adj_speed = 30.0


turbulent = False

if turbulent == True:
    I=0.06
    l0 = 230.60
    k_init = 1.5*(u_in[0]*I)**2
    k_wall = 1.5*(u_in[0]*I)**2
    e_init = 0.09**(3/4)*k_init**1.5/l0
    e_wall = 0.09**(3/4)*k_init**1.5/l0



tstep = 10.0
nStep = 3


# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

# Setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Setup the adjoint solver
ad_solver = solver.add_unsteady_solver('cf3.UFEM.adjoint.AdjointCt')
ad_solver.maxU = max_adj_speed
ad_solver.area = D
if turbulent == False:
    ad_solver.turbulence = 0.0
else:
    ad_solver.turbulence = 1.0
ad_solver.th = thickness

# Setup the adjoint actuator disk model
ad_disk1 = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDiskCtAdjoint')
ad_disk1.area = area
ad_disk1.th = thickness
ad_disk1.ct = Ct1


ad_disk2 = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDiskCtAdjoint')
ad_disk2.area = area
ad_disk2.th = thickness
ad_disk2.ct = Ct2


gradient1 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient1.options.gradient_tag = 'Adjvelocity_gradient'
gradient1.options.velocity_variable = 'AdjVelocity'
gradient1.options.velocity_tag = 'adjoint_solution'
gradient1.options.gradient_name = 'U'

gradient2 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient2.options.gradient_tag = 'velocity_gradient'
gradient2.options.velocity_variable = 'Velocity'
gradient2.options.velocity_tag = 'navier_stokes_solution'
gradient2.options.gradient_name = 'u'


if turbulent == True:
    # Adjoint k-e solver
    kaea = solver.add_unsteady_solver('cf3.UFEM.adjoint.keAdjoint')
    kaea.options.theta = 1.
    kaea.options.l_max = 1000.


mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
reader = domain.create_component('CF3MeshReader', 'cf3.mesh.cf3mesh.Reader')
reader.mesh = mesh
reader.file = cf.URI('out-fluid.cf3mesh')
reader.execute()

ad_solver.regions = [mesh.topology.uri()]
ad_disk1.regions = [mesh.topology.actuator1.uri()]
ad_disk2.regions = [mesh.topology.actuator2.uri()]
gradient1.regions = [mesh.topology.uri()]
gradient2.regions = [mesh.topology.uri()]

if turbulent == True:
    kaea.regions = [mesh.topology.uri()]

lss = ad_solver.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'

# lss.SolutionStrategy.Parameters.preconditioner_type = 'Ifpack'

lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000


if turbulent == True:
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



# Initial condition
solver.InitialConditions.adjoint_solution.AdjVelocity = initial_velocity


if turbulent == True:
    ic_ka = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'keAdjoint_solution')
    ic_ka.variable_name = 'ka'
    ic_ka.value = [str(0.1)]
    ic_ka.regions = [mesh.topology.uri()]
    ic_epsilona = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'keAdjoint_solution')
    ic_epsilona.variable_name = 'epsilona'
    ic_epsilona.value = [str(0.1)]
    ic_epsilona.regions = [mesh.topology.uri()]


# properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = mu

# boundary conditions


bca = ad_solver.BoundaryConditions
bca.add_constant_bc(region_name = 'inlet', variable_name = 'AdjVelocity').value = [0.0, 0.0]
# bca.add_constant_component_bc(region_name = 'inlet', variable_name = 'AdjVelocity', component = 1).value = 0.0

bca.add_constant_bc(region_name = 'outlet', variable_name = 'AdjVelocity').value = [0.0, 0.0]
bca.add_constant_bc(region_name = 'outlet', variable_name = 'AdjPressure').value = 0.0

bc_adj_p1 = bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjoint.BCAdjointpressurexNew')
bc_adj_p1.turbulence = 0




if turbulent == True:
    bca = kaea.LSS.BoundaryConditions
    bca.add_constant_bc(region_name = 'inlet', variable_name = 'epsilona').value = 0.
    bca.add_constant_bc(region_name = 'inlet', variable_name = 'ka').value = 0.
    bca_ks=bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjoint.kaRobinke')
    bca_ks=bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjoint.kaRobinke')
    bca_ks=bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjoint.kaRobinke')


# Solver setup
# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = nStep * time.time_step

probe1 = solver.add_probe(name = 'Probe', parent = ad_solver, dict = mesh.geometry)
probe1.Log.variables = ['AdjVelocity[0]', 'AdjPressure']
probe1.coordinate = [20, 0]
if turbulent == True:
    probe1.History.file = cf.URI('out-adjoint-probe.tsv')
else:
    probe1.History.file = cf.URI('out-adjoint-probe.tsv')

model.simulate()

if turbulent == True:
    domain.write_mesh(cf.URI('out-adjoint-turb.pvtu'))
    domain.write_mesh(cf.URI('out-adjoint-turb.cf3mesh'))
else:
    domain.write_mesh(cf.URI('out-adjoint.pvtu'))
    domain.write_mesh(cf.URI('out-adjoint.cf3mesh'))
model.print_timing_tree()