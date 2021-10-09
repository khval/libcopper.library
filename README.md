# libcopper.library

This is library intended to be linked into the binary, to emulate the rendering of the Amiga unique graphic chipset.
if you just going to make something that looks like amiga graphics, you do it in where different way, more effetively with modern effects,
but if you’re working on updating old code, or porting old code to new hardware, this is a more true path.

This library was made for AmigaOS4.1 and will need some changes to work on anything else.

About the copper.
---

Copper is display of Amiga, its responsible controlling pixel clock, it draws the images, 
and color and sprites, the Copper is part of what is called AGA/OCS or ECS, chipset.

The copper is simple coprocessor, it has few instructions like MOVE, WAIT and SKIP.

the MOVE is used change hardware registers (values in the graphic chip), WAIT can wait beam positions, 
and SKIP allows you repeat part of the copper list, until a beam position is has been reached.

the copper list can’t be executed all at once, it has to be interpreted as the beam is drawing the screen.

the source image is stored in bitmap (this means etch pixel is a bit), data in bitmap use 16bit alignment, this means 16bit equals 16 pixels as minimum per line, to get more colors you simply layer more bitmaps on top of each other, 1 bit bitmap equals 2 color, this are called bit Plaines. Etch lines starts at bit 15, and you need to get a bit from etch of bit plains, and you need to reserialize the bits, to get color index value (shift bits in correct place and or the bits).  The bit plains do not need to be stored in one big chunk, they can be stored any place in memory, read position can be changed as screen is drawn, the source data can also be left shifted, to move graphics by 1 to 16 pixels, to the left. The bit plains can also be used for parallaxes, combined with sprites, you can create 3-layer parallax.

The Amiga chipset can produce unique effects, and was revolutionary at its time (July 23, 1985).

because old Amiga chipset is so different from what is use today in modern computers, its whery CPU intensive to emulate. 

and because operates its on a bit per bit bases, it’s not wherry GPU/shader friendly, 
but if some knows how to convert into shader it can have huge impact on Amiga Emulation.
