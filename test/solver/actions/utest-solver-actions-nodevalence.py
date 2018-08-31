import coolfluid as cf

env = cf.Core.environment()
env.log_level = 4
env.only_cpu0_writes = True

root = cf.Core.root()
domain = root.create_component('Domain', 'cf3.mesh.Domain')
mesh = domain.create_component('OriginalMesh','cf3.mesh.Mesh')

blocks = root.create_component('model', 'cf3.mesh.BlockMesh.BlockArrays')
points = blocks.create_points(dimensions = 2, nb_points = 4)
points[0]  = [0., 0.]
points[1]  = [1., 0.]
points[2]  = [1., 1.]
points[3]  = [0., 1.]
block_nodes = blocks.create_blocks(1)
block_nodes[0] = [0, 1, 2, 3]
block_subdivs = blocks.create_block_subdivisions()
block_subdivs[0] = [64,64]
gradings = blocks.create_block_gradings()
gradings[0] = [1., 1., 1., 1.]
blocks.create_patch_nb_faces(name = 'bottom', nb_faces = 1)[0] = [0, 1]
blocks.create_patch_nb_faces(name = 'right', nb_faces = 1)[0] = [1, 2]
blocks.create_patch_nb_faces(name = 'top', nb_faces = 1)[0] = [2, 3]
blocks.create_patch_nb_faces(name = 'left', nb_faces = 1)[0] = [3, 0]

blocks.create_mesh(mesh.uri())

valfield = mesh.geometry.create_field(name = 'node_valence', variables='Valence[scalar]')
valfield.add_tag('node_valence')

valence = domain.create_component('Valences', 'cf3.solver.actions.NodeValence')
valence.regions = [mesh.topology.uri()]
valence.execute()

# take the opportunity to test the copier
copyfd = mesh.geometry.create_field(name = 'copy_node_valence', variables='copy_Valence[scalar]')
copyfd.add_tag('copy_node_valence')
copier = domain.create_component('Copier', 'cf3.solver.actions.CopyScalar')
copier.regions = [mesh.topology.uri()]
copier.source_field_tag = 'node_valence'
copier.source_variable_name = 'Valence'
copier.execute()

writer = domain.create_component('PVWriter', 'cf3.mesh.VTKXML.Writer')
writer.fields = [valfield.uri(), copyfd.uri()]
writer.mesh = mesh
writer.file = cf.URI('node_valence.pvtu')
writer.execute()