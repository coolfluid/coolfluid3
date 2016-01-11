import coolfluid as cf
from math import pi
import sys

# Flow properties
h = 1.
nu = 1e-5
u_in = [1., 0.]

y_segs = 32
x_size = 2.*h
x_segs = 32

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the k-epsilon turbulence model solver(ke)
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Generate mesh
# blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
# points = blocks.create_points(dimensions = 2, nb_points = 6)
# points[0]  = [0, 0.]
# points[1]  = [x_size, 0.]
# points[2]  = [0., h]
# points[3]  = [x_size, h]
# points[4]  = [0.,2.*h]
# points[5]  = [x_size, 2.*h]
#
# block_nodes = blocks.create_blocks(2)
# block_nodes[0] = [0, 1, 3, 2]
# block_nodes[1] = [2, 3, 5, 4]
#
# block_subdivs = blocks.create_block_subdivisions()
# block_subdivs[0] = [x_segs, y_segs]
# block_subdivs[1] = block_subdivs[0]
#
# gradings = blocks.create_block_gradings()
# gradings[0] = [1., 1., 1., 1.]
# gradings[1] = [1., 1., 1., 1.]
#
# inlet_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 1)
# inlet_patch[0] = [2, 0]
#
# left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)
# left_patch[0] = [4, 2]
#
# bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
# bottom_patch[0] = [0, 1]
#
# top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
# top_patch[0] = [5, 4]
#
# right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
# right_patch[0] = [1, 3]
# right_patch[1] = [3, 5]
#
# mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
# blocks.create_mesh(mesh.uri())

mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

# actve region
ns_solver.regions = [mesh.topology.uri()]

# initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = [0.,0.]

# set physical constants
physics.density = 1.
physics.dynamic_viscosity = nu

# Compute the wall distance
# make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
# make_boundary_global.mesh = mesh
# make_boundary_global.execute()
# wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
# wall_distance.mesh = mesh
# wall_distance.regions = [mesh.topology.wall]
# wall_distance.execute()

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

# Boundary conditions
bc = ns_solver.BoundaryConditions
# bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
# bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = [0., 0.]
# #bc.add_constant_component_bc(region_name = 'right', variable_name = 'Velocity', component = 0).value =  0.
# #bc.add_constant_component_bc(region_name = 'bottom', variable_name = 'Velocity', component = 1).value =  0.
# rbc = bc.create_bc_action(region_name = 'right', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
# bbc = bc.create_bc_action(region_name = 'bottom', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
# bc.add_constant_bc(region_name = 'top', variable_name = 'Pressure').value = 0.

bc.add_constant_bc(region_name = 'in', variable_name = 'Velocity').value = u_in
bc.add_constant_bc(region_name = 'symm', variable_name = 'Velocity').value = u_in
#bc.add_constant_bc(region_name = 'wall', variable_name = 'Velocity').value = [0., 0.]
bc.create_bc_action(region_name = 'wall', builder_name = 'cf3.UFEM.BCWallFunctionNSImplicit')
bc.add_constant_bc(region_name = 'out', variable_name = 'Pressure').value = 0.

# time setup
time = model.create_time()
time.time_step = 0.1
time.end_time = 5.

writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
writer.mesh = mesh
writer.file = cf.URI('atest-kepsilon-wallfunctions.vtm')

# run the simulation
model.simulate()

writer.execute()
