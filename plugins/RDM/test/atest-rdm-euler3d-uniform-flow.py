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
env.options().set('log_level', 3)
env.options().set('regist_signal_handlers', False)

### create model

wizard = root.create_component('Wizard',  'cf3.RDM.SteadyExplicit')

wizard.create_model(model_name='Model', physical_model='cf3.physics.NavierStokes.NavierStokes3D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
domain.load_mesh(file=cf.URI('box-tet-p1-3112.msh'), name='mesh')
internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().set('update_vars', 'Cons3D')
solver.options().set('solution_space', 'LagrangeP1')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().set('maxiter', 5)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('cfl', 0.25)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().set('saverate', 10)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().set('filepath', cf.URI('file:periodic.plt'))

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().set('functions', ['0.500','1.500','1.500','1.500','7.000'])
iconds.get_child('INIT').options().set('regions', internal_regions)

### boundary conditions

#solver.get_child('BoundaryConditions').create_boundary_condition(name='INLET',type='cf3.RDM.BcDirichlet')
#solver.get_child('BoundaryConditions').get_child('INLET').options().set('functions',['0.5','1.183','1.183','3.425'])
# configure Model/RDSolver/BoundaryConditions/INLET "functions:array[string]=0.5,1.183,1.183,3.425"

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.LDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().set('regions', internal_regions)

### simulate and write the result

iconds.execute()

fields=[
  cf.URI('//Model/Domain/mesh/geometry/solution'),
  cf.URI('//Model/Domain/mesh/geometry/residual'),
  cf.URI('//Model/Domain/mesh/geometry/wave_speed')]
#  cf.URI('//Model/Domain/mesh/geometry_fields/solution'),
#  cf.URI('//Model/Domain/mesh/geometry_fields/residual'),
#  cf.URI('//Model/Domain/mesh/geometry_fields/wave_speed')]
#  cf.URI('//Model/Domain/mesh/solution/solution'),
#  cf.URI('//Model/Domain/mesh/solution/residual'),
#  cf.URI('//Model/Domain/mesh/solution/wave_speed')]

tecplot_writer = model.create_component('tecplot_writer','cf3.mesh.tecplot.Writer')
tecplot_writer.options().set('mesh',root.access_component('//Model/Domain/mesh'))
tecplot_writer.options().set('fields',fields)
tecplot_writer.options().set('file',cf.URI('file:initial.plt'))
tecplot_writer.execute()

model.simulate()

tecplot_writer.options().set('file',cf.URI('file:final.plt'))
tecplot_writer.execute()
