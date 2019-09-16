Import('env')

CPPDEFINES=[
    ("USE_OSLIB", 1),
    ("USE_ARDUINO", 1)
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