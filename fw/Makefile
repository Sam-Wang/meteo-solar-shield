BINARY = main
OBJS += sensor.o si7021.o mpl3115.o can.o


LDSCRIPT = stm32f050g6u6.ld
LDFLAGS += --specs=rdimon.specs
LDFLAGS += --specs=nano.specs
LDLIBS	+= -lrdimon

include Makefile.include
