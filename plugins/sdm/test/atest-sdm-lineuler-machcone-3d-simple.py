#!/usr/bin/env python

import sys

from coolfluid import *
from math import *

###########################################################################

order = 5

final_time = 120

step = 20
mach = 1.5
p0   = 1
rho0 = 1
c0   = 1
u0   = mach*c0

###########################################################################

### Create simulation

model   = Core.root().create_component('machcone_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
solver  = model.create_solver('cf3.sdm.SDSolver')
domain  = model.create_domain()

### Create Cubic 3D Hexahedral mesh

mesh = domain.load_mesh( file=URI('cube_10x10x10.msh'), name="mesh" )

### Configure solver

solver.options().set( 'mesh',mesh ) \
                .set( 'time',time ) \
                .set( 'solution_vars','cf3.physics.LinEuler.Cons3D' ) \
                .set( 'solution_order', order )

dd = solver.get_child( 'DomainDiscretization' )

### Configure timestepping

solver.access_component('TimeStepping').options().set('cfl','0.1');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)

solver.get_child('PrepareMesh').execute()

### Set the initial condition

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
    mesh.access_component('solution_space/residual').uri() ]

### simulate

simulate_to_time = 0.
while ( simulate_to_time < final_time ) :

  simulate_to_time += step

  time.options().set( 'end_time',simulate_to_time );

  model.simulate()

  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.plt'), fields=fields)

### high resolution visualization

vis_mesh = domain.load_mesh( file=URI('cube_100x100x100.msh'), name='vis_mesh' )

vis_solution = vis_mesh.access_component('geometry').create_field(name='solution',variables='rho,rho0U[3],p')
interpolator = vis_mesh.create_component('interpolator','cf3.mesh.actions.Interpolate')

interpolator.interpolate( source=mesh.access_component('solution_space/solution').uri(),\
                          target=vis_solution.uri() )

vis_mesh.write_mesh( file=URI('file:mach_cone_vis.plt'),fields=[vis_solution.uri()] )
vis_mesh.write_mesh( file=URI('file:mach_cone_vis.msh'),fields=[vis_solution.uri()] )
