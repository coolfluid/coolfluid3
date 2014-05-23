import sys
import coolfluid as cf
from math import pi

env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

rho_f = 0.9
mu = 0.19e-4 * rho_f

r0 = 0.01
omega0 = 18.
g = 10.

dt = 0.1

numsteps = 5
write_interval = 50

resolution = 64

diams = [20.e-6, 40.e-6, 80.e-6]
volumes = [4./3.*pi*(d/2.)**3 for d in diams]
ref_vol = 1e-15
c_reduced = [1, 1., 1.]
zeta = [v/ref_vol*c for (v,c) in zip(volumes,c_reduced)]

# Create the model and solvers
model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
physics.options.dimension = 2
solver = model.create_solver('cf3.UFEM.Solver')

polydisp = solver.add_unsteady_solver('cf3.UFEM.particles.Polydisperse')
polydisp.options.velocity_variable = 'Velocity'
polydisp.options.velocity_tag = 'navier_stokes_u_solution'
polydisp.options.save_collision_rate = True
polydisp.options.collision_kernel_type = 'DNSCollisionKernel'

# Set up the physical constants
physics.density = rho_f
physics.dynamic_viscosity = mu

polydisp.initial_diameters = diams
polydisp.reference_volume = ref_vol
polydisp.initial_concentrations = [x/polydisp.reference_volume for x in c_reduced]
polydisp.nb_phases = 3

# Create the mesh
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')

x_min = -0.09
x_max = 0.2
y_min = -0.2
y_max = 0.2
x_segs = resolution
y_segs = resolution
z_segs = 2

blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [x_min, y_min]
points[1]  = [x_max, y_min]
points[2]  = [x_min, y_max]
points[3]  = [x_max, y_max]

block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 3, 2]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)
left_patch[0] = [2, 0]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [3, 2]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)
right_patch[0] = [1, 3]

blocks.extrude_blocks(positions=[0.05], nb_segments=[z_segs], gradings=[1.])
blocks.create_mesh(mesh.uri())

partitioner = domain.create_component('Partitioner', 'cf3.mesh.actions.PeriodicMeshPartitioner')
partitioner.mesh = mesh
partitioner.load_balance = False

link_horizontal = partitioner.create_link_periodic_nodes()
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-0.29, 0., 0.]

link_front_back = partitioner.create_link_periodic_nodes()
link_front_back.source_region = mesh.topology.back
link_front_back.destination_region = mesh.topology.front
link_front_back.translation_vector = [0., 0., -0.05]

partitioner.execute()

polydisp.regions = [mesh.topology.uri()]

#polydisp.children.ConcentrationSolver.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'

series_writer = solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
writer.file = cf.URI('polydisperse-burgers-{iteration}.pvtu')
writer.mesh = mesh
series_writer.interval = write_interval

solver.create_fields()
writer.fields = [mesh.geometry.particle_concentration_1.uri(), mesh.geometry.weighted_particle_volume_1.uri()]

u_arr = ['-((1 - exp(-(x^2 + y^2)/{r0}^2))*{omega0}*{r0}^2*y)/(2.*(x^2 + y^2))'.format(r0=r0,omega0=omega0), '((1 - exp(-(x^2 + y^2)/{r0}^2))*{omega0}*{r0}^2*x)/(2.*(x^2 + y^2))'.format(r0=r0,omega0=omega0), '0']
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = u_arr

ic_lin_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'linearized_velocity')
ic_lin_u.variable_name = 'AdvectionVelocity1'
ic_lin_u.regions = [mesh.topology.uri()]
ic_lin_u.value = u_arr

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionConstant', field_tag = 'body_force')
ic_g.regions = [mesh.topology.uri()]
ic_g.Force = [0.,-g, 0.]

for i in range(len(zeta)):
  zeta_name = 'zeta_'+str(i)
  c_name = 'c_'+str(i)
  print str(polydisp.BC.children[c_name].regions[0])
  polydisp.BC.children[c_name].add_constant_bc(region_name = 'top', variable_name = c_name).value = c_reduced[i]
  polydisp.BC.children[zeta_name].add_constant_bc(region_name = 'top', variable_name = zeta_name).value = zeta[i]
  #polydisp.BC.children[c_name].add_constant_bc(region_name = 'bottom', variable_name = c_name).value = 0.
  #polydisp.BC.children[zeta_name].add_constant_bc(region_name = 'bottom', variable_name = zeta_name).value = 0.

# Time setup
time = model.create_time()
time.time_step = dt
time.end_time = numsteps*dt

#solver.InitialConditions.execute()
model.simulate()

domain.write_mesh(cf.URI('polydisperse-burgers-end.pvtu'))
model.print_timing_tree()
