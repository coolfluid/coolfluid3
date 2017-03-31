import coolfluid as cf
import math
from math import pi
from math import sqrt
import numpy as np

# Flow properties
h = 1.# height of the inlet = 2*h, so 2 units in this case
mu = 0.5
nu = mu/1.2 # viscosity
u_in = 2. # inlet velocity
sa_visc = 5.0*nu # Spalart Allmaras viscosity

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

## Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')


dirdiff_solver = solver.add_unsteady_solver('cf3.UFEM.adjointtube.DirectDifferentiation')

gradient2 = solver.add_unsteady_solver('cf3.UFEM.VelocityGradient')
gradient2.options.gradient_tag = 'velocity_gradient'
gradient2.options.velocity_variable = 'Velocity'
gradient2.options.velocity_tag = 'navier_stokes_solution'
gradient2.options.gradient_name = 'u'

# Generate mesh
y_segs = 64 # number of segments in the y direction
x_size = 12*h # length of the tube, so 12*1 units
s_start = x_size/3.0 # beginning of the bend
s_end = 2.0*x_size/3.0 # end of the bend
x_segs1 = 20 # number of segments for the first straight part
x_segs2 = 35 # number of segments for bend
x_segs3 = x_segs1 # number of segments for the last straight part
ungraded_h = float(y_segs)

blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 8)
points[0] = [0.0, 0.0]
points[1] = [s_start, 0.0]
points[2] = [s_start, ungraded_h]
points[3] = [0.0, ungraded_h]
points[4] = [s_end, 0.0]
points[5] = [s_end, ungraded_h]
points[6] = [x_size, 0.0]
points[7] = [x_size, ungraded_h]

block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 1, 2, 3] # before bend
block_nodes[1] = [1, 4, 5 ,2] # the bend
block_nodes[2] = [4, 6, 7, 5] # after the bend

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs1, y_segs]
block_subdivs[1] = [x_segs2, y_segs]
block_subdivs[2] = [x_segs3, y_segs]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]

left_patch = blocks.create_patch_nb_faces(name = 'inlet', nb_faces = 1)
left_patch[0] = [3, 0]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom_straight', nb_faces = 2)
bottom_patch[0] = [0, 1]
bottom_patch[1] = [4, 6]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom_curve', nb_faces = 1) # bend
bottom_patch[0] = [1, 4]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 3)
top_patch[0] = [7, 5]
top_patch[1] = [5, 2]
top_patch[2] = [2, 3]

right_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 1)
right_patch[0] = [6, 7]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

coordmap = {}
b = 0.975
xi = np.linspace(-h, h, y_segs+1)
y_graded = h/b * np.tanh(xi*np.arctanh(b))

coords = mesh.geometry.coordinates
for i in range(len(coords)):
  y_key = int(round(coords[i][1]))
  coords[i][1] = y_graded[y_key]

# Create bend (straight line here)
def curve_equation(x):
  bend_height = 1.0
  if x < s_start:
    return 0.0 # return 0.0
  if x > s_end:
    return -bend_height # return - bend_height
  return bend_height*1.035*np.tanh(-x+6)/2-bend_height/2 # the bend follows a tanh law

for i in range(len(coords)):
  old_y = coords[i][1]
  x = coords[i][0]
  coords[i][1] = curve_equation(x) + old_y
#

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
# ad_solver.regions = [mesh.topology.uri()] # voor de adjoint oplossing uit comment plaatsen
# satm.regions = [mesh.topology.uri()]
# gradient1.regions = [mesh.topology.uri()]
gradient2.regions = [mesh.topology.uri()]

compute_normals = domain.create_component('ComputeNormals','cf3.UFEM.TweedeStap')
compute_normals.mesh = mesh
compute_normals.regions = [mesh.topology.access_component('bottom_curve').uri(),mesh.topology.access_component('bottom_straight').uri(),mesh.topology.access_component('top').uri()]
compute_normals.execute()

u_wall = [0., 0.]

dirdiff_solver.regions = [mesh.topology.uri()]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.sensitivity_solution.SensU = u_wall

#properties for Navier-Stokes
physics.density = 1.2
physics.dynamic_viscosity = mu
# physics.kinematic_viscosity = nu

# Make the boundary global, to allow wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

