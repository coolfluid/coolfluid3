#!python
# -*- coding: utf-8 -*-

import sys
import subprocess
import numpy as np
import pylab
import os

# process arguments
mpi_run = sys.argv[1]
test_command = sys.argv[2]
max_nb_procs = int(sys.argv[3])

# build a list of the number of processes to use
nb_procs_lst = [1]
while 2*nb_procs_lst[-1] <= max_nb_procs:
  nb_procs_lst.append(2*nb_procs_lst[-1])

search_str = 'time" type="numeric/double">'

testmap = {}

for nb_procs in nb_procs_lst:
  cmd = [mpi_run]
  cmd.append('-np')
  cmd.append(str(nb_procs))
  cmd.append(test_command)
  for extra_arg in sys.argv[4:len(sys.argv)]:
    cmd.append(extra_arg)
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE)
  if p.wait() != 0:
    raise "Test exited with error"
  for line in p.stdout:
    idx = line.find(search_str)
    if idx > 0:
      testname = line[0:idx].split('"')[-1].strip()
      timing = float(line[idx+len(search_str):-1].split('<')[0])
      if not testmap.has_key(testname):
        testmap[testname] = []
      testmap[testname].append(timing)

nb_procs_arr = np.array(nb_procs_lst)
for testname in testmap:
  timing_arr = np.array(testmap[testname])
  pylab.plot(nb_procs_arr, timing_arr)
  pylab.plot(nb_procs_arr, timing_arr, 'ko')

  pylab.xlabel('Number of processes')
  pylab.ylabel('Time (s)')
  pylab.title(testname)
  pylab.grid(True)
  pylab.axis([0, max_nb_procs+1, 0, max(timing_arr)*1.05])
  
  fig_filename = testname + '-timing.png'
  pylab.savefig(fig_filename)
  print '<DartMeasurementFile name="' + testname + ' scaling graph" type="image/png">' + os.path.join(os.path.dirname(test_command), fig_filename) + '</DartMeasurementFile>'
  pylab.clf()
