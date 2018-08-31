import sys
import coolfluid as cf

# parameters
u_lid = [2., 0.]
segments = 32

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global confifuration
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

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
ns_solver.options.theta = 0.5
ns_solver.options.nb_iterations = 2
ns_solver.PressureLSS.solution_strategy = 'cf3.math.LSS.DirectStrategy'

ic_visc = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_viscosity')
ic_visc.variable_name = 'EffectiveViscosity'

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [-0.5, -0.5]
points[1]  = [0.5, -0.5]
points[2]  = [-0.5, 0.]
points[3]  = [0.5, 0.]
points[4]  = [-0.5,0.5]
points[5]  = [0.5, 0.5]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [segments, segments/2]
block_subdivs[1] = [segments, segments/2]

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



mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
create_point_region.coordinates = [0., 0.]
create_point_region.region_name = 'center'
create_point_region.mesh = mesh
create_point_region.execute()

ns_solver.regions = [mesh.topology.interior.uri()]

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = ns_solver.regions
ic_u.value = ['0']

ic_p = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_p_solution')
ic_p.regions = ns_solver.regions
ic_p.variable_name = 'Pressure'
ic_p.value = ['10']

# Physical constants
physics.options().set('density', 1000.)
physics.options().set('dynamic_viscosity', 10.)

# Boundary conditions
bc = ns_solver.VelocityLSS.BC
bc.regions = [mesh.topology.uri()]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = u_lid
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = [0., 0.]
bc.add_constant_bc(region_name = 'right', variable_name = 'Velocity').value = [0., 0.]
bc = ns_solver.PressureLSS.BC
bc.regions = [mesh.topology.uri()]
bc.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 10.

#linear solver parameters
ns_solver.VelocityLSS.LSS.SolutionStrategy.print_settings = False
lss = ns_solver.VelocityLSS.LSS

lss.SolutionStrategy.preconditioner_reset = 20
lss.SolutionStrategy.Parameters.preconditioner_type = 'Ifpack'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-6
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

# direct solve
#ns_solver.PressureLSS.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
#ns_solver.VelocityLSS.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'


# Time setup
time = model.create_time()
time.options().set('time_step', 0.01)

# Setup a time series write
final_end_time = 1.
save_interval = 0.1
iteration = 0

while time.end_time < final_end_time:
  time.end_time += save_interval
  model.simulate()
  domain.write_mesh(cf.URI('cavity-' +str(iteration) + '.pvtu'))
  iteration += 1
  if iteration == 1:
    solver.options().set('disabled_actions', ['InitialConditions'])

# print timings
model.print_timing_tree()
