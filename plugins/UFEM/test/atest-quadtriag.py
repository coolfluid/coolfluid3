import sys
import coolfluid as cf

# Global configuration
cf.Core.environment().options().configure_option('log_level', 4)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
model.setup(solver_builder = 'cf3.UFEM.HeatConductionSteady', physics_builder = 'cf3.physics.DynamicModel')
solver = model.get_child('HeatConductionSteady')
domain = model.get_child('Domain')

# load the mesh (passed as first argument to the script)
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

# lss setup
lss = model.create_component('LSS', 'cf3.math.LSS.System')
lss.options().configure_option('solver', 'Trilinos');
solver.options().configure_option('lss', lss)
lss.get_child('Matrix').options().configure_option('settings_file', sys.argv[2]);

# Boundary conditions
bc = solver.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Temperature').options().configure_option('value', 10)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Temperature').options().configure_option('value', 35)

# run the simulation
model.simulate()

# check the result
coords = mesh.access_component('geometry/coordinates')
temperature = mesh.access_component('geometry/solution')
length = 0.
for i in range(len(coords)):
  x = coords[i][0]
  if x > length:
    length = x

for i in range(len(temperature)):
  if abs(10. + 25.*(coords[i][0] / length) - temperature[i][0]) > 1e-12:
    raise Exception('Incorrect temperature ' + str(temperature[i][0]) + ' for point ' + str(coords[i][0]))

# Write result
domain.write_mesh(cf.URI('atest-quadtriag_output.pvtu'))

