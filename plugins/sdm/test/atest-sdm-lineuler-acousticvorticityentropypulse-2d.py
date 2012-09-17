# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/release/dso/')

from coolfluid import *
from math import *

### Logging configuration
env.assertion_backtrace = True
env.exception_backtrace = True
env.exception_log_level = 4
env.log_level = 3

############################
# Create simulation
############################
model   = root.create_component('accousticpulse_2d','cf3.sdm.Model');

### Load the mesh
mesh = model.domain.load_mesh(file = URI('../../../resources/square100-quad-p2-50x50.msh'), name = 'square');

####### Alternatively, following generates the same square mesh with P1 elements
#mesh = model.domain.create_component('mesh','cf3.mesh.Mesh')
#mesh_generator = model.tools.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
#mesh_generator.mesh     = mesh
#mesh_generator.nb_cells = [60,60]
#mesh_generator.lengths  = [250,250]
#mesh_generator.offsets  = [-100,-100]
#mesh_generator.execute()
#load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
#load_balance.mesh = mesh
#load_balance.execute()
######

### Configure solver
solution_space = model.create_solution_space( order=4, regions=[mesh.topology])

gamma = 1.
rho0 = 1.
p0 = 1.
U0 = [0.5,0]
c2 = gamma*p0/rho0
c0 = sqrt(c2)

solution = model.create_field( name='solution',
    functions=[ 'rho[scal]  =  exp( -log(2.)*((x)^2+y^2)/9. ) + 0.1*exp( -log(2.)*((x-67.)^2 + y^2)/25. )',
                'rho0U[vec] =  [0.04*y      *exp( -log(2.)*((x-67.)^2+y^2)/25. ), -0.04*(x-67.)*exp( -log(2.)*((x-67.)^2+y^2)/25. )]',
                'p[scal]    =  '+str(c2)+'*exp( -log(2.)*((x)^2+y^2)/9. )' ])

mean_flow = model.create_field( name='mean_flow',
    functions=[ 'rho0[scal] = '+str(rho0),
                'U0[vec]    = '+str(U0),
                'p0[scal]   = '+str(p0) ])

# Time integration
time_integration = model.set_time_integration( scheme='cf3.sdm.ExplicitRungeKuttaLowStorage2' )
time_integration.scheme.nb_stages = 4
time_integration.step.cfl = 0.2

# Domain discretization
convection = model.domain_discretization.create_term(name='convection',type='cf3.sdm.lineuler.ConvectionNonUniformMeanflow2D')
convection.mean_flow = mean_flow
convection.gamma = gamma

model.time_stepping.end_time = 90
model.time_stepping.end_time = 1
model.time_stepping.time_step = 10

## function describing entropy and vortex without acoustic pulse
#entropy_vortex = [
# '0.1*exp( -log(2.)*((x-67)^2 + y^2)/25. )',
# ' 0.04*y      *exp( -log(2.)*((x-67)^2+y^2)/25. )',
# '-0.04*(x-67)*exp( -log(2.)*((x-67)^2+y^2)/25. )',
# '0'
#]

## function describing acoustic pulse only
#acoustic = [
# 'exp( -log(2.)*((x)^2+y^2)/9. )',
# ' 0',
# '-0',
# 'exp( -log(2.)*((x)^2+y^2)/9. )'
#]

## Configure which of the above functions to set as initial condition
#initial_condition.options().set('functions',acoustic_entropy_vortex)
#solver.get_child('InitialConditions').execute();

#### Create convection term
#convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.lineuler.Convection2D', regions=[mesh.access_component('topology/domain').uri()])
#convection.options().set('gamma',gamma)
#convection.options().set('rho0',1.)
#convection.options().set('U0',[.5,0.])
#convection.options().set('p0',1.)

### subsonic outlet BC at tob bottom right
bc = model.boundary_conditions.create_boundary_condition(name='null',type='cf3.sdm.lineuler.BCSubsonicOutlet2D',regions=[
mesh.topology.right,
mesh.topology.bottom,
mesh.topology.top,
])
bc.c0 = c0
bc.U0 = U0

### Impose zero variation at left boundary. This should be replaced by a subsonic inlet BC
impose = model.boundary_conditions.create_boundary_condition(
  name='impose',
  type='cf3.sdm.BCConstant<4,2>',
  regions=[mesh.topology.left] )
