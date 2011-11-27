import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.options().configure_option('assertion_backtrace', False)
env.options().configure_option('exception_backtrace', False)
env.options().configure_option('regist_signal_handlers', False)
env.options().configure_option('exception_log_level', 0)
#env.options().configure_option('log_level', 4)
env.options().configure_option('exception_outputs', False)

model = root.create_component('model', 'cf3.solver.CModel')
model.create_domain()
domain = model.get_child('Domain')
generator = domain.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().configure_option('mesh', cf.URI('//model/Domain/mesh'))

generator.options().configure_option('x_segments', 64)
generator.execute()

domain.create_component('writer', 'cf3.mesh.VTKXML.Writer')
domain.write_mesh(cf.URI('utest-blockmesh-channelgenerator_output.pvtu'))
