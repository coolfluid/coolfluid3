import numpy as np
import pylab as pl
import sys

conv_data = np.loadtxt(sys.argv[1])
pl.plot(conv_data[:,0]) # plot column 1
pl.show()
