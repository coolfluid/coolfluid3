import coolfluid as cf

root = cf.root
cf.env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.KEpsilonPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the unsteady body force. This should be added before the NavierStokes solver to make sure the body force is updated before the solution step
body_force = solver.add_unsteady_solver('cf3.UFEM.InitialConditionFunction')

nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Generate a simple rectangle
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0.0, 0.0]
points[1]  = [1.0, 0.0]
points[2]  = [1.0, 1.0]
points[3]  = [0.0, 1.0]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [1,1]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
walls_patch = blocks.create_patch_nb_faces(name = 'walls', nb_faces = 4)
walls_patch[0] = [0, 1]
walls_patch[1] = [1, 2]
walls_patch[2] = [2, 3]
walls_patch[3] = [3, 0]
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# Set the body force on the entire mesh
body_force.regions = [mesh.topology.uri()]
nstokes.regions = [mesh.topology.uri()]

# Setup of the values must happen after the regions are set (can't know mesh dimensionality otherwise)
body_force.options.field_tag = 'body_force'
body_force.variable_name = 'Force'
body_force.value = ['0', 't'] # Set the time itself as Y-component

# Probe in the center of the domain
probe0 = solver.add_probe(name = 'Probe', parent = solver.TimeLoop, dict = mesh.geometry)
probe0.Log.variables = ['Force[1]']
probe0.coordinate = [0.5,0.5]
probe0.History.file = cf.URI('atest-unsteady-bodyforce-probe.tsv')

# Time setup
time = model.create_time()
time.time_step = 1.0
time.end_time = 10.001

# Disable NavierStokes for this test, to test only the body force
solver.TimeLoop.options.disabled_actions = ['NavierStokes']

# Run the simulation
model.simulate()

# Write final result
domain.write_mesh(cf.URI('atest-unsteady-bodyforce.pvtu'))

model.print_timing_tree()
