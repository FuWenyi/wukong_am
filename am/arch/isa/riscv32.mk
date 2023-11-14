CROSS_COMPILE := riscv64-unknown-elf-
COMMON_FLAGS  := -fno-pic -march=rv32g -mabi=ilp32
CFLAGS        += $(COMMON_FLAGS) -static
ASFLAGS       += $(COMMON_FLAGS) -O2
LDFLAGS       += -melf32lriscv
