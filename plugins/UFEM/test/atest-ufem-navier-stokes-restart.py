import sys
import coolfluid as cf
import numpy as np
import os
from optparse import OptionParser

env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

class TaylorGreen:
  Ua = 0.
  Va = 0.
  D = 0.5
  
  model = None
  solver = None
  mesh = None
  
  
  def __init__(self, builder, dt, element, prefix):
    self.dt = dt
    self.element = element
    self.prefix = prefix
    self.setup_model(builder)
    self.builder = builder
    
  def __del__(self):
    if self.model != None:
      self.model.delete_component()
  
  def create_mesh(self, segments):
    domain = self.model.Domain
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
    self.mesh = mesh
    
    blocks.create_mesh(mesh.uri())
    
    create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
    create_point_region.coordinates = [0., 0.]
    create_point_region.region_name = 'center'
    create_point_region.mesh = mesh
    create_point_region.execute()
    
    if self.element == 'triag':
      triangulator = domain.create_component('Triangulator', 'cf3.mesh.MeshTriangulator')
      triangulator.mesh = mesh
      triangulator.execute()
    
    partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
    partitioner.mesh = mesh

    link_horizontal = partitioner.create_link_periodic_nodes()
    link_horizontal.source_region = mesh.topology.right
    link_horizontal.destination_region = mesh.topology.left
    link_horizontal.translation_vector = [-1., 0.]

    link_vertical = partitioner.create_link_periodic_nodes()
    link_vertical.source_region = mesh.topology.top
    link_vertical.destination_region = mesh.topology.bottom
    link_vertical.translation_vector = [0., -1.]

    partitioner.execute()
    
    domain.write_mesh(cf.URI(self.prefix + '.cf3mesh'))
    
  def read_mesh(self, filename):
    reader = self.model.Domain.create_component('CF3MeshReader', 'cf3.mesh.cf3mesh.Reader')
    reader.mesh = self.model.Domain.create_component('Mesh','cf3.mesh.Mesh')
    reader.file = cf.URI(filename)
    reader.execute()
    self.mesh = reader.mesh
    
  def setup_ic(self):
    if self.builder == 'cf3.UFEM.NavierStokes':
      ic_comp = self.solver.InitialConditions
      u_tag = 'navier_stokes_solution'
      p_tag = 'navier_stokes_solution'
    else:
      ic_comp = self.solver.InitialConditions.NavierStokes
      u_tag = 'navier_stokes_u_solution'
      p_tag = 'navier_stokes_p_solution'
    #initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
    ic_u = ic_comp.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = u_tag)
    ic_u.variable_name = 'Velocity'
    ic_u.regions = [self.mesh.topology.interior.uri()]
    ic_u.value = ['{Ua} - cos(pi/{D}*x)*sin(pi/{D}*y)'.format(Ua = self.Ua, D = self.D), '{Va} + sin(pi/{D}*x)*cos(pi/{D}*y)'.format(Va = self.Va, D = self.D)]

    ic_p = ic_comp.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = p_tag)
    ic_p.regions = [self.mesh.topology.interior.uri()]
    ic_p.variable_name = 'Pressure'
    ic_p.value = ['-0.25*(cos(2*pi/{D}*x) + cos(2*pi/{D}*y))'.format(D = self.D)]
    
  def set_restart(self, filename):
    if self.builder == 'cf3.UFEM.NavierStokes':
      reader = self.solver.InitialConditions.create_component('Reader', 'cf3.solver.actions.ReadRestartFile')
    else:
      reader = self.solver.InitialConditions.Restarts.create_component('Reader', 'cf3.solver.actions.ReadRestartFile')
    reader.mesh = self.mesh
    reader.file = cf.URI(filename)
  
  def setup_model(self, builder):
    if self.model != None:
      self.model.delete_component()
    
    model = cf.Core.root().create_component('NavierStokes'+self.prefix, 'cf3.solver.ModelUnsteady')
    self.model = model
    domain = model.create_domain()
    physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
    self.solver = model.create_solver('cf3.UFEM.Solver')
    self.ns_solver = self.solver.add_unsteady_solver(builder)
    self.restart_writer = self.solver.add_restart_writer()
    self.restart_writer.Writer.file = cf.URI(self.prefix+'-{iteration}.cf3restart')
    
    # Physical constants
    physics.density = 1.
    physics.dynamic_viscosity = 0.001
    
    return self.solver

  def add_pressure_bc(self, bc, var_name = 'Pressure'):
    bc.regions = [self.mesh.topology.uri()]
    nu = self.model.NavierStokesPhysics.kinematic_viscosity
    bc.add_function_bc(region_name = 'center', variable_name = var_name).value = ['-0.25 * (cos(2*pi/{D}*(x - {Ua}*(t+{dt}))) + cos(2*pi/{D}*(y - {Va}*(t+{dt})))) * exp(-4*{nu}*pi^2/{D}^2*(t+{dt})) '.format(D = self.D, nu = nu, Ua = self.Ua, Va = self.Va, dt = self.dt)]
  
  def setup(self, Ua, Va, D, theta):
    self.Ua = Ua
    self.Va = Va
    self.D = D
    
    mesh = self.mesh
    self.ns_solver.regions = [mesh.topology.interior.uri()]
    self.ns_solver.options.theta = theta
    self.theta = theta
    
    series_writer = self.solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
    writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
    writer.file = cf.URI(self.prefix+'-{iteration}.pvtu')
    writer.mesh = self.mesh
    
    if self.builder == 'cf3.UFEM.NavierStokes':
      self.add_pressure_bc(self.ns_solver.BoundaryConditions)
      self.solver.create_fields()
      writer.fields = [mesh.geometry.navier_stokes_solution.uri()]
      lss = self.ns_solver.LSS
      lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
      lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-14
      lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.eigen_analysis_type = 'Anorm'
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'PDE equations', value =3)
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled-MIS'
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'Chebyshev'#'symmetric block Gauss-Seidel'
      lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 3

    else:
      self.ns_solver.PressureLSS.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
      self.ns_solver.VelocityLSS.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
      self.ns_solver.PressureLSS.LSS.SolutionStrategy.print_settings = False
      self.ns_solver.VelocityLSS.LSS.SolutionStrategy.print_settings = False
      
      self.add_pressure_bc(self.ns_solver.PressureLSS.BC)

      self.solver.create_fields()
      writer.fields = [mesh.geometry.navier_stokes_p_solution.uri(), mesh.geometry.navier_stokes_u_solution.uri()]

  def iterate(self, numsteps, save_interval = 1):    
    tstep = self.dt
    self.restart_writer.interval = save_interval
    
    # Time setup
    time = self.model.create_time()
    time.time_step = tstep
    if self.builder == 'cf3.UFEM.NavierStokes':
      time.end_time = numsteps*tstep
      self.model.simulate()
    else:
      time.end_time = 0.
      while time.end_time < numsteps*tstep:
        time.end_time += save_interval*tstep
        self.model.simulate()
        self.solver.InitialConditions.options.disabled_actions = ['NavierStokes', 'linearized_velocity']
    self.model.print_timing_tree()
    
  def iterate_restart(self, end_time, save_interval = 1):    
    self.restart_writer.interval = save_interval
    
    # Time setup
    time = self.model.create_time()
    time.end_time = end_time
    self.model.simulate()
    self.model.print_timing_tree()

