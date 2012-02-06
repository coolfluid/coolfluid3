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

wizard = root.create_component('Wizard',  'cf3.RDM.UnsteadyExplicit')
wizard.options().configure_option('rkorder',1)
wizard.create_model(model_name='Model', physical_model='cf3.physics.LinEuler.LinEuler2D')
model = root.get_child('Model')

### read mesh

domain = model.get_child('Domain')
domain.load_mesh(file=cf.URI('circleP2.msh', cf.URI.Scheme.file), name='mesh')

internal_regions = [cf.URI('//Model/Domain/mesh/topology/domain')]

### solver

solver = model.get_child('RDSolver')
solver.options().configure_option('update_vars', 'Cons2D')
solver.options().configure_option('solution_space', 'LagrangeP2')

#solver.get_child('IterativeSolver').get_child('MaxIterations').options().configure_option('maxiter', 10)

#print("----------------------------------------------------------------------------------------------------------")
#solver.get_child('TimeStepping').list_options_recursive()
#root.list_options_recursive()
#print("----------------------------------------------------------------------------------------------------------")
#solver.list_signals_recursive()
#print("----------------------------------------------------------------------------------------------------------")
#root.list_tree_recursive()
#print("----------------------------------------------------------------------------------------------------------")

solver.get_child('TimeStepping').get_child('Time').options().configure_option('time', 0.)
solver.get_child('TimeStepping').get_child('Time').options().configure_option('time_step', 0.13)
solver.get_child('TimeStepping').get_child('Time').options().configure_option('end_time', 50.)
solver.get_child('TimeStepping').get_child('MaxIterations').options().configure_option('maxiter', 50)
solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().configure_option('cfl', 1.) #0.25)
#solver.get_child('IterativeSolver').get_child('Update').get_child('Step').options().configure_option('regions', internal_regions)


### ??? configure Model/RDSolver  solution_space:string=LagrangeP1
### ??? configure Model/RDSolver/TimeStepping/Time    time:real=0.0 time_step:real=1 end_time:real=50

### initial conditions

iconds = solver.get_child('InitialConditions')
iconds.create_initial_condition(name='INIT')
iconds.get_child('INIT').options().configure_option('functions', 
  ['exp((-0.301)/25.*(x*x+y*y))+0.1*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   '0.04*(y)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   '-0.04*(x-67.)*exp((-0.301)/25.*((x-67.)*(x-67.)+y*y))',
   'exp((-0.301)/25.*(x*x+y*y))'])
iconds.get_child('INIT').options().configure_option('regions', internal_regions)

### boundary conditions

#call Model/RDSolver/BoundaryConditions/create_boundary_condition \
#     name:string=FARFIELD \
#     type:string=cf3.RDM.BcDirichlet \
#     regions:array[uri]=//Model/Domain/mesh/topology/perimeter
#configure Model/RDSolver/BoundaryConditions/FARFIELD functions:array[string]=0.,0.,0.,0.

#bcs = solver.get_child('BoundaryConditions')
#
#bcs.create_boundary_condition(name='INLET', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/default_id1084/inlet')])
#bcs.get_child('INLET').options().configure_option('functions', ['if(x>=-1.4,if(x<=-0.6,0.5*(cos(3.141592*(x+1.0)/0.4)+1.0),0.),0.)'])
#
#bcs.create_boundary_condition(name='FARFIELD', type='cf3.RDM.BcDirichlet', regions=[cf.URI('//Model/Domain/mesh/topology/default_id1084/farfield')])
#bcs.get_child('FARFIELD').options().configure_option('functions', ['0'])

### domain discretization

solver.get_child('DomainDiscretization').create_cell_term(name='INTERNAL', type='cf3.RDM.Schemes.RKLDA')
solver.get_child('DomainDiscretization').get_child('CellTerms').get_child('INTERNAL').options().configure_option('regions', internal_regions)

### simulate and write the result

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
gmsh_writer.options().configure_option('mesh',cf.URI('//Model/Domain/mesh'))
gmsh_writer.options().configure_option('fields',fgeo)
gmsh_writer.options().configure_option('file',cf.URI('file:_geo_initial.msh'))
gmsh_writer.execute()
gmsh_writer.options().configure_option('fields',fsol)
gmsh_writer.options().configure_option('file',cf.URI('file:_sol_initial.msh'))
gmsh_writer.execute()

#tecplot_writer = model.create_component('tecplot_writer','cf3.mesh.tecplot.Writer')
#tecplot_writer.options().configure_option('mesh',cf.URI('//Model/Domain/mesh'))
#tecplot_writer.options().configure_option('fields',fields)
#tecplot_writer.options().configure_option('file',cf.URI('file:initial.plt'))
#tecplot_writer.execute()

model.simulate()

gmsh_writer.options().configure_option('fields',fgeo)
gmsh_writer.options().configure_option('file',cf.URI('file:_geo_final.msh'))
gmsh_writer.execute()
gmsh_writer.options().configure_option('fields',fsol)
gmsh_writer.options().configure_option('file',cf.URI('file:_sol_final.msh'))
gmsh_writer.execute()

#tecplot_writer.options().configure_option('file',cf.URI('file:final.plt'))
#tecplot_writer.execute()

import networkxpython as nx
nx.show_graph(cf.URI('//Model/Domain/mesh'),depth=1000,tree='clf',caption='clf',printdestination='s',hidden='')



