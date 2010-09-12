# coolfluid release version
set( CF_VERSION_MAJOR    2010)
set( CF_VERSION_MINOR    0)
set( CF_VERSION "${CF_VERSION_MAJOR}.${CF_VERSION_MINOR}" )

# coolfluid kernel version
set( CF_KERNEL_VERSION_MAJOR    3)
set( CF_KERNEL_VERSION_MINOR    0)
set( CF_KERNEL_VERSION_MICRO    0)
set( CF_KERNEL_VERSION          "${CF_KERNEL_VERSION_MAJOR}.${CF_KERNEL_VERSION_MINOR}.${CF_KERNEL_VERSION_MICRO}" )


# Append the library version information to the library target properties.
# A parent project may set its own properties and/or may block this.
# TO avoid it set CF_NO_LIBRARY_VERSION to TRUE
if ( NOT CF_NO_LIBRARY_VERSION )

  set(  coolfluid_LIBRARY_PROPERTIES ${coolfluid_LIBRARY_PROPERTIES}
         VERSION   "${CF_KERNEL_VERSION}"
         SOVERSION "${CF_KERNEL_VERSION_MAJOR}.${CF_KERNEL_VERSION_MINOR}"
      )

endif()


