#!/usr/bin/env python

#sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

import sys

from coolfluid import *
from math import *

###########################################################################


final_time = 5 # should be 120
output_simulation_every = 1
mach = 1.5

###########################################################################

### Create simulation

model   = Core.root().create_component('machcone_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
solver  = model.create_solver('cf3.sdm.SDSolver')
domain  = model.create_domain()

### Create Cubic 3D Hexahedral mesh

#mesh = domain.load_mesh(file=URI('cube_20x20x20.msh'),name='mesh')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

nb_div=10
length=120.
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri()) \
                        .set("nb_cells",[nb_div,nb_div,nb_div]) \
                        .set("lengths",[length,length,length]) \
                        .set("offsets",[0.,-length/2,-length/2])
mesh_generator.execute()

mesh.write_mesh( file = URI("cube_10x10x10.msh") )

vis_mesh = domain.create_component('vis_mesh','cf3.mesh.Mesh')
mesh_generator.options().set("mesh",vis_mesh.uri())\
                        .set("nb_cells",[50,50,50])
mesh_generator.execute()

repartitioner=mesh.create_component('repartitioner','cf3.mesh.actions.LoadBalance')
repartitioner.options().set('mesh',mesh)
repartitioner.execute()


### Configure solver

solver.options().set('mesh',mesh) \
                .set('time',time) \
                .set('solution_vars','cf3.physics.LinEuler.Cons3D') \
                .set('solution_order',5)

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
convection.options().set('gamma', c0**2*rho0/p0) \
                    .set('rho0',rho0) \
                    .set('U0',[u0,0.,0.]) \
                    .set('p0',p0)

### create monopole term

monopole = dd.create_term( name = 'monopole', type = 'cf3.sdm.lineuler.SourceMonopole3D' )
monopole.options().set('omega',2*pi/30 ) \
                  .set('alpha',log(2)/2) \
                  .set('epsilon',0.5) \
                  .set('source_location',[25,0.,0.]) \
                  .set('time', time)


### fields to output

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/wave_speed').uri(),
mesh.access_component('solution_space/residual').uri()
]

vis_solution = vis_mesh.access_component('geometry').create_field(name='solution',variables='rho,rho0U[3],p')
interpolator = vis_mesh.create_component('interpolator','cf3.mesh.Interpolator')

solver.add_probe(
  name="probe",
  coordinate=[25.,0.,0.],
  functions=['ptot=P+'+str(p0)],
  log_variables=['ptot'])

### simulate

simulate_to_time = 0.
while (simulate_to_time < final_time) :
  simulate_to_time += output_simulation_every
  time.options().set('end_time',simulate_to_time);

  model.simulate()

  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.msh'), fields=fields)


### output final results

### create boundary condition

bc_extrapolate = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'function', type = 'cf3.sdm.BCExtrapolate<5,3>',
regions=[
mesh.access_component('topology/front').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/right').uri(),
mesh.access_component('topology/back').uri(),
mesh.access_component('topology/top').uri(),
])
solver.get_child('BoundaryConditions').execute()
interpolator.interpolate(source=mesh.access_component('solution_space/solution').uri(),target=vis_solution.uri())
vis_mesh.write_mesh(file=URI('file:mach_cone_vis.plt'),fields=[vis_solution.uri()])
