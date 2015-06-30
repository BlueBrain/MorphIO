
set(BRION_PACKAGE_VERSION 1.5)
set(BRION_REPO_URL https://github.com/BlueBrain/Brion)
set(BRION_DEPENDS REQUIRED Boost HDF5 Lunchbox vmmlib
                  OPTIONAL BBPTestData OpenMP)
set(BRION_BOOST_COMPONENTS
  "date_time filesystem regex system unit_test_framework program_options")
set(BRION_HDF5_COMPONENTS "C CXX")
set(BRION_DEB_DEPENDS libhdf5-serial-dev libboost-date-time-dev
  libboost-filesystem-dev libboost-regex-dev libboost-system-dev
  libboost-test-dev)
set(BRION_SUBPROJECT ON)

if(CI_BUILD_COMMIT)
  set(BRION_REPO_TAG ${CI_BUILD_COMMIT})
else()
  set(BRION_REPO_TAG master)
endif()
set(BRION_FORCE_BUILD ON)
set(BRION_SOURCE ${CMAKE_SOURCE_DIR})