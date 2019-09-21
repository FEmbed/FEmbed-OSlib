Import('env')

CPPDEFINES=[
    ("USE_OSLIB", "1"),
    ("USE_ESPRESSIF8266", "1"),
    ("FE_IS_IN_ISR", "xPortInIsrContext")
]

env.Append(
    CPPDEFINES=CPPDEFINES
)

from os.path import join, realpath

if env["PIOFRAMEWORK"] == "esp8266-rtos-sdk":
    pass

#print(env.Dump())

global_env = DefaultEnvironment()
global_env.Append(
    CPPDEFINES=CPPDEFINES
)