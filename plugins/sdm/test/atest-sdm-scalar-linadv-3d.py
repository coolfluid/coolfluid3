import sys
from coolfluid import *

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
model   = root.create_component('linear_advection_3d','cf3.solver.ModelUnsteady');
time    = model.create_time()
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.Scalar.Scalar3D')
domain  = model.create_domain()

###### Following generates a square mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.BlockMesh.ChannelGenerator")
mesh_generator.options().configure_option("mesh",mesh.uri())
mesh_generator.options().configure_option("x_segments",10)
mesh_generator.options().configure_option("y_segments_half",5)
mesh_generator.options().configure_option("z_segments",25)
mesh_generator.options().configure_option("length",10.)
mesh_generator.options().configure_option("half_height",5.)  #-5 to 5
mesh_generator.options().configure_option("width",25.)
mesh_generator.options().configure_option("grading",1.)
mesh_generator.execute()
# load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
# load_balance.options().configure_option("mesh",mesh)
# load_balance.execute()
#####

### Configure solver
solver.options().configure_option('time',time)
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.Scalar.LinearAdv2D')
solver.options().configure_option('solution_order',3)
solver.options().configure_option('iterative_solver','cf3.sdm.RungeKuttaLowStorage2')

### Configure timestepping
time.options().configure_option('time_step',1.);
time.options().configure_option('end_time',15);
solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
# solver.access_component('TimeStepping').options().configure_option('max_iteration',10);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
solver.get_child('InitialConditions').create_initial_condition( name = 'init')
functions = [
'sigma:=2; mu:=5; exp(-((x-mu)^2+(y)^2+(z-mu)^2)/(2*sigma^2))'
]
solver.get_child('InitialConditions').get_child('init').options().configure_option("functions",functions)
solver.get_child('InitialConditions').execute();


### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.scalar.LinearAdvection3D')
convection.options().configure_option("advection_speed",[0,0,1])
# nullbc = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'nullbc', type = 'cf3.sdm.BCNull',
# regions=[
# mesh.access_component('topology/left').uri(),
# mesh.access_component('topology/right').uri(),
# mesh.access_component('topology/top').uri(),
# mesh.access_component('topology/bottom').uri(),
# mesh.access_component('topology/front').uri(),
# mesh.access_component('topology/back').uri()
# ])

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# OUTPUT
########################

fields = [
mesh.access_component("solution_space/solution").uri(),
mesh.access_component("solution_space/wave_speed").uri(),
mesh.access_component("solution_space/residual").uri(),
mesh.access_component("solution_space/convection").uri(),
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().configure_option("mesh",mesh)
tec_writer.options().configure_option("fields",fields)
tec_writer.options().configure_option("cell_centred",False)
tec_writer.options().configure_option("file",URI("file:sdm_output.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().configure_option("mesh",mesh)
gmsh_writer.options().configure_option("fields",fields)
gmsh_writer.options().configure_option("file",URI("file:sdm_output.msh"))
gmsh_writer.options().configure_option("enable_surfaces",False)
gmsh_writer.execute()
