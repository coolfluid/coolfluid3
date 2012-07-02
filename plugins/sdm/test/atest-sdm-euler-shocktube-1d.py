import sys
import coolfluid

# The cf root component
root = coolfluid.Core.root()
env =  coolfluid.Core.environment()

### Logging configuration
env.options().set('assertion_backtrace', True)
env.options().set('exception_backtrace', True)
env.options().set('regist_signal_handlers', True)
env.options().set('exception_log_level', 10)
env.options().set('log_level', 1)
env.options().set('exception_outputs', True)

############################
# Create simulation
############################
model   = root.create_component('shocktube_1d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes1D')
domain  = model.create_domain()

###### Following generates a line mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("nb_cells",[70])
mesh_generator.options().set("lengths",[10])
mesh_generator.options().set("offsets",[-5])
mesh_generator.execute()
load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
load_balance.options().set("mesh",mesh)
load_balance.execute()
#####

### Configure physics
physics.options().set('gamma',1.4)
physics.options().set('R',287.05)

### Configure solver
solver.options().set('mesh',mesh)
solver.options().set('solution_vars','cf3.physics.NavierStokes.Cons1D')
solver.options().set('solution_order',3)
solver.options().set('iterative_solver','cf3.sdm.ExplicitRungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('Time').options().set('time_step',1.);
solver.access_component('Time').options().set('end_time',0.008);
solver.access_component('TimeStepping').options().set('time_accurate',True);
solver.access_component('TimeStepping').options().set('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
functions = [
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0,r_L,r_R)',
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0,r_L*u_L,r_R*u_R)',
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0,p_L/(g-1)+0.5*r_L*u_L*u_L,p_R/(g-1)+0.5*r_R*u_R*u_R)'
]
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'shocktube')
initial_condition.options().set("functions",functions)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokes.Convection1D')

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
mesh.access_component("solution_space/residual").uri()
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().set("mesh",mesh)
tec_writer.options().set("fields",fields)
tec_writer.options().set("cell_centred",False)
tec_writer.options().set("file",coolfluid.URI("file:sdm_output.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().set("mesh",mesh)
gmsh_writer.options().set("fields",fields)
gmsh_writer.options().set("file",coolfluid.URI("file:sdm_output.msh"))
gmsh_writer.execute()
