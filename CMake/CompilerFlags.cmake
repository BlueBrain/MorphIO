if (WIN32)
  set(FLAGS "${FLAGS} /DH5_BUILT_AS_DYNAMIC_LIB")

  # Exception handling
  # s: Enables standard C++ stack unwinding.
  # c: [assume] functions declared as extern "C" never throw a C++ exception.
  set(FLAGS "${FLAGS} /EHsc")

  # get access to M_PI constant
  set(FLAGS "${FLAGS} /D_USE_MATH_DEFINES")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  # Taken from https://github.com/BlueBrain/hpc-coding-conventions/blob/master/cpp/cmake/bob.cmake#L192-L255
  if(${PROJECT_NAME}_CXX_WARNINGS)
    if(${PROJECT_NAME}_WERROR)
      set(FLAGS "${FLAGS} -Werror")
    endif()
    set(FLAGS "${FLAGS} -Weverything")
    # Using std=c++11, no need for 98 compat
    set(FLAGS "${FLAGS} -Wno-c++98-compat")
    set(FLAGS "${FLAGS} -Wno-c++98-compat-pedantic")

    # allow static objects
    set(FLAGS "${FLAGS} -Wno-global-constructors")
    set(FLAGS "${FLAGS} -Wno-exit-time-destructors")

    set(FLAGS "${FLAGS} -Wno-documentation")
    set(FLAGS "${FLAGS} -Wno-documentation-unknown-command")

    set(FLAGS "${FLAGS} -Wno-padded")
    set(FLAGS "${FLAGS} -Wno-weak-vtables")
    set(FLAGS "${FLAGS} -Wno-covered-switch-default")
    set(FLAGS "${FLAGS} -Wno-poison-system-directories")
  endif()
  if(APPLE)
    set(FLAGS "${FLAGS} -Wno-undef")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8.0.0")
      set(FLAGS "${FLAGS} -fcomment-block-commands=file")
    endif()
  else()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_EQUAL "5.0.0")
      set(FLAGS "${FLAGS} -fcomment-block-commands=file")
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "5.0.0")
      set(FLAGS "${FLAGS} -fcomment-block-commands=file")
    endif()
  endif()

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
  if(${PROJECT_NAME}_CXX_WARNINGS)
    if(${PROJECT_NAME}_WERROR)
      set(FLAGS "${FLAGS} -Werror")
    endif()
    set(FLAGS "${FLAGS} -Wall")
    set(FLAGS "${FLAGS} -Wcast-align")
    set(FLAGS "${FLAGS} -Wconversion")
    set(FLAGS "${FLAGS} -Wdouble-promotion")
    set(FLAGS "${FLAGS} -Wextra")
    set(FLAGS "${FLAGS} -Wformat=2")
    set(FLAGS "${FLAGS} -Wnon-virtual-dtor")
    set(FLAGS "${FLAGS} -Wold-style-cast")
    set(FLAGS "${FLAGS} -Woverloaded-virtual")
    set(FLAGS "${FLAGS} -Wshadow")
    set(FLAGS "${FLAGS} -Wsign-conversion")
    set(FLAGS "${FLAGS} -Wunused")
    set(FLAGS "${FLAGS} -Wuseless-cast")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "6.0")
      set(FLAGS "${FLAGS} -Wduplicated-cond")
      set(FLAGS "${FLAGS} -Wmisleading-indentation")
      set(FLAGS "${FLAGS} -Wnull-dereference")
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.0")
      set(FLAGS "${FLAGS} -Wduplicated-branches")
      set(FLAGS "${FLAGS} -Wlogical-op")
      set(FLAGS "${FLAGS} -Wrestrict")
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "8.0")
      set(FLAGS "${FLAGS} -Wclass-memaccess")
    endif()
  endif()
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")

else()
  message(WARNING "Unexpected compiler type ${CMAKE_CXX_COMPILER_ID}")
endif()
