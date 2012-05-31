import math
import sys
from coolfluid import *

# The cf root component
root = Core.root()
env =  Core.environment()

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
model   = root.create_component('linear_advection_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.Scalar.Scalar3D')
domain  = model.create_domain()

###### Following generates a square mesh
nb_div = 10
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.BlockMesh.ChannelGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("x_segments",nb_div)
mesh_generator.options().set("y_segments_half",nb_div/2)
mesh_generator.options().set("z_segments",nb_div)
mesh_generator.options().set("length",1.)
mesh_generator.options().set("half_height",.5)  #-5 to 5
mesh_generator.options().set("width",1.)
mesh_generator.options().set("grading",1.)
mesh_generator.execute()
# load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
# load_balance.options().set("mesh",mesh)
# load_balance.execute()
#####

### Configure solver
solver.options().set('time',time)
solver.options().set('mesh',mesh)
solver.options().set('solution_vars','cf3.physics.Scalar.LinearAdv2D')
solver.options().set('solution_order',3)
solver.options().set('iterative_solver','cf3.sdm.ExplicitRungeKuttaLowStorage2')

### Configure timestepping
time.options().set('end_time',1.5);
solver.access_component('TimeStepping').options().set('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',4)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
#solver.get_child('InitialConditions').create_initial_condition( name = 'init')
#functions = [
#'sigma:=2; mu:=5; exp(-((x-mu)^2+(y)^2+(z-mu)^2)/(2*sigma^2))'
#]
#functions = ['sin(x)']
#solver.get_child('InitialConditions').get_child('init').options().set("functions",functions)
#solver.get_child('InitialConditions').execute();


### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.scalar.LinearAdvection3D')
convection.options().set("advection_speed",[1,0,0])

bc_function = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'function', type = 'cf3.sdm.BCFunction<1,3>',
regions=[
mesh.access_component('topology/left').uri(),
])
bc_function.options().set('functions',['cos(2*pi*(y-0.5+z))'])

bc_extrapolate = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'function', type = 'cf3.sdm.BCExtrapolate<1,3>',
regions=[
mesh.access_component('topology/front').uri(),
mesh.access_component('topology/bottom').uri(),
mesh.access_component('topology/right').uri(),
mesh.access_component('topology/back').uri(),
mesh.access_component('topology/top').uri(),
])

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# OUTPUT
########################

fields = [
mesh.access_component("solution_space/solution").uri(),
]


# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().set("mesh",mesh)
tec_writer.options().set("fields",fields)
tec_writer.options().set("cell_centred",False)
tec_writer.options().set("file",URI("file:sdm_output.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().set("mesh",mesh)
gmsh_writer.options().set("fields",fields)
gmsh_writer.options().set("file",URI("file:sdm_output.msh"))
gmsh_writer.options().set("enable_surfaces",False)
gmsh_writer.execute()
