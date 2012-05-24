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
env.options().set('log_level', 5)
env.options().set('regist_signal_handlers', False)

### create model

wizard = root.create_component('Wizard',  'cf3.RDM.UnsteadyExplicit')
wizard.options().set('rkorder',2)
wizard.create_model(model_name='Model', physical_model='cf3.physics.LinEuler.LinEuler2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
mesh = domain.load_mesh(file=cf.URI('circle150r-tg-p1-3471.msh', cf.URI.Scheme.file), name='mesh')
#mesh = domain.load_mesh(file=cf.URI('circle.msh', cf.URI.Scheme.file), name='mesh')

internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().set('update_vars', 'Cons2D')
solver.options().set('solution_space', 'LagrangeP1')

#print("----------------------------------------------------------------------------------------------------------")
#solver.get_child('TimeStepping').list_options_recursive()
#root.list_options_recursive()
#print("----------------------------------------------------------------------------------------------------------")
#solver.list_signals_recursive()
#print("----------------------------------------------------------------------------------------------------------")
#root.list_tree_recursive()
#print("----------------------------------------------------------------------------------------------------------")

solver.get_child('TimeStepping').get_child('Time').options().set('time_step', 0.13)
solver.get_child('TimeStepping').get_child('Time').options().set('end_time', 50.)
solver.get_child('TimeStepping').get_child('MaxIterations').options().set('maxiter', 10)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('cfl', 1.) #0.25)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().set('regions', internal_regions)


### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().set('functions', 
  ['exp((-0.301)/25.*(x*x+y*y))+0.1*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   '0.04*(y)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   '-0.04*(x-67.)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   'exp((-0.301)/25.*(x*x+y*y))'])
iconds.get_child('INIT').options().set('regions', internal_regions)

### boundary conditions

#call Model/RDSolver/BoundaryConditions/create_boundary_condition \
#     name:string=FARFIELD \
#     type:string=cf3.RDM.BcDirichlet \
#     regions:array[uri]=//Model/Domain/mesh/topology/perimeter
#configure Model/RDSolver/BoundaryConditions/FARFIELD functions:array[string]=0.,0.,0.,0.

#bcs = solver.get_child('BoundaryConditions')
#
#bcs.create_boundary_condition(name='INLET', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/default_id1084/inlet')])
#bcs.get_child('INLET').options().set('functions', ['if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)'])
#
#bcs.create_boundary_condition(name='FARFIELD', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/default_id1084/farfield')])
#bcs.get_child('FARFIELD').options().set('functions', ['0'])

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.RKLDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().set('regions', internal_regions)

### simulate and write the result

#import networkxpython as nx
#nx.show_graph(cf.URI('//Model/Domain/mesh'),depth=1000,tree='clf',caption='clf',printdestination='s',hidden='')


iconds.execute()


fgeo=[cf.URI('//Model/Domain/mesh/geometry/solution'),
      cf.URI('//Model/Domain/mesh/geometry/residual'),
      cf.URI('//Model/Domain/mesh/geometry/wave_speed'),
      cf.URI('//Model/Domain/mesh/geometry/dual_area')]
fsol=[cf.URI('//Model/Domain/mesh/solution/solution'),
      cf.URI('//Model/Domain/mesh/solution/residual'),
      cf.URI('//Model/Domain/mesh/solution/wave_speed'),
      cf.URI('//Model/Domain/mesh/solution/dual_area')]

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

#interpolator = model.get_child('tools').create_component('interpolator','cf3.mesh.actions.Interpolate')
#interpolator.interpolate(source=cf.URI('//Model/Domain/mesh/solution/solution'),
                         #target=cf.URI('//Model/Domain/mesh/geometry/solution'))

gmsh_writer.options().set('fields',fgeo)
gmsh_writer.options().set('file',cf.URI('file:_geo_final_lineuler.msh'))
gmsh_writer.execute()
gmsh_writer.options().set('fields',fsol)
gmsh_writer.options().set('file',cf.URI('file:_sol_final_lineuler.msh'))
gmsh_writer.execute()

#tecplot_writer.options().set('file',cf.URI('file:final.plt'))
#tecplot_writer.execute()

#import networkxpython as nx
#nx.show_graph(cf.URI('//Model/Domain/mesh'),depth=1000,tree='clf',caption='clf',printdestination='s',hidden='')#
#nx.show_graph(solver.uri(),depth=1000,tree='coltf',caption='coltf',printdestination='s',hidden='')




