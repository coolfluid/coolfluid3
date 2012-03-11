import sys
sys.path.append("/home/nils/Documents/Thesis/coolfluid3/build/dso")
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
model   = root.create_component('channel_movingref_2d','cf3.solver.ModelUnsteady');
time    = model.create_time()
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
domain  = model.create_domain()

###### Following generates a square mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().configure_option("mesh",mesh.uri())
mesh_generator.options().configure_option("nb_cells",[20,20])
mesh_generator.options().configure_option("lengths",[50,10])
mesh_generator.options().configure_option("offsets",[-25,-5])
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
solver.get_child('InitialConditions').create_initial_condition( name = 'channel')
functions = [
'4.696',
'20*4.696',
'0*4.696',
'101300/(1.4-1)+0.5*4.696*(20*20+0*0)'
]
solver.get_child('InitialConditions').get_child('channel').options().configure_option("functions",functions)
solver.get_child('InitialConditions').execute();

### Create convection term
# convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokes.Convection2D')
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokesmovingreference.Convection2D')
convection.options().configure_option("Omega", (0.0, 0.0, 0.0))
convection.options().configure_option("Vtrans", (0.0, 0.0))
convection.options().configure_option("gamma", 1.4)

### Create source term
source_term = solver.get_child('DomainDiscretization').create_term( name='source_term',
  type='cf3.sdm.navierstokesmovingreference.Source2D',
  regions=[mesh.access_component('topology/interior').uri()] )
source_term.options().configure_option("Omega", (0.0, 0.0, 10.0))
source_term.options().configure_option("Vtrans", (0.0, 0.0))
source_term.options().configure_option("gamma", 1.4)

wallbc = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'wallbc', type = 'cf3.sdm.navierstokes.BCWallEuler2D',
regions=[
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/bottom').uri()
])

inlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.sdm.navierstokes.BCSubsonicInletUT2D',
regions=[
mesh.access_component('topology/left').uri()
])
inlet.options().configure_option('U', (20., 0.))
inlet.options().configure_option('gamma', 1.4)
inlet.options().configure_option('T', 273.15)
inlet.options().configure_option('R', 287.05)

outlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.sdm.navierstokes.BCSubsonicOutlet2D',
regions=[
mesh.access_component('topology/right').uri()
])
outlet.options().configure_option('p', 101300)
outlet.options().configure_option('gamma', 1.4)

fields = [
mesh.access_component("solution_space/solution").uri(),
mesh.access_component("solution_space/wave_speed").uri(),
mesh.access_component("solution_space/residual").uri()
]

##### Write initial condition to file
gmsh_writer_init = model.create_component("writer", "cf3.mesh.gmsh.Writer")
gmsh_writer_init.options().configure_option("mesh",mesh)
gmsh_writer_init.options().configure_option('fields',fields)
gmsh_writer_init.options().configure_option("file",coolfluid.URI("file:sdm_output_init_channel.msh"))
gmsh_writer_init.options().configure_option("enable_surfaces",False)
gmsh_writer_init.execute()

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# OUTPUT
########################


# tecplot
#########
# tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
# tec_writer.options().configure_option("mesh",mesh)
# tec_writer.options().configure_option("fields",fields)
# tec_writer.options().configure_option("cell_centred",False)
# tec_writer.options().configure_option("file",coolfluid.URI("file:sdm_output.plt"))
# tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().configure_option("mesh",mesh)
gmsh_writer.options().configure_option("fields",fields)
gmsh_writer.options().configure_option("file",coolfluid.URI("file:sdm_output_channel.msh"))
gmsh_writer.options().configure_option("enable_surfaces",False)
gmsh_writer.execute()
