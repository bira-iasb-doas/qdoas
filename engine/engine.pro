#----------------------------------------------
# General Configuration
#----------------------------------------------

TEMPLATE = lib

CONFIG -= qt thread
CONFIG += staticlib


# Platform dependency ...
#----------------------------------------------

#----------------------------------------------
# Engine Source files
#----------------------------------------------
SOURCES += actn-read.c
SOURCES += analyse.c
SOURCES += ascii.c
SOURCES += bin_read.c
SOURCES += ccd-read.c
SOURCES += curfit.c
SOURCES += debug.c
SOURCES += easoeread.c
SOURCES += erf.c
SOURCES += error.c
SOURCES += evalpoly.c
SOURCES += filter.c
SOURCES += fvoigt.c
SOURCES += gdp_asc_read.c
SOURCES += gdp_bin_read.c
SOURCES += kurucz.c
SOURCES += matrix.c
SOURCES += memory.c
SOURCES += mfc-read.c
SOURCES += moon.c
SOURCES += noaa-read.c
SOURCES += opus-read.c
SOURCES += pda-read.c
SOURCES += ras-read.c
SOURCES += read1c_subs.c
SOURCES += ring.c
SOURCES += saoz-read.c
SOURCES += scia-read.c
SOURCES += spline.c
SOURCES += stdfunc.c
SOURCES += svd.c
SOURCES += uoft-read.c
SOURCES += usamp.c
SOURCES += utc_string.c
SOURCES += vector.c
SOURCES += winfiles.c
SOURCES += winsites.c
SOURCES += winthrd.c
SOURCES += wvlen_det_pix.c
SOURCES += xsconv.c
SOURCES += zenithal.c

#----------------------------------------------
# Engine Header files
#----------------------------------------------
HEADERS += bin_read.h
HEADERS += comdefs.h
HEADERS += doas.h
HEADERS += lv1_defs.h
HEADERS += lv1_struct.h
HEADERS += lv1c_struct.h
HEADERS += read1c_defs.h
HEADERS += read1c_struct.h
HEADERS += utc_string.h
HEADERS += windoas.h  