#!/usr/bin/env python
# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/debug/dso/')

from coolfluid import *
import math

# The cf root component
root = Core.root()
env =  Core.environment()

### Logging configuration
env.options().configure_option('assertion_backtrace', True)
env.options().configure_option('exception_backtrace', True)
env.options().configure_option('regist_signal_handlers', True)
env.options().configure_option('exception_log_level', 10)
env.options().configure_option('log_level', 3)
env.options().configure_option('exception_outputs', True)

############################
# Create simulation
############################
model   = root.create_component('machcone_3d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.LinEuler.LinEuler3D')
domain  = model.create_domain()

### Load the mesh

mesh = domain.create_component( 'mesh', 'cf3.mesh.Mesh' )

#mesh_reader = model.create_component('mesh_reader','cf3.mesh.gmsh.Reader')
#mesh_reader.options().configure_option( 'mesh', mesh )
#mesh_reader.options().configure_option( 'file', URI('../../../resources/sqduct_3000e.msh') )
#mesh_reader.execute()

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

### write initial mesh

gmsh_writer = model.create_component('load_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh)
gmsh_writer.options().configure_option('file',URI('file:load.msh'))
gmsh_writer.execute()

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.LinEuler.Cons3D')
solver.options().configure_option('solution_order',3)
solver.options().configure_option('iterative_solver','cf3.sdm.RungeKuttaLowStorage2')

### Configure timestepping
time = solver.access_component('Time')

time.options().configure_option('time_step',50.0);
time.options().configure_option('end_time',300.0);

solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.
rho0  = 1.
p0    = 1.
c2    = gamma*p0/rho0
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'init')
functions = [
 '0',
 '0',
 '0',
 '0',
 '0'
]
initial_condition.options().configure_option('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term

dd = solver.get_child('DomainDiscretization')

convection = dd.create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection3D')
convection.options().configure_option('gamma',gamma)
convection.options().configure_option('rho0',1.)
convection.options().configure_option('U0',[0.5,0.,0.])
convection.options().configure_option('p0',1.)

### create monopole term
monopole = dd.create_term( name = 'monopole', type = 'cf3.sdm.lineuler.SourceMonopole3D' )
monopole.options().configure_option('omega',2*math.pi/30)
monopole.options().configure_option('alpha',math.log(2)/2)
monopole.options().configure_option('epsilon',0.5)
monopole.options().configure_option('source_location',[50,0.,100])
monopole.options().configure_option('time', time)


### create BCs
#BCs = solver.access_component('TimeStepping/IterativeSolver/PreUpdate').create_component('BoundaryConditions','cf3.sdm.BoundaryConditions')
#BCs.options().configure_option('solver',solver)
#BCs.options().configure_option('mesh',mesh)
#BCs.options().configure_option('physical_model',physics)

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# OUTPUT
########################

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/wave_speed').uri(),
mesh.access_component('solution_space/residual').uri()
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().configure_option('mesh',mesh)
tec_writer.options().configure_option('fields',fields)
tec_writer.options().configure_option('cell_centred',False)
tec_writer.options().configure_option('file',URI('file:sdm_output.plt'))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh)
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',URI('file:sdm_output.msh'))
gmsh_writer.execute()
