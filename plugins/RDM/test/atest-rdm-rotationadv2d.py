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

wizard.create_model(model_name='Model', physical_model='cf3.physics.Scalar.Scalar2D')
model = root.get_child('Model')

### read mesh
domain = model.get_child('Domain')
domain.load_mesh(file=cf.URI('rotation-tg-p1.msh', cf.URI.Scheme.file), name='mesh')

internal_regions = [cf.URI('//Model/Domain/mesh/topology/fluid')]

# file:rotation-tg-p1.msh
# file:rotation-tg-p2.msh
# file:rotation-tg-p4.msh
# file:rotation-qd-p1.msh
# file:rotation-qd-p2.msh
# file:rotation-qd-p3.msh
# file:rotation-qd-p4.msh

### solver
solver = model.get_child('RDSolver')
solver.options().set('update_vars', 'RotationAdv2D')
solver.options().set('solution_space', 'LagrangeP1')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().set('maxiter', 50)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('cfl', 0.25)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('regions', internal_regions)

### initial conditions
iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().set('functions', ['x*x+y*y'])
iconds.get_child('INIT').options().set('regions', internal_regions)

## configure Model/RDSolver/InitialConditions use_strong_bcs:bool=true


### boundary conditions

bcs = solver.get_child('BoundaryConditions')

bcs.create_boundary_condition(name='INLET', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/inlet')])
bcs.get_child('INLET').options().set('functions', ['if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)'])

bcs.create_boundary_condition(name='FARFIELD', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/farfield')])
bcs.get_child('FARFIELD').options().set('functions', ['0'])

### domain discretization
solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.LDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().set('regions', internal_regions)

### simulate and write the result

iconds.execute()

domain.create_component('tecwriter', 'cf3.mesh.tecplot.Writer')

domain.write_mesh(cf.URI('initial.msh'))
domain.write_mesh(cf.URI('initial.plt'))

model.simulate()

domain.write_mesh(cf.URI('solution.msh'))
domain.write_mesh(cf.URI('solution.plt'))
