
#default settings...

opts += -D__USE_INLINE__

opts_inline = ${opts} -D__USE_INLINE__  -Wall

incdir = -I./
incdir += -I../

# Enable or disable patches... 1=enable, 0=disable

# files to compile..

dependent_c_files += init.c 
dependent_c_files += render.c 
dependent_c_files += planes.c 
dependent_c_files += useful_for_tests.c

dependent_files = ${dependent_c_files:.c=.o}

elf_files += tests/copper.c
elf_files += tests/two-bitplanes.c
elf_files += tests/copperbar.c
elf_files += tests/CopperCheck.c
elf_files += tests/CopperSplit.c
elf_files += tests/OSCopper.c

elf_files_elf = ${elf_files:.c=.elf}

all_files +=  ${elf_files_elf} ${dependent_files}

# the process

all: ${all_files} 

%.elf: %.c ${dependent_files} 
		gcc ${incdir} ${libdir} $(opts) ${dependent_files} $< $(libs) -o $@

%.o:	%.c
		gcc ${incdir} ${libdir} $(opts) -c $< $(libs) -o $@

clean:
	delete $(all_files) 

