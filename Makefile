ARMGNU ?= aarch64-none-elf

COPS = -nostdlib -nostartfiles -ffreestanding -Iinclude -mgeneral-regs-only -Wunused-variable -Wint-conversion -Wint-to-pointer-cast -mtune=cortex-a72 -mlittle-endian -mcmodel=small -O2 -g
ASMOPS = -Iinclude -mtune=cortex-a72 -mlittle-endian -mcmodel=small

BUILD_DIR = build
SRC_DIR = src


all : kernel8.img

clean :
	rm -rf $(BUILD_DIR) *.img 

$(BUILD_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(COPS) -MMD -c $< -o $@

$(BUILD_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(BUILD_DIR)/arch/arm64
	mkdir -p $(BUILD_DIR)/kernel
	$(ARMGNU)-gcc $(ASMOPS) -MMD -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/kernel/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/arch/arm64/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(BUILD_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

copy:
	cp kernel8.img /Volumes/boot/
	sleep 2
	cp kernel8.img /Volumes/boot/

kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf  $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary kernel8.img
	cp kernel8.img /Volumes/boot/
	sleep 2
	cp kernel8.img /Volumes/boot/