dt = 0.004
elem = 'quad'
segs = 32
theta = 0.5

# Run a simulation, saving restarts every 5 steps
tg_impl = TaylorGreen(builder = 'cf3.UFEM.NavierStokes', dt = dt, element=elem, prefix='restart-implicit-full')
tg_impl.create_mesh(segs)
tg_impl.setup(0.3, 0.2, D=0.5, theta=theta)
tg_impl.setup_ic()
tg_impl.iterate(10, 5)

# Restart after the 5 first steps
tg_impl_restart = TaylorGreen(builder = 'cf3.UFEM.NavierStokes', dt = dt, element=elem, prefix='restart-implicit-restarted')
tg_impl_restart.read_mesh(tg_impl.prefix + '.cf3mesh')
tg_impl_restart.setup(0.3, 0.2, D=0.5, theta=theta)
tg_impl_restart.set_restart(tg_impl.prefix+'-5.cf3restart')
tg_impl_restart.iterate_restart(10.*dt, 5)

root = cf.Core.root()
meshdiff = root.create_component('MeshDiff', 'cf3.mesh.actions.MeshDiff')
meshdiff.left = tg_impl.mesh
meshdiff.right = tg_impl_restart.mesh
meshdiff.max_ulps = 1000000000 # this relatively high number is due to the use of the iterative LSS solver
meshdiff.execute()
if not meshdiff.properties()['mesh_equal']:
  raise Exception('Bad restart for implicit solve')

# Run a simulation, saving restarts every 5 steps
tg_semi_impl = TaylorGreen(builder = 'cf3.UFEM.NavierStokesSemiImplicit', dt = dt, element=elem, prefix='restart-semi-full')
tg_semi_impl.create_mesh(segs)
tg_semi_impl.setup(0.3, 0.2, D=0.5, theta=theta)
tg_semi_impl.setup_ic()
tg_semi_impl.iterate(10, 5)

# Restart after the 5 first steps
tg_semi_impl_restart = TaylorGreen(builder = 'cf3.UFEM.NavierStokesSemiImplicit', dt = dt, element=elem, prefix='restart-semi-restarted')
tg_semi_impl_restart.read_mesh(tg_semi_impl.prefix + '.cf3mesh')
tg_semi_impl_restart.setup(0.3, 0.2, D=0.5, theta=theta)
tg_semi_impl_restart.set_restart(tg_semi_impl.prefix+'-5.cf3restart')
tg_semi_impl_restart.iterate_restart(10.*dt, 5)

meshdiff.max_ulps = 1000000000
meshdiff.left = tg_semi_impl.mesh
meshdiff.right = tg_semi_impl_restart.mesh
meshdiff.execute()
if not meshdiff.properties()['mesh_equal']:
  raise Exception('Bad restart for semi-implicit solve')
