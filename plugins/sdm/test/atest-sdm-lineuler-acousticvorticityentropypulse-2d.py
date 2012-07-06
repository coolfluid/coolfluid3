# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

import coolfluid

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
model   = root.create_component('accousticpulse_2d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.LinEuler.LinEuler2D')
domain  = model.create_domain()

### Load the mesh
mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/square100-quad-p2-50x50.msh'), name = 'square');

####### Alternatively, following generates the same square mesh with P1 elements
#mesh = domain.create_component('mesh','cf3.mesh.Mesh')
#mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
#mesh_generator.options().set("mesh",mesh.uri())
#mesh_generator.options().set("nb_cells",[50,50])
#mesh_generator.options().set("lengths",[200,200])
#mesh_generator.options().set("offsets",[-100,-100])
#mesh_generator.execute()
#load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
#load_balance.options().set("mesh",mesh)
#load_balance.execute()
######

### Configure solver
solver.options().set('mesh',mesh)
solver.options().set('solution_vars','cf3.physics.LinEuler.Cons2D')
solver.options().set('solution_order',4)
solver.options().set('iterative_solver','cf3.sdm.ExplicitRungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('Time').options().set('time_step',30);
solver.access_component('Time').options().set('end_time',2);
solver.access_component('TimeStepping').options().set('cfl','0.2');
#solver.access_component('TimeStepping').options().set('max_iteration',1);
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',4)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
gamma = 1.
rho0 = 1.
p0 = 1.
c2 = gamma*p0/rho0
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'init')

# function describing an acoustic pulse, with entropy and vortex
acoustic_entropy_vortex = [
 'exp( -log(2.)*((x)^2+y^2)/9. ) + 0.1*exp( -log(2.)*((x-67.)^2 + y^2)/25. )',
 ' 0.04*y      *exp( -log(2.)*((x-67.)^2+y^2)/25. )',
 '-0.04*(x-67.)*exp( -log(2.)*((x-67.)^2+y^2)/25. )',
 'exp( -log(2.)*((x)^2+y^2)/9. )'
]

# function describing entropy and vortex without acoustic pulse
entropy_vortex = [
 '0.1*exp( -log(2.)*((x-67)^2 + y^2)/25. )',
 ' 0.04*y      *exp( -log(2.)*((x-67)^2+y^2)/25. )',
 '-0.04*(x-67)*exp( -log(2.)*((x-67)^2+y^2)/25. )',
 '0'
]

# function describing acoustic pulse only
acoustic = [
 'exp( -log(2.)*((x)^2+y^2)/9. )',
 ' 0',
 '-0',
 'exp( -log(2.)*((x)^2+y^2)/9. )'
]

# Configure which of the above functions to set as initial condition
initial_condition.options().set('functions',acoustic_entropy_vortex)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D', regions=[mesh.access_component('topology/domain').uri()])
convection.options().set('gamma',gamma)
convection.options().set('rho0',1.)
convection.options().set('U0',[.5,0.])
convection.options().set('p0',1.)

### subsonic outlet BC at tob bottom right
bc = solver.access_component('BoundaryConditions').create_boundary_condition(name='null',type='cf3.sdm.lineuler.BCSubsonicOutlet2D',regions=[
mesh.access_component('topology/right').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/top').uri(),
])

### Impose zero variation at left boundary. This should be replaced by a subsonic inlet BC
impose = solver.access_component('BoundaryConditions').create_boundary_condition(name='impose',type='cf3.sdm.BCConstant<4,2>',regions=[
mesh.access_component('topology/left').uri(),])
impose.options().set('constants',[0,0,0,0])

#######################################
# SIMULATE
#######################################

model.simulate()

#######################################
# POST-PROCESSING
#######################################

compute_char = model.create_component('compute_characteristics','cf3.sdm.lineuler.ComputeCharacteristicVariables')
compute_char.options().set('normal',[1.,0.])
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

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().set('mesh',mesh)
gmsh_writer.options().set('fields',fields)
gmsh_writer.options().set('file',coolfluid.URI('file:sdm_output.msh'))
gmsh_writer.execute()


# Tecplot
#########
# Tecplot cannot write high-order meshes. A finer P1 mesh is generated,
# and fields are interpolated to the P1-mesh. The mesh is finer to visualize
# the high-order solution better.

# Generate visualization mesh
visualization_mesh = domain.create_component('visualization_mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",visualization_mesh.uri())
mesh_generator.options().set("nb_cells",[400,400])
mesh_generator.options().set("lengths",[200,200])
mesh_generator.options().set("offsets",[-100,-100])
mesh_generator.execute()

# Interpolate fields using solution polynomial
visualization_mesh.get_child('geometry').create_field(name='solution',       variables='rho[1],rho0U[2],p[1]')
visualization_mesh.get_child('geometry').create_field(name='char', variables='S[1],Shear[1],Aplus[1],Amin[1],A[1],omega[1]')

interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.Interpolator')
interpolator.interpolate(source=mesh.access_component("solution_space/solution").uri(),
												 target=visualization_mesh.access_component("geometry/solution").uri())
interpolator.interpolate(source=mesh.access_component("solution_space/char").uri(),
												 target=visualization_mesh.access_component("geometry/char").uri())

fields = [
visualization_mesh.access_component('geometry/solution').uri(),
visualization_mesh.access_component('geometry/char').uri()
]

# Write visualization mesh
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().set('mesh',visualization_mesh)
tec_writer.options().set('fields',fields)
tec_writer.options().set('cell_centred',True)
tec_writer.options().set('file',coolfluid.URI('file:sdm_output.plt'))
tec_writer.execute()

#####################
# Probe line y=0
#####################

# Generate 1D line mesh, for now only y=0 can be probed as the line has 1D coordinates only
probe_mesh = domain.create_component('probe_mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",probe_mesh.uri())
mesh_generator.options().set("nb_cells",[1000])
mesh_generator.options().set("lengths",[200])
mesh_generator.options().set("offsets",[-100])
mesh_generator.execute()

# Interpolate fields
probe_mesh.get_child('geometry').create_field(name='solution', variables='rho[1],rho0U[2],p[1]')
probe_mesh.get_child('geometry').create_field(name='char',     variables='S[1],Shear[1],Aplus[1],Amin[1],A[1],omega[1]')

interpolator.interpolate(source=mesh.access_component("solution_space/solution").uri(),
												 target=probe_mesh.access_component("geometry/solution").uri())
interpolator.interpolate(source=mesh.access_component("solution_space/char").uri(),
												 target=probe_mesh.access_component("geometry/char").uri())

fields = [
probe_mesh.access_component('geometry/solution').uri(),
probe_mesh.access_component('geometry/char').uri()
]

# Write probe mesh
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().set('mesh',probe_mesh)
tec_writer.options().set('fields',fields)
tec_writer.options().set('cell_centred',True)
tec_writer.options().set('file',coolfluid.URI('file:probe_liney0.plt'))
tec_writer.execute()

