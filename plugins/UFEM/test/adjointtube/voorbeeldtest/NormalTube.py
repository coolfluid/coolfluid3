import coolfluid as cf

u_in = [1.,0.] # 2 dimensioneel
u_wall = [0.,0.]
initial_velocity = u_in
#rho = 1.225
#mu   = 0.0000178 # p 18
# obv Re getal paper= 2*10

rho = 4.0e4
mu = 1

tstep = 1 #dit is stap in de tijd
num_steps = 40 #aantal stappen

env = cf.Core.environment()
env.log_level = 3

model = cf.root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the Spalat-Allmaras turbulence model
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

mesh = domain.load_mesh(file = cf.URI('AlternatieveBuis.msh'), name = 'Mesh')

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.spalart_allmaras_solution.SAViscosity = 0.001

#properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = mu

# Compute the wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.0
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall

bc = satm.children.BoundaryConditions
bc.add_constant_bc(region_name = 'inlet', variable_name = 'SAViscosity').value = 0.001
bc.add_constant_bc(region_name = 'bottom', variable_name = 'SAViscosity').value = 0.0
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').value = 0.0

# Solver setup
lss = nstokes.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
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

# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# # Run the simulation
# model.simulate()
#
# writer = domain.create_component('CF3ToVTK', 'cf3.vtk.MultiblockWriter')
# writer.mesh =  mesh
# writer.file = cf.URI('out-normaltube.vtm')
# writer.execute()

write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 4
writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML')
writer.mesh = mesh
writer.file = cf.URI('out-sbuis-sa-{iteration}.pvtu')

# run the simulation (forward, NS only)
#solver.TimeLoop.options.disabled_actions = ['Adjoint']
model.simulate()

#domain.write_mesh(cf.URI('LC_1_V_25.pvtu'))
