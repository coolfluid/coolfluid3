import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

def run_case(modeltype):
  implicit = modeltype == 'implicit'
  # setup a model
  model = root.create_component('NavierStokes'+modeltype, 'cf3.solver.ModelUnsteady')
  domain = model.create_domain()
  physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
  solver = model.create_solver('cf3.UFEM.Solver')

  # Add the Navier-Stokes solver as an unsteady solver
  if implicit:
    ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
  else:
    ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokesSemiImplicit')
    ns_solver.options.pressure_rcg_solve = True
    ns_solver.enable_body_force = True

  # Generate mesh
  blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
  points = blocks.create_points(dimensions = 2, nb_points = 6)
  points[0]  = [0, 0.]
  points[1]  = [10., 0.]
  points[2]  = [0., 0.5]
  points[3]  = [10., 0.5]
  points[4]  = [0.,1.]
  points[5]  = [10., 1.]

  block_nodes = blocks.create_blocks(2)
  block_nodes[0] = [0, 1, 3, 2]
  block_nodes[1] = [2, 3, 5, 4]

  block_subdivs = blocks.create_block_subdivisions()
  block_subdivs[0] = [40, 20]
  block_subdivs[1] = [40, 20]

  gradings = blocks.create_block_gradings()
  gradings[0] = [1., 1., 10., 10.]
  gradings[1] = [1., 1., 0.1, 0.1]

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

  if not implicit:
    link_horizontal = domain.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
    link_horizontal.mesh = mesh
    link_horizontal.source_region = mesh.topology.right
    link_horizontal.destination_region = mesh.topology.left
    link_horizontal.translation_vector = [-10., 0.]
    link_horizontal.execute()

  ns_solver.regions = [mesh.topology.uri()]

  if implicit:
    lss = ns_solver.LSS
    lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'aggregation: aux: enable', value = True)
    lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'aggregation: aux: threshold', value = 0.0001)
  else:
    for strat in [ns_solver.children.FirstPressureStrategy, ns_solver.children.SecondPressureStrategy]:
      strat.MLParameters.aggregation_type = 'Uncoupled'
      strat.MLParameters.max_levels = 4
      strat.MLParameters.smoother_sweeps = 3
      strat.MLParameters.coarse_type = 'Amesos-KLU'
      strat.MLParameters.add_parameter(name = 'repartition: start level', value = 0)
      strat.MLParameters.add_parameter(name = 'repartition: max min ratio', value = 1.1)
      strat.MLParameters.add_parameter(name = 'aggregation: aux: enable', value = True)
      strat.MLParameters.add_parameter(name = 'aggregation: aux: threshold', value = 0.05)
    lss = ns_solver.VelocityLSS.LSS
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-6
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 300
    lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 100

  u_in = [1., 0.]

  solver.create_fields()

  #initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
  if implicit:
    solver.InitialConditions.navier_stokes_solution.Velocity = u_in
  else:
    ic_u = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
    ic_u.variable_name = 'Velocity'
    ic_u.regions = [mesh.topology.uri()]
    ic_u.value = ['0', '0']
    ic_g = solver.InitialConditions.NavierStokes.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
    ic_g.variable_name = 'Force'
    ic_g.regions = [mesh.topology.uri()]
    ic_g.value = ['2', '0']

  # Physical constants
  physics.options().set('density', 1000.)
  physics.options().set('dynamic_viscosity', 10.)

  # Boundary conditions
  if implicit:
    bc_u = ns_solver.BoundaryConditions
    bc_p = bc_u
    bc_u.add_constant_bc(region_name = 'left', variable_name = 'Velocity').value = u_in
    bc_p.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.
  else:
    bc_u = ns_solver.VelocityLSS.BC
    bc_p = ns_solver.PressureLSS.BC

  bc_u.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value = [0., 0.]
  bc_u.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value = [0., 0.]

  # Time setup
  time = model.create_time()
  time.time_step = 0.1
  time.end_time = 2.*time.time_step

  model.simulate()
  domain.write_mesh(cf.URI('mlaux-{type}.pvtu'.format(type=modeltype)))

  model.print_timing_tree()

run_case('implicit')
run_case('semi')
