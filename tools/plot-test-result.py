#!python
# -*- coding: utf-8 -*-

import matplotlib
matplotlib.use('AGG')
import sys
import subprocess
import numpy as np
import pylab
import os

var_prefix = 'PyVar'

# process arguments
test_command = sys.argv[1]

# run the test
p = subprocess.Popen(test_command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

# print test output in case of error
if p.wait() != 0:
  for line in p.stdout:
    print line,
  for line in p.stderr:
    print line,
  raise RuntimeError("Test exited with error")

# expects a list of lines that are ordered: title, X-values, Y-values, legend label
varnames = []
for line in p.stdout:
  if line.startswith(var_prefix):
    varnames.append(line.split('=')[0].strip().replace(var_prefix, ''))
    exec(line.replace(var_prefix, ''))
    if len(varnames) == 3:
      pylab.plot(locals()[varnames[0]], locals()[varnames[1]], label=locals()[varnames[2]])
      pylab.xlabel(varnames[0])
      pylab.ylabel(varnames[1])
      varnames = []

pylab.grid(True)
pylab.legend()

fig_filename = test_command.split('/')[-1] + '.png'
pylab.savefig(fig_filename)
print '<DartMeasurementFile name="Graph" type="image/png">' + os.path.join(os.path.dirname(test_command), fig_filename) + '</DartMeasurementFile>'
pylab.clf()
