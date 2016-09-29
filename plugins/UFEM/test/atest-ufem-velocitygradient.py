import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.log_level = 4

# setup a model
model = cf.Core.root().create_component('GradModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
grad = solver.add_direct_solver('cf3.UFEM.VelocityGradient')

# Generate a channel mesh
generator = domain.create_component('generator', 'cf3.mesh.BlockMesh.ChannelGenerator')
generator.options().set('mesh', cf.URI('//GradModel/Domain/mesh'))
generator.options().set('x_segments', 64)
generator.options().set('cell_overlap', 1)
generator.execute()

mesh = domain.mesh


grad.regions = [mesh.topology.uri()]
solver.create_fields()

velocity = mesh.geometry.navier_stokes_u_solution
coords = mesh.geometry.coordinates
for i in range(len(coords)):
  [x,y,z] = coords[i]
  v = velocity[i]
  v[0] = x
  v[1] = y**2/2.
  v[2] = z**3/3.

# run the simulation
model.simulate()

# print timings
model.print_timing_tree()

# Write result
writer = domain.create_component('VTKWriter', 'cf3.vtk.MultiblockWriter')
writer.mesh = mesh
writer.file = cf.URI('velocitygradient_output.vtm')
writer.execute()
