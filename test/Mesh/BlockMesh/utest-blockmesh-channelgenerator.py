import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.configure_option('assertion_backtrace', False)
env.configure_option('exception_backtrace', False)
env.configure_option('regist_signal_handlers', False)
env.configure_option('exception_log_level', 0)
#env.configure_option('log_level', 4)
env.configure_option('exception_outputs', False)

model = root.create_component('model', 'CF.Solver.CModel')
model.create_domain()
domain = model.get_child('Domain')
generator = domain.create_component('generator', 'CF.Mesh.BlockMesh.ChannelGenerator')
generator.configure_option('parent', domain.uri())

generator.configure_option('x_segments', 64)
generator.execute()

domain.create_component('writer', 'CF.Mesh.VTKXML.CWriter')
domain.write_mesh(cf.URI('utest-blockmesh-channelgenerator_output.pvtu'))
