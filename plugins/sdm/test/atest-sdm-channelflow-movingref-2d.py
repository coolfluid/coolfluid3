import sys
sys.path.append("/home/nils/Documents/Thesis/coolfluid3/build/dso")
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
model   = root.create_component('channel_movingref_2d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
domain  = model.create_domain()

###### Following generates a square mesh
mesh = domain.create_component('mesh','cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("nb_cells",[40,20])
mesh_generator.options().set("lengths",[100,10])
mesh_generator.options().set("offsets",[0,-5])
mesh_generator.execute()
load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
load_balance.options().set("mesh",mesh)
load_balance.execute()
#####

omega = 2.0

g = 1.4
R = 287.05
T = 273.15
u = 50.
p = 100000.
rho = p/(R*T)
c = math.sqrt(g*R*T)
M = u/c
alpha = 0.
Pt=p*( (1.+((g-1.)/2.)*M**2) )**(g/(g-1.))
Tt=T*(1.+((g-1.)/2.)*M**2)

print "c=",c
print "M=",M
print "rho=",rho
print "Pt=",Pt
print "Tt=",Tt

### Configure physics
physics.options().set('gamma',g)
physics.options().set('R',R)

### Configure solver
#solver.options().set('time',time)
solver.options().set('mesh',mesh)
solver.options().set('solution_vars','cf3.physics.NavierStokes.Cons2D')
solver.options().set('solution_order',1)
solver.options().set('iterative_solver','cf3.sdm.ExplicitRungeKuttaLowStorage2')

### Configure timestepping
solver.access_component('Time').options().set('end_time',100)
solver.access_component('Time').options().set('time_step',1)
solver.access_component('TimeStepping').options().set('time_accurate',True);
solver.access_component('TimeStepping').options().set('cfl','0.5');
solver.access_component('TimeStepping').options().set('max_iteration',10);
solver.access_component('TimeStepping/IterativeSolver').options().set('nb_stages',1)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
solver.get_child('InitialConditions').create_initial_condition( name = 'channel')
functions = [
str(rho),
str(rho*u),
'0.0',
str(p/(g-1.)+0.5*rho*(u*u))
]
solver.get_child('InitialConditions').get_child('channel').options().set("functions",functions)
solver.get_child('InitialConditions').execute();

### Create convection term
## original
#convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokes.Convection2D')

## movingref
convection = solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.sdm.navierstokesmovingreference.Convection2D')
convection.options().set("gamma", g)
convection.options().set("Omega", [0.0, 0.0, omega])
convection.options().set("Vtrans", [0.0, 0.0])


### Create source term
source_term = solver.get_child('DomainDiscretization').create_term( name='source_term',
  type='cf3.sdm.navierstokesmovingreference.Source2D',
  regions=[mesh.access_component('topology/interior').uri()] )
source_term.options().set("Omega", [0.0, 0.0, omega])
source_term.options().set("Vtrans", [0.0, 0.0])
source_term.options().set("gamma", g)

wallbc = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'wallbc', type = 'cf3.sdm.navierstokes.BCWallEuler2D',
regions=[
mesh.access_component('topology/top').uri(),
mesh.access_component('topology/bottom').uri()
])

# inlet movingref
inlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.sdm.navierstokesmovingreference.BCSubsonicInletTtPtAlpha2D',
regions=[mesh.access_component('topology/left').uri()])
inlet.options().set('gamma',g)
inlet.options().set('Tt',str(Tt))
inlet.options().set('Pt',str(Pt))
inlet.options().set('alpha',str(alpha))
inlet.options().set('R', R)
inlet.options().set('Omega', [0.0, 0.0, omega])

## original inlet TtPtAlpha
#inlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.sdm.navierstokes.BCSubsonicInletTtPtAlpha2D',
#regions=[
#mesh.access_component('topology/left').uri()
#])
#inlet.options().set('gamma',g)
#inlet.options().set('Tt',str(Tt))
#inlet.options().set('Pt',str(Pt))
#inlet.options().set('alpha',str(alpha))
#inlet.options().set('R', R)

# closed outlet , also nice to see
#outlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.sdm.navierstokes.BCWallEuler2D',
#regions=[
#mesh.access_component('topology/right').uri(),
#])

# pressure outlet, movingref
outlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.sdm.navierstokesmovingreference.BCSubsonicOutlet2D',
regions=[
mesh.access_component('topology/right').uri()
])
outlet.options().set('P', str(p))
outlet.options().set('gamma', g)
outlet.options().set('Omega', [0.0, 0.0, omega])

# original outlet, navierstokes
#outlet = solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.sdm.navierstokes.BCSubsonicOutlet2D',
#regions=[
#mesh.access_component('topology/right').uri()
#])
#outlet.options().set('p', str(p))
#outlet.options().set('gamma', g)

fields = [
mesh.access_component("solution_space/solution").uri(),
mesh.access_component("solution_space/wave_speed").uri(),
mesh.access_component("solution_space/residual").uri()
]

##### Write initial condition to file
gmsh_writer_init = model.create_component("writer", "cf3.mesh.gmsh.Writer")
gmsh_writer_init.options().set("mesh",mesh)
gmsh_writer_init.options().set('fields',fields)
gmsh_writer_init.options().set("file",coolfluid.URI("file:sdm_output_init_channel.msh"))
# gmsh_writer_init.options().set("enable_surfaces",False)
gmsh_writer_init.execute()

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# POST PROCESSING
########################
gamma=1.4
R=287.05
post_proc = mesh.access_component('solution_space').create_field(name='post_proc',variables='U[vec],p[1],T[1],M[1],Pt[1],Tt[1],S[1]')
solution=mesh.access_component('solution_space/solution')
for index in range(len(solution)):
   # compute variables per solution point
   rho=solution[index][0];
   u=solution[index][1]/solution[index][0];
   v=0 #solution[index][2]/solution[index][0];
   rhoE=solution[index][3];
   p=(gamma-1)*(rhoE - 0.5*rho*(u**2+v**2));
   T=p/(rho*R);
   M=math.sqrt(u**2+v**2)/math.sqrt(abs(gamma*p/rho));
   Pt=p*( (1.+((g-1.)/2.)*M**2) )**(g/(g-1.))
   Tt=T*(1.+((g-1.)/2.)*M**2)
   S=p/(abs(rho)**gamma);

   # copy now in post_proc field for this solution point
   post_proc[index][0] = u;
   post_proc[index][1] = v;
   post_proc[index][2] = p;
   post_proc[index][3] = T;
   post_proc[index][4] = M;
   post_proc[index][5] = Pt;
   post_proc[index][6] = Tt;
   post_proc[index][7] = S;


########################
# OUTPUT
########################

fields = [
mesh.access_component("solution_space/solution").uri(),
mesh.access_component("solution_space/wave_speed").uri(),
mesh.access_component("solution_space/residual").uri(),
mesh.access_component("solution_space/post_proc").uri()
]


# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().set("mesh",mesh)
tec_writer.options().set("fields",fields)
tec_writer.options().set("cell_centred",False)
tec_writer.options().set("file",coolfluid.URI("file:sdm_output_channel.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().set("mesh",mesh)
gmsh_writer.options().set("fields",fields)
gmsh_writer.options().set("file",coolfluid.URI("file:sdm_output_channel.msh"))
# gmsh_writer.options().set("enable_surfaces",False)
gmsh_writer.execute()
