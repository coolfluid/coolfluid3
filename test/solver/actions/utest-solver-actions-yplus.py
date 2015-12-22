import sys
import coolfluid as cf
from numpy import sin,cos,sqrt,array

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
writer = root.create_component('CF3ToVTK', 'cf3.vtk.MultiblockWriter')

def run_test(domain, mesh):
    wall_region = mesh.topology.get_child('step')
    if wall_region == None:
        wall_region = mesh.topology.inner

    make_boundary_global = domain.create_component('MakeBoundaryGlobal', 'cf3.mesh.actions.MakeBoundaryGlobal')
    make_boundary_global.mesh = mesh
    make_boundary_global.execute()

    physics = domain.create_component('Physics', 'cf3.UFEM.NavierStokesPhysics')
    physics.density = 1.
    physics.dynamic_viscosity = 1.

    wall_distance = domain.create_component('WallDistance', 'cf3.mesh.actions.WallDistance')
    wall_distance.mesh = mesh
    wall_distance.regions = [wall_region]
    wall_distance.execute()

    yplus = domain.create_component('YPlus', 'cf3.solver.actions.YPlus')
    yplus.mesh = mesh
    yplus.regions = [wall_region.uri()]
    yplus.physical_model = physics
    yplus.execute()

domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 9)
points[0] = [0., 0.]
points[1] = [1., 0.]
points[2] = [1., 1.]
points[3] = [0., 1.]
points[4] = [0.5, 0.5]
points[5] = [0.5, 1.]
points[6] = [1., 0.5]
points[7] = [0.5, 0.]
points[8] = [0., 0.5]
block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 7, 4, 8]
block_nodes[1] = [8, 4, 5, 3]
block_nodes[2] = [4, 6, 2, 5]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
block_subdivs[1] = [10,10]
block_subdivs[2] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 7]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [6, 2]
top = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top[0] = [2, 5]
top[1] = [5, 3]
left = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left[0] = [3, 8]
left[1] = [8, 0]
step = blocks.create_patch_nb_faces(name = 'step', nb_faces = 2)
step[0] = [7, 4]
step[1] = [4, 6]
blocks.create_mesh(mesh.uri())

velocity_field = mesh.geometry.create_field(name = 'navier_stokes_solution', variables='Velocity[vector]')
velocity_field.add_tag('navier_stokes_solution')
coords = mesh.geometry.coordinates
for (i,(x,y)) in enumerate(coords):
    velocity_field[i][0] = y-0.5
    velocity_field[i][1] = 0.

run_test(domain, mesh)

yplus_field = mesh.geometry.yplus
for ((x,y),(yp,)) in zip(coords,yplus_field):
    if x>=0.5 and y>=0.5:
        if abs(y-0.5-yp) > 1e-9:
            raise Exception("Error: bad y+ value: {yp}".format(yp=yp))

writer.mesh =  mesh
writer.file = cf.URI('yplus-2dstep.vtm')
writer.execute()

domain.delete_component()

domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 9)
points[0] = [0., 0.]
points[1] = [1., 0.]
points[2] = [1., 1.]
points[3] = [0., 1.]
points[4] = [0.5, 0.5]
points[5] = [0.5, 1.]
points[6] = [1., 0.5]
points[7] = [0.5, 0.]
points[8] = [0., 0.5]
block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 7, 4, 8]
block_nodes[1] = [8, 4, 5, 3]
block_nodes[2] = [4, 6, 2, 5]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
block_subdivs[1] = [10,10]
block_subdivs[2] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 7]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [6, 2]
top = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top[0] = [2, 5]
top[1] = [5, 3]
left = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left[0] = [3, 8]
left[1] = [8, 0]
step = blocks.create_patch_nb_faces(name = 'step', nb_faces = 2)
step[0] = [7, 4]
step[1] = [4, 6]
blocks.create_mesh(mesh.uri())

velocity_field = mesh.geometry.create_field(name = 'navier_stokes_solution', variables='Velocity[vector]')
velocity_field.add_tag('navier_stokes_solution')
coords = mesh.geometry.coordinates
for (i,(x,y)) in enumerate(coords):
    velocity_field[i][0] = y-0.5
    velocity_field[i][1] = 0.

