import sys
sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/debug/dso/')
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
model = root.create_component('cylinder_2d','cf3.solver.CModel');
model.create_solver('cf3.SFDM.SFDSolver')
model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
model.create_domain()
physics = model.get_child('NavierStokes2D')
physics.options().configure_option('gamma',1.4)
physics.options().configure_option('R',287.05)
solver  = model.get_child('SFDSolver')
domain  = model.get_child('Domain')
domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/cylinderO2d.msh'), name = 'cylinder2d');
mesh = domain.access_component('cylinder2d');
# domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/rectangle2d.msh'), name = 'rectangle2d');
# mesh = domain.access_component('rectangle2d');
# domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/circle.msh'), name = 'circle');
# mesh = domain.access_component('circle');

gmsh_writer = model.create_component('load_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('file',coolfluid.URI('file:load.msh'))
gmsh_writer.execute()


# ###### Following generates a square mesh
# mesh = domain.create_component('mesh','cf3.mesh.Mesh')
# mesh_generator = domain.create_component('mesh_generator','cf3.mesh.SimpleMeshGenerator')
# mesh_generator.options().configure_option('mesh',mesh.uri())
# mesh_generator.options().configure_option('nb_cells',[10,4])
# mesh_generator.options().configure_option('lengths',[5,2])
# mesh_generator.options().configure_option('offsets',[0,0])
# mesh_generator.execute()
# load_balance = mesh_generator.create_component('load_balancer','cf3.mesh.actions.LoadBalance')
# load_balance.options().configure_option('mesh',mesh)
# load_balance.execute()
# #####

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.NavierStokes.Cons2D')
solver.options().configure_option('solution_order',4)

### Configure timestepping
solver.access_component('TimeStepping').options().configure_option('time_accurate',True);
solver.access_component('TimeStepping').options().configure_option('cfl','min(0.2,0.0001*i)');
solver.access_component('TimeStepping').options().configure_option('max_iteration',20);
solver.access_component('TimeStepping/Time').options().configure_option('time_step',1.);
solver.access_component('TimeStepping/Time').options().configure_option('end_time',3000000.0);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
M_inf = 0.38
p_inf = 1
rho_inf = 1
c_inf = math.sqrt(1.4*p_inf/rho_inf)
u_inf = M_inf*c_inf
solver.get_child('InitialConditions').create_initial_condition( name = 'uniform')
functions = [
'1.',
'if(x<-5|x>5,0.38*sqrt(1.4),0.5*0.38*sqrt(1.4))',
'0.',
'1.'
]
physics.create_variables(name='input_vars',type='cf3.physics.NavierStokes.Prim2D')
solver.get_child('InitialConditions').get_child('uniform').options().configure_option('functions',functions)
solver.get_child('InitialConditions').get_child('uniform').options().configure_option('input_vars',physics.access_component('input_vars'))
solver.get_child('InitialConditions').execute();

### Create convection term
solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.SFDM.navierstokes.Convection2D')
convection = solver.access_component('DomainDiscretization/Terms/convection')

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.SFDM.navierstokes.BCSubsonicInletUT2D', regions=[mesh.access_component('topology/left').uri()])
# inlet = solver.access_component('BoundaryConditions/BCs/inlet')
# inlet.options().configure_option('T',298.15)
# inlet.options().configure_option('U',[65.,0.])

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.SFDM.navierstokes.BCSubsonicInletTtPtAlpha2D', regions=[mesh.access_component('topology/left').uri()])
# inlet = solver.access_component('BoundaryConditions/BCs/inlet')
# inlet.options().configure_option('Tt',300)
# inlet.options().configure_option('Pt',110000)
# # inlet.options().configure_option('alpha',3.1415/4.)

solver.get_child('BoundaryConditions').create_boundary_condition(name= 'farfield', type = 'cf3.SFDM.BCConstant<4,2>', regions=[
mesh.access_component('topology/boundary').uri()
# mesh.access_component('topology/bottom').uri(),
# mesh.access_component('topology/left').uri(),
# mesh.access_component('topology/right').uri(),
])
farfield = solver.access_component('BoundaryConditions/BCs/farfield')
farfield.options().configure_option('constants',[1.,u_inf*1.,0.,1./0.4+0.5*1.*u_inf**2])
# p = (g-1) * ( rhoE - 0.5 * rho * ( u**2 ) )
# rhoE = p/(g-1) + 0.5 * rho * u**2

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'wall', type = 'cf3.SFDM.navierstokes.BCWallEuler2D', regions=[
# mesh.access_component('topology/top').uri(),
# mesh.access_component('topology/bottom').uri()
# ])
# wall = solver.access_component('BoundaryConditions/BCs/wall')

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'nullbc', type = 'cf3.SFDM.BCNull', 
# regions=[
# mesh.access_component('topology/left').uri(),
# mesh.access_component('topology/right').uri(),
# mesh.access_component('topology/top').uri(),
# mesh.access_component('topology/bottom').uri()
# ])
# nullbc = solver.access_component('BoundaryConditions/BCs/nullbc')


# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.SFDM.navierstokes.BCSubsonicOutlet2D', regions=[
# mesh.access_component('topology/right').uri()
# ])
# outlet = solver.access_component('BoundaryConditions/BCs/outlet')
# outlet.options().configure_option('p',1.04e5)

solver.get_child('BoundaryConditions').create_boundary_condition(name= 'cylinder', type = 'cf3.SFDM.navierstokes.BCWallEuler2D', regions=[mesh.access_component('topology/cylinder').uri()])
cylinder = solver.access_component('BoundaryConditions/BCs/cylinder')

solver.get_child('BoundaryConditions').execute();

fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/jacobian_determinant').uri(),
mesh.access_component('solution_space/delta').uri()
]

gmsh_writer = model.create_component('init_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:initial.msh'))
gmsh_writer.execute()

