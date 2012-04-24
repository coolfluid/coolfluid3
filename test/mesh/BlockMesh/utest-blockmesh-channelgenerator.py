import coolfluid as cf
import sys

root = cf.Core.root()
env = cf.Core.environment()

env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('exception_log_level', 0)
#env.options().set('log_level', 4)
env.options().set('exception_outputs', False)

model = root.create_component('model', 'cf3.solver.Model')
model.create_domain()
domain = model.get_child('Domain')
generator = domain.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().set('mesh', cf.URI('//model/Domain/mesh'))

generator.options().set('x_segments', 64)
generator.execute()

domain.create_component('writer', 'cf3.mesh.VTKXML.Writer')
domain.write_mesh(cf.URI('utest-blockmesh-channelgenerator_output.pvtu'))
