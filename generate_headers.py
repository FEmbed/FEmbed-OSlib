Import('env')
#print(env.Dump())
env.Append(
    CPPPATH=["src",
             "port/esp8266/include/freertos",
             "RTOS/FreeRTOS/include"]
)
print(env["PIOPLATFORM"]) #espressif8266

global_env = DefaultEnvironment()
global_env.Append(
    CPPDEFINES=[
        ("USE_OSLIB", 1),
        ("USE_ARDUINO", 1)
    ]
)