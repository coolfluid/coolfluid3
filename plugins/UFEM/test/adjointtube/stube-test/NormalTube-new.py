import coolfluid as cf
from math import pi
import numpy as np

# Flow properties
h = 1.
nu = 0.0001
u_in = 2.0
sa_visc = 5.0*nu

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
# De adjoint oplosser toevoegen
ad_solver = solver.add_unsteady_solver('cf3.UFEM.adjointtube.AdjointTube')
ad_solver.turbulence = 0.
# Add the Spalart-Allmaras turbulence model
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

gradient1 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient1.options.gradient_tag = 'velocity_gradient'
gradient1.options.velocity_variable = 'Velocity'
gradient1.options.velocity_tag = 'navier_stokes_solution'
gradient1.options.gradient_name = 'U'

# Generate mesh
y_segs = 64
x_size = 12*h
s_start = x_size/3.0
s_mid = 1.5*x_size/3.0
s_end = 2.0*x_size/3.0
x_segs1 = 10
x_segs2 = 20
x_segs3 = x_segs1
ungraded_h = float(y_segs)

blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 10)
points[0] = [0.0, 0.0]
points[1] = [s_start, 0.0]
points[2] = [s_start, ungraded_h]
points[3] = [0.0, ungraded_h]
points[4] = [s_mid, 0.0]
points[5] = [s_mid, ungraded_h]
points[6] = [s_end, 0.0]
points[7] = [s_end, ungraded_h]
points[8] = [x_size, 0.0]
points[9] = [x_size, ungraded_h]

block_nodes = blocks.create_blocks(4)
block_nodes[0] = [0, 1, 2, 3] # before bend
block_nodes[1] = [1, 4, 5 ,2] # the bend left
block_nodes[2] = [4, 6, 7, 5] # the bend right
block_nodes[3] = [6, 8, 9, 7] # after bend

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs1, y_segs]
block_subdivs[1] = [x_segs2, y_segs]
block_subdivs[2] = [x_segs2, y_segs]
block_subdivs[3] = [x_segs3, y_segs]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [0.2, 0.2, 1., 1.]
gradings[2] = [5.0, 5.0, 1., 1.]
gradings[3] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 1)
left_patch[0] = [3, 0]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 4)
bottom_patch[0] = [0, 1]
bottom_patch[1] = [1, 4]
bottom_patch[2] = [4, 6]
bottom_patch[3] = [6, 8]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 4)
top_patch[0] = [9, 7]
top_patch[1] = [7, 5]
top_patch[2] = [5, 2]
top_patch[3] = [2, 3]

right_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 1)
right_patch[0] = [8, 9]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

coordmap = {}
b = 0.975
xi = np.linspace(-h, h, y_segs+1)
y_graded = h/b * np.tanh(xi*np.arctanh(b))

coords = mesh.geometry.coordinates
for i in range(len(coords)):
  y_key = int(round(coords[i][1]))
  coords[i][1] = y_graded[y_key]

# Create bend (straight line here)
def curve_equation(x):
  bend_height = 1.0
  if x < s_start:
    return 0.0 # return 0.0
  if x > s_end:
    return -bend_height # return - bend_height
  return bend_height*np.tanh(-x+6)/2-bend_height/2 #(x-s_start) / (s_end-s_start)

for i in range(len(coords)):
  old_y = coords[i][1]
  x = coords[i][0]
  coords[i][1] = curve_equation(x) + old_y

domain.write_mesh(cf.URI("meshed.pvtu"))
exit()

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
ad_solver.regions = [mesh.topology.uri()] # voor de adjoint oplossing uit comment plaatsen
satm.regions = [mesh.topology.uri()]
ad_solver.regions = [mesh.topology.uri()] # voor de adjoint oplossing uit comment plaatsen
gradient1.regions = [mesh.topology.uri()]

u_wall = [0., 0.]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.spalart_allmaras_solution.SAViscosity = sa_visc
# solver.InitialConditions.adjoint_solution.AdjVelocity = [0., 0.]

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Make the boundary global, to allow wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

# Compute the wall distance
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_function_bc(region_name = 'inlet', variable_name = 'Velocity').value =  ['{u_in}'.format(u_in=u_in)]
#bc.add_function_bc(region_name = 'inlet', variable_name = 'Velocity').value = ['{u_in}*(1-(y)^2)'.format(u_in=u_in), '0'] #['1-((y+35)/35)^2', '0'] # -y*(y+70)/35/35
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.0
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall


bca = ad_solver.BoundaryConditions
bc_adj_p1 = bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjointtube.BCAdjointpressurex')
bc_adj_p1.turbulence = 0
bc_adj_p2 = bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjointtube.BCAdjointpressurey')
bc_adj_p2.turbulence = 0
bc_adj_p3 = bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjointtube.BCAdjointpressurey')
bc_adj_p3.turbulence = 0
bc_adj_u0 = bca.create_bc_action(region_name = 'outlet', builder_name = 'cf3.UFEM.adjointtube.RobinUt')
bc_adj_u0.u_index1 = 1
bc_adj_u1 = bca.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjointtube.RobinUt')
bc_adj_u1.u_index1 = 0
bc_adj_u2 = bca.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.adjointtube.RobinUt')
bc_adj_u2.u_index1 = 0
bca.add_constant_component_bc(region_name = 'inlet', variable_name = 'AdjVelocity', component =1).value = 0.
bca.add_constant_component_bc(region_name = 'bottom', variable_name = 'AdjVelocity', component =1).value = 0.
bca.add_constant_component_bc(region_name = 'top', variable_name = 'AdjVelocity', component =1).value = 0.

bc = satm.children.BoundaryConditions
bc.add_function_bc(region_name = 'inlet', variable_name = 'SAViscosity').value = ['{sa_visc}*(1-(y)^2)'.format(sa_visc=sa_visc)]
bc.add_constant_bc(region_name = 'bottom', variable_name = 'SAViscosity').value = 0.
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').value = 0.

# lss = nstokes.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
# lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'#
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
# lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
# lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

# Setup a time series write
write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 500
writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/spalart_allmaras_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/velocity_gradient')]
writer.file = cf.URI('out-sa-{iteration}.pvtu')

# Time setup
time = model.create_time()
time.time_step = 0.01
time.end_time = 1500.0 * time.time_step

probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'EffectiveViscosity']
probe0.coordinate = [s_end, 0.0]
probe0.History.file = cf.URI('out-probe.tsv')

# Run the simulation
solver.TimeLoop.options.disabled_actions = ['AdjointTube']
model.simulate()

writer.file = cf.URI('out-sa-end.pvtu')
writer.execute()

model.print_timing_tree()

# Adjoint oplossing

time = model.create_time()
time.time_step = 0.0001
time.end_time = 20000.0 * time.time_step # add again the same number of steps as in the forward solution

probe1 = solver.add_probe(name = 'Probe', parent = ad_solver, dict = mesh.geometry)
probe1.Log.variables = ['AdjVelocity[0]', 'AdjPressure']
probe1.coordinate = [s_end, 0.0]
probe1.History.file = cf.URI('out-probe-adj.tsv')

solver.TimeLoop.options.disabled_actions = ['NavierStokes'] # NS disabled, adjoint enabled
solver.options.disabled_actions = ['InitialConditions'] # disable initial conditions

model.simulate()

domain.write_mesh(cf.URI('output-adjoint.pvtu'))
model.print_timing_tree()
