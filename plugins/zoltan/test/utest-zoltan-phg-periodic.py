import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True
env.exception_backtrace = False
env.assertion_backtrace = False
#env.assertion_throws = False

#cf.Core.wait_for_debugger(1)

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')


mesh_generator = domain.create_component("MeshGenerator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.mesh = mesh.uri()
mesh_generator.nb_cells = [20,20]
mesh_generator.lengths = [1.,1.]
mesh_generator.offsets = [0.,0.]
mesh_generator.execute()

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

domain.write_mesh(cf.URI('periodic-input.pvtu'))

make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

link_horizontal = domain.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
link_horizontal.mesh = mesh
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-1., 0.]
link_horizontal.execute()



link_vertical = domain.create_component('LinkVertical', 'cf3.mesh.actions.LinkPeriodicNodes')
link_vertical.mesh = mesh
link_vertical.source_region = mesh.topology.top
link_vertical.destination_region = mesh.topology.bottom
link_vertical.translation_vector = [0., -1.]
link_vertical.execute()

my_rank = cf.Core.rank()
ranks = mesh.geometry.children.rank
periodic_links_active = mesh.geometry.children.periodic_links_active
print periodic_links_active
my_nb_nodes = 0
for (r, active) in zip(ranks, periodic_links_active):
  if r == my_rank and not active:
    my_nb_nodes += 1

print 'nodes (non-periodic and non-ghost) for rank before:', my_rank, ':', my_nb_nodes

#link_depth = domain.create_component('LinkDepth', 'cf3.mesh.actions.LinkPeriodicNodes')
#link_depth.mesh = mesh
#link_depth.source_region = mesh.topology.back
#link_depth.destination_region = mesh.topology.front
#link_depth.translation_vector = [0., 0., -1.]
#link_depth.execute()

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

my_rank = cf.Core.rank()
ranks = mesh.geometry.children.rank
periodic_links_active = mesh.geometry.children.periodic_links_active
print periodic_links_active
my_nb_nodes = 0
for (r, active) in zip(ranks, periodic_links_active):
  if r == my_rank and not active:
    my_nb_nodes += 1

print 'nodes (non-periodic and non-ghost) for rank after:', my_rank, ':', my_nb_nodes

make_par_data.execute()

domain.write_mesh(cf.URI('periodic.pvtu'))

