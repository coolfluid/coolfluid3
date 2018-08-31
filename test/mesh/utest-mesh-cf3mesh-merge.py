import sys
import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True
env.assertion_throws = False

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')

reader = domain.create_component('CF3MeshReader', 'cf3.mesh.cf3mesh.Reader')
reader.mesh = domain.create_component('ReadBackMesh','cf3.mesh.Mesh')
reader.file = cf.URI('cf3test.cf3mesh')
reader.execute()

reader.mesh.print_tree()

domain.write_mesh(cf.URI('cf3test-merged.pvtu'))
