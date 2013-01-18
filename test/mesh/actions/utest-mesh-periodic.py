import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')


blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [100,100]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]
blocks.partition_blocks(nb_partitions = 2, direction = 0)
blocks.partition_blocks(nb_partitions = 2, direction = 1)
blocks.create_mesh(mesh.uri())

make_boundary_global = root.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

link_horizontal = root.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
link_horizontal.mesh = mesh
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0.]
link_horizontal.execute()

link_vertical = root.create_component('LinkVertical', 'cf3.mesh.actions.LinkPeriodicNodes')
link_vertical.mesh = mesh
link_vertical.source_region = mesh.topology.top
link_vertical.destination_region = mesh.topology.bottom
link_vertical.translation_vector = [0., -1.]
link_vertical.execute()

remove_ghost_elements = root.create_component('RemoveGhostElements', 'cf3.mesh.actions.RemoveGhostElements')
remove_ghost_elements.mesh = mesh
remove_ghost_elements.execute()

load_balancer = domain.create_component('LoadBalancer', 'cf3.mesh.actions.LoadBalance')
load_balancer.mesh = mesh
load_balancer.execute()

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

domain.write_mesh(cf.URI('periodic.pvtu'))
