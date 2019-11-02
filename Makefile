
ifeq ($(detected_OS),Darwin)
	#ARMGNU ?= aarch64-linux-gnu
	ARMGNU ?= aarch64-unknown-linux-gnueabi
	ARMGNU = /opt/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu
else
	#ARMGNU ?= aarch64-none-elf
	#ARMGNU ?= aarch64-linux-gnu
	ARMGNU ?= aarch64-unknown-linux-gnueabi
	ARMGNU = /opt/gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu/bin/aarch64-linux-gnu
endif

COPS = -std=c11 -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -Wunused-variable -Wint-to-pointer-cast
ASMOPS = -Iinclude 

BUILD_DIR = build
SRC_DIR = src


all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_cpp.o: $(SRC_DIR)/%.cpp
	mkdir -p $(@D)
	$(ARMGNU)-g++ -nodefaultlibs -fno-exceptions -nostdlib -nostartfiles -x c++ -fno-rtti -c -std=c++11 -Iinclude -Wunused-variable -Wint-to-pointer-cast -MMD $< -o $@

#-nostdlib -nostartfiles -ffreestanding 

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(BUILD_DIR)/arch/arm64
	mkdir -p $(BUILD_DIR)/kernel
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/kernel/*.c)
CPP_FILES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/*/*.cpp) $(wildcard $(SRC_DIR)/drivers/*/*.cpp)
ASM_FILES = $(wildcard $(SRC_DIR)/arch/arm64/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)
OBJ_FILES += $(CPP_FILES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%_cpp.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
