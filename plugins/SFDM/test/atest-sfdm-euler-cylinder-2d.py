import sys
sys.path.append('/Users/willem/workspace/coolfluid3/dev/builds/clang/debug/dso/')
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
model = root.create_component('cylinder_2d','cf3.solver.CModel');
model.create_solver('cf3.SFDM.SFDSolver')
model.create_physics('cf3.physics.NavierStokes.NavierStokes2D')
model.create_domain()
physics = model.get_child('NavierStokes2D')
solver  = model.get_child('SFDSolver')
domain  = model.get_child('Domain')
# domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/cylinder2d.msh'), name = 'cylinder2d');
# mesh = domain.access_component('cylinder2d');
domain.load_mesh(file = coolfluid.URI('/Users/willem/Desktop/rectangle2d.msh'), name = 'rectangle2d');
mesh = domain.access_component('rectangle2d');

# ###### Following generates a square mesh
# mesh = domain.create_component('mesh','cf3.mesh.Mesh')
# mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
# mesh_generator.options().configure_option("mesh",mesh.uri())
# mesh_generator.options().configure_option("nb_cells",[10,4])
# mesh_generator.options().configure_option("lengths",[5,2])
# mesh_generator.options().configure_option("offsets",[0,0])
# mesh_generator.execute()
# load_balance = mesh_generator.create_component("load_balancer","cf3.mesh.actions.LoadBalance")
# load_balance.options().configure_option("mesh",mesh)
# load_balance.execute()
# #####

### Configure solver
solver.options().configure_option('mesh',mesh)
solver.options().configure_option('solution_vars','cf3.physics.NavierStokes.Cons2D')
solver.options().configure_option('solution_order',1)

### Configure timestepping
solver.access_component('TimeStepping/Time').options().configure_option("time_step",1.);
solver.access_component('TimeStepping/Time').options().configure_option("end_time",.02);
solver.access_component('TimeStepping/IterativeSolver').options().configure_option('nb_stages',1)

### Prepare the mesh for Spectral Difference (build faces and fields etc...)
solver.get_child('PrepareMesh').execute()

### Set the initial condition
solver.get_child('InitialConditions').create_initial_condition( name = 'shocktube')
functions = [
'101300/(287.05*298.15)',
'30.',
'0.',
'101300'
]
solver.get_child('InitialConditions').get_child('shocktube').options().configure_option("functions",functions)
physics.create_variables(name='input_vars',type='cf3.physics.NavierStokes.Prim2D')
solver.get_child('InitialConditions').get_child('shocktube').options().configure_option("input_vars",physics.access_component('input_vars'))
solver.get_child('InitialConditions').execute();

### Create convection term
solver.get_child('DomainDiscretization').create_term(name = 'convection', type = 'cf3.SFDM.navierstokes.Convection2D')
convection = solver.access_component('DomainDiscretization/Terms/convection')
convection.options().configure_option('gamma',1.4)
convection.options().configure_option('R',287.05)

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.SFDM.navierstokes.BCSubsonicInletUT2D', regions=[mesh.access_component('topology/left').uri()])
# inlet = solver.access_component('BoundaryConditions/BCs/inlet')
# inlet.options().configure_option('T',298.15)
# inlet.options().configure_option('U',[110.,0.])

solver.get_child('BoundaryConditions').create_boundary_condition(name= 'inlet', type = 'cf3.SFDM.navierstokes.BCSubsonicInletTtPtAlpha2D', regions=[mesh.access_component('topology/left').uri()])
inlet = solver.access_component('BoundaryConditions/BCs/inlet')
inlet.options().configure_option('Tt',300)
inlet.options().configure_option('Pt',110000)
# inlet.options().configure_option('alpha',3.1415/4.)


solver.get_child('BoundaryConditions').create_boundary_condition(name= 'wall', type = 'cf3.SFDM.navierstokes.BCWallEuler2D', regions=[mesh.access_component('topology/right').uri()])
wall = solver.access_component('BoundaryConditions/BCs/wall')


solver.get_child('BoundaryConditions').create_boundary_condition(name= 'outlet', type = 'cf3.SFDM.navierstokes.BCSubsonicOutlet2D', regions=[mesh.access_component('topology/top').uri(),mesh.access_component('topology/bottom').uri()])
outlet = solver.access_component('BoundaryConditions/BCs/outlet')
outlet.options().configure_option('p',101300)
outlet.options().configure_option('gamma',1.4)

# solver.get_child('BoundaryConditions').create_boundary_condition(name= 'cylinder', type = 'cf3.SFDM.navierstokes.BCWallEuler2D', regions=[mesh.access_component('topology/cylinder').uri()])
# cylinder = solver.access_component('BoundaryConditions/BCs/cylinder')

solver.get_child('BoundaryConditions').execute();

fields = [
mesh.access_component("solution_space/solution").uri(),
]

gmsh_writer = model.create_component("init_writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().configure_option("mesh",mesh.uri())
gmsh_writer.options().configure_option("fields",fields)
gmsh_writer.options().configure_option("file",coolfluid.URI("file:initial.msh"))
gmsh_writer.execute()

# the messy part, to be improved one day
solver.access_component('TimeStepping/IterativeSolver/PreUpdate/compute_time_step').options().configure_option("cfl",0.2);
solver.access_component('TimeStepping/IterativeSolver/PreUpdate/compute_time_step').options().configure_option("milestone_dt",0.1);
solver.access_component('TimeStepping/PostActions/Periodic/milestone_dt').options().configure_option("milestone_dt",0.1);

#######################################
# SIMULATE
#######################################
model.simulate()

########################
# POST PROCESSING
########################

mesh.access_component('solution_space').create_field(name='primitive',variables="U[vector],p[scalar],T[scalar]")
primitive=mesh.access_component('solution_space/primitive')
solution=mesh.access_component('solution_space/solution')
for index in range(len(solution)):
   primitive[index][0] = solution[index][1]/solution[index][0];
   primitive[index][1] = solution[index][2]/solution[index][0];
   primitive[index][2] = 0.4 * ( solution[index][3] - 0.5 * solution[index][0] * ( (primitive[index][0])**2 + (primitive[index][1])**2))
   primitive[index][3] = primitive[index][2]/solution[index][0]/287.15

########################
# OUTPUT
########################

fields = [
mesh.access_component("solution_space/solution").uri(),
mesh.access_component("solution_space/primitive").uri()
]

# tecplot
#########
tec_writer = model.get_child('tools').create_component("writer","cf3.mesh.tecplot.Writer")
tec_writer.options().configure_option("mesh",mesh.uri())
tec_writer.options().configure_option("fields",fields)
tec_writer.options().configure_option("cell_centred",False)
tec_writer.options().configure_option("file",coolfluid.URI("file:sfdm_output.plt"))
tec_writer.execute()

# gmsh
######
gmsh_writer = model.create_component("writer","cf3.mesh.gmsh.Writer")
gmsh_writer.options().configure_option("mesh",mesh.uri())
gmsh_writer.options().configure_option("fields",fields)
gmsh_writer.options().configure_option("file",coolfluid.URI("file:sfdm_output.msh"))
gmsh_writer.execute()
