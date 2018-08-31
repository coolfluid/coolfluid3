import coolfluid as cf
from math import pi

# Flow properties
h = 1.
nu = 1e-10
u0 = 10.
k0 = 5.
epsilon0 = 10.
k_init = 0.
eps_init = 0.

y_segs = 2
x_size = 10.
x_segs = 32

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

# Add the k-epsilon turbulence model solver(ke)
ke = solver.add_unsteady_solver('cf3.UFEM.StandardKEpsilon')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., h]
points[3]  = [x_size, h]

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

mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
blocks.create_mesh(mesh.uri())

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

# Initialize velocity, wall distance and yplus
velocity_field = mesh.geometry.create_field(name = 'navier_stokes_solution', variables='Velocity[vector]')
velocity_field.add_tag('navier_stokes_solution')
wall_distance_field = mesh.geometry.create_field(name = 'wall_distance', variables='wall_distance')
wall_distance_field.add_tag('wall_distance')
yplus_field = mesh.geometry.create_field(name = 'yplus', variables='yplus')
yplus_field.add_tag('yplus')

coords = mesh.geometry.coordinates
for (i,(x,y)) in enumerate(coords):
    velocity_field[i][0] = u0
    velocity_field[i][1] = 0.
    wall_distance_field[i][0] = 1e12;
    yplus_field[i][0] = 1e12;

ke.regions = [mesh.topology.uri()]

#initial conditions
ic_k = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_k.variable_name = 'k'
ic_k.value = [str(k_init)]
ic_k.regions = [mesh.topology.uri()]

ic_epsilon = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'ke_solution')
ic_epsilon.variable_name = 'epsilon'
ic_epsilon.value = [str(eps_init)]
ic_epsilon.regions = [mesh.topology.uri()]

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Boundary conditions for k
bc = ke.LSS.BoundaryConditions
bc.add_constant_bc(region_name = 'left', variable_name = 'k').value =  k0

# Boundary conditions for epsilon
bc.add_constant_bc(region_name = 'left', variable_name = 'epsilon').value =  epsilon0

# Time setup
time = model.create_time()
time.time_step = 0.1
time.end_time = 50.

# Run the simulation
model.simulate()
model.print_timing_tree()

try:
    import numpy as np
    import pylab as pl
    import os
    if os.environ.get('NOPLOT', '0') != '0':
        raise Exception("no plots wanted")
    ke_fd = mesh.geometry.ke_solution
    nu_eff_fd = mesh.geometry.navier_stokes_viscosity

    k = np.array(ke_fd)[:,0]
    epsilon = np.array(ke_fd)[:,1]
    nu_eff = np.array(nu_eff_fd)[:,0]
    xy = np.array(coords)

    c_e_2 = 1.8
    x_th = np.linspace(0., 10., 100)
    th_func = 1 + epsilon0/k0 * (c_e_2-1.)*x_th/u0
    k_th = k0*th_func**(1./(1.-c_e_2))
    eps_th = epsilon0*th_func**(c_e_2/(1.-c_e_2))

    center = np.abs(xy[:,1])<1e-3
    pl.figure()
    pl.plot(xy[center,0], k[center])
    pl.plot(x_th, k_th)

    pl.figure()
    pl.plot(xy[center,0], epsilon[center])
    pl.plot(x_th, eps_th)

    pl.figure()
    pl.plot(xy[center,0], nu_eff[center])

    pl.show()
except:
    print('Not plotting due to Python exception')
