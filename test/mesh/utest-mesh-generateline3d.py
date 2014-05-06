import coolfluid as cf

cf.env.log_level = 4
cf.env.assertion_throws = False

domain = cf.root.create_component('Domain', 'cf3.mesh.Domain')
generator = domain.create_component('Generator', 'cf3.mesh.GenerateLine3D')
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')


generator.mesh = mesh.uri()
generator.origin = [1., 2., 3.]
generator.end = [4., 5., 6.]
generator.segments = 10
generator.execute()

for (x,y,z) in mesh.geometry.coordinates:
    print x, y, z
