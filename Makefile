# Project Name
TARGET = tapeecho

# Sources
CPP_SOURCES = tapedelay.cpp utils.cpp

# Headers
H_HEADERS = tapedelay.h utils.h

# Library Locations
LIBDAISY_DIR = DaisyExamples/libDaisy
DAISYSP_DIR = DaisyExamples/DaisySP


# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

