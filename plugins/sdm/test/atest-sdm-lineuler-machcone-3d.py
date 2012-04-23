#!/usr/bin/env python
import sys
#sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

from coolfluid import *
import math

###########################################################################

final_time = 30.
output_simulation_every = 10.

###########################################################################

### Create simulation

model   = Core.root().create_component('machcone_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
solver  = model.create_solver('cf3.sdm.SDSolver')
domain  = model.create_domain()

### Create Cubic 3D Hexahedral mesh

mesh = domain.create_component( 'mesh', 'cf3.mesh.Mesh' )
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.BlockMesh.ChannelGenerator")
mesh_generator.options().configure_option("mesh",mesh.uri())
mesh_generator.options().configure_option("x_segments",20)
mesh_generator.options().configure_option("y_segments_half",10)
mesh_generator.options().configure_option("z_segments",20)
mesh_generator.options().configure_option("length",200)
mesh_generator.options().configure_option("half_height",100)
mesh_generator.options().configure_option("width",200)
mesh_generator.options().configure_option("grading",1.)
mesh_generator.execute()

### Configure solver

solver.options().configure_option('mesh',mesh)
solver.options().configure_option('time',time)
solver.options().configure_option('solution_vars','cf3.physics.LinEuler.Cons3D')
solver.options().configure_option('solution_order',2)
dd = solver.get_child('DomainDiscretization')

### Configure timestepping

solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)

solver.get_child('PrepareMesh').execute()

### Set the initial condition

initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'init')
functions = ['0','0','0','0','0']
initial_condition.options().configure_option('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term

p0   = 1
rho0 = 1
u0   = 1.5
c0   = 1
convection = dd.create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection3D')
convection.options().configure_option('gamma', c0**2*rho0/p0)
convection.options().configure_option('rho0',rho0)
convection.options().configure_option('U0',[u0,0.,0.])
convection.options().configure_option('p0',p0)

### create monopole term

monopole = dd.create_term( name = 'monopole', type = 'cf3.sdm.lineuler.SourceMonopole3D' )
monopole.options().configure_option('omega',2*math.pi/30)
monopole.options().configure_option('alpha',math.log(2)/2)
monopole.options().configure_option('epsilon',0.5)
monopole.options().configure_option('source_location',[50,0.,100])
monopole.options().configure_option('time', time)

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
  time.options().configure_option('end_time',simulate_to_time);

  model.simulate()

  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.plt'), fields=fields)
  mesh.write_mesh(file=URI('file:mach_cone_time'+str(simulate_to_time)+'.msh'), fields=fields)

### output final results

mesh.write_mesh(file=URI('file:mach_cone.plt'),  fields=fields)
mesh.write_mesh(file=URI('file:mach_cone.msh'),  fields=fields)
mesh.write_mesh(file=URI('file:mach_cone.pvtu'), fields=fields)

