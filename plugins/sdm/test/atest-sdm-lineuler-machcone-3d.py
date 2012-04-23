#!/usr/bin/env python
import sys
#sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

from coolfluid import *
import math

###########################################################################

final_time = 0.3
output_simulation_every = 0.1
mach = 0

###########################################################################

### Create simulation

model   = Core.root().create_component('machcone_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
solver  = model.create_solver('cf3.sdm.SDSolver')
domain  = model.create_domain()

### Create Cubic 3D Hexahedral mesh

mesh = domain.load_mesh(file=URI('cube_20x20x20.msh'),name='mesh')

### Configure solver

solver.options().set('mesh',mesh)
solver.options().set('time',time)
solver.options().set('solution_vars','cf3.physics.LinEuler.Cons3D')
solver.options().set('solution_order',3)
dd = solver.get_child('DomainDiscretization')

### Configure timestepping

solver.access_component('TimeStepping').options().set('cfl','0.1');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)

solver.get_child('PrepareMesh').execute()

### Set the initial condition
p0   = 1
rho0 = 1
c0   = 1
u0   = mach*c0

initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'init')
functions = ['0','0','0','0','0']
initial_condition.options().set('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term

convection = dd.create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection3D')
convection.options().set('gamma', c0**2*rho0/p0)
convection.options().set('rho0',rho0)
convection.options().set('U0',[u0,0.,0.])
convection.options().set('p0',p0)

### create monopole term

monopole = dd.create_term( name = 'monopole', type = 'cf3.sdm.lineuler.SourceMonopole3D' )
monopole.options().set('omega',2*math.pi/30)
monopole.options().set('alpha',math.log(2)/2)
monopole.options().set('epsilon',0.5)
monopole.options().set('source_location',[xc-50,yc,zc])
monopole.options().set('time', time)

### fields to output

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/wave_speed').uri(),
mesh.access_component('solution_space/residual').uri()
]

### simulate
simulate_to_time = 0.
while (simulate_to_time < final_time-1e-10) :
  simulate_to_time += output_simulation_every
  time.options().set('end_time',simulate_to_time);

  model.simulate()

  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.plt'), fields=fields)
  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.msh'), fields=fields)

### output final results

mesh.write_mesh(file=URI('file:mach_cone.plt'),  fields=fields)
mesh.write_mesh(file=URI('file:mach_cone.msh'),  fields=fields)
mesh.write_mesh(file=URI('file:mach_cone.pvtu'), fields=fields)
