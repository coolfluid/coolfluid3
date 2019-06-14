import sys 
import coolfluid as cf
import math

# inlet velocity
thickness = 10.0
u_infty = 15.0
u_in = [u_infty, 0.0]
initial_velocity = [u_infty, 0.0]
rho = 1.225
mu = 0.0000178
D = 126 #0.5
l0 = 230.60
I = 0.096
area = D
Ct = 2.4
c_mu = 0.09
# k_init = 1.5*(u_in[0]*I)**2
# k_wall = 1.5*(u_in[0]*I)**2
# e_init = c_mu**(0.75)*k_init**1.5/l0
# e_wall = c_mu**(0.75)*k_init**1.5/l0
turbulent = True

kappa = 0.4
u_ref = u_infty
h_ref = 100.0
z0 = 7.056e-3
# c_mu = 0.09

u_tau = kappa * u_ref / (math.log((h_ref+z0)/z0))
k_wall = u_tau**2 / math.sqrt(c_mu)
k_init = k_wall
e_wall = u_tau**3 / (kappa * (z0+h_ref))
e_init = e_wall



# tstep = 10.0
# nStep = 10
tstep = 0.1
nStep = 5


# Some shortcuts
env = cf.Core.environment()
root = cf.Core.root()

# Global configuration
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

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the k-e turbulence model
if turbulent:
    ke = solver.add_unsteady_solver('cf3.UFEM.ModifiedKEpsilon') # Contains El Kasmi Masson and Ren Zhang models
    ke.options.theta = 1.0
    ke.options.l_max = 1000.0
    ke.th = thickness
    ke.ren = 0.0 # Set to 0.0 to use the El Kasmi model, set to 1.0 to use the Ren Zhang model
    ke.Ct = Ct
    ke.u_infty = u_infty

# Add the actuator disk model
disk = solver.add_unsteady_solver('cf3.UFEM.adjoint.ActuatorDiskCt')
disk.area = area
disk.u_in = u_in[0]
disk.th = thickness
disk.ct = Ct

# Load the mesh
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')
# mesh = domain.load_mesh(file = cf.URI('Actuator2pgHybrid.msh'), name = 'Mesh')

# Make the boundary global, to allow wall distance  --> To be compatible with multithread --> so that each thread can access the wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

# Compute the wall distance  
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

link_vertical = domain.create_component('LinkVertical', 'cf3.mesh.actions.LinkPeriodicNodes')
link_vertical.mesh = mesh
link_vertical.source_region = mesh.topology.top
link_vertical.destination_region = mesh.topology.bottom
link_vertical.translation_vector = [0.0, -3000.0]
link_vertical.execute()

# Make sure the boundary is non-global again  --> to avoid the double data in each thread
partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

# Define the active regions
# The solvers do not have automatically assigned regions --> we must assign them manually
nstokes.regions = [mesh.topology.uri()]
if turbulent:
    ke.regions = [mesh.topology.uri()]
disk.regions = [mesh.topology.actuator.uri()]

# Because the matrices are often bad conditioned, we use a method the make them better conditioned
# Solver setup
# lss = nstokes.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosCrsMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
# lss.SolutionStrategy.Parameters.preconditioner_type = 'Ifpack'

# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000


lss = nstokes.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosCrsMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
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
solver.InitialConditions.navier_stokes_solution.Velocity = initial_velocity
# solver.InitialConditions.density_ratio.density_ratio = 1.0 # This enables the body force

if turbulent:
    ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
    ic_k.variable_name = 'k'
    ic_k.value = [str(k_init)]
    ic_k.regions = [mesh.topology.uri()]

    ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
    ic_epsilon.variable_name = 'epsilon'
    ic_epsilon.value = [str(e_init)]
    ic_epsilon.regions = [mesh.topology.uri()]

    ic_wall_distance = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'wall_distance')
    ic_wall_distance.variable_name = 'wall_distance'
    ic_wall_distance.value = ['100']
    ic_wall_distance.regions = [mesh.topology.uri()]

    ic_dens_ratio = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'density_ratio')
    ic_dens_ratio.variable_name = 'density_ratio'
    ic_dens_ratio.value = [str(1.0)]
    ic_dens_ratio.regions = [mesh.topology.out_actuator.uri()]

# properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = mu
physics.c_mu = c_mu

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.0

if turbulent:
    bc = ke.LSS.BoundaryConditions
    bc.add_constant_bc(region_name = 'inlet', variable_name = 'k').value = k_wall
    bc.add_constant_bc(region_name = 'inlet', variable_name = 'epsilon').value = e_wall

restart_writer = solver.add_restart_writer()
restart_writer.Writer.file = cf.URI('out-fluid.cf3restart')
restart_writer.interval = nStep

# Compute the normals

compute_normals = domain.create_component('ComputeNormals','cf3.UFEM.NormalizedNormals')
compute_normals.physical_model = physics
compute_normals.mesh = mesh
compute_normals.regions = [mesh.topology.outlet.uri(), mesh.topology.inlet.uri(), mesh.topology.top.uri(), mesh.topology.bottom.uri()]
compute_normals.execute()

# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = nStep * time.time_step

# probe to see the convergence
probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'Pressure']
probe0.coordinate = [100, 0.0]
probe0.History.file = cf.URI('out-fluid-probe.tsv')

model.simulate()

domain.write_mesh(cf.URI('out-fluid.pvtu'))
domain.write_mesh(cf.URI('out-fluid.cf3mesh'))



model.print_timing_tree()