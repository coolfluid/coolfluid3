import sys
import coolfluid as cf

# Some shortcuts
root = cf.Core.root()
env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

# setup a model
model = root.create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
solver = model.create_solver('cf3.UFEM.Solver')

# Add the Navier-Stokes solver as an unsteady solver
ns_solver = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [10., 0.]
points[2]  = [0., 0.5]
points[3]  = [10., 0.5]
points[4]  = [0.,1.]
points[5]  = [10., 1.]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [20, 10]
block_subdivs[1] = [20, 10]

grading = 5.
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., grading, grading]
gradings[1] = [1., 1., 1./grading, 1./grading]

left_patch = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left_patch[0] = [2, 0]
left_patch[1] = [4, 2]

bottom_patch = blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)
bottom_patch[0] = [0, 1]

top_patch = blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)
top_patch[0] = [5, 4]

right_patch = blocks.create_patch_nb_faces(name = 'right', nb_faces = 2)
right_patch[0] = [1, 3]
right_patch[1] = [3, 5]

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

ns_solver.regions = [mesh.topology.uri()]

u_in = [1., 0.]

solver.create_fields()

#initial condition for the velocity. Unset variables (i.e. the pressure) default to zero
solver.InitialConditions.navier_stokes_solution.Velocity = u_in
solver.InitialConditions.navier_stokes_solution.Pressure = 1.
solver.InitialConditions.execute()

pressure_integral = solver.add_unsteady_solver('cf3.UFEM.SurfaceIntegral')
pressure_integral.variable_name = 'Pressure'
pressure_integral.field_tag = 'navier_stokes_solution'
pressure_integral.regions = [mesh.topology.access_component('right').uri()]
pressure_integral.execute()

if abs(pressure_integral.result[0] - 1.) > 1e-10:
  raise Exception('Wrong integration result: ' + str(pressure_integral.result[0]) + ', expected 1')

velocity_integral = solver.add_unsteady_solver('cf3.UFEM.SurfaceIntegral')
velocity_integral.variable_name = 'Velocity'
velocity_integral.field_tag = 'navier_stokes_solution'
velocity_integral.regions = [mesh.topology.access_component('left').uri(), mesh.topology.access_component('bottom').uri()]
velocity_integral.execute()

print pressure_integral.result, velocity_integral.result
