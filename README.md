COOLFluiD 3
===========

Download and installation
-------------------------

  + To install from the Git repository
  
```
  mkdir coolfluid3
  cd coolfluid3
  export CF3=$PWD
  git clone https://github.com/coolfluid/coolfluid3 kernel
  ./kernel/tools/install-deps.pl --install=basic,zoltan --install-dir=$CF3/deps
  mkdir build
  cd build
  cmake $CF3/kernel -DDEPS_ROOT=$CF3/deps
  make
```

  + For developers
  
    Create a github account and fork from coolfluid3 project.
    Follow the same steps as described above, but use your own github fork instead.
    Send pull-request when you are done developing.

  + To install from sources see [[https://github.com/coolfluid/coolfluid3/wiki/Installation]]
