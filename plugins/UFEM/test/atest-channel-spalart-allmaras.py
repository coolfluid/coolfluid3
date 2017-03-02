import coolfluid as cf
from math import pi
import numpy as np

# Flow properties
h = 1.
nu = 0.0001
re_tau = 590.
u_tau = re_tau * nu / h
a_tau = u_tau**2 / h
Uc = u_tau**2*h/nu

y_segs = 32
x_size = 4.*pi*h
x_segs = 4
ungraded_h = float(y_segs)

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

# Add the yplus computation
yplus = solver.add_unsteady_solver('cf3.solver.actions.YPlus')
# Add the Navier-Stokes solver as an unsteady solver
nstokes = solver.add_unsteady_solver('cf3.UFEM.NavierStokes')

# Add the Spalat-Allmaras turbulence model
satm = solver.add_unsteady_solver('cf3.UFEM.SpalartAllmaras')

# Generate mesh
blocks = domain.create_component('blocks', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 6)
points[0]  = [0, 0.]
points[1]  = [x_size, 0.]
points[2]  = [0., ungraded_h]
points[3]  = [x_size, ungraded_h]
points[4]  = [0.,2.*ungraded_h]
points[5]  = [x_size, 2.*ungraded_h]

block_nodes = blocks.create_blocks(2)
block_nodes[0] = [0, 1, 3, 2]
block_nodes[1] = [2, 3, 5, 4]

block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [x_segs, y_segs]
block_subdivs[1] = block_subdivs[0]

gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]

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

coordmap = {}
b = 0.98
xi = np.linspace(-h, h, y_segs*2+1)
y_graded = h/b * np.tanh(xi*np.arctanh(b))

coords = mesh.geometry.coordinates
for i in range(len(coords)):
  y_key = int(coords[i][1])
  coords[i][1] = y_graded[y_key]

# Make the boundary global, to allow wall distance and periodics to work correctly
make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
make_boundary_global.mesh = mesh
make_boundary_global.execute()

link_horizontal = domain.create_component('LinkHorizontal', 'cf3.mesh.actions.LinkPeriodicNodes')
link_horizontal.mesh = mesh
link_horizontal.source_region = mesh.topology.right
link_horizontal.destination_region = mesh.topology.left
link_horizontal.translation_vector = [-x_size, 0.]
link_horizontal.execute()

partitioner = domain.create_component('Partitioner', 'cf3.zoltan.PHG')
partitioner.mesh = mesh
partitioner.execute()

create_point_region = domain.create_component('CreatePointRegion', 'cf3.mesh.actions.AddPointRegion')
create_point_region.coordinates = [x_size/2., 0.]  #[x_size/2., h, z_size/2.]
create_point_region.region_name = 'center'
create_point_region.mesh = mesh
create_point_region.execute()

yplus.mesh = mesh

# Because of multi-region support, solvers do not automatically have a region assigned, so we must manually set the solvers to work on the whole mesh
yplus.regions = [mesh.topology.bottom.uri(), mesh.topology.top.uri()]
nstokes.regions = [mesh.topology.uri()]
satm.regions = [mesh.topology.uri()]

u_wall = [0., 0.]

#initial conditions
solver.InitialConditions.navier_stokes_solution.Velocity = u_wall
solver.InitialConditions.density_ratio.density_ratio = 1.
# solver.InitialConditions.spalart_allmaras_solution.SAViscosity = 5*nu

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'body_force')
ic_g.variable_name = 'Force'
ic_g.regions = [mesh.topology.uri()]
ic_g.value = [str(a_tau), '0']

ic_sa = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'spalart_allmaras_solution')
ic_sa.variable_name = 'SAViscosity'
ic_sa.regions = [mesh.topology.uri()]
ic_sa.value = ['{sa_visc}*(1-(y)^2)'.format(sa_visc=5.0*nu)]

#properties for Navier-Stokes
physics.density = 1.
physics.dynamic_viscosity = nu

# Compute the wall distance
make_boundary_global.execute()
wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
wall_distance.mesh = mesh
wall_distance.regions = [mesh.topology.bottom, mesh.topology.top]
wall_distance.execute()

# Needed for yplus to work
conn = domain.create_component("SurfaceToVolumeConnectivity", "cf3.mesh.actions.SurfaceToVolumeConnectivity")
conn.mesh = mesh
conn.execute()

