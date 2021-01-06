Import('env', 'pio_lib_builder')
from os.path import join, realpath

platform = env.PioPlatform()
framework = env.subst('$PIOFRAMEWORK')

#print(env.Dump())

if framework != "fembed":
    CPPDEFINES=[
        ("USE_OSLIB", "1"),
        ("FE_IS_IN_ISR", "xPortInIsrContext")
    ]

    env.Append(
        CPPDEFINES=CPPDEFINES
    )

    from os.path import join, realpath

    if env["PIOFRAMEWORK"] == "esp8266-rtos-sdk":
        pass
    env.Replace(SRC_FILTER = [
            "-<freertos/*.c>",
            "-<port>",
            "+<port/freertos/Common>",
            "+<*.c>",
            "+<*.cpp>",
            "+<*.h>"
        ])
    global_env = DefaultEnvironment()
    global_env.Append(
        CPPDEFINES=CPPDEFINES
    )
else:
    global_env = DefaultEnvironment()

    CPPDEFINES=[
        ("USE_OSLIB", "1"),
    ]
    global_env.Append(CPPDEFINES=CPPDEFINES)

    CPPPATH = [
        realpath("src"),
        realpath("src/port/freertos/Common/include"),
        realpath("src/freertos/include"),
        realpath("src/port/freertos/%s" % env['SDKCONFIG']['CONFIG_MCU']),
        ]
    global_env.Append(CPPPATH=CPPPATH)
    env.Append(CPPPATH=CPPPATH)

    env.Replace(SRC_FILTER = [
            "+<freertos/*>",
            "+<port/freertos/Common>",
            "+<port/freertos/%s>" % env['SDKCONFIG']['CONFIG_MCU'],
            "+<*.c>",
            "+<*.cpp>",
            "+<*.h>"
        ])
    #print(global_env.Dump())