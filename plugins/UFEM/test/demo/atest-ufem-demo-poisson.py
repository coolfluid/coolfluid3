import coolfluid as cf

# Global configuration
cf.env.log_level = 4

# Setup a model
model = cf.Core.root().create_component('PoissonModel', 'cf3.solver.Model')
# The domain holds the mesh
domain = model.create_domain()
# Physical model keeps track of all the variables in the problem and any material properties (absent here)
physics = model.create_physics('cf3.physics.DynamicModel')
# Manager for finite element solvers
solver = model.create_solver('cf3.UFEM.Solver')
# Add a solver for the Poisson problem
poisson_solver = solver.add_direct_solver('cf3.UFEM.demo.PoissonProto')

# Generate a unit square with 6x4 elements
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
mesh_generator = domain.create_component("MeshGenerator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.mesh = mesh.uri()
mesh_generator.nb_cells = [6,4]
mesh_generator.lengths = [1.,1.]
mesh_generator.offsets = [0.,0.]
mesh_generator.execute()

# Triangulate it
triangulator = domain.create_component('triangulator', 'cf3.mesh.MeshTriangulator')
triangulator.mesh = mesh
triangulator.execute()

# Set the region over which the solver operates
poisson_solver.regions = [mesh.topology.uri()]

# The default solution method is Belos Block GMRES
# Here we choose a direct solve for a mesh this small by setting the appropriate Trilinos parameter
poisson_solver.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'
poisson_solver.LSS.SolutionStrategy.print_settings = False

# Boundary conditions
bc = poisson_solver.BoundaryConditions.add_function_bc(region_name = 'left', variable_name = 'u')
bc.value = ['1 + x^2 + 2*y^2']
bc.regions = [mesh.topology.left.uri(), mesh.topology.right.uri(), mesh.topology.top.uri(), mesh.topology.bottom.uri()]

# Source term can be set using an initial condition
ic_f = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionConstant', field_tag = 'source_term')
ic_f.f = -6.
ic_f.regions = [mesh.topology.uri()]

# run the simulation
model.simulate()
model.print_timing_tree()

# Write result
domain.write_mesh(cf.URI('atest-ufem-demo-poisson.pvtu'))

# Check result
for [x,y], [u] in zip(mesh.geometry.coordinates, mesh.geometry.poisson_solution):
  if abs(1. + x**2 + 2.*y**2 - u) > 1e-12:
    raise Exception('Solution is incorrect')
