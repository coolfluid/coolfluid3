import coolfluid as cf
import xml.etree.ElementTree as ET

# Global configuration
cf.env.log_level = 4
cf.env.assertion_throws = False
cf.env.assertion_backtrace = False
cf.env.exception_backtrace = False
cf.env.exception_outputs = False
cf.env.regist_signal_handlers = False

n = 10

# Print out the problem size to the testing dashboard
measurement = ET.Element('DartMeasurement', name = 'Problem size', type = 'numeric/integer')
measurement.text = str(n)
print ET.tostring(measurement)

# Setup a model
model = cf.Core.root().create_component('Model', 'cf3.solver.ModelUnsteady')
# The domain holds the mesh
domain = model.create_domain()
# Physical model keeps track of all the variables in the problem and any material properties (absent here)
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
physics.density = 1.
physics.dynamic_viscosity = 1.
# Manager for finite element solvers
solver = model.create_solver('cf3.UFEM.Solver')
ns_solver = solver.add_unsteady_solver('cf3.UFEM.demo.NavierStokesChorin')

# Generate a 2D channel mesh
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

# Initial conditions
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.options.field_space_name = 'cf3.mesh.LagrangeP2'
ic_u.value = ['0', '0']
ic_u.regions = ns_solver.regions

ic_p = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_p_solution')
ic_p.variable_name = 'Pressure'
ic_p.value = ['20-2*x']
ic_p.regions = ns_solver.regions

# Boundary conditions
ns_solver.VelocityBC.regions = ns_solver.regions
ns_solver.PressureBC.regions = ns_solver.regions
# Set no-slip at the walls
bc_wall = ns_solver.VelocityBC.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.BCDirichletFunction')
bc_wall.solving_for_difference = False
bc_wall.variable_name = 'Velocity'
bc_wall.field_tag = 'navier_stokes_u_solution'
bc_wall.space = 'cf3.mesh.LagrangeP2'
bc_wall.value = ['0', '0']
bc_wall.regions = [mesh.topology.top.uri(), mesh.topology.bottom.uri()]
# Imposed pressure difference
bc_p_out = ns_solver.PressureBC.create_bc_action(region_name = 'right', builder_name = 'cf3.UFEM.BCDirichletFunction')
bc_p_out.solving_for_difference = False
bc_p_out.variable_name = 'Pressure'
bc_p_out.field_tag = 'navier_stokes_p_solution'
bc_p_out.value = ['0']
bc_p_in = ns_solver.PressureBC.create_bc_action(region_name = 'left', builder_name = 'cf3.UFEM.BCDirichletFunction')
bc_p_in.solving_for_difference = False
bc_p_in.variable_name = 'Pressure'
bc_p_in.field_tag = 'navier_stokes_p_solution'
bc_p_in.value = ['20']

# Create the fields, to ensure LSS creation
solver.create_fields()
# LSS options
lss = ns_solver.children.AuxiliaryLSS.create_lss()
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'SA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'MIS'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 2
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'both'

# Timestepping
time = model.create_time()
time.time_step = 0.1
time.end_time = 10.*time.time_step

# Run the simulation
model.simulate()

# Print timings
model.print_timing_tree()

# Save result
domain.write_mesh(cf.URI('chorin.msh'))