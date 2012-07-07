# import sys
# sys.path.append('/Users/willem/workspace/cf3/dev/builds/clang/debug/dso/')

import coolfluid as cf
import math

### Logging configuration
cf.environment.assertion_backtrace = True
cf.environment.exception_backtrace = True
cf.environment.regist_signal_handlers = True
cf.environment.exception_log_level = 10
cf.environment.log_level = 3
cf.environment.exception_outputs = True

############################
# Create simulation
############################
model   = cf.root.create_component('cylinder_2d','cf3.solver.Model');
solver  = model.create_solver('cf3.sdm.SDSolver')
physics = model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
domain  = model.create_domain()

# mesh = domain.load_mesh(file = cf.URI('../../../resources/cylinder-quad-p2-16x4.msh'), name = 'cylinder2d');
mesh = domain.load_mesh(file = cf.URI('../../../resources/cylinder-quad-p2-32x8.msh'), name = 'cylinder2d');
# mesh = domain.load_mesh(file = cf.URI('../../../resources/cylinder-quad-p2-64x16.msh'), name = 'cylinder2d');
# mesh = domain.load_mesh(file = cf.URI('../../../resources/cylinder-quad-p2-128x32.msh'), name = 'cylinder2d');

### Configure physics
gamma = 1.4
R = 287.05
M_inf = 0.38
p_inf = 1.0
rho_inf = 1.0
c_inf = math.sqrt(gamma*p_inf/rho_inf)
u_inf = M_inf*c_inf
rhoE_inf = p_inf/(gamma-1) + 0.5 * rho_inf * u_inf**2
# p = (g-1) * ( rhoE - 0.5 * rho * ( u**2 ) )

physics.gamma = gamma
physics.R = R

### Configure solver
solver.mesh = mesh
solver.solution_vars = 'cf3.physics.NavierStokes.Cons2D'
solver.solution_order = 3
solver.iterative_solver = 'cf3.sdm.ExplicitRungeKuttaLowStorage2'

### Configure timestepping
solver.Time.end_time = 3000000
solver.Time.time_step = 1
solver.TimeStepping.time_accurate = True          # time accurate for initial stability
solver.TimeStepping.cfl = 'min(0.5,0.0001*i)'     # increasing cfl number
solver.TimeStepping.max_iteration = 100           # limit the number of iterations (default = no limit)
solver.TimeStepping.IterativeSolver.nb_stages = 3 # Runge Kutta number of stages

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.PrepareMesh.execute()

### Set the initial condition
functions = [
str(rho_inf),
'if( (x<-5) | (x>5) , '+str(rho_inf*u_inf)+' , 0.5*'+str(rho_inf*u_inf)+' )',
'0.',
str(rhoE_inf)
]
initial_condition = solver.InitialConditions.create_initial_condition( name = 'uniform')
initial_condition.functions = functions
solver.InitialConditions.execute()

### Create convection term
convection = solver.DomainDiscretization.create_term(name = 'convection', type = 'cf3.sdm.navierstokes.Convection2D')

### Create farfield boundary condition
bc_farfield = solver.BoundaryConditions.create_boundary_condition(
   name = 'farfield',
   type = 'cf3.sdm.BCConstant<4,2>',
   regions=[mesh.topology.boundary.uri()])

bc_farfield.constants = [rho_inf,rho_inf*u_inf,0.,rhoE_inf]

### Create wall boundary condition
bc_wall = solver.BoundaryConditions.create_boundary_condition(
   name = 'cylinder',
   type = 'cf3.sdm.navierstokes.BCWallEuler2D',
   regions=[mesh.topology.cylinder.uri()])

########################################
# Output initial condition (optional)
#######################################

### execute boundary condition for output for visualization
#solver.BoundaryConditions.execute();

# fields to output:
fields = [
mesh.solution_space.solution.uri(),
#mesh.solution_space.jacobian_determinant.uri(),
#mesh.solution_space.delta.uri()
]

gmsh_writer = model.create_component('init_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.mesh = mesh
gmsh_writer.fields = fields
gmsh_writer.file = cf.URI('file:initial.msh')
gmsh_writer.execute()

#######################################
# SIMULATE
#######################################
model.simulate()
print 'WARNING: not converged to steady state solution (would take too long) \n'

########################
# POST PROCESSING
########################

post_proc = mesh.solution_space.create_field(name='post_proc',variables='U[vec],p[1],T[1],M[1],Pt[1],Tt[1],Cp[1],S[1]')
solution=mesh.solution_space.solution
for index in range(len(solution)):
	 # compute variables per solution point
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

	 # copy now in post_proc field for this solution point
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
mesh.solution_space.solution.uri(),
mesh.solution_space.post_proc.uri(),
mesh.solution_space.wave_speed.uri()
]


# gmsh
######
gmsh_writer = model.create_component('writer','cf3.mesh.gmsh.Writer')
gmsh_writer.mesh = mesh
gmsh_writer.fields = fields
gmsh_writer.file = cf.URI('file:sdm_output.msh')
gmsh_writer.execute()

## Tecplot
##########
## Tecplot cannot write high-order meshes. A finer P1 mesh is loaded,
## and fields are interpolated to the P1-mesh. The mesh is finer to visualize
## the high-order solution better.

## Generate visualization mesh
#visualization_mesh = domain.load_mesh(file = cf.URI('../../../resources/cylinder-quad-p1-128x32.msh'), name = 'visualization_mesh');

## Interpolate fields using solution polynomial
#visualization_mesh.access_component('geometry').create_field(name='solution',  variables='rho[1],rhoU[2],rhoE[1]')
#visualization_mesh.access_component('geometry').create_field(name='wave_speed',variables='ws[1]')
#visualization_mesh.access_component('geometry').create_field(name='post_proc', variables='U[vec],p[1],T[1],M[1],Pt[1],Tt[1],Cp[1],S[1]')

#interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.actions.Interpolate')
#interpolator.interpolate(source=mesh.access_component("solution_space/solution").uri(),
#												 target=visualization_mesh.access_component("geometry/solution").uri())
#interpolator.interpolate(source=mesh.access_component("solution_space/wave_speed").uri(),
#													target=visualization_mesh.access_component("geometry/wave_speed").uri())
#interpolator.interpolate(source=mesh.access_component("solution_space/post_proc").uri(),
#												 target=visualization_mesh.access_component("geometry/post_proc").uri())

#fields = [
#visualization_mesh.access_component('geometry/solution').uri(),
#visualization_mesh.access_component('geometry/wave_speed').uri(),
#visualization_mesh.access_component('geometry/post_proc').uri()
#]

## Write visualization mesh
#tec_writer = model.get_child('tools').create_component('writer','cf3.mesh.tecplot.Writer')
#tec_writer.options().set('mesh',visualization_mesh)
#tec_writer.options().set('fields',fields)
#tec_writer.options().set('cell_centred',True)
#tec_writer.options().set('file',cf.URI('file:sdm_output.plt'))
#tec_writer.execute()
