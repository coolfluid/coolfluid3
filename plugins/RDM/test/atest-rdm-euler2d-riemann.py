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
env.options().configure_option('log_level', 3)
env.options().configure_option('regist_signal_handlers', False)

### create model

wizard = root.create_component('Wizard',  'cf3.RDM.SteadyExplicit')

wizard.create_model(model_name='Model', physical_model='cf3.physics.NavierStokes.NavierStokes2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
#domain.load_mesh(file=cf.URI('file:trapezium1x1-tg-p1-508.msh'), name='mesh')
domain.load_mesh(file=cf.URI('file:trapezium1x1-tg-p1-508.msh'), name='mesh')
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
solver.options().configure_option('update_vars', 'Cons2D')
solver.options().configure_option('solution_space', 'LagrangeP1')

solver.get_child('IterativeSolver').get_child('MaxIterations').options().configure_option('maxiter', 100)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().configure_option('cfl', 0.25)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().configure_option('saverate', 10)
#solver.get_child('IterativeSolver').get_child('PostActions').get_child('PeriodicWriter').options().configure_option('filepath', cf.URI('file:periodic.plt'))

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().configure_option('functions', 
     ['if(x>0.5,0.5,1.)',
      '0.0',
      'if(x>0.5,1.67332,2.83972)',
      'if(x>0.5,3.425,6.532)'])
iconds.get_child('INIT').options().configure_option('regions', internal_regions)

### boundary conditions

solver.get_child('BoundaryConditions').create_boundary_condition(name='INLET',type='cf3.RDM.BcDirichlet')
solver.get_child('BoundaryConditions').get_child('INLET').options().configure_option('functions',
     ['if(x>0.5,0.5,1.)',
      '0.0',
      'if(x>0.5,1.67332,2.83972)',
      'if(x>0.5,3.425,6.532)'])

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.LDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().configure_option('regions', internal_regions)

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

#tecplot_writer = model.create_component('tecplot_writer','cf3.mesh.tecplot.Writer')
#tecplot_writer.options().configure_option('mesh',cf.URI('//Model/Domain/mesh'))
#tecplot_writer.options().configure_option('fields',fields)
#tecplot_writer.options().configure_option('file',cf.URI('file:initial_euler2d-riemann.plt'))
#tecplot_writer.execute()

gmsh_writer = model.create_component('gmsh_writer','cf3.mesh.gmsh.Writer')
gmsh_writer.options().configure_option('mesh',cf.URI('//Model/Domain/mesh'))
gmsh_writer.options().configure_option('fields',fields)
gmsh_writer.options().configure_option('file',cf.URI('file:initial_euler2d-riemann.msh'))
gmsh_writer.execute()


model.simulate()

gmsh_writer.options().configure_option('file',cf.URI('file:final_euler2d-riemann.msh'))
gmsh_writer.execute()

#tecplot_writer.options().configure_option('file',cf.URI('file:final_euler2d-riemann.plt'))
#tecplot_writer.execute()