#######################################
# SIMULATE
#######################################
print 'START SIMULATION'
model.simulate()

########################
# POST PROCESSING
########################

mesh.access_component('solution_space').create_field(name='post_proc',variables='U[vec],p[1],T[1],M[1],Pt[1],Tt[1],Cp[1],S[1]')
post_proc=mesh.access_component('solution_space/post_proc')
solution=mesh.access_component('solution_space/solution')
g=1.4;
gm1=0.4;
R=287.05;
for index in range(len(solution)):
	 rho=solution[index][0];
	 u=solution[index][1]/solution[index][0];
	 v=solution[index][2]/solution[index][0];
	 rhoE=solution[index][3];
	 p=gm1*(rhoE - 0.5*rho*(u**2+v**2));
	 T=p/(rho*R);
	 M=math.sqrt(u**2+v**2)/math.sqrt(abs(g*p/rho))
	 Pt=p+0.5*rho*(u**2+v**2);
	 Tt=T*(1+(gm1/2))*M**2;
	 Cp=(p-p_inf)/(0.5*rho_inf*u_inf**2);
	 S=p/(abs(rho)**g)
	 post_proc[index][0] = u;
	 post_proc[index][1] = v;
	 post_proc[index][2] = p;
	 post_proc[index][3] = T;
	 post_proc[index][4] = M;
	 post_proc[index][5] = Pt;
	 post_proc[index][6] = Tt;
	 post_proc[index][7] = Cp;
	 post_proc[index][8] = S;

########################
# OUTPUT
########################

# fields to output:
fields = [
mesh.access_component('solution_space/solution').uri(),
mesh.access_component('solution_space/post_proc').uri(),
mesh.access_component('solution_space/wave_speed').uri(),
# mesh.access_component('solution_space/update_coefficient').uri(),
# mesh.access_component('solution_space/residual').uri(),
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
tec_writer.options().configure_option('mesh',mesh.uri())
tec_writer.options().configure_option('fields',fields)
tec_writer.options().configure_option('cell_centred',False)
tec_writer.options().configure_option('file',coolfluid.URI('file:sfdm_output.plt'))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:sfdm_output.msh'))
gmsh_writer.execute()
