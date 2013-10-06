import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True
env.exception_backtrace = False
env.assertion_backtrace = False

#cf.Core.wait_for_debugger(2)

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')


blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
blocks.options.overlap = 0
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40,20]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.extrude_blocks(positions=[1.], nb_segments=[20], gradings=[1.])
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(mesh.uri())

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

domain.write_mesh(cf.URI('periodic-input.pvtu'))

periodic_partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
periodic_partitioner.mesh = mesh
periodic_partitioner.load_balance = True

link_horizontal = periodic_partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0., 0.]
# link_horizontal.translation_vector = [-1., 0.]

link_vertical = periodic_partitioner.create_link_periodic_nodes()
link_vertical.source_region = mesh.topology.top
link_vertical.destination_region = mesh.topology.bottom
link_vertical.translation_vector = [0., -1., 0.]
# link_vertical.translation_vector = [0., -1.]

link_depth = periodic_partitioner.create_link_periodic_nodes()
link_depth.source_region = mesh.topology.back
link_depth.destination_region = mesh.topology.front
link_depth.translation_vector = [0., 0., -1.]

periodic_partitioner.execute()

make_par_data.execute()

domain.write_mesh(cf.URI('periodic.pvtu'))

