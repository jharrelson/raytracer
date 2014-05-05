CC	= g++

INCLUDE	= -I.

#CFLAGS	= -g -m32 -DDEBUG
CFLAGS	= -g -m32 -fopenmp

LDFLAGS = \
  -L. \
  -L/usr/lib

LDLIBS	= \
  -lvec \
  -list \
  -lc -lm

.cpp.o:
	$(CC) -c $(INCLUDE) $(CFLAGS) $<

OBJS = \
pixel.o \
material.o \
object.o \
plane.o \
sphere.o \
model.o \
camera.o \
light.o \
ray.o \
photon.o \
point.o \
kdtree.o \
timer.o

all: libvec libist main

libvec: libvec.a
libvec.a: vector.o
	ar rcs $@ $?
	ranlib $@

libist: libist.a
libist.a: list.o
	ar rcs $@ $?
	ranlib $@

main: main.cpp main.o $(OBJS) libvec.a libist.a
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $@.o $(OBJS) $(LDFLAGS) $(LDLIBS)

clean:
	rm -f *.o core
	rm -f *.a
	rm -f main
	rm -f *.ps *.pdf

ENSCRIPTFLAGS = \
  --fancy-header=mya2ps \
  --columns=1 \
  --pretty-print=makefile \
  --ul-font=Times-Roman100 \
  --underlay="Timer" \
  --portrait

PS2PDFFLAGS = \
  -dCompatibilityLevel=1.3 \
  -dMaxSubsetPct=100 \
  -dSubsetFonts=true \
  -dEmbedAllFonts=true \
  -dAutoFilterColorImages=false \
  -dAutoFilterGrayImages=false \
  -dColorImageFilter=/FlateEncode \
  -dGrayImageFilter=/FlateEncode \
  -dMonoImageFilter=/FlateEncode

ps:
	enscript $(ENSCRIPTFLAGS) Makefile -p makefile.ps
	enscript $(ENSCRIPTFLAGS) *.h -p interface.ps
	enscript $(ENSCRIPTFLAGS) *.c* -p implementation.ps

pdf:
	ps2pdf $(PS2PDFFLAGS) makefile.ps
	ps2pdf $(PS2PDFFLAGS) interface.ps
	ps2pdf $(PS2PDFFLAGS) implementation.ps

