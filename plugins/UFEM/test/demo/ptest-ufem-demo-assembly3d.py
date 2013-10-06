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
cf.env.log_level = 4
cf.env.assertion_throws = False
cf.env.assertion_backtrace = False
cf.env.exception_backtrace = False
cf.env.exception_outputs = False
cf.env.regist_signal_handlers = False
cf.env.log_level = 1

n = 16

measurement = ET.Element('DartMeasurement', name = 'Problem size', type = 'numeric/integer')
measurement.text = str(n)
print ET.tostring(measurement)

profiler = Profiler()

# We loop over all available implementations, to test them all
for lss_name in ['EmptyLSS', 'TrilinosCrs']:
  for modelname in ['Proto']:
    # Setup a model
    model = cf.Core.root().create_component(modelname+'Model', 'cf3.solver.Model')
    # The domain holds the mesh
    domain = model.create_domain()
    # Physical model keeps track of all the variables in the problem and any material properties (absent here)
    physics = model.create_physics('cf3.physics.DynamicModel')
    # Manager for finite element solvers
    solver = model.create_solver('cf3.UFEM.Solver')
    # Add a solver for the Poisson problem
    poisson_solver = solver.add_direct_solver('cf3.UFEM.demo.Poisson'+modelname)
    poisson_solver.matrix_builder = 'cf3.math.LSS.{name}Matrix'.format(name=lss_name)

    # Generate a unit square
    mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
    mesh_generator = domain.create_component("MeshGenerator","cf3.mesh.SimpleMeshGenerator")
    mesh_generator.mesh = mesh.uri()
    mesh_generator.nb_cells = [n,n,n]
    mesh_generator.lengths = [1.,1.,1.]
    mesh_generator.offsets = [0.,0.,0.]
    mesh_generator.execute()

    # Triangulate it
    try:
      triangulator = domain.create_component('triangulator', 'cf3.vtk.Tetrahedralize')
      triangulator.mesh = mesh
      triangulator.execute()
    except:
      print 'Tetrahedralizer not found, running using Hexas'

    # Set the region over which the solver operates
    poisson_solver.regions = [mesh.topology.uri()]
    # No solve for the assembly benchmark
    poisson_solver.options.disabled_actions = ['SolveLSS']

    # Boundary conditions
    bc = poisson_solver.BoundaryConditions.add_function_bc(region_name = 'left', variable_name = 'u')
    bc.value = ['1 + x^2 + 2*y^2']
    bc.regions = [mesh.topology.left.uri(), mesh.topology.right.uri(), mesh.topology.top.uri(), mesh.topology.bottom.uri()]

    # Source term can be set using an initial condition
    ic_f = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionConstant', field_tag = 'source_term')
    ic_f.f = -6.
    ic_f.regions = [mesh.topology.uri()]

    # run the simulation to ensure all setup is OK
    model.simulate()
    # Profile only the assembly
    profiler.start('Poisson' + modelname + '-' + lss_name + '.pprof')
    for i in range(10):
      poisson_solver.Assembly.execute()
    profiler.stop()
    model.store_timings()
    
    try:
      assembly_time = poisson_solver.Assembly.properties()["timer_mean"]
      measurement = ET.Element('DartMeasurement', name = modelname + ' ' + lss_name + ' timing', type = 'numeric/double')
      measurement.text = str(assembly_time)
      print ET.tostring(measurement)
    except:
      pass
    model.delete_component()