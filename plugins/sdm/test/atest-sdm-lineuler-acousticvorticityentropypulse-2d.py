# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

import coolfluid

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
mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/square100-quad-p2-50x50.msh'), name = 'square');

####### Alternatively, following generates the same square mesh with P1 elements
#mesh = domain.create_component('mesh','cf3.mesh.Mesh')
#mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
#mesh_generator.options().configure_option("mesh",mesh.uri())
#mesh_generator.options().configure_option("nb_cells",[50,50])
#mesh_generator.options().configure_option("lengths",[200,200])
#mesh_generator.options().configure_option("offsets",[-100,-100])
#mesh_generator.execute()
#load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
#load_balance.options().configure_option("mesh",mesh)
#load_balance.execute()
######

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.LinEuler.Cons2D')
solver.options().configure_option('solution_order',4)
solver.options().configure_option('iterative_solver','cf3.sdm.RungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('Time').options().configure_option('time_step',30);
solver.access_component('Time').options().configure_option('end_time',5);
solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
#solver.access_component('TimeStepping').options().configure_option('max_iteration',1);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',4)

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
initial_condition.options().configure_option('functions',acoustic_entropy_vortex)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D', regions=[mesh.access_component('topology/domain').uri()])
convection.options().configure_option('gamma',gamma)
convection.options().configure_option('rho0',1.)
convection.options().configure_option('U0',[.5,0.])
convection.options().configure_option('p0',1.)

### extrapolation boundary condition, for visualization of domain-boundary
null_bc = solver.access_component('BoundaryConditions').create_boundary_condition(name='null',type='cf3.sdm.BCNull',regions=[
mesh.access_component('topology/right').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/top').uri(),
])

### Impose zero variation at left boundary. This should be replaced by a subsonic inlet BC
impose = solver.access_component('BoundaryConditions').create_boundary_condition(name='impose',type='cf3.sdm.BCConstant<4,2>',regions=[
mesh.access_component('topology/left').uri(),])
impose.options().configure_option('constants',[0,0,0,0])

### Non-reflecting outlet boundary condition. Special treatment is required as it is used as a domain-term in boundary cells
modify_subsonic_outlet = solver.access_component('TimeStepping/IterativeSolver/PreUpdate').create_component('modify_subsonic_outlet','cf3.sdm.BoundaryConditions')
modify_subsonic_outlet.options().configure_option('solver',solver)
modify_subsonic_outlet.options().configure_option('mesh',mesh)
modify_subsonic_outlet.options().configure_option('physical_model',physics)
non_refl_bc = modify_subsonic_outlet.create_boundary_condition(name='non_refl_bc',type='cf3.sdm.lineuler.BCSubsonicOutlet2D',regions=[
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/right').uri(),
]);
non_refl_bc.get_child('non_reflective_convection').options().configure_option('gamma',gamma)
non_refl_bc.get_child('non_reflective_convection').options().configure_option('rho0',1.)
non_refl_bc.get_child('non_reflective_convection').options().configure_option('U0',[.5,0.])
non_refl_bc.get_child('non_reflective_convection').options().configure_option('p0',1.)

#######################################
# SIMULATE
#######################################

model.simulate()

#######################################
# POST-PROCESSING
#######################################

compute_char = model.create_component('compute_characteristics','cf3.sdm.lineuler.ComputeCharacteristicVariables')
compute_char.options().configure_option('normal',[1.,0.])
compute_char.options().configure_option('solver',solver)
compute_char.options().configure_option('mesh',mesh)
compute_char.options().configure_option('physical_model',physics)
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
gmsh_writer.options().configure_option('mesh',mesh)
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.msh'))
gmsh_writer.execute()


# Tecplot
#########
# Tecplot cannot write high-order meshes. A finer P1 mesh is generated,
# and fields are interpolated to the P1-mesh. The mesh is finer to visualize
# the high-order solution better.

# Generate visualization mesh
visualization_mesh = domain.create_component('visualization_mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().configure_option("mesh",visualization_mesh.uri())
mesh_generator.options().configure_option("nb_cells",[400,400])
mesh_generator.options().configure_option("lengths",[200,200])
mesh_generator.options().configure_option("offsets",[-100,-100])
mesh_generator.execute()

# Interpolate fields using solution polynomial
visualization_mesh.get_child('geometry').create_field(name='solution',       variables='rho[1],rho0U[2],p[1]')
visualization_mesh.get_child('geometry').create_field(name='char', variables='S[1],Shear[1],Aplus[1],Amin[1],A[1],omega[1]')

interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.actions.Interpolate')
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
tec_writer.options().configure_option('mesh',visualization_mesh)
tec_writer.options().configure_option('fields',fields)
tec_writer.options().configure_option('cell_centred',True)
tec_writer.options().configure_option('file',coolfluid.URI('file:sdm_output.plt'))
tec_writer.execute()

#####################
# Probe line y=0
#####################

# Generate 1D line mesh, for now only y=0 can be probed as the line has 1D coordinates only
probe_mesh = domain.create_component('probe_mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().configure_option("mesh",probe_mesh.uri())
mesh_generator.options().configure_option("nb_cells",[1000])
mesh_generator.options().configure_option("lengths",[200])
mesh_generator.options().configure_option("offsets",[-100])
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
tec_writer.options().configure_option('mesh',probe_mesh)
tec_writer.options().configure_option('fields',fields)
tec_writer.options().configure_option('cell_centred',True)
tec_writer.options().configure_option('file',coolfluid.URI('file:probe_liney0.plt'))
tec_writer.execute()

