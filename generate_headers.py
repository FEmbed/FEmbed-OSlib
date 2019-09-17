Import('env')

CPPDEFINES=[
    ("USE_OSLIB", 1),
    ("USE_ESPRESSIF8266", 1),
    ("configSUPPORT_STATIC_ALLOCATION", 1),
    ("FE_IS_IN_ISR", "xPortInIsrContext")
]

env.Append(
    CPPDEFINES=CPPDEFINES
)

from os.path import join, realpath

#print(env.Dump())
if env["PIOPLATFORM"] == "espressif8266":
    pass

global_env = DefaultEnvironment()
global_env.Append(
    CPPDEFINES=CPPDEFINES
)