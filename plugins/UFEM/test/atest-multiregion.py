import sys
import coolfluid as cf

# This test runs two separate heat conduction problems on two different regions of the same mesh

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global confifuration
env.options().set('assertion_throws', False)
env.options().set('assertion_backtrace', False)
env.options().set('exception_backtrace', False)
env.options().set('regist_signal_handlers', False)
env.options().set('log_level', 4)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc_bottom = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')
hc_top = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')

# Generate mesh (1x1 square, cut in half horizontally)
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [1, 0.]
points[2]  = [0., 0.5]
points[3]  = [1., 0.5]
points[4]  = [0.,1.]
points[5]  = [1., 1.]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [40, 20]
block_subdivs[1] = [40, 20]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]

# Put each block in a different region
blocks.options().set('block_regions', ['solid_bottom', 'solid_top'])

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

hc_bottom.options().set('regions', [mesh.access_component('topology/solid_bottom').uri()])
hc_top.options().set('regions', [mesh.access_component('topology/solid_top').uri()])

# Boundary conditions for the bottom part
bc_bot = hc_bottom.get_child('BoundaryConditions')
bc_bot.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work
bc_bot.add_constant_bc(region_name = 'bottom', variable_name = 'Temperature').options().set('value', 10)
# the boundary between the regions had an automatically generated region, named 'region_bnd_<name of first region>_<name of second region>
bc_bot.add_constant_bc(region_name = 'region_bnd_solid_bottom_solid_top', variable_name = 'Temperature').options().set('value', 50)

# Boundary conditions for the top part
bc_top = hc_top.get_child('BoundaryConditions')
bc_top.options().set('regions', [mesh.access_component('topology').uri()]) # needed to make the lookup work
bc_top.add_constant_bc(region_name = 'top', variable_name = 'Temperature').options().set('value', 10)
bc_top.add_constant_bc(region_name = 'region_bnd_solid_top_solid_bottom', variable_name = 'Temperature').options().set('value', 50)

# run the simulation
model.simulate()

# Write result
domain.write_mesh(cf.URI('atest-multiregion.pvtu'))