# Compute the wall distance
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom_straight,mesh.topology.bottom_curve,mesh.topology.top]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_function_bc(region_name = 'inlet', variable_name = 'Velocity').value =  ['{u_in}*(1-(y)^2)'.format(u_in=u_in), '0']
bot_bc = bc.add_constant_bc(region_name = 'bottom_straight', variable_name = 'Velocity')
bot_bc.value =  u_wall
bot_bc.regions = [mesh.topology.bottom_straight.uri(), mesh.topology.bottom_curve.uri()]
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.0

# Randvoorwaarden direct differentiation
bcd = dirdiff_solver.BoundaryConditions
bcd.add_constant_bc(region_name = 'inlet', variable_name = 'SensU').value =  [0.,0.]
bcd.add_constant_bc(region_name = 'outlet', variable_name = 'SensP').value =  0.
bc_dirdiff_p1 = bcd.create_bc_action(region_name = 'bottom_curve', builder_name = 'cf3.UFEM.adjointtube.BCSensUx')
bc_dirdiff_p2 = bcd.create_bc_action(region_name = 'bottom_straight', builder_name = 'cf3.UFEM.adjointtube.BCSensUx')
# bc_dirdiff_p3 = bcd.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjointtube.BCSensUx')
bc_dirdiff_p4 = bcd.create_bc_action(region_name = 'bottom_curve', builder_name = 'cf3.UFEM.adjointtube.BCSensUy')
bc_dirdiff_p5 = bcd.create_bc_action(region_name = 'bottom_straight', builder_name = 'cf3.UFEM.adjointtube.BCSensUy')
# bc_dirdiff_p6 = bcd.create_bc_action(region_name = 'top', builder_name = 'cf3.UFEM.adjointtube.BCSensUy')


# Setup a time series write
# write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
# write_manager.interval = 200
# writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
# writer.mesh = mesh
# #writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/spalart_allmaras_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/velocity_gradient')]
# writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/velocity_gradient')]
#
# writer.file = cf.URI('parab-out-sa-{iteration}.pvtu')

writer = domain.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/velocity_gradient')]

# Time setup
time = model.create_time()
time.time_step = 0.01
time.end_time = 1.* time.time_step

probe0 = solver.add_probe(name = 'Probe', parent = nstokes, dict = mesh.geometry)
probe0.Log.variables = ['Velocity[0]', 'EffectiveViscosity']
probe0.coordinate = [s_end, 0.0]
probe0.History.file = cf.URI('parab-out-probe.tsv')

# Run the simulation
solver.TimeLoop.options.disabled_actions = ['AdjointTube', 'EersteStap', 'DirectDifferentiation']
model.simulate()
model.print_timing_tree()

writer.file = cf.URI('parab-out-sa-end.pvtu')
writer.execute()

solver.TimeLoop.options.disabled_actions = ['NavierStokes', 'EersteStap', 'AdjointTube'] # NS disabled, adjoint enabled
solver.options.disabled_actions = ['InitialConditions'] # disable initial conditions

mesh.print_tree()

interesting_points = []
bottom_conn = mesh.topology.bottom_curve.children["elements_cf3.mesh.LagrangeP1.Line2D"].spaces.geometry.children.connectivity
for [start,end] in bottom_conn:
    interesting_points.append(start)

bottom_conn = mesh.topology.bottom_curve.children["elements_cf3.mesh.LagrangeP1.Line2D"].spaces.geometry.children.connectivity
for [start,end] in bottom_conn:
    interesting_points.append(start)

normals_field = mesh.geometry.NodalNormal

for p in interesting_points:

    n = normals_field[p]
    mag = sqrt(n[0]*n[0]+n[1]*n[1]) # magnitude of the vector
    print 'simulating for point', p, 'with normal', n

    bc_dirdiff_p1.n_x = n[0]/mag # genormaliseerde vector
    bc_dirdiff_p1.n_y = n[1]/mag
    print 'simulating for point', p, 'with normal x component', bc_dirdiff_p1.n_x
    print 'simulating for point', p, 'with normal y component', bc_dirdiff_p1.n_y

    # DirectDifferentiation oplossing
    time.end_time += 1. * time.time_step/50

    model.simulate()

    domain.write_mesh(cf.URI('parab-output-dirdiff-{p}.pvtu'.format(p=p)))
    model.print_timing_tree()
