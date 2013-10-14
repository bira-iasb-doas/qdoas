#ifndef TROPOMI_H
#define TROPOMI_H


#define TROPOMI_BANDS \
  EXPAND(UV1)        \
  EXPAND(UV2)        \
  EXPAND(UVIS3)      \
  EXPAND(UVIS4)      \
  EXPAND(NIR5)       \
  EXPAND(NIR6)       \
  EXPAND(SWIR7)      \
  EXPAND(SWIR8)

enum tropomiSpectralBand {
#define EXPAND(enum) enum,
  TROPOMI_BANDS
#undef EXPAND
};

struct instrumental_tropomi {
  enum tropomiSpectralBand spectralBand;
};

#endif