impose.options().set('constants',[0,0,0,0])

#######################################
# SIMULATE
#######################################

while not model.time_stepping.properties.finished :

    model.time_stepping.do_step()

    mesh.write_mesh(file=URI('solution'+str(model.time_stepping.step)+'.msh'),fields=[solution.uri()])

#######################################
# POST-PROCESSING
#######################################

#compute_char = model.tools.create_component('compute_characteristics','cf3.sdm.lineuler.ComputeCharacteristicVariables')
#compute_char.options().set('normal',[1.,0.])
#compute_char.options().set('solver',solver)
#compute_char.options().set('mesh',mesh)
#compute_char.options().set('physical_model',physics)
#compute_char.execute()

########################
# OUTPUT
########################

fields = [
mesh.access_component('solution_space/solution').uri(),
#mesh.access_component('solution_space/char').uri()
]

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().set('mesh',mesh)
gmsh_writer.options().set('fields',fields)
gmsh_writer.options().set('file',URI('file:sdm_output.msh'))
gmsh_writer.execute()


# Tecplot
#########
# Tecplot cannot write high-order meshes. A finer P1 mesh is generated,
# and fields are interpolated to the P1-mesh. The mesh is finer to visualize
# the high-order solution better.

# Generate visualization mesh
visualization_mesh = model.domain.create_component('visualization_mesh','cf3.mesh.Mesh')
mesh_generator = model.domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",visualization_mesh.uri())
mesh_generator.options().set("nb_cells",[400,400])
mesh_generator.options().set("lengths",[200,200])
mesh_generator.options().set("offsets",[-100,-100])
mesh_generator.execute()

# Interpolate fields using solution polynomial
visualization_mesh.get_child('geometry').create_field(name='solution',       variables='rho[1],rho0U[2],p[1]')
#visualization_mesh.get_child('geometry').create_field(name='char', variables='S[1],Shear[1],Aplus[1],Amin[1],A[1],omega[1]')

interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.Interpolator')
interpolator.interpolate(source=mesh.access_component("solution_space/solution").uri(),
												 target=visualization_mesh.access_component("geometry/solution").uri())
#interpolator.interpolate(source=mesh.access_component("solution_space/char").uri(),
#												 target=visualization_mesh.access_component("geometry/char").uri())

fields = [
visualization_mesh.access_component('geometry/solution').uri(),
#visualization_mesh.access_component('geometry/char').uri()
]

# Write visualization mesh
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().set('mesh',visualization_mesh)
tec_writer.options().set('fields',fields)
tec_writer.options().set('cell_centred',True)
tec_writer.options().set('file',URI('file:sdm_output.plt'))
tec_writer.execute()

#####################
# Probe line y=0
#####################

# Generate 1D line mesh, for now only y=0 can be probed as the line has 1D coordinates only
probe_mesh = model.domain.create_component('probe_mesh','cf3.mesh.Mesh')
mesh_generator = model.domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",probe_mesh.uri())
mesh_generator.options().set("nb_cells",[1000])
mesh_generator.options().set("lengths",[200])
mesh_generator.options().set("offsets",[-100])
mesh_generator.execute()

# Interpolate fields
probe_mesh.get_child('geometry').create_field(name='solution', variables='rho[1],rho0U[2],p[1]')
#probe_mesh.get_child('geometry').create_field(name='char',     variables='S[1],Shear[1],Aplus[1],Amin[1],A[1],omega[1]')

interpolator.interpolate(source=mesh.access_component("solution_space/solution").uri(),
												 target=probe_mesh.access_component("geometry/solution").uri())
#interpolator.interpolate(source=mesh.access_component("solution_space/char").uri(),
#												 target=probe_mesh.access_component("geometry/char").uri())

fields = [
probe_mesh.access_component('geometry/solution').uri(),
#probe_mesh.access_component('geometry/char').uri()
]

# Write probe mesh
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().set('mesh',probe_mesh)
tec_writer.options().set('fields',fields)
tec_writer.options().set('cell_centred',True)
tec_writer.options().set('file',URI('file:probe_liney0.plt'))
tec_writer.execute()

