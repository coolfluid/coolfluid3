# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/debug/dso/')

import coolfluid
import math

# The cf root component
root = coolfluid.Core.root()
env =  coolfluid.Core.environment()

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
model   = root.create_component('accousticpulse_2d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.LinEuler.LinEuler2D')
domain  = model.create_domain()

### Load the mesh
mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/circle-quad-p1-32.msh'), name = 'circle');

gmsh_writer = model.create_component('load_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('file',coolfluid.URI('file:load.msh'))
gmsh_writer.execute()

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.LinEuler.Cons2D')
solver.options().configure_option('solution_order',4)

### Configure timestepping
solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
solver.access_component('TimeStepping/Time').options().configure_option('time_step',1.);
solver.access_component('TimeStepping/Time').options().configure_option('end_time',0.3);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.4
rho0 = 1.
p0 = 1.
c2 = gamma*p0/rho0
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'shocktube')
functions = [
 '0.001*exp( -( (x)^2 + (y)^2 )/(0.05)^2 )',
 '0',
 '0',
 str(c2)+' * 0.001*exp( -( (x)^2 + (y)^2 )/(0.05)^2 )'
]
initial_condition.options().configure_option('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D')
convection.options().configure_option('gamma',gamma)
convection.options().configure_option('rho0',1.)
convection.options().configure_option('U0',[0.,0.])
convection.options().configure_option('p0',1.)

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
tec_writer.options().configure_option('mesh',mesh.uri())
tec_writer.options().configure_option('fields',fields)
tec_writer.options().configure_option('cell_centred',False)
tec_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.plt'))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.msh'))
gmsh_writer.execute()
