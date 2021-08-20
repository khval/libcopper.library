
#default settings...

opts += -D__USE_INLINE__

opts_inline = ${opts} -D__USE_INLINE__  -Wall

incdir = -I./
incdir += -I../

# Enable or disable patches... 1=enable, 0=disable

# files to compile..

dependent_c_files += init.c 
dependent_c_files += render.c 

dependent_files = ${dependent_c_files:.c=.o}

elf_files = copper.elf

all_files +=  ${elf_files} ${dependent_files}

# the process

all: ${all_files} 

%.elf: %.c ${dependent_files} 
		gcc ${incdir} ${libdir} $(opts) ${dependent_files} $< $(libs) -o $@

%.o:	%.c
		gcc ${incdir} ${libdir} $(opts) -c $< $(libs) -o $@

clean:
	delete $(all_files) 

