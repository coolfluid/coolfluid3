#!/usr/bin/env python
import sys
#sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

from coolfluid import *
import math

###########################################################################

final_time = 0.3
output_simulation_every = 0.1

###########################################################################

### Create simulation

model   = Core.root().create_component('machcone_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
solver  = model.create_solver('cf3.sdm.SDSolver')
domain  = model.create_domain()

### Create Cubic 3D Hexahedral mesh

length = 1.
dx = 0.05
nb_div = int(length/dx)
print "nb_div=",nb_div
mesh = domain.create_component( 'mesh', 'cf3.mesh.Mesh' )
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.BlockMesh.ChannelGenerator")

mesh_generator.options() \
              .set("mesh",mesh.uri()) \
              .set("x_segments",nb_div) \
              .set("y_segments_half",nb_div//2) \
              .set("z_segments",nb_div) \
              .set("length",length) \
              .set("half_height",length/2) \
              .set("width",length) \
              .set("grading",1.)

mesh_generator.execute()

### Configure solver

solver.options().set('mesh',mesh)
solver.options().set('time',time)
solver.options().set('solution_vars','cf3.physics.LinEuler.Cons3D')
solver.options().set('solution_order',2)
dd = solver.get_child('DomainDiscretization')

### Configure timestepping

solver.access_component('TimeStepping').options().set('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)

solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.4
rho0 = 1.
p0 = 1.
c2 = gamma*p0/rho0

initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'init')
functions = [
'0.001*exp( -( (x-0.5)^2 + (y)^2 + (z-0.5)^2 )/(0.05)^2 )',
'0',
'0',
'0',
str(c2)+' * 0.001*exp( -( (x-0.5)^2 + (y)^2 + (z-0.5)^2 )/(0.05)^2)'
]
initial_condition.options().set('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term

convection = dd.create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection3D')
convection.options().set('gamma', gamma)
convection.options().set('rho0',rho0)
convection.options().set('U0',[0.,0.,0.])
convection.options().set('p0',p0)

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

  mesh.write_mesh(file=URI('file:acousticpulse3d'+str(simulate_to_time)+'.plt'), fields=fields)
  mesh.write_mesh(file=URI('file:acousticpulse3d'+str(simulate_to_time)+'.msh'), fields=fields)

### output final results

mesh.write_mesh(file=URI('file:acousticpulse3d.plt'),  fields=fields)
mesh.write_mesh(file=URI('file:acousticpulse3d.msh'),  fields=fields)
mesh.write_mesh(file=URI('file:acousticpulse3d.pvtu'), fields=fields)


vis_mesh = domain.create_component( 'vis_mesh', 'cf3.mesh.Mesh' )
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.BlockMesh.ChannelGenerator")
mesh_generator.options().set("mesh",vis_mesh.uri())
mesh_generator.options().set("x_segments",50)
mesh_generator.options().set("y_segments_half",25)
mesh_generator.options().set("z_segments",50)
mesh_generator.options().set("length",1.)
mesh_generator.options().set("half_height",0.5)
mesh_generator.options().set("width",1.)
mesh_generator.options().set("grading",1.)
mesh_generator.execute()

vis_solution = vis_mesh.access_component('geometry').create_field(name='solution',variables='rho,rho0U[vector],p')

interpolator = vis_mesh.create_component('interpolator','cf3.mesh.actions.Interpolate')
interpolator.interpolate(source=mesh.access_component('solution_space/solution').uri(),target=vis_solution.uri())

vis_mesh.write_mesh(file=URI('file:acousticpulse3d_vis.plt'),fields=[vis_solution.uri()])
