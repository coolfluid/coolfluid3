# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

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
#mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/circle-quad-p1-32.msh'), name = 'circle');
mesh = domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/nonreflective.msh'), name = 'nonreflective');
####### Following generates a square mesh
#mesh = domain.create_component('mesh','cf3.mesh.Mesh')
#mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
#mesh_generator.options().configure_option("mesh",mesh.uri())
#mesh_generator.options().configure_option("nb_cells",[20,40])
#mesh_generator.options().configure_option("lengths",[80,80])
#mesh_generator.options().configure_option("offsets",[-40,-40])
#mesh_generator.execute()
#load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
#load_balance.options().configure_option("mesh",mesh)
#load_balance.execute()
######

gmsh_writer = model.create_component('load_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('file',coolfluid.URI('file:load.msh'))
gmsh_writer.execute()


### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.LinEuler.Cons2D')
solver.options().configure_option('solution_order',4)
solver.options().configure_option('iterative_solver','cf3.sdm.RungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('TimeStepping').options().configure_option('cfl','0.1');
#solver.access_component('TimeStepping').options().configure_option('max_iteration',1);
solver.access_component('TimeStepping/Time').options().configure_option('time_step',5);
solver.access_component('TimeStepping/Time').options().configure_option('end_time',90);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.
rho0 = 1.
p0 = 1.
c2 = gamma*p0/rho0
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'shocktube')
#functions = [
# '0.001*exp( -( (x)^2 + (y)^2 )/(0.05)^2 )',
# '0',
# '0',
# str(c2)+' * 0.001*exp( -( (x)^2 + (y)^2 )/(0.05)^2 )'
#]
functions = [
 ' exp( -log(2.)*((x+20)^2+y^2)/9. ) + 0.1*        exp( -log(2.)*((x)^2 + (y)^2)/25. )',
 ' 0.04*(y+0)* exp( -log(2.)*((x)^2 + (y)^2)/25. )',
 '-0.04*(x-0)* exp( -log(2.)*((x)^2 + (y)^2)/25. )',
 ' exp( -log(2.)*((x+20)^2+y^2)/9. )'
]
functions = [
 '0.1*exp( -log(2.)*((x-95)^2 + y^2)/25. )',
 ' 0.04*y      *exp( -log(2.)*((x-95.)^2+y^2)/25. )',
 '-0.04*(x-95.)*exp( -log(2.)*((x-95.)^2+y^2)/25. )',
 '0'
]
#functions = [
# ' exp( -log(2.)*((x+20)^2+y^2)/9. ) ',
# ' 0.',
# ' 0.',
# ' exp( -log(2.)*((x+20)^2+y^2)/9. ) '
#]
functions = [
 'exp( -log(2.)*((x)^2+y^2)/9. ) + 0.1*exp( -log(2.)*((x-67.)^2 + y^2)/25. )',
 ' 0.04*y      *exp( -log(2.)*((x-67.)^2+y^2)/25. )',
 '-0.04*(x-67.)*exp( -log(2.)*((x-67.)^2+y^2)/25. )',
 'exp( -log(2.)*((x)^2+y^2)/9. )'
]
#functions = [
# 'exp( -log(2.)*((x)^2+y^2)/9. )',
# ' 0',
# '-0',
# 'exp( -log(2.)*((x)^2+y^2)/9. )'
#]



initial_condition.options().configure_option('functions',functions)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D', regions=[mesh.access_component('topology/domain').uri()])
convection.options().configure_option('gamma',gamma)
convection.options().configure_option('rho0',1.)
convection.options().configure_option('U0',[.5,0.])
convection.options().configure_option('p0',1.)

#charconvection = solver.get_child('DomainDiscretization').create_term(name = 'charconvection', type = 'cf3.sdm.lineuler.CharConvection2D', regions=[mesh.access_component('topology/buffer').uri()])
#charconvection.options().configure_option('gamma',gamma)
#charconvection.options().configure_option('rho0',1.)
#charconvection.options().configure_option('U0',[.5,0.])
#charconvection.options().configure_option('p0',1.)

null_bc = solver.access_component('BoundaryConditions').create_boundary_condition(name='null',type='cf3.sdm.BCNull',regions=[
#mesh.access_component('topology/left').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/right').uri(),
#mesh.access_component('topology/interface_domain_to_buffer').uri()
])

#convectA = solver.access_component('BoundaryConditions').create_boundary_condition(name='convectA',type='cf3.sdm.lineuler.BCConvectA2D',regions=[
#mesh.access_component('topology/interface_domain_to_buffer').uri()
##mesh.access_component('topology/right').uri(),
##mesh.access_component('topology/left').uri(),
##mesh.access_component('topology/bottom').uri(),
##mesh.access_component('topology/top').uri(),
#])
#convectA.options().configure_option('indicator_treshold',0.10)
#convectA.options().configure_option('indicator_treshold2',0.20)
#convectA.options().configure_option('gamma',gamma)
#convectA.options().configure_option('rho0',1.)
#convectA.options().configure_option('U0',[.5,0.])
#convectA.options().configure_option('p0',1.)


impose = solver.access_component('BoundaryConditions').create_boundary_condition(name='impose',type='cf3.sdm.BCConstant<4,2>',regions=[
mesh.access_component('topology/left').uri(),
#mesh.access_component('topology/bottom').uri(),
#mesh.access_component('topology/top').uri(),
#mesh.access_component('topology/right').uri()
])
impose.options().configure_option('constants',[0,0,0,0])

BCs = solver.access_component('TimeStepping/IterativeSolver/PreUpdate').create_component('BoundaryConditions','cf3.sdm.BoundaryConditions')
BCs.options().configure_option('solver',solver)
BCs.options().configure_option('mesh',mesh)
BCs.options().configure_option('physical_model',physics)
non_refl_bc = BCs.create_boundary_condition(name='non_refl_bc',type='cf3.sdm.lineuler.BCSubsonicOutlet2D',regions=[
#mesh.access_component('topology/boundary').uri(),
#mesh.access_component('topology/left').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/right').uri(),
#mesh.access_component('topology/interface_domain_to_buffer').uri()
]);
non_refl_bc.get_child('non_reflective_convection').options().configure_option('gamma',gamma)
non_refl_bc.get_child('non_reflective_convection').options().configure_option('rho0',1.)
non_refl_bc.get_child('non_reflective_convection').options().configure_option('U0',[.5,0.])
non_refl_bc.get_child('non_reflective_convection').options().configure_option('p0',1.)

#######################################
# SIMULATE
#######################################
model.simulate()

compute_char = model.create_component('compute_characteristics','cf3.sdm.lineuler.ComputeCharacteristicVariables')
compute_char.options().configure_option('normal',[1.,0.])
compute_char.options().configure_option('solver',solver)
compute_char.options().configure_option('mesh',mesh)
compute_char.options().configure_option('physical_model',physics)
compute_char.execute()

rhs_non_refl = mesh.access_component('solution_space/char_resid')
rhs_refl     = mesh.access_component('solution_space/char_convection')
rhs_diff = mesh.access_component('solution_space').create_field(name='rhs_diff',variables='dS,dOmega,dA,domega')
for i in range(len(rhs_non_refl)):
    rhs_diff[i][0] = rhs_refl[i][0]-rhs_non_refl[i][0]
    rhs_diff[i][1] = rhs_refl[i][1]-rhs_non_refl[i][1]
    rhs_diff[i][2] = rhs_refl[i][4]-rhs_non_refl[i][4]
    rhs_diff[i][3] = rhs_refl[i][5]-rhs_non_refl[i][5]

########################
# OUTPUT
########################

fields = [
mesh.access_component('solution_space/solution').uri(),
#mesh.access_component('solution_space/wave_speed').uri(),
#mesh.access_component('solution_space/residual').uri(),
#mesh.access_component('solution_space/convection').uri(),
#mesh.access_component('solution_space/non_reflective_convection').uri(),
mesh.access_component('solution_space/char').uri(),
mesh.access_component('solution_space/char_resid').uri(),
mesh.access_component('solution_space/char_convection').uri(),
rhs_diff.uri()
]

# tecplot
#########
#tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
#tec_writer.options().configure_option('mesh',mesh.uri())
#tec_writer.options().configure_option('fields',fields)
#tec_writer.options().configure_option('cell_centred',False)
#tec_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.plt'))
#tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.msh'))
gmsh_writer.execute()
