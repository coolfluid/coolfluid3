# import sys
# sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/debug/dso/')

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
############################
model   = root.create_component('cylinder_2d','cf3.solver.CModel');
solver  = model.create_solver('cf3.SFDM.SFDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
domain  = model.create_domain()

# mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/cylinder-quad-p2-16x4.msh'), name = 'cylinder2d');
mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/cylinder-quad-p2-32x8.msh'), name = 'cylinder2d');
# mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/cylinder-quad-p2-64x16.msh'), name = 'cylinder2d');
# mesh = domain.load_mesh(file = coolfluid.URI('../../../resources/cylinder-quad-p2-128x32.msh'), name = 'cylinder2d');

gmsh_writer = model.create_component('load_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('file',coolfluid.URI('file:load.msh'))
gmsh_writer.execute()

### Configure physics
gamma = 1.4
R = 287.05
M_inf = 0.38
p_inf = 1
rho_inf = 1
c_inf = math.sqrt(gamma*p_inf/rho_inf)
u_inf = M_inf*c_inf
rhoE_inf = p_inf/(gamma-1) + 0.5 * rho_inf * u_inf**2
# p = (g-1) * ( rhoE - 0.5 * rho * ( u**2 ) )

physics.options().configure_option('gamma',gamma)
physics.options().configure_option('R',R)

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.NavierStokes.Cons2D')
solver.options().configure_option('solution_order',3)

### Configure timestepping
solver.access_component('TimeStepping').options().configure_option('time_accurate',True);         # time accurate for initial stability
solver.access_component('TimeStepping').options().configure_option('cfl','min(0.5,0.0001*i)');    # increasing cfl number
solver.access_component('TimeStepping').options().configure_option('max_iteration',10);           # limit the number of iterations (default = no limit)
solver.access_component('TimeStepping/Time').options().configure_option('time_step',1.);          # timestep that must be reached
solver.access_component('TimeStepping/Time').options().configure_option('end_time',3000000.0);    # limit the final time
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',3) # Runge Kutta number of stages

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
physics.create_variables(name='input_vars',type='cf3.physics.NavierStokes.Prim2D')
functions = [
str(rho_inf),
'if( (x<-5) | (x>5) , '+str(u_inf)+' , 0.5*'+str(u_inf)+' )',
'0.',
str(p_inf)
]
initial_condition = solver.get_child('InitialConditions').create_initial_condition( name = 'uniform')
initial_condition.options().configure_option('functions',functions)
initial_condition.options().configure_option('input_vars',physics.access_component('input_vars'))
solver.get_child('InitialConditions').execute();

### Create convection term
solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.SFDM.navierstokes.Convection2D')
convection = solver.access_component('DomainDiscretization/Terms/convection')

### Create farfield boundary condition
bc_farfield = solver.get_child('BoundaryConditions').create_boundary_condition(
   name = 'farfield', 
   type = 'cf3.SFDM.BCConstant<4,2>', 
   regions=[mesh.access_component('topology/boundary').uri()])
bc_farfield.options().configure_option('constants',[rho_inf,rho_inf*u_inf,0.,rhoE_inf])

### Create wall boundary condition
bc_wall = solver.get_child('BoundaryConditions').create_boundary_condition(
   name = 'cylinder', 
   type = 'cf3.SFDM.navierstokes.BCWallEuler2D', 
   regions=[mesh.access_component('topology/cylinder').uri()])

########################################
# Output initial condition (optional)
#######################################

### execute boundary condition for output for visualization
solver.get_child('BoundaryConditions').execute();

# fields to output:
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
model.simulate()
print 'WARNING: not converged to steady state solution (would take too long) \n'

########################
# POST PROCESSING
########################

post_proc = mesh.access_component('solution_space').create_field(name='post_proc',variables='U[vec],p[1],T[1],M[1],Pt[1],Tt[1],Cp[1],S[1]')
solution=mesh.access_component('solution_space/solution')
for index in range(len(solution)):
	 rho=solution[index][0];
	 u=solution[index][1]/solution[index][0];
	 v=solution[index][2]/solution[index][0];
	 rhoE=solution[index][3];
	 p=(gamma-1)*(rhoE - 0.5*rho*(u**2+v**2));
	 T=p/(rho*R);
	 M=math.sqrt(u**2+v**2)/math.sqrt(abs(gamma*p/rho));
	 Pt=p+0.5*rho*(u**2+v**2);
	 Tt=T*(1+((gamma-1)/2))*M**2;
	 Cp=(p-p_inf)/(0.5*rho_inf*u_inf**2);
	 S=p/(abs(rho)**gamma);
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
mesh.access_component('solution_space/wave_speed').uri()
]


# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',mesh.uri())
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',coolfluid.URI('file:sfdm_output.msh'))
gmsh_writer.execute()

# tecplot (cannot output P2 meshes)
#########
# tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
# tec_writer.options().configure_option('mesh',mesh.uri())
# tec_writer.options().configure_option('fields',fields)
# tec_writer.options().configure_option('cell_centred',False)
# tec_writer.options().configure_option('file',coolfluid.URI('file:sfdm_output.plt'))
# tec_writer.execute()
