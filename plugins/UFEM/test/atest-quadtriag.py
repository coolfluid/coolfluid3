import sys
import coolfluid as cf

# Global configuration
cf.Core.environment().options().set('log_level', 4)

# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')


# load the mesh (passed as first argument to the script)
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')

hc.options().set('regions', [mesh.access_component('topology').uri()])
hc.children.SetSolution.options.relaxation_factor_hc = 1.

# Boundary conditions
bc = hc.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'inlet', variable_name = 'Temperature').options().set('value', 10)
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Temperature').options().set('value', 35)

# run the simulation
model.simulate()

# check the result
coords = mesh.access_component('geometry/coordinates')
temperature = mesh.access_component('geometry/heat_conduction_solution')
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

