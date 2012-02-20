import sys
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
model   = root.create_component('shocktube_2d','cf3.solver.ModelUnsteady');
time    = model.create_time()
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
domain  = model.create_domain()

###### Following generates a square mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().configure_option("mesh",mesh.uri())
mesh_generator.options().configure_option("nb_cells",[20,20])
mesh_generator.options().configure_option("lengths",[10,10])
mesh_generator.options().configure_option("offsets",[-5,-5])
mesh_generator.execute()
load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
load_balance.options().configure_option("mesh",mesh)
load_balance.execute()
#####

### Configure physics
physics.options().configure_option('gamma',1.4)
physics.options().configure_option('R',287.05)

### Configure solver
solver.options().configure_option('time',time)
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.NavierStokes.Cons2D')
solver.options().configure_option('solution_order',3)
solver.options().configure_option('iterative_solver','cf3.sdm.RungeKuttaLowStorage2')

### Configure timestepping
time.options().configure_option('time_step',1.);
time.options().configure_option('end_time',0.008);
solver.access_component('TimeStepping').options().configure_option('cfl','0.2');
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
solver.get_child('InitialConditions').create_initial_condition( name = 'shocktube')
functions = [
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; v_L:=0; v_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0 & y<=0,r_L,r_R)',
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; v_L:=0; v_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0 & y<=0,r_L*u_L,r_R*u_R)',
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; v_L:=0; v_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0 & y<=0,r_L*v_L,r_R*v_R)',
'r_L:=4.696; r_R:=1.408; u_L:=0; u_R:=0; v_L:=0; v_R:=0; p_L:=404400; p_R:=101100; g:=1.4; if(x<=0 & y<=0,p_L/(g-1)+0.5*r_L*(u_L*u_L+v_L*v_L),p_R/(g-1)+0.5*r_R*(u_R*u_R*v_R*v_R))'
]
solver.get_child('InitialConditions').get_child('shocktube').options().configure_option("functions",functions)
solver.get_child('InitialConditions').execute();

### Create convection term
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokes.Convection2D')

nullbc = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'nullbc', type = 'cf3.sdm.BCNull',
regions=[
mesh.access_component('topology/left').uri(),
mesh.access_component('topology/right').uri(),
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/bottom').uri()
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
mesh.access_component("solution_space/wave_speed").uri(),
mesh.access_component("solution_space/residual").uri()
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().configure_option("mesh",mesh.uri())
tec_writer.options().configure_option("fields",fields)
tec_writer.options().configure_option("cell_centred",False)
tec_writer.options().configure_option("file",coolfluid.URI("file:sdm_output.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().configure_option("mesh",mesh.uri())
gmsh_writer.options().configure_option("fields",fields)
gmsh_writer.options().configure_option("file",coolfluid.URI("file:sdm_output.msh"))
gmsh_writer.execute()
