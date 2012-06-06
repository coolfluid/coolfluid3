import coolfluid
import math

# The cf root component
root = coolfluid.Core.root()
env =  coolfluid.Core.environment()

### Logging configuration
env.options().set('assertion_backtrace', True)
env.options().set('exception_backtrace', True)
env.options().set('regist_signal_handlers', True)
env.options().set('exception_log_level', 10)
env.options().set('log_level', 3)
env.options().set('exception_outputs', True)

############################
# Create simulation
############################
model   = root.create_component('wallbc_2d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.LinEuler.LinEuler2D')
domain  = model.create_domain()

### Load the mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_gen = domain.create_component('mesh_gen','cf3.mesh.SimpleMeshGenerator')
mesh_gen.options().set('mesh',mesh.uri()) \
                  .set('lengths',[200,200]) \
                  .set('offsets',[-100,0]) \
                  .set('nb_cells',[30,30])
mesh_gen.execute()

repartitioner=mesh.create_component('repartitioner','cf3.mesh.actions.LoadBalance')
repartitioner.options().set('mesh',mesh)
repartitioner.execute()

### Configure solver
solver.options().set('mesh',mesh)
solver.options().set('solution_vars','cf3.physics.LinEuler.Cons2D')
solver.options().set('solution_order',3)
solver.options().set('iterative_solver','cf3.sdm.ExplicitRungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('Time').options().set('time_step',1.);
solver.access_component('Time').options().set('end_time',60.);
solver.access_component('TimeStepping').options().set('cfl','0.3');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.
rho0 = 1.
p0 = 1.
M = 0.5
c0 = math.sqrt(gamma*p0/rho0)
u0 = M*c0
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'pulse')
functions = [
 'exp( -log(2)*( (x)^2 + (y-25)^2 )/(25) )',
 '0',
 '0',
 'exp( -log(2)*( (x)^2 + (y-25)^2 )/(25) )'
]
initial_condition.options().set('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D')
convection.options().set('gamma',gamma)
convection.options().set('rho0',1.)
convection.options().set('U0',[u0,0.])
convection.options().set('p0',1.)

wallbc = solver.get_child('BoundaryConditions').create_boundary_condition(name='wallbc',type='cf3.sdm.lineuler.BCWall2D',regions=[mesh.access_component('topology/bottom').uri()]);
wallbc.options().set('gamma',gamma) \
                .set('rho0',1.) \
                .set('U0',[u0,0.]) \
                .set('p0',1.)

#######################################
# SIMULATE
#######################################

solver.get_child('TimeStepping').options().set('max_iteration',20)

model.simulate()

print 'WARNING: End time not reached: comment out max_iteration line'

#######################################
# POST-PROCESSING
#######################################

compute_char = model.create_component('compute_characteristics','cf3.sdm.lineuler.ComputeCharacteristicVariables')
compute_char.options().set('normal',[0.,-1.])
compute_char.options().set('solver',solver)
compute_char.options().set('mesh',mesh)
compute_char.options().set('physical_model',physics)
compute_char.execute()

########################
# OUTPUT
########################

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/char').uri()
]

########################
# OUTPUT
########################

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/residual').uri(),
mesh.access_component('solution_space/char').uri()
]

vis_mesh = domain.create_component('vis_mesh','cf3.mesh.Mesh')
mesh_gen.options().set('mesh',vis_mesh.uri())\
                  .set('nb_cells',[100,100])
mesh_gen.execute()
vis_solution = vis_mesh.get_child('geometry').create_field(name='solution',variables='rho,rho0u[vector],p')

tools = coolfluid.Core.root().get_child('Tools')
interpolator = tools.create_component('interpolator','cf3.mesh.ShapeFunctionInterpolator')
interpolator.interpolate(source=mesh.access_component('solution_space/solution').uri(),target=vis_solution.uri())

vis_mesh.write_mesh(file=coolfluid.URI('file:wallbc.plt'), fields=[vis_solution.uri()])
vis_mesh.write_mesh(file=coolfluid.URI('file:wallbc.msh'), fields=[vis_solution.uri()])

## gmsh
#######
#gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
#gmsh_writer.options().set('mesh',mesh)
#gmsh_writer.options().set('fields',fields)
#gmsh_writer.options().set('file',coolfluid.URI('file:sdm_output.msh'))
#gmsh_writer.execute()
