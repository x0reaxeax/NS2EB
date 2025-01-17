ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)

# Add ProbeRAM.o to the list of objects
OBJS            = efi.o CPU.o MSR.o x86intrin.o stdfuncs.o Disassembler.o Screen.o Zydis.o
TARGET          = efi.efi

EFIINC          = /usr/include/efi
EFIINCS         = -I$(EFIINC) -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
LIB32           = /usr/lib32
LIB64           = /usr/lib

CFLAGS          = -I./ $(EFIINCS) -fno-stack-protector -fpic \
		  -fshort-wchar -mno-red-zone -Wall -Wno-unused-const-variable -ggdb3

ifeq ($(ARCH),x86_64)
  CFLAGS += -DEFI_FUNCTION_WRAPPER -DGNU_EFI_USE_MS_ABI
  LIB           = $(LIB64)
  EFILIB        = $(LIB64)
endif

ifeq ($(ARCH),ia32)
  LIB           = $(LIB32)
  EFILIB        = $(LIB32)
endif

EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi-stable.lds

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS) 

# Rule to assemble the NASM file
%.o: %.asm
	nasm -f elf64 -o $@ $<

all: $(TARGET)

efi.so: $(OBJS)
	ld $(LDFLAGS) $(OBJS)  -o $@ -lefi -lgnuefi

%.efi: %.so
	objcopy --only-keep-debug $^ $@.debug
	objcopy --strip-all -j .text -j .sdata -j .data -j .dynamic \
		-j .dynsym  -j .rel -j .rela -j .reloc \
		--target=efi-app-$(ARCH) $^ $@

clean:
	rm -f efi.efi efi.so *.o efi.efi.debug $(OBJS)
