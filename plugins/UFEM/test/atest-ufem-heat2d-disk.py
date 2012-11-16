import sys
import coolfluid as cf

# Global configuration
cf.environment.assertion_backtrace = False
cf.environment.exception_backtrace = False
cf.environment.regist_signal_handlers = False
cf.environment.exception_log_level = 0
cf.environment.log_level = 1
cf.environment.exception_outputs = False

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

# Write result
domain.write_mesh(cf.URI('atest-ufem-heat2d-disk.pvtu'))
