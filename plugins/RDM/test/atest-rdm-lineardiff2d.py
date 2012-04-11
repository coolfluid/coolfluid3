#!/usr/bin/python

import coolfluid as cf

### Global settings

root = cf.Core.root()
env = cf.Core.environment()

env.options().configure_option('assertion_throws', False)
env.options().configure_option('assertion_backtrace', True)
env.options().configure_option('exception_backtrace', True)
env.options().configure_option('exception_aborts', True)
env.options().configure_option('exception_outputs', True)
env.options().configure_option('log_level', 4)
env.options().configure_option('regist_signal_handlers', False)

### create model

wizard = root.create_component('Wizard',  'cf3.RDM.SteadyExplicit')
wizard.create_model(model_name='Model', physical_model='cf3.physics.Scalar.Scalar2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
mesh = domain.load_mesh(file=cf.URI('rectangle2x1-tg-p1-953.msh', cf.URI.Scheme.file), name='mesh')

internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().configure_option('update_vars', 'LinearAdv2D')
solver.options().configure_option('solution_space', 'LagrangeP1')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().configure_option('maxiter', 250)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().configure_option('cfl', 0.25)

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().configure_option('functions',['sin(x)'])
iconds.get_child('INIT').options().configure_option('regions', internal_regions)

### boundary conditions

bcs = solver.get_child('BoundaryConditions')
bcs.create_boundary_condition(name='INLET', type='cf3.RDM.BcDirichlet', regions=[
  cf.URI('//Model/Domain/mesh/topology/bottom'),
  cf.URI('//Model/Domain/mesh/topology/left'),
  cf.URI('//Model/Domain/mesh/topology/right')
])
bcs.get_child('INLET').options().configure_option('functions', ['cos(2*3.141592*(x+y))'])

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.LDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().configure_option('regions', internal_regions)

### simulate and write the result

iconds.execute()

fgeo=[cf.URI('//Model/Domain/mesh/geometry/solution'),
      cf.URI('//Model/Domain/mesh/geometry/residual'),
      cf.URI('//Model/Domain/mesh/geometry/wave_speed')]
fsol=[cf.URI('//Model/Domain/mesh/solution/solution'),
      cf.URI('//Model/Domain/mesh/solution/residual'),
      cf.URI('//Model/Domain/mesh/solution/wave_speed')]

gmsh_writer = model.create_component('gmsh_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',root.access_component('//Model/Domain/mesh'))
gmsh_writer.options().configure_option('fields',fgeo)
gmsh_writer.options().configure_option('file',cf.URI('file:_geo_initial_lineuler.msh'))
gmsh_writer.execute()
gmsh_writer.options().configure_option('fields',fsol)
gmsh_writer.options().configure_option('file',cf.URI('file:_sol_initial_lineuler.msh'))
gmsh_writer.execute()

#tecplot_writer = model.create_component('tecplot_writer','cf3.mesh.tecplot.Writer')
#tecplot_writer.options().configure_option('mesh',cf.URI('//Model/Domain/mesh'))
#tecplot_writer.options().configure_option('fields',fields)
#tecplot_writer.options().configure_option('file',cf.URI('file:initial.plt'))
#tecplot_writer.execute()

model.simulate()

interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.actions.Interpolate')
interpolator.interpolate(source=cf.URI('//Model/Domain/mesh/solution/solution'),
                         target=cf.URI('//Model/Domain/mesh/geometry/solution'))

gmsh_writer.options().configure_option('fields',fgeo)
gmsh_writer.options().configure_option('file',cf.URI('file:_geo_final_lineuler.msh'))
gmsh_writer.execute()
gmsh_writer.options().configure_option('fields',fsol)
gmsh_writer.options().configure_option('file',cf.URI('file:_sol_final_lineuler.msh'))
gmsh_writer.execute()

#tecplot_writer.options().configure_option('file',cf.URI('file:final.plt'))
#tecplot_writer.execute()
