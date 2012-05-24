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

wizard.create_model(model_name='Model', physical_model='cf3.physics.NavierStokes.NavierStokes2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
domain.load_mesh(file=cf.URI('trapezium1x1-tg-p1-508.msh'), name='mesh')
#domain.load_mesh(file=cf.URI('file:trapezium1x1-tg-p2-508.msh'), name='mesh')
#domain.load_mesh(file=cf.URI('file:trapezium1x1-qd-p1-441.msh'), name='mesh')
#domain.load_mesh(file=cf.URI('file:square1x1-tg-p2-2kn.msh'), name='mesh')
#domain.load_mesh(file=cf.URI('trapezium1x1-tg-p2-508.msh', cf.URI.Scheme.file), name='mesh')
#domain.load_mesh(file=cf.URI('trapezium-refined.msh', cf.URI.Scheme.file), name='mesh')
# trapezium1x1-tg-p2-1949.msh
# trapezium1x1-tg-p1-11512.msh
# "file:square1x1-tg-p1-303n.msh"      // works
# "file:square1x1-tg-p1-7614.msh"      // works
#  file:trapezium1x1-tg-p1-508.msh     // works
# "file:square1x1-tg-p2-333n.msh"
# "file:square1x1-tg-p2-2kn.msh"       // works
# "file:trapezium1x1-tg-p2-1949.msh"   // works
# "file:square1x1-qd-p1-6561n.msh"
# "file:square1x1-qd-p1-1369.msh"      // works
# "file:square1x1-qd-p1-256n.msh"
# "file:square1x1-qd-p2-289n.msh"      // works
# "file:trapezium1x1-qd-p1-441.msh"    // LDA works
# "file:trapezium1x1-qd-p2-1681.msh"   // B crashes but LDA works?
# "file:trapezium1x1-qd-p3-3721.msh"   // B crashes but LDA works?
# "file:trapezium1x1-tg-p3-4306.msh"
# "file:square1x1-tgqd-p1-298n.msh"    // works
internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().set('update_vars', 'Cons2D')
solver.options().set('solution_space', 'LagrangeP2')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().set('maxiter', 100)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('cfl', 0.25)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().set('saverate', 10)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().set('filepath', cf.URI('file:periodic.plt'))

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().set('functions', 
     ['if(x>0.5,0.5,1.)',
      '0.0',
      'if(x>0.5,1.67332,2.83972)',
      'if(x>0.5,3.425,6.532)'])
iconds.get_child('INIT').options().set('regions', internal_regions)

### boundary conditions

solver.get_child('BoundaryConditions').create_boundary_condition(name='INLET',type='cf3.RDM.BcDirichlet')
solver.get_child('BoundaryConditions').get_child('INLET').options().set('functions',
     ['if(x>0.5,0.5,1.)',
      '0.0',
      'if(x>0.5,1.67332,2.83972)',
      'if(x>0.5,3.425,6.532)'])

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
gmsh_writer.options().set('file',cf.URI('file:_geo_initial_riemann.msh'))
gmsh_writer.execute()
gmsh_writer.options().set('fields',fsol)
gmsh_writer.options().set('file',cf.URI('file:_sol_initial_riemann.msh'))
gmsh_writer.execute()

model.simulate()

#import networkxpython as nx
#nx.show_graph(cf.URI('//Model/Domain/mesh'),depth=1000,tree='clf',caption='clf',printdestination='sc',hidden='')
#nx.show_graph(solver.uri(),depth=1000,tree='coltf',caption='coltf',printdestination='s',hidden='')

gmsh_writer.options().set('fields',fgeo)
gmsh_writer.options().set('file',cf.URI('file:_geo_final_riemann.msh'))
gmsh_writer.execute()
gmsh_writer.options().set('fields',fsol)
gmsh_writer.options().set('file',cf.URI('file:_sol_final_riemann.msh'))
gmsh_writer.execute()

#tecplot_writer.options().set('file',cf.URI('file:final_euler2d-riemann.plt'))
#tecplot_writer.execute()
