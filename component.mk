#
# Component Makefile
#

ifeq ($(CONFIG_RTOS_LIB_FREERTOS),y)
COMPONENT_ADD_INCLUDEDIRS += src/port/freertos/Common/include
COMPONENT_ADD_INCLUDEDIRS += src/freertos/include
COMPONENT_ADD_INCLUDEDIRS += src/port/freertos/$(CONFIG_MCU)

COMPONENT_SRCDIRS += src/port/freertos/Common
COMPONENT_SRCDIRS += src/freertos
COMPONENT_SRCDIRS += src/port/freertos/$(CONFIG_MCU)
endif

ifeq ($(CONFIG_RTOS),y)
COMPONENT_ADD_INCLUDEDIRS += src
COMPONENT_SRCDIRS += src

# remove all " in paths
COMPONENT_ADD_INCLUDEDIRS := $(shell echo $(COMPONENT_ADD_INCLUDEDIRS))
COMPONENT_SRCDIRS := $(shell echo $(COMPONENT_SRCDIRS))
endif