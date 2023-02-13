MKCWD=mkdir -p $(@D)

CC ?= gcc

AS = nasm


CFLAGS_WARNS ?= 	\
		-Werror 	\
		-Wextra 	\
		-Wall 		\
		-Wundef 	\
		-Wshadow 	\
		-Wvla

CFLAGS = 			\
		-Ofast 		\
		-g 		 	\
		-std=c99 	\
		-Isrc/      \
		$(CFLAGS_WARNS)

AFLAGS = 			\
		-f elf64

LDFLAGS=$(SANITIZERS)

# some people likes to use sources/source instead of src
PROJECT_NAME = test
BUILD_DIR = build
SRC_DIR = src

CFILES = $(wildcard $(SRC_DIR)/*.c) $(wildcard $(SRC_DIR)/*/*.c) $(wildcard $(SRC_DIR)/*/*/*.c)
ASFILES = $(wildcard $(SRC_DIR)/*.asm) $(wildcard $(SRC_DIR)/*/*.asm) $(wildcard $(SRC_DIR)/*/*/*.asm)


DFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.d, $(CFILES))
OFILES = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CFILES)) \
			$(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.o, $(ASFILES)) 

OUTPUT = build/$(PROJECT_NAME)


$(OUTPUT): $(OFILES) 
	@$(MKCWD)
	@echo " LD [ $@ ] $<"
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKCWD)
	@echo " CC [ $@ ] $<"
	@$(CC) $(CFLAGS) -MMD -MP $< -c -o $@ 
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@$(MKCWD)
	@echo " AS [ $@ ] $<"
	@$(AS) $(AFLAGS) $< -o $@ 


run: $(OUTPUT)
	@$(OUTPUT)

all: $(OUTPUT)

clean:
	@rm -rf build/

.PHONY: clean all run

-include $(DFILES)
