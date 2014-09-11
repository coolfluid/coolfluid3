import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True
env.exception_backtrace = False
env.assertion_backtrace = False
env.assertion_throws = False

#cf.Core.wait_for_debugger(0)

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

mesh_generator = domain.create_component("MeshGenerator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.mesh = mesh.uri()
mesh_generator.nb_cells = [20,20]
mesh_generator.lengths = [1.,1.]
mesh_generator.offsets = [0.,0.]
mesh_generator.execute()

p2space = mesh.create_continuous_space(name = 'TestP2Space', shape_function = 'cf3.mesh.LagrangeP2')

my_rank = cf.Core.rank()

ranks = mesh.geometry.children.rank
my_nb_nodes = 0
for r in ranks:
  if r == my_rank:
    my_nb_nodes += 1
  
print 'nodes for rank before:', my_rank, ':', my_nb_nodes

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh

partitioner.execute()

ranks = mesh.geometry.children.rank
my_nb_nodes = 0
for r in ranks:
  if r == my_rank:
    my_nb_nodes += 1
  
print 'nodes for rank after:', my_rank, ':', my_nb_nodes

make_par_data = root.create_component('MakeParData', 'cf3.solver.actions.ParallelDataToFields')
make_par_data.mesh = mesh
make_par_data.execute()

domain.write_mesh(cf.URI('phg-output.pvtu'))

writer = domain.create_component('PVWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.options.dictionary = p2space
writer.fields = [p2space.node_rank.uri(), mesh.children.elems_P0.element_rank.uri()]
writer.file = cf.URI('phg-p2out.pvtu')
writer.execute()

