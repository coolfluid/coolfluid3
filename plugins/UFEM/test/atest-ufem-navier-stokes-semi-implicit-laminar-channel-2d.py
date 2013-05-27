import sys
import coolfluid as cf
import math

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4
env.only_cpu0_writes = True

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
ns_solver.InnerLoop.options.max_iter = 2

refinement_level = 1

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [10., 0.]
points[2]  = [0., 1.]
points[3]  = [10., 1.]
points[4]  = [0.,2.]
points[5]  = [10., 2.]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [refinement_level*20, refinement_level*10]
block_subdivs[1] = block_subdivs[0]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]

#blocks.partition_blocks(nb_partitions = 2, direction = 0)
#blocks.partition_blocks(nb_partitions = 2, direction = 1)

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# Physical constants
physics.options().set('density', 1.)
physics.options().set('dynamic_viscosity', 1.)
physics.options().set('reference_velocity', 1.)

tstep = 0.15

ns_solver.regions = [mesh.topology.uri()]

lss_p = ns_solver.InnerLoop.PressureSystem.LSS
lss_p.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
#lss_p.SolutionStrategy.use_ml_preconditioner = True
#lss_p.SolutionStrategy.MLParameters.coarse_max_size = 100
#lss_p.SolutionStrategy.MLParameters.smoother_sweeps = 4
#lss_p.SolutionStrategy.MLParameters.smoother_pre_or_post = 'both'
#lss_p.SolutionStrategy.MLParameters.aggregation_type = 'MIS'
#lss_p.SolutionStrategy.coordinates = mesh.geometry.coordinates
#lss_p.SolutionStrategy.used_nodes = ns_solver.InnerLoop.PressureSystem.children.nodes_used

#lss_u = ns_solver.InnerLoop.VelocitySystem.create_lss('cf3.math.LSS.TrilinosFEVbrMatrix')
#lss_u.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block CG'
#lss_u.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockCG.convergence_tolerance = 1e-8

# Initial conditions
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = ['y*(2-y)', '0']
ic_p = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
ic_p.variable_name = 'Pressure'
ic_p.regions = [mesh.topology.uri()]
ic_p.value = ['20*(1-x/10)']

# Boundary conditions
bc_u = ns_solver.InnerLoop.VelocitySystem.BC
bc_u.add_constant_bc(region_name = 'bottom', variable_name = 'a').value = [0., 0.]
bc_u.add_constant_bc(region_name = 'top', variable_name = 'a').value = [0., 0.]
bc_u.add_constant_bc(region_name = 'left', variable_name = 'a').value = [0., 0.]
bc_u.add_constant_bc(region_name = 'right', variable_name = 'a').value = [0., 0.]
# The variable name here is delta_p, but we really are setting the pressure
ns_solver.InnerLoop.PressureSystem.BC.add_constant_bc(region_name = 'right', variable_name = 'delta_p').value = 0.

# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = 10.*tstep
model.simulate()

domain.write_mesh(cf.URI('explicit-implicit-laminar-channel-2d.pvtu'))

# print timings
model.print_timing_tree()
