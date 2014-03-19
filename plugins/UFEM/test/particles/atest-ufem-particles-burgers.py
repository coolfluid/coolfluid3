import sys
import coolfluid as cf
import numpy as np
import os
from optparse import OptionParser
import pylab as pl

env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 4

rho_f = 0.9
mu = 0.19e-4 * rho_f

r0 = 0.01
omega0 = 18.
g = 10.

dt = 0.00001

numsteps = 10
write_interval = 50

resolution = 64

# Create the model and solvers
model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
physics.options.dimension = 2
solver = model.create_solver('cf3.UFEM.Solver')

polydisp = solver.add_unsteady_solver('cf3.UFEM.particles.Polydisperse')
polydisp.options.velocity_variable = 'Velocity'
polydisp.options.velocity_tag = 'navier_stokes_u_solution'
polydisp.options.save_beta = True

# Set up the physical constants
physics.density = rho_f
physics.dynamic_viscosity = mu

polydisp.initial_diameters = [20.e-6, 40.e-6, 80.e-6]
polydisp.initial_concentrations = [100., 0.01, 0.00001]
polydisp.nb_phases = 3

# Create the mesh
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("nb_cells",[resolution, resolution, 1])
mesh_generator.options().set("lengths",[0.29, 0.4, 0.05])
mesh_generator.options().set("offsets",[-0.09,-0.2, 0.])
mesh_generator.execute()

polydisp.regions = [mesh.topology.interior.uri()]

polydisp.children.ConcentrationSolver.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'

series_writer = solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
writer.file = cf.URI('polydisperse-burgers-{iteration}.pvtu')
writer.mesh = mesh
series_writer.interval = write_interval

solver.create_fields()
writer.fields = [mesh.geometry.particle_concentration_1.uri(), mesh.geometry.weighted_particle_volume_1.uri()]

u_arr = ['-((1 - exp(-(x^2 + y^2)/{r0}^2))*{omega0}*{r0}^2*y)/(2.*(x^2 + y^2))'.format(r0=r0,omega0=omega0), '((1 - exp(-(x^2 + y^2)/{r0}^2))*{omega0}*{r0}^2*x)/(2.*(x^2 + y^2))'.format(r0=r0,omega0=omega0), '0']
ic_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'navier_stokes_u_solution')
ic_u.variable_name = 'Velocity'
ic_u.regions = [mesh.topology.uri()]
ic_u.value = u_arr

ic_lin_u = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionFunction', field_tag = 'linearized_velocity')
ic_lin_u.variable_name = 'AdvectionVelocity1'
ic_lin_u.regions = [mesh.topology.uri()]
ic_lin_u.value = u_arr

ic_g = solver.InitialConditions.create_initial_condition(builder_name = 'cf3.UFEM.InitialConditionConstant', field_tag = 'body_force')
ic_g.regions = [mesh.topology.uri()]
ic_g.Force = [0.,-g, 0.]
    
# Time setup
time = model.create_time()
time.time_step = dt
time.end_time = numsteps*dt

model.simulate()
model.print_timing_tree()

c0 = np.array(mesh.geometry.particle_concentration_0)
c1 = np.array(mesh.geometry.particle_concentration_1)
zeta0 = np.array(mesh.geometry.weighted_particle_volume_0)
zeta1 = np.array(mesh.geometry.weighted_particle_volume_1)

domain.write_mesh(cf.URI('polydisperse-burgers-end.pvtu'))
