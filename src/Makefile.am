#stuff for building a lib file
lib_LTLIBRARIES=libswmm.la

#Libtool flags

#compiler and linker flags
libswmm_la_LIB= -lm -lpthread
libswmm_la_CFLAGS= -fopenmp -lm -lpthread

#start version info at 0:0:0 since this is our first version
#pass release as 5 to match current version number in repository
libswmm_la_LDFLAGS= -release 5 -version-info 0:0:0
libswmm_la_SOURCES=  routing.c  climate.c  controls.c culvert.c     datetime.c dwflow.c   dynwave.c  error.c    \
                     exfil.c    findroot.c flowrout.c forcmain.c    gage.c     gwater.c   hash.c     hotstart.c \
                     iface.c    infil.c    inflow.c   input.c       inputrpt.c keywords.c kinwave.c  landuse.c  \
                     lid.c      lidproc.c  link.c     massbal.c     mathexpr.c mempool.c  node.c     odesolve.c \
                     output.c   project.c  qualrout.c rain.c        rdii.c     report.c   roadway.c  runoff.c   \
                     shape.c    snow.c     stats.c    statsrpt.c    subcatch.c surfqual.c swmm5.c    table.c    \
                     toposort.c transect.c treatmnt.c xsect.c                                                   \
                     consts.h   datetime.h enums.h    error.h       findroot.h funcs.h    globals.h  hash.h     \
                     headers.h  infil.h    keywords.h lid.h         macros.h   mathexpr.h mempool.h  objects.h  \
                     odesolve.h swmm5.h    text.h



#for executable
bin_PROGRAMS=swmm5
swmm5_SOURCES=       routing.c  climate.c  controls.c culvert.c     datetime.c dwflow.c   dynwave.c  error.c    \
                     exfil.c    findroot.c flowrout.c forcmain.c    gage.c     gwater.c   hash.c     hotstart.c \
                     iface.c    infil.c    inflow.c   input.c       inputrpt.c keywords.c kinwave.c  landuse.c  \
                     lid.c      lidproc.c  link.c     massbal.c     mathexpr.c mempool.c  node.c     odesolve.c \
                     output.c   project.c  qualrout.c rain.c        rdii.c     report.c   roadway.c  runoff.c   \
                     shape.c    snow.c     stats.c    statsrpt.c    subcatch.c surfqual.c swmm5.c    table.c    \
                     toposort.c transect.c treatmnt.c xsect.c                                                   \
                     consts.h   datetime.h enums.h    error.h       findroot.h funcs.h    globals.h  hash.h     \
                     headers.h  infil.h    keywords.h lid.h         macros.h   mathexpr.h mempool.h  objects.h  \
                     odesolve.h swmm5.h    text.h

swmm5_LIB=-lm -lpthread
swmm5_CFLAGS= -DCLE -fopenmp -lm -lpthread
