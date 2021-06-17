idf_build_get_property(components_to_build BUILD_COMPONENTS)

set(src)
set(include)
set(priv_requires)

set(srcs    "src/osMutex.cpp"
            "src/osTask.cpp" 
            "src/osTimer.cpp"
            "src/port/freertos/Common/rtos_stub.c"
            )
set(include "src" )

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS ${include}
                       REQUIRES "freertos fe-arduino"
                       PRIV_REQUIRES ${priv_requires})

target_compile_definitions(${COMPONENT_TARGET} PUBLIC -DUSE_OSLIB=1 -DconfigSUPPORT_STATIC_ALLOCATION=1 -DFE_IS_IN_ISR=xPortInIsrContext)
