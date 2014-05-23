import coolfluid as cf

cf.env.log_level = 4
cf.env.assertion_throws = False

domain = cf.root.create_component('Domain', 'cf3.mesh.Domain')
generator = domain.create_component('Generator', 'cf3.mesh.GeneratePlane3D')
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')


generator.mesh = mesh.uri()
generator.origin = [1., 2., 3.]
generator.normal = [1., 1., 1.]
generator.size = [2., 5.]
generator.segments = [4, 7]
generator.execute()

domain.write_mesh(cf.URI('plane3d.msh'))