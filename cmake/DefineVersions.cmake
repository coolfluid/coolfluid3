# coolfluid release version
set( CF3_VERSION_MAJOR    2011)
set( CF3_VERSION_MINOR    3)
set( CF3_VERSION "${CF3_VERSION_MAJOR}.${CF3_VERSION_MINOR}" )

# coolfluid kernel version
set( CF3_KERNEL_VERSION_MAJOR    3)
set( CF3_KERNEL_VERSION_MINOR    0)
set( CF3_KERNEL_VERSION_MICRO    alpha2)
set( CF3_KERNEL_VERSION          "${CF3_KERNEL_VERSION_MAJOR}.${CF3_KERNEL_VERSION_MINOR}.${CF3_KERNEL_VERSION_MICRO}" )


# Append the library version information to the library target properties.
# A parent project may set its own properties and/or may block this.
# TO avoid it set CF3_NO_LIBRARY_VERSION to TRUE
if( NOT CF3_NO_LIBRARY_VERSION )

  set(  coolfluid_LIBRARY_PROPERTIES ${coolfluid_LIBRARY_PROPERTIES}
         VERSION   "${CF3_KERNEL_VERSION}"
         SOVERSION "${CF3_KERNEL_VERSION_MAJOR}.${CF3_KERNEL_VERSION_MINOR}"
      )

endif()


