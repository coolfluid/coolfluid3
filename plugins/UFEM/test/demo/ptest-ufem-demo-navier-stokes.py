import coolfluid as cf
import xml.etree.ElementTree as ET

# Helper class to profile only if the profiler is available
class Profiler:
  def __init__(self):
    self.profiler = None
    try:
      self.profiler = cf.Core.root().create_component('Profiler', 'cf3.Tools.GooglePerfTools.GooglePerfProfiling')
      print 'Profiling is on'
    except:
      print 'GooglePerfTools not found, not profiling'
      
  def start(self, filename):
    if self.profiler == None:
      return
    self.profiler.file_path = cf.URI(filename)
    self.profiler.start_profiling()
    
  def stop(self):
    if self.profiler == None:
      return
    self.profiler.stop_profiling()

# Global configuration
cf.env.log_level = 1
cf.env.assertion_throws = False
cf.env.assertion_backtrace = False
cf.env.exception_backtrace = False
cf.env.exception_outputs = False
cf.env.regist_signal_handlers = False

n = 400

measurement = ET.Element('DartMeasurement', name = 'Problem size', type = 'numeric/integer')
measurement.text = str(n)
print ET.tostring(measurement)

profiler = Profiler()

# We loop over all available implementations, to test them all
# Loop over LSS matrix implementations
for lss_name in ['EmptyLSS', 'TrilinosFEVbr', 'TrilinosCrs']:
  for modelname in ['Manual', 'Specialized', 'Specialized2', 'Proto']:
    # Setup a model
    model = None
    model = cf.Core.root().create_component(modelname+'Model', 'cf3.solver.ModelUnsteady')
    # The domain holds the mesh
    domain = model.create_domain()
    # Physical model keeps track of all the variables in the problem and any material properties (absent here)
    physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
    physics.density = 1.
    physics.dynamic_viscosity = 1.
    # Manager for finite element solvers
    solver = model.create_solver('cf3.UFEM.Solver')
    ns_solver = None
    if modelname == 'Specialized2':
      ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
      ns_solver.options.use_specializations = True
    elif modelname == 'Proto':
      ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')
      ns_solver.options.use_specializations = False
    else:
      ns_solver = solver.add_unsteady_solver('cf3.UFEM.demo.NavierStokes' + modelname)
    ns_solver.matrix_builder = 'cf3.math.LSS.{name}Matrix'.format(name=lss_name)

    # Generate a unit square
    mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
    mesh_generator = domain.create_component("MeshGenerator","cf3.mesh.SimpleMeshGenerator")
    mesh_generator.mesh = mesh.uri()
    mesh_generator.nb_cells = [n,n]
    mesh_generator.lengths = [10.,2.]
    mesh_generator.offsets = [0.,0.]
    mesh_generator.execute()

    # Triangulate it
    triangulator = domain.create_component('triangulator', 'cf3.mesh.MeshTriangulator')
    triangulator.mesh = mesh
    triangulator.execute()

    # Set the region over which the solver operates
    ns_solver.regions = [mesh.topology.uri()]
    # No solve for the assembly benchmark
    ns_solver.options.disabled_actions = ['SolveLSS']

    # Boundary conditions
    # Set no-slip at the walls
    bc_wall = ns_solver.BoundaryConditions.add_constant_bc(region_name = 'top', variable_name = 'Velocity')
    bc_wall.value = [0., 0.]
    bc_wall.regions = [mesh.topology.top.uri(), mesh.topology.bottom.uri()]
    # Parabolic inlet
    ns_solver.BoundaryConditions.add_function_bc(region_name = 'left', variable_name = 'Velocity').value = ['y*(2-y)', '0']
    # Fix outlet pressure
    ns_solver.BoundaryConditions.add_constant_bc(region_name = 'right', variable_name = 'Pressure').value = 0.

    # Take one time step
    time = model.create_time()
    time.time_step = 0.1
    time.end_time = 10.*time.time_step

    # run the simulation to ensure all setup is OK
    profiler.start('NS' + modelname + '-' + lss_name + '.pprof')
    model.simulate()
    profiler.stop()
    ns_solver.store_timings()
    
    try:
      assembly_time = ns_solver.Assembly.properties()["timer_mean"]
      measurement = ET.Element('DartMeasurement', name = modelname + ' ' + lss_name + ' timing', type = 'numeric/double')
      measurement.text = str(assembly_time)
      print ET.tostring(measurement)
    except:
      print 'no timing info found'
      
    model.delete_component()