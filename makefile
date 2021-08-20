
#default settings...

opts += -D__USE_INLINE__

# Enable or disable patches... 1=enable, 0=disable

# files to compile..

files = init.c 

files_o = ${files:.c=.o}


opts_inline = ${opts} -D__USE_INLINE__  -Wall

incdir = -I./
incdir += -I../

all_files = copper ${incdir} ${files_o}

all: ${files_o}
	gcc  $(opts) copper.c ${files_o} -o copper

%.o:	%.c
		gcc -c ${incdir} ${libdir} $(opts) $< $(libs) -o $@

clean:
	delete $(all_files)

