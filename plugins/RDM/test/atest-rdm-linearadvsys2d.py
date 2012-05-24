#!/usr/bin/python

import coolfluid as cf

### Global settings

root = cf.Core.root()
env = cf.Core.environment()

env.options().set('assertion_throws', False)
env.options().set('assertion_backtrace', True)
env.options().set('exception_backtrace', True)
env.options().set('exception_aborts', True)
env.options().set('exception_outputs', True)
env.options().set('log_level', 4)
env.options().set('regist_signal_handlers', False)

### create model

wizard = root.create_component('Wizard',  'cf3.RDM.SteadyExplicit')
wizard.create_model(model_name='Model', physical_model='cf3.physics.Scalar.ScalarSys2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
mesh = domain.load_mesh(file=cf.URI('rectangle2x1-tg-p1-953.msh', cf.URI.Scheme.file), name='mesh')
# "file:rectangle2x1-tg-p2-3689.msh"
# "file:rectangle2x1-qd-p1-861.msh"
# "file:rectangle2x1-qd-p2-3321.msh"

internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().set('update_vars', 'LinearAdvSys2D')
solver.options().set('solution_space', 'LagrangeP1')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().set('maxiter', 250)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('cfl', 0.25)

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().set('functions',['x*x+y*y','0'])
iconds.get_child('INIT').options().set('regions', internal_regions)

### boundary conditions

bcs = solver.get_child('BoundaryConditions')
bcs.create_boundary_condition(name='INLET', type='cf3.RDM.BcDirichlet', regions=[
  cf.URI('//Model/Domain/mesh/topology/bottom'),
  cf.URI('//Model/Domain/mesh/topology/left')
])
bcs.get_child('INLET').options().set('functions', ['2.0*cos(2*3.141592*(x+y))','3.0*cos(2*3.141592*(x+y))'])

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.LDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().set('regions', internal_regions)

### simulate and write the result

iconds.execute()

fgeo=[cf.URI('//Model/Domain/mesh/geometry/solution'),
      cf.URI('//Model/Domain/mesh/geometry/residual'),
      cf.URI('//Model/Domain/mesh/geometry/wave_speed')]
fsol=[cf.URI('//Model/Domain/mesh/solution/solution'),
      cf.URI('//Model/Domain/mesh/solution/residual'),
      cf.URI('//Model/Domain/mesh/solution/wave_speed')]

gmsh_writer = model.create_component('gmsh_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().set('mesh',root.access_component('//Model/Domain/mesh'))
gmsh_writer.options().set('fields',fgeo)
gmsh_writer.options().set('file',cf.URI('file:_geo_initial_lineuler.msh'))
gmsh_writer.execute()
gmsh_writer.options().set('fields',fsol)
gmsh_writer.options().set('file',cf.URI('file:_sol_initial_lineuler.msh'))
gmsh_writer.execute()

#tecplot_writer = model.create_component('tecplot_writer','cf3.mesh.tecplot.Writer')
#tecplot_writer.options().set('mesh',cf.URI('//Model/Domain/mesh'))
#tecplot_writer.options().set('fields',fields)
#tecplot_writer.options().set('file',cf.URI('file:initial.plt'))
#tecplot_writer.execute()

model.simulate()

interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.actions.Interpolate')
interpolator.interpolate(source=cf.URI('//Model/Domain/mesh/solution/solution'),
                         target=cf.URI('//Model/Domain/mesh/geometry/solution'))

gmsh_writer.options().set('fields',fgeo)
gmsh_writer.options().set('file',cf.URI('file:_geo_final_lineuler.msh'))
gmsh_writer.execute()
gmsh_writer.options().set('fields',fsol)
gmsh_writer.options().set('file',cf.URI('file:_sol_final_lineuler.msh'))
gmsh_writer.execute()

#tecplot_writer.options().set('file',cf.URI('file:final.plt'))
#tecplot_writer.execute()
