import sys
import coolfluid as cf

# Global configuration
cf.env.assertion_backtrace = False
cf.env.exception_backtrace = False
cf.env.regist_signal_handlers = False
cf.env.exception_log_level = 0
cf.env.log_level = 1
cf.env.exception_outputs = False


# setup a model
model = cf.Core.root().create_component('HotModel', 'cf3.solver.Model')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')
hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')

# load the mesh (passed as first argument to the script)
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'Mesh')
hc.regions = [mesh.topology.uri()]

# Boundary conditions
bc = hc.BoundaryConditions
bc.add_constant_bc(region_name = 'inner', variable_name = 'Temperature').value = 10
bc.add_constant_bc(region_name = 'outer', variable_name = 'Temperature').value = 35

# run the simulation
model.simulate()

# Print temperature along a line y=0, x > 0
print '----------------------- Temperatures profile -------------------'
temperatures = mesh.geometry.heat_conduction_solution
X = []
T = []
for (i, (x, y)) in enumerate(mesh.geometry.coordinates):
  if abs(y) < 1e-2 and x > 0.:
    X.append(x)
    T.append(temperatures[i][0])
for (x, T) in sorted(zip(X, T)):
 print x, T

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat2d-disk.pvtu'))