# Boundary conditions for Navier-Stokes
bc = nstokes.get_child('BoundaryConditions')
bc.add_constant_bc(region_name = 'bottom', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'top', variable_name = 'Velocity').value =  u_wall
bc.add_constant_bc(region_name = 'center', variable_name = 'Pressure').value = 0.

bc = satm.children.BoundaryConditions
bc.add_constant_bc(region_name = 'bottom', variable_name = 'SAViscosity').value = 0.
bc.add_constant_bc(region_name = 'top', variable_name = 'SAViscosity').value = 0.

# Setup a time series write
write_manager = solver.add_unsteady_solver('cf3.solver.actions.TimeSeriesWriter')
write_manager.interval = 200
writer = write_manager.create_component('VTKWriter', 'cf3.mesh.VTKXML.Writer')
writer.mesh = mesh
writer.fields = [cf.URI('/NavierStokes/Domain/Mesh/geometry/navier_stokes_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/spalart_allmaras_solution'), cf.URI('/NavierStokes/Domain/Mesh/geometry/yplus')]
writer.file = cf.URI('atest-channel-spalart-allmaras-{iteration}.pvtu')

# Time setup
time = model.create_time()
time.time_step = 0.1
time.end_time = 2000.0

# Run the simulation
model.simulate()
# Plot simulation velocity
try:
    import pylab as pl
    import os
    if cf.Core.rank() == 0 and os.environ.get('NOPLOT', '0') == '0':
        coords = np.array(mesh.geometry.coordinates)
        ns_sol = np.array(mesh.geometry.navier_stokes_solution)
        yplus_arr = np.array(mesh.geometry.yplus)
        eff_visc = np.array(mesh.geometry.navier_stokes_viscosity)
        line = np.abs(coords[:,0])<1e-6
        u = ns_sol[line, 0]
        nu_line = eff_visc[line,0]
        y = coords[line, 1]
        pl.figure()
        pl.plot(y, nu_line, 'o')
        pl.figure()
        pl.plot(y, u, 'o')
        u_tau_num = np.sqrt(nu*(u[1]-u[0])/(y[1]-y[0]))
        yplus_cen = u_tau_num*h/nu
        print u_tau_num #0.0538948365866
        print u_tau # 0.059

        # Plot MKM velocity
        y_mkm = np.array([0.0000e+00, 7.5298e-05, 3.0118e-04, 6.7762e-04, 1.2045e-03, 1.8819e-03, 2.7095e-03, 3.6874e-03, 4.8153e-03, 6.0930e-03, 7.5205e-03, 9.0974e-03, 1.0823e-02, 1.2699e-02, 1.4722e-02, 1.6895e-02, 1.9215e-02, 2.1683e-02, 2.4298e-02, 2.7060e-02, 2.9969e-02, 3.3024e-02, 3.6224e-02, 3.9569e-02, 4.3060e-02, 4.6694e-02, 5.0472e-02, 5.4393e-02, 5.8456e-02, 6.2661e-02, 6.7007e-02, 7.1494e-02, 7.6120e-02, 8.0886e-02, 8.5790e-02, 9.0832e-02, 9.6011e-02, 1.0133e-01, 1.0678e-01, 1.1236e-01, 1.1808e-01, 1.2393e-01, 1.2991e-01, 1.3603e-01, 1.4227e-01, 1.4864e-01, 1.5515e-01, 1.6178e-01, 1.6853e-01, 1.7541e-01, 1.8242e-01, 1.8954e-01, 1.9679e-01, 2.0416e-01, 2.1165e-01, 2.1926e-01, 2.2699e-01, 2.3483e-01, 2.4279e-01, 2.5086e-01, 2.5905e-01, 2.6735e-01, 2.7575e-01, 2.8427e-01, 2.9289e-01, 3.0162e-01, 3.1046e-01, 3.1940e-01, 3.2844e-01, 3.3758e-01, 3.4683e-01, 3.5617e-01, 3.6561e-01, 3.7514e-01, 3.8477e-01, 3.9449e-01, 4.0430e-01, 4.1420e-01, 4.2419e-01, 4.3427e-01, 4.4443e-01, 4.5467e-01, 4.6500e-01, 4.7541e-01, 4.8590e-01, 4.9646e-01, 5.0710e-01, 5.1782e-01, 5.2860e-01, 5.3946e-01, 5.5039e-01, 5.6138e-01, 5.7244e-01, 5.8357e-01, 5.9476e-01, 6.0601e-01, 6.1732e-01, 6.2868e-01, 6.4010e-01, 6.5158e-01, 6.6311e-01, 6.7469e-01, 6.8632e-01, 6.9799e-01, 7.0972e-01, 7.2148e-01, 7.3329e-01, 7.4513e-01, 7.5702e-01, 7.6894e-01, 7.8090e-01, 7.9289e-01, 8.0491e-01, 8.1696e-01, 8.2904e-01, 8.4114e-01, 8.5327e-01, 8.6542e-01, 8.7759e-01, 8.8978e-01, 9.0198e-01, 9.1420e-01, 9.2644e-01, 9.3868e-01, 9.5093e-01, 9.6319e-01, 9.7546e-01, 9.8773e-01, 1.0000e-00])
        u_mkm = np.array([0.0000e+00, 4.4231e-02, 1.7699e-01, 3.9816e-01, 7.0750e-01, 1.1046e+00, 1.5886e+00, 2.1573e+00, 2.8061e+00, 3.5272e+00, 4.3078e+00, 5.1307e+00, 5.9748e+00, 6.8174e+00, 7.6375e+00, 8.4177e+00, 9.1455e+00, 9.8142e+00, 1.0421e+01, 1.0968e+01, 1.1458e+01, 1.1895e+01, 1.2287e+01, 1.2636e+01, 1.2950e+01, 1.3233e+01, 1.3489e+01, 1.3722e+01, 1.3935e+01, 1.4131e+01, 1.4313e+01, 1.4482e+01, 1.4640e+01, 1.4789e+01, 1.4931e+01, 1.5066e+01, 1.5196e+01, 1.5321e+01, 1.5442e+01, 1.5560e+01, 1.5674e+01, 1.5786e+01, 1.5896e+01, 1.6003e+01, 1.6110e+01, 1.6214e+01, 1.6317e+01, 1.6418e+01, 1.6518e+01, 1.6616e+01, 1.6713e+01, 1.6808e+01, 1.6902e+01, 1.6995e+01, 1.7087e+01, 1.7179e+01, 1.7269e+01, 1.7358e+01, 1.7446e+01, 1.7533e+01, 1.7620e+01, 1.7705e+01, 1.7789e+01, 1.7873e+01, 1.7956e+01, 1.8038e+01, 1.8120e+01, 1.8202e+01, 1.8282e+01, 1.8362e+01, 1.8441e+01, 1.8520e+01, 1.8598e+01, 1.8676e+01, 1.8754e+01, 1.8831e+01, 1.8907e+01, 1.8982e+01, 1.9056e+01, 1.9128e+01, 1.9199e+01, 1.9269e+01, 1.9338e+01, 1.9406e+01, 1.9473e+01, 1.9539e+01, 1.9604e+01, 1.9668e+01, 1.9731e+01, 1.9794e+01, 1.9855e+01, 1.9916e+01, 1.9976e+01, 2.0035e+01, 2.0093e+01, 2.0149e+01, 2.0204e+01, 2.0258e+01, 2.0311e+01, 2.0364e+01, 2.0415e+01, 2.0464e+01, 2.0513e+01, 2.0561e+01, 2.0608e+01, 2.0653e+01, 2.0698e+01, 2.0741e+01, 2.0784e+01, 2.0826e+01, 2.0866e+01, 2.0905e+01, 2.0943e+01, 2.0979e+01, 2.1013e+01, 2.1046e+01, 2.1076e+01, 2.1105e+01, 2.1131e+01, 2.1156e+01, 2.1178e+01, 2.1198e+01, 2.1215e+01, 2.1230e+01, 2.1242e+01, 2.1251e+01, 2.1258e+01, 2.1262e+01, 2.1263e+01])

        pl.plot(y_mkm-1., u_mkm*u_tau_num)

        pl.show()
except:
    print('Skipping plot due to python errors')

writer.file = cf.URI('atest-channel-spalart-allmaras-end.pvtu')
writer.execute()
