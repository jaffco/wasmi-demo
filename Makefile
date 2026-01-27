# Project Name
TARGET = wasmi-demo

# Sources
CPP_SOURCES = src/main.cpp

# wasmi-daisy integration
WASMI_DAISY_PATH = ./wasmi-daisy
C_INCLUDES += -I$(WASMI_DAISY_PATH)
LDFLAGS += $(WASMI_DAISY_PATH)/target/thumbv7em-none-eabihf/release/libwasmi_daisy.a

# Library Locations
include ./common.mk