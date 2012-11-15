import sys
import coolfluid as cf
try:
  import pylab as pl
  import numpy as np
  have_pylab = True
except:
  have_pylab = False

# Global configuration
cf.environment.assertion_backtrace = False
cf.environment.exception_backtrace = False
cf.environment.regist_signal_handlers = False
cf.environment.exception_log_level = 0
cf.environment.log_level = 4
cf.environment.exception_outputs = False

k = 1./12.
Tb = 0.

def run_simulation(sf_name):
  # setup a model
  model = cf.root.create_component(sf_name, 'cf3.solver.Model')
  domain = model.create_domain()
  physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
  solver = model.create_solver('cf3.UFEM.Solver')
  hc = solver.add_direct_solver('cf3.UFEM.HeatConductionSteady')
  hc.options.heat_space_name = sf_name
  hc.children.Assembly.options.k = k
  hc.children.SetSolution.options.relaxation_factor_hc = 1.

  ic_heat = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'source_terms')
  ic_heat.variable_name = 'Heat'
  ic_heat.options.field_space_name = sf_name
  ic_heat.value = ['x^2']

  # Generate a 1D line mesh
  mesh = domain.create_component('mesh','cf3.mesh.Mesh')
  mesh_generator = cf.root.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
  mesh_generator.mesh = mesh.uri()
  mesh_generator.nb_cells = [10]
  mesh_generator.lengths = [2.]
  mesh_generator.offsets = [-1.]
  mesh_generator.execute()

  # Set the region for the simulation
  hc.regions = [mesh.topology.uri()]
  ic_heat.regions = hc.regions

  # Boundary conditions
  bc = hc.BoundaryConditions
  bc.add_constant_bc(region_name = 'xneg', variable_name = 'Temperature').value = Tb
  bc.add_constant_bc(region_name = 'xpos', variable_name = 'Temperature').value = Tb

  # run the simulation
  model.simulate()
  
  return mesh

mesh_p1 = run_simulation('cf3.mesh.LagrangeP1')
x_p1 = mesh_p1.geometry.coordinates
T_p1 = mesh_p1.geometry.heat_conduction_solution

mesh_p2 = run_simulation('cf3.mesh.LagrangeP2')
x_p2 = mesh_p2.geometry.coordinates
T_p2 = mesh_p2.geometry.heat_conduction_solution

if have_pylab:
  pl.plot(x_p1, T_p1, 'ko', mfc='none', label='P1')
  pl.plot(x_p2, T_p2, 'kx', mfc='none', label='P2')

  x_th = np.linspace(-1., 1., 500)
  pl.plot(x_th, (-x_th**4. + 1.) + Tb, 'k-', label='Analytical')
  pl.grid()
  pl.legend(loc = 'lower center')
  pl.xlabel('x')
  pl.ylabel('T')
  pl.savefig('heat-p1-p2.eps')