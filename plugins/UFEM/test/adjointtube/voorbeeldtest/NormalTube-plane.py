import coolfluid as cf
import numpy as np


# Re = rho.v.l/mu

u_in = [0.3,0.] # 2 dimensioneel
u_wall = [0.,0.]
initial_velocity = u_in
# rho = 1.225
# mu = 0.0000178 # p 18

rho = 1
mu = 0.0001 # laminiaire viscositeit

tstep = 0.001 #dit is stap in de tijd
num_steps = 50000.0 #aantal stappen

env = cf.Core.environment()
env.log_level = 3

model = cf.root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the Spalart-Allmaras turbulence model
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

#mesh = domain.load_mesh(file = cf.URI('AlternatieveBuis.msh'), name = 'Mesh')

# mesh parameters
h = 1.0
y_segs = 128
x_size = 12*h
s_start = x_size/3.0
s_end = 2.0*x_size/3.0
x_segs1 = 20
x_segs2 = 40
x_segs3 = x_segs1
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

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 3)
bottom_patch[0] = [0, 1]
bottom_patch[1] = [1, 4]
bottom_patch[2] = [4, 6]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 3)
top_patch[0] = [7, 5]
top_patch[1] = [5, 2]
top_patch[2] = [2, 3]

right_patch = blocks.create_patch_nb_faces(name = 'outlet', nb_faces = 1)
right_patch[0] = [6, 7]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

# Adjust vertical size and grading
coordmap = {}
b = 0.9544
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
  return bend_height*np.tanh(-x+6)/2-bend_height/2 #(x-s_start) / (s_end-s_start)

for i in range(len(coords)):
  old_y = coords[i][1]
  x = coords[i][0]
  coords[i][1] = curve_equation(x) + old_y

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
nstokes.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.spalart_allmaras_solution.SAViscosity = 5*mu

#properties for Navier-Stokes
physics.density = rho
physics.dynamic_viscosity = mu

# Compute the wall distance
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
# bc.add_constant_bc(region_name = 'inlet', variable_name = 'Velocity').value = u_in
bc.add_function_bc(region_name = 'inlet', variable_name = 'Velocity').value = ['1-(y)^2', '0'] #['1-((y+35)/35)^2', '0'] # -y*(y+70)/35/35
bc.add_constant_bc(region_name = 'outlet', variable_name = 'Pressure').value = 0.0
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall

bc = satm.children.BoundaryConditions
# bc.add_constant_bc(region_name = 'inlet', variable_name = 'SAViscosity').value = 0.1
bc.add_constant_bc(region_name = 'bottom', variable_name = 'SAViscosity').value = 0.0
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').value = 0.0

# Solver setup
lss = nstokes.create_lss(matrix_builder = 'cf3.math.LSS.TrilinosFEVbrMatrix', solution_strategy = 'cf3.math.LSS.TrilinosStratimikosStrategy')
lss.SolutionStrategy.Parameters.preconditioner_type = 'ML'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.add_parameter(name = 'ML output', value = 0)
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.default_values = 'NSSA'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.aggregation_type = 'Uncoupled'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_type = 'symmetric block Gauss-Seidel'#'Chebyshev'
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_sweeps = 4
lss.SolutionStrategy.Parameters.PreconditionerTypes.ML.MLSettings.smoother_pre_or_post = 'post'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.solver_type = 'Block GMRES'
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.convergence_tolerance = 1e-5
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.maximum_iterations = 2000
lss.SolutionStrategy.Parameters.LinearSolverTypes.Belos.SolverTypes.BlockGMRES.num_blocks = 1000

solver.create_fields()

write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 1000
writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.fields = [mesh.geometry.navier_stokes_solution.uri(), mesh.geometry.spalart_allmaras_solution.uri()]
writer.file = cf.URI('Conv-{iteration}.pvtu')

# Time setup
time = model.create_time()
time.time_step = tstep
time.end_time = num_steps*tstep

# # Run the simulation
# model.simulate()
#
# writer = domain.create_component('CF3ToVTK', 'cf3.vtk.MultiblockWriter')
# writer.mesh =  mesh
# writer.file = cf.URI('out-normaltube.vtm')
# writer.execute()

# run the simulation (forward, NS only)
#solver.TimeLoop.options.disabled_actions = ['Adjoint']

model.simulate()
#domain.write_mesh(cf.URI('out-end.pvtu'))
