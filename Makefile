SHELL := /bin/sh
TARGET ?= x86_64-elf
CC ?= gcc
LD ?= ld
OBJCOPY ?= objcopy
OBJDUMP ?= objdump
READELF ?= readelf
QEMU ?= qemu-system-x86_64
DEBUG ?= 0
BUILD := build
KERNEL = $(BUILD)/novaos.elf
LIMINE_DIR ?= $(CURDIR)/toolchain/limine
CFLAGS := -std=c11 -ffreestanding -fno-builtin -fno-stack-protector -fno-pie -mno-red-zone -mcmodel=kernel -m64 -Wall -Wextra -Werror -Iinclude -I$(LIMINE_DIR)
ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O2
endif
LDFLAGS := -T linker/x86_64.ld -nostdlib
SRC := arch/x86_64/entry.S arch/x86_64/interrupts/entry.S arch/x86_64/gdt/gdt.c arch/x86_64/idt/idt.c kernel/main.c kernel/console.c kernel/panic.c kernel/cpu.c kernel/init.c kernel/mm/pmm.c kernel/mm/paging.c kernel/mm/heap.c kernel/mm/diagnostics.c arch/x86_64/tss.c arch/x86_64/ring3.S arch/x86_64/scheduler.S kernel/scheduler.c drivers/serial/serial.c boot/limine/adapter.c
OBJ = $(patsubst %.c,$(BUILD)/%.o,$(patsubst %.S,$(BUILD)/%.o,$(SRC)))
.PHONY: all check-build check-image limine kernel image run debug debug-check test exception-test clean
all: image
check-build:
	@command -v $(CC) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(CC)'; echo 'Install GCC: sudo apt-get install build-essential'; exit 1; }
	@command -v $(LD) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(LD)'; echo 'Install Binutils: sudo apt-get install binutils'; exit 1; }
	@command -v $(OBJCOPY) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(OBJCOPY)'; echo 'Install Binutils: sudo apt-get install binutils'; exit 1; }
	@command -v $(OBJDUMP) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(OBJDUMP)'; echo 'Install Binutils: sudo apt-get install binutils'; exit 1; }
	@command -v $(READELF) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(READELF)'; echo 'Install Binutils: sudo apt-get install binutils'; exit 1; }
limine:
	@command -v git >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: git'; echo 'Install Git: sudo apt-get install git'; exit 1; }
	sh scripts/fetch-limine.sh
	$(MAKE) -C $(LIMINE_DIR)
$(BUILD)/%.o: %.c check-build
	@mkdir -p $(dir $@); $(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/%.o: %.S check-build
	@mkdir -p $(dir $@); $(CC) $(CFLAGS) -c $< -o $@
$(KERNEL): $(OBJ) linker/x86_64.ld
	@mkdir -p $(BUILD); $(LD) $(LDFLAGS) -o $@ $(OBJ)
kernel: $(KERNEL)
	sh scripts/check-elf.sh $(KERNEL)
	$(OBJCOPY) --only-keep-debug $(KERNEL) $(BUILD)/novaos.debug
	$(OBJDUMP) -h $(KERNEL) > $(BUILD)/novaos.sections
image: kernel limine.conf limine
	@command -v xorriso >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: xorriso'; echo 'Install it: sudo apt-get install xorriso'; exit 1; }
	@test -x $(LIMINE_DIR)/limine || { echo 'Limine is not built; run: make limine'; exit 1; }
	@mkdir -p $(BUILD)/iso/boot/limine $(BUILD)/iso/EFI/BOOT
	cp $(KERNEL) $(BUILD)/iso/boot/novaos.elf; cp limine.conf $(BUILD)/iso/boot/limine.conf
	cp $(LIMINE_DIR)/limine-bios.sys $(LIMINE_DIR)/limine-bios-cd.bin $(LIMINE_DIR)/limine-uefi-cd.bin $(BUILD)/iso/boot/limine/
	@if [ -f $(LIMINE_DIR)/BOOTX64.EFI ]; then cp $(LIMINE_DIR)/BOOTX64.EFI $(BUILD)/iso/EFI/BOOT/; fi
	xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table --efi-boot boot/limine/limine-uefi-cd.bin -efi-boot-part --efi-boot-image -o $(BUILD)/novaos.iso $(BUILD)/iso
	$(LIMINE_DIR)/limine bios-install $(BUILD)/novaos.iso
run: image
	@command -v $(QEMU) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(QEMU)'; echo 'Install QEMU: sudo apt-get install qemu-system-x86'; exit 1; }
	$(QEMU) -M q35 -m 128M -boot order=d -cdrom $(BUILD)/novaos.iso -serial stdio -display none -no-reboot
debug:
	$(MAKE) DEBUG=1 BUILD=build-debug KERNEL=build-debug/novaos.elf image
	@command -v gdb >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: gdb'; echo 'Install GDB: sudo apt-get install gdb'; exit 1; }
	@command -v $(QEMU) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(QEMU)'; echo 'Install QEMU: sudo apt-get install qemu-system-x86'; exit 1; }
	$(QEMU) -M q35 -m 128M -boot order=d -cdrom build-debug/novaos.iso -serial stdio -display none -no-reboot -S -s
exception-test:
	$(MAKE) limine
	$(MAKE) DEBUG=1 BUILD=build-exception KERNEL=build-exception/novaos.elf CFLAGS="$(CFLAGS) -DNOVAOS_TEST_EXCEPTION" image
	@command -v $(QEMU) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(QEMU)'; echo 'Install QEMU: sudo apt-get install qemu-system-x86'; exit 1; }
	sh scripts/exception-test.sh build-exception/novaos.iso build-exception/serial.log

debug-check:
	$(MAKE) DEBUG=1 BUILD=build-debug KERNEL=build-debug/novaos.elf kernel
	@$(READELF) -S build-debug/novaos.elf | grep -q '\.debug_info'
	@echo 'NovaOS debug configuration: PASS'

test: kernel image
	sh scripts/check-elf.sh $(KERNEL)
	@command -v $(QEMU) >/dev/null 2>&1 || { echo 'NovaOS build dependency missing: $(QEMU)'; echo 'Install QEMU: sudo apt-get install qemu-system-x86'; exit 1; }
	sh scripts/smoke-test.sh $(BUILD)/novaos.iso $(BUILD)/serial.log
clean:
	rm -rf $(BUILD)
