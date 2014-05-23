import sys
import coolfluid as cf
import numpy as np

env = cf.Core.environment()

# Global configuration
env.assertion_throws = False
env.assertion_backtrace = False
env.exception_backtrace = False
env.regist_signal_handlers = False
env.log_level = 3

nu = 1./5000.

dt = 0.001

numsteps = 200
write_interval = 50


# Create the model and solvers
model = cf.Core.root().create_component('NavierStokes', 'cf3.solver.ModelUnsteady')
domain = model.create_domain()
physics = model.create_physics('cf3.UFEM.NavierStokesPhysics')
physics.options.dimension = 2
solver = model.create_solver('cf3.UFEM.Solver')

polydisp = solver.add_unsteady_solver('cf3.UFEM.particles.Polydisperse')
polydisp.options.velocity_variable = 'Velocity'
polydisp.options.velocity_tag = 'navier_stokes_u_solution'

# Set up the physical constants
physics.density = 1.
physics.dynamic_viscosity = nu

polydisp.initial_diameters = [(6./np.pi)**(1./3.), (12./np.pi)**(1./3.)]
polydisp.initial_concentrations = [1, 0.0001]
polydisp.nb_phases = 2
polydisp.options.collision_kernel_type = 'UnityCollisionKernel'

# Create the mesh
mesh = domain.create_component('Mesh', 'cf3.mesh.Mesh')
mesh_generator = domain.create_component("mesh_generator","cf3.mesh.SimpleMeshGenerator")
mesh_generator.options().set("mesh",mesh.uri())
mesh_generator.options().set("nb_cells",[1, 1])
mesh_generator.options().set("lengths",[1.,1.])
mesh_generator.options().set("offsets",[0.,0.])
mesh_generator.execute()

polydisp.regions = [mesh.topology.interior.uri()]

polydisp.children.ConcentrationSolver.LSS.SolutionStrategy.Parameters.linear_solver_type = 'Amesos'

series_writer = solver.TimeLoop.create_component('TimeWriter', 'cf3.solver.actions.TimeSeriesWriter')
writer = series_writer.create_component('Writer', 'cf3.mesh.VTKXML.Writer')
writer.file = cf.URI('polydisperse-uniform-{iteration}.pvtu')
writer.mesh = mesh
series_writer.interval = write_interval

solver.create_fields()
writer.fields = [mesh.geometry.particle_concentration_1.uri(), mesh.geometry.weighted_particle_volume_1.uri()]
    
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

print 'zeta:', zeta0[0,0], zeta1[0,0]
print 'c:', c0[0,0], c1[0,0]
print 'v:', zeta0[0,0]/c0[0,0], zeta1[0,0]/c1[0,0]

domain.write_mesh(cf.URI('polydisperse-uniform-end.pvtu'))