run_test(domain, mesh)

yplus_field = mesh.geometry.yplus
for ((x,y),(yp,)) in zip(coords,yplus_field):
    if x>=0.5 and y>=0.5:
        if abs(y-0.5-yp) > 1e-9:
            raise Exception("Error: bad y+ value: {yp}".format(yp=yp))

writer.mesh =  mesh
writer.file = cf.URI('yplus-2dstep.vtm')
writer.execute()

domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('mesh','cf3.mesh.Mesh')

blocks = root.create_component('model2', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 9)
points[0] = [0., 0.]
points[1] = [1., 0.]
points[2] = [1., 1.]
points[3] = [0., 1.]
points[4] = [0.5, 0.5]
points[5] = [0.5, 1.]
points[6] = [1., 0.5]
points[7] = [0.5, 0.]
points[8] = [0., 0.5]
block_nodes = blocks.create_blocks(3)
block_nodes[0] = [0, 7, 4, 8]
block_nodes[1] = [8, 4, 5, 3]
block_nodes[2] = [4, 6, 2, 5]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [10,10]
block_subdivs[1] = [10,10]
block_subdivs[2] = [10,10]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
gradings[1] = [1., 1., 1., 1.]
gradings[2] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 7]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [6, 2]
top = blocks.create_patch_nb_faces(name = 'top', nb_faces = 2)
top[0] = [2, 5]
top[1] = [5, 3]
left = blocks.create_patch_nb_faces(name = 'left', nb_faces = 2)
left[0] = [3, 8]
left[1] = [8, 0]
step = blocks.create_patch_nb_faces(name = 'step', nb_faces = 2)
step[0] = [7, 4]
step[1] = [4, 6]
blocks.extrude_blocks(positions=[1.], nb_segments=[10], gradings=[1.])
blocks.create_mesh(mesh.uri())

velocity_field = mesh.geometry.create_field(name = 'navier_stokes_solution', variables='Velocity[vector]')
velocity_field.add_tag('navier_stokes_solution')
coords = mesh.geometry.coordinates
for (i,(x,y,z)) in enumerate(coords):
    velocity_field[i][0] = y-0.5
    velocity_field[i][1] = 0.
    velocity_field[i][2] = 0.

run_test(domain, mesh)

yplus_field = mesh.geometry.yplus
for ((x,y,z),(yp,)) in zip(coords,yplus_field):
    if x>=0.5 and y>=0.5:
        if abs(y-0.5-yp) > 1e-9:
            raise Exception("Error: bad y+ value: {yp}".format(yp=yp))

writer.mesh =  mesh
writer.file = cf.URI('yplus-3dstep.vtm')
writer.execute()

domain.delete_component()

domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.load_mesh(file = cf.URI(sys.argv[1]), name = 'mesh')

velocity_field = mesh.geometry.create_field(name = 'navier_stokes_solution', variables='Velocity[vector]')
velocity_field.add_tag('navier_stokes_solution')
coords = mesh.geometry.coordinates
for (i,(x,y,z)) in enumerate(coords):
    r = sqrt(x**2+y**2+z**2)
    n = array([x,y,z]) / r
    t = array([y,-x,0.]) / sqrt(x**2+y**2)
    v = 9*t*abs(r-1.)
    velocity_field[i][0] = v[0]
    velocity_field[i][1] = v[1]
    velocity_field[i][2] = v[2]

run_test(domain, mesh)

yplus_field = mesh.geometry.yplus
for ((x,y,z),(yp,)) in zip(coords,yplus_field):
    r = sqrt(x**2+y**2+z**2)
    yp_expected = 3.*(r-1.)
    if abs(yp-yp_expected) > 0.1:
        raise Exception("Error: bad y+ value: {yp}, expected: {yp_expected}".format(yp=yp, yp_expected=yp_expected))

writer.mesh =  mesh
writer.file = cf.URI('yplus-sphere.vtm')
writer.execute()
