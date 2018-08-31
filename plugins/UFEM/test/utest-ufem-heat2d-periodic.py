import sys
import coolfluid as cf

# Global configuration
cf.env.assertion_backtrace = False
cf.env.exception_backtrace = True
cf.env.regist_signal_handlers = False
cf.env.exception_log_level = 0
cf.env.log_level = 4
cf.env.exception_outputs = False

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')

# load the mesh (passed as first argument to the script)
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = domain.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [20,20]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]

blocks.create_mesh(mesh.uri())

partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
partitioner.mesh = mesh

link_horizontal = partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0.]

partitioner.execute()
# link_horizontal.mesh = mesh
# link_horizontal.execute()

make_par_data = domain.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

domain.write_mesh(cf.URI('heat2d-periodic-input.pvtu'))

hc.regions = [mesh.topology.interior.uri()]

writer = domain.create_component('MshWriter', 'cf3.mesh.gmsh.Writer')
writer.enable_overlap = True
writer.mesh = mesh
writer.fields = [mesh.geometry.node_gids.uri()]
writer.file = cf.URI('heat2d-periodic-input.msh')
writer.execute()

# Boundary conditions
bc = hc.BoundaryConditions
bc.regions = [mesh.topology.uri()]
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Temperature').value = 10
bc.add_constant_bc(region_name = 'top', variable_name = 'Temperature').value = 30

# run the simulation
model.simulate()

# Write result
writer.fields = [mesh.geometry.heat_conduction_solution.uri()]
writer.file = cf.URI('heat2d-periodic.msh')
writer.execute()
domain.write_mesh(cf.URI('heat2d-periodic.pvtu'))
