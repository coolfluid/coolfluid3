import coolfluid as cf
import sys

root = cf.Core.root()
model = root.create_component('model', 'cf3.solver.CModel')

domain = model.create_domain()
solver = model.create_solver('cf3.solver.CSimpleSolver')
phys_model = model.create_physics('cf3.physics.DynamicModel')

if domain.name() != 'Domain':
  raise Exception('Failed to create domain')
if solver.name() != 'CSimpleSolver':
  raise Exception('Failed to create solver')
if phys_model.name() != 'DynamicModel':
  raise Exception('Failed to create physics')