import sys
import coolfluid as cf
import numpy as np
try:
  import pylab as pl
  have_pylab = True
except:
  have_pylab = False

env = cf.Core.environment()

# Global confifuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 1

class TaylorGreen:
  Ua = 0.
  Va = 0.
  D = 0.5
  t = 0.
  
  max_error = np.array([])
  
  model = None
  mesh = None
  
  
  def setup(self, segments, Ua, Va, D):
    if self.model != None:
      self.model.delete_component()
      
    self.Ua = Ua
    self.Va = Va
    self.D = D
    
    model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
    self.model = model
    domain = model.create_domain()
    physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
    solver = model.create_solver('cf3.UFEM.Solver')
    ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
    
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

    ns_solver.regions = [mesh.topology.interior.uri()]

    solver.create_fields()

    #initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
    ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
    ic_u.variable_name = 'Velocity'
    ic_u.regions = [mesh.topology.interior.uri()]
    ic_u.value = ['{Ua} - cos(pi/{D}*x)*sin(pi/{D}*y)'.format(Ua = Ua, D = D), '{Va} + sin(pi/{D}*x)*cos(pi/{D}*y)'.format(Va = Va, D = D)]

    ic_p = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_solution')
    ic_p.regions = [mesh.topology.interior.uri()]
    ic_p.variable_name = 'Pressure'
    ic_p.value = ['-0.25*(cos(2*pi/{D}*x) + cos(2*pi/{D}*y))'.format(D = D)]

    ic_u.execute()
    ic_p.execute()
    domain.write_mesh(cf.URI('taylor-green-init.pvtu'))

    # Physical constants
    physics.density = 1.
    physics.dynamic_viscosity = 0.0001
    physics.reference_velocity = 1.
  
  def iterate(self, tstep, numsteps, save_interval = 1):
    
    if (numsteps % save_interval) != 0:
      raise RuntimeError('Number of time steps cannot be divided by save_interval')
    
    # Time setup
    time = self.model.create_time()
    time.time_step = tstep

    # Setup a time series write
    final_end_time = numsteps*tstep
    self.iteration = 0
    time.end_time = 0.
    
    # Resize the error array
    self.max_error = np.zeros((3, numsteps/save_interval))

    while time.current_time < final_end_time:
      time.end_time += save_interval*tstep
      self.model.simulate()
      self.check_result()
      self.model.Domain.write_mesh(cf.URI('taylor-green-' +str(self.iteration) + '.pvtu'))
      self.iteration += 1
      self.t = time.current_time
      if self.iteration == 1:
        self.model.Solver.options.disabled_actions = ['InitialConditions']

    # print timings
    self.model.print_timing_tree()


  def check_result(self):
    t = self.t
    Ua = self.Ua
    Va = self.Va
    D = self.D
    
    nu = self.model.NavierStokesPhysics.kinematic_viscosity
    
    sol = self.mesh.geometry.navier_stokes_solution
    coords = self.mesh.geometry.coordinates
    x_arr = np.zeros(len(coords))
    y_arr = np.zeros(len(coords))
    p_num = np.zeros(len(coords))
    u_num = np.zeros(len(coords))
    v_num = np.zeros(len(coords))
    p_th = np.zeros(len(coords))
    u_th = np.zeros(len(coords))
    v_th = np.zeros(len(coords))
    for i in range(len(coords)):
      (x, y) = (x_arr[i], y_arr[i]) = coords[i]
      (u_num[i], v_num[i], p_num[i]) = sol[i]
      u_th[i] = Ua - np.cos(np.pi/D*(x-Ua*t))*np.sin(np.pi/D*(y-Va*t))*np.exp(-2.*nu*np.pi**2/D**2*t)
      v_th[i] = Va + np.sin(np.pi/D*(x-Ua*t))*np.cos(np.pi/D*(y-Va*t))*np.exp(-2.*nu*np.pi**2/D**2*t)
      p_th[i] = -0.25 * (np.cos(2*np.pi/D*(x - Ua*t)) + np.cos(2*np.pi/D*(x - Ua*t)))*np.exp(-4.*nu*np.pi**2/D**2*t)
    
    print p_th
    print u_th
    print v_th
    print p_num
    print u_num
    print v_num
      
    self.max_error[0, self.iteration] = np.max(np.abs(u_th - u_num))
    self.max_error[1, self.iteration] = np.max(np.abs(v_th - v_num))
    self.max_error[2, self.iteration] = np.max(np.abs(p_th - p_num))


taylor_green = TaylorGreen()
taylor_green.setup(40, 0.3, 0.2, 0.5)
taylor_green.iterate(0.01, 100)
print taylor_green.max_error