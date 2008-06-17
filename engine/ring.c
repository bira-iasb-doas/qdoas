
//  ----------------------------------------------------------------------------
//
//  Product/Project   :  QDOAS
//  Module purpose    :  RING EFFECT
//  Name of module    :  RING.C
//  Creation date     :  1998
//
//  QDOAS is a cross-platform application developed in QT for DOAS retrieval
//  (Differential Optical Absorption Spectroscopy).
//
//  The QT version of the program has been developed jointly by the Belgian
//  Institute for Space Aeronomy (BIRA-IASB) and the Science and Technology
//  company (S[&]T) - Copyright (C) 2007
//
//      BIRA-IASB                                   S[&]T
//      Belgian Institute for Space Aeronomy        Science [&] Technology
//      Avenue Circulaire, 3                        Postbus 608
//      1180     UCCLE                              2600 AP Delft
//      BELGIUM                                     THE NETHERLANDS
//      caroline.fayt@aeronomie.be                  info@stcorp.nl
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
//  ----------------------------------------------------------------------------
//  FUNCTIONS
//
//  ================
//  RING CALCULATION
//  ================
//
//  raman_n2 - N2 Raman function
//  raman_o2 - O2 Raman function
//  RingHeader - write options in the file header
//  Ring - build a ring cross section
//
//  =====================================
//  RING EFFECT PANEL MESSAGES PROCESSING
//  =====================================
//
//  RingTypeMolecular - show/hide fields to create a molecular ring XS (obsolete function)
//  RingDlgInit - WM_INIT message processing;
//  RingOK - OK command processing;
//  RingCommand - WM_COMMAND message processing;
//
//  RING_WndProc - Dispatch messages from ring effect tool panel;
//
//  =======================
//  PROGRESS BAR PROCESSING
//  =======================
//
//  RingProgressInit - WM_INITDIALOG message processing;
//  RingProgressUser - WM_USER message processing (update progress bar on user message);
//  RING_ProgressWndProc - dispatch messages from progress bar panel;
//
//  =============================
//  CONFIGURATION FILE MANAGEMENT
//  =============================
//
//  RING_ResetConfiguration - reset Ring options
//  RING_LoadConfiguration - load data and input files needed for building a ring cross section;
//  RING_SaveConfiguration - save data needed for building a ring effect cross section;
//  ----------------------------------------------------------------------------
//  REMARK
//
//  This program has been traducted from Kelly Chance's Fortran 77 program, April 1997
//
//  Reference :
//
//  Ring effect studies : Rayleigh scattering, including molecular parameters for rotational
//  Ralan scattering and the Franhofer spectrum, K.Chance and R.J.D.Spurr, Applied optics (in press)
//  ----------------------------------------------------------------------------

#include "doas.h"

// =====================
// CONSTANTS DEFINITIONS
// =====================

#define RING_SECTION "Ring"
#define RING_SLIT_WIDTH (double)6.
#define RING_TEMPERATURE 250                                                    // default temperature (in the original code)

// ===================
// STATIC DECLARATIONS
// ===================

XSCONV RING_options,RING_buffer;
INT    RING_progress;
HANDLE RING_event;

// =================
// N2 RAMAN FUNCTION
// =================

// Statistical parameters, for partition function calculation

#define N2_SIZE  48    // size of n2 data
#define N2_STAT  31    // size of n2 statistical data

static double n2stat_1[N2_STAT]=
 {
  (double)  0., (double)  1., (double)  2., (double)  3., (double)  4., (double)  5.,
  (double)  6., (double)  7., (double)  8., (double)  9., (double) 10., (double) 11.,
  (double) 12., (double) 13., (double) 14., (double) 15., (double) 16., (double) 17.,
  (double) 18., (double) 19., (double) 20., (double) 21., (double) 22., (double) 23.,
  (double) 24., (double) 25., (double) 26., (double) 27., (double) 28., (double) 29.,
  (double) 30.
 };

static double n2stat_2[N2_STAT]=
 {
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6.
 };

static double n2stat_3[N2_STAT]=
 {
  (double)    0.0000, (double)    3.9791, (double)   11.9373, (double)   23.8741, (double)   39.7892,
  (double)   59.6821, (double)   83.5521, (double)  111.3983, (double)  143.2197, (double)  179.0154,
  (double)  218.7839, (double)  262.5240, (double)  310.2341, (double)  361.9126, (double)  417.5576,
  (double)  477.1673, (double)  540.7395, (double)  608.2722, (double)  679.7628, (double)  755.2090,
  (double)  834.6081, (double)  917.9574, (double) 1005.2540, (double) 1096.4948, (double) 1191.6766,
  (double) 1290.7963, (double) 1393.8503, (double) 1500.8350, (double) 1611.7467, (double) 1726.5816,
  (double) 1845.3358
 };

// Rotation Raman line-specific parameters

static double n2term[N2_SIZE]=
 {
  (double) 1290.7963, (double) 1191.6766, (double) 1096.4948, (double) 1005.2540, (double) 917.9574,
  (double)  834.6081, (double)  755.2090, (double)  679.7628, (double)  608.2722, (double) 540.7395,
  (double)  477.1673, (double)  417.5576, (double)  361.9126, (double)  310.2341, (double) 262.5240,
  (double)  218.7839, (double)  179.0154, (double)  143.2197, (double)  111.3983, (double)  83.5521,
  (double)   59.6821, (double)   39.7892, (double)   23.8741, (double)   11.9373, (double)   0.0000,
  (double)    3.9791, (double)   11.9373, (double)   23.8741, (double)   39.7892, (double)  59.6821,
  (double)   83.5521, (double)  111.3983, (double)  143.2197, (double)  179.0154, (double) 218.7839,
  (double)  262.5240, (double)  310.2341, (double)  361.9126, (double)  417.5576, (double) 477.1673,
  (double)  540.7395, (double)  608.2722, (double)  679.7628, (double)  755.2090, (double) 834.6081,
  (double)  917.9574, (double) 1005.2540, (double) 1096.4948
 };

static double n2plactel[N2_SIZE]=
 {
  (double) 3.601e-1, (double) 3.595e-1, (double) 3.589e-1, (double) 3.581e-1, (double) 3.573e-1,
  (double) 3.565e-1, (double) 3.555e-1, (double) 3.544e-1, (double) 3.532e-1, (double) 3.519e-1,
  (double) 3.504e-1, (double) 3.487e-1, (double) 3.467e-1, (double) 3.443e-1, (double) 3.416e-1,
  (double) 3.383e-1, (double) 3.344e-1, (double) 3.294e-1, (double) 3.231e-1, (double) 3.147e-1,
  (double) 3.030e-1, (double) 2.857e-1, (double) 2.571e-1, (double) 2.000e-1, (double) 1.000e+0,
  (double) 6.000e-1, (double) 5.143e-1, (double) 4.762e-1, (double) 4.545e-1, (double) 4.406e-1,
  (double) 4.308e-1, (double) 4.235e-1, (double) 4.180e-1, (double) 4.135e-1, (double) 4.099e-1,
  (double) 4.070e-1, (double) 4.044e-1, (double) 4.023e-1, (double) 4.004e-1, (double) 3.988e-1,
  (double) 3.974e-1, (double) 3.961e-1, (double) 3.950e-1, (double) 3.940e-1, (double) 3.931e-1,
  (double) 3.922e-1, (double) 3.915e-1, (double) 3.908e-1
 };

static double n2deg[N2_SIZE]=
 {
  (double) 25., (double) 24., (double) 23., (double) 22., (double) 21.,
  (double) 20., (double) 19., (double) 18., (double) 17., (double) 16.,
  (double) 15., (double) 14., (double) 13., (double) 12., (double) 11.,
  (double) 10., (double)  9., (double)  8., (double)  7., (double)  6.,
  (double)  5., (double)  4., (double)  3., (double)  2., (double)  0.,
  (double)  1., (double)  2., (double)  3., (double)  4., (double)  5.,
  (double)  6., (double)  7., (double)  8., (double)  9., (double) 10.,
  (double) 11., (double) 12., (double) 13., (double) 14., (double) 15.,
  (double) 16., (double) 17., (double) 18., (double) 19., (double) 20.,
  (double) 21., (double) 22., (double) 23.
 };

static double n2nuc[N2_SIZE]=
 {
  (double) 3., (double) 6., (double) 3., (double) 6., (double) 3., (double) 6.,
  (double) 3., (double) 6., (double) 3., (double) 6., (double) 3., (double) 6.,
  (double) 3., (double) 6., (double) 3., (double) 6., (double) 3., (double) 6.,
  (double) 3., (double) 6., (double) 3., (double) 6., (double) 3., (double) 6.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.,
  (double) 6., (double) 3., (double) 6., (double) 3., (double) 6., (double) 3.
 };

// -----------------------------------------------------------------------------
// FUNCTION      raman_n2
// -----------------------------------------------------------------------------
// PURPOSE       N2 Raman function
//
// INPUT         temp : temperature
//
// OUTPUT        N2 cross section
// -----------------------------------------------------------------------------

void raman_n2(double temp,double *n2xsec)
 {
  // Declarations and initializations

  double pi,c2,emult,qn2,prefix,n2frac;
  int i;

  pi = (double) 3.14159265358979;
  c2 = (double) 1.438769;
  prefix = (double) 256. * pow(pi,(double)5.) * 0.79 / 27.;

  emult = - c2 / temp;

  // Calculate partition function

  for (qn2=(double)0.,i=0;i<N2_STAT;i++)
   qn2+=(n2stat_1[i]*2.+1.)*n2stat_2[i]*exp(emult*n2stat_3[i]);

  // Calculate population fractions for rotational Raman lines and
  // the cross sections (in cm2 * 1.e48), except for gamma**2/lambda**4

  for (i=0;i<N2_SIZE;i++)
   {
    n2frac=(n2deg[i]*2.+1.)*n2nuc[i]*exp(emult*n2term[i])/qn2;
    n2xsec[i]=prefix*n2frac*n2plactel[i];
   }
 }

// =================
// O2 RAMAN FUNCTION
// =================

// Statistical parameters, for partition function calculation

#define O2_SIZE  185    // size of o2 data
#define O2_STAT   54    // size of o2 statistical data

static double o2stat_1[O2_STAT]=
 {
  (double)  0., (double)  2., (double)  1., (double)  2., (double)  4., (double)  3., (double)  4.,
  (double)  6., (double)  5., (double)  8., (double)  6., (double)  7., (double) 10., (double)  8.,
  (double)  9., (double) 12., (double) 10., (double) 11., (double) 14., (double) 12., (double) 13.,
  (double) 16., (double) 14., (double) 15., (double) 18., (double) 16., (double) 17., (double) 20.,
  (double) 18., (double) 19., (double) 22., (double) 20., (double) 21., (double) 24., (double) 22.,
  (double) 23., (double) 26., (double) 24., (double) 25., (double) 28., (double) 26., (double) 27.,
  (double) 30., (double) 28., (double) 29., (double) 32., (double) 30., (double) 31., (double) 34.,
  (double) 32., (double) 33., (double) 36., (double) 34., (double) 35.
 };

static double o2stat_2[O2_STAT]=
 {
  (double)    0.0000, (double)    2.0843, (double)    3.9611, (double)   16.2529, (double)   16.3876, (double)  18.3372,
  (double)   42.2001, (double)   42.2240, (double)   44.2117, (double)   79.5646, (double)   79.6070, (double)  81.5805,
  (double)  128.3978, (double)  128.4921, (double)  130.4376, (double)  188.7135, (double)  188.8532, (double) 190.7749,
  (double)  260.5011, (double)  260.6826, (double)  262.5829, (double)  343.7484, (double)  343.9697, (double) 345.8500,
  (double)  438.4418, (double)  438.7015, (double)  440.5620, (double)  544.5658, (double)  544.8628, (double) 546.7050,
  (double)  662.1030, (double)  662.4368, (double)  664.2610, (double)  791.0344, (double)  791.4045, (double) 793.2100,
  (double)  931.3390, (double)  931.7450, (double)  933.5330, (double) 1082.9941, (double) 1083.4356,
  (double) 1085.2060, (double) 1245.9750, (double) 1246.4518, (double) 1248.2040, (double) 1420.2552,
  (double) 1420.7672, (double) 1422.5020, (double) 1605.8064, (double) 1606.3533, (double) 1608.0710,
  (double) 1802.5983, (double) 1803.1802, (double) 1804.8810
 };

// Rotation Raman line-specific parameters

static double o2term[O2_SIZE]=
 {
  (double) 1606.3533, (double) 1608.0710, (double) 1605.8064, (double) 1420.7672, (double) 1422.5020,
  (double) 1420.2552, (double) 1246.4518, (double) 1248.2040, (double) 1245.9750, (double) 1083.4356,
  (double) 1085.2060, (double) 1082.9941, (double)  931.7450, (double)  933.5330, (double)  931.3390,
  (double)  791.4045, (double)  793.2100, (double)  791.0344, (double)  662.4368, (double)  664.2610,
  (double)  662.1030, (double)  546.7050, (double)  544.8628, (double)  546.7050, (double)  544.5658,
  (double)  544.8628, (double)  440.5620, (double)  438.7015, (double)  440.5620, (double)  438.4418,
  (double)  438.7015, (double)  345.8500, (double)  343.9697, (double)  345.8500, (double)  343.7484,
  (double)  343.9697, (double)  262.5829, (double)  260.6826, (double)  262.5829, (double)  260.5011,
  (double)  260.6826, (double)  190.7749, (double)  188.8532, (double)  190.7749, (double)  188.7135,
  (double)  188.8532, (double)  130.4376, (double)  128.4921, (double)  130.4376, (double)  128.3978,
  (double)  128.4921, (double)   42.2001, (double)   81.5805, (double)   79.6070, (double)   81.5805,
  (double)   79.5646, (double)   79.6070, (double)   44.2117, (double)   42.2001, (double)   44.2117,
  (double)   42.2240, (double)   42.2001, (double)   42.2001, (double)   16.2529, (double)   18.3372,
  (double)   18.3372, (double)   16.3876, (double)   16.2529, (double)   16.2529, (double)  546.7050,
  (double)  440.5620, (double)  345.8500, (double)    2.0843, (double)   18.3372, (double)  262.5829,
  (double)  190.7749, (double)  130.4376, (double)   81.5805, (double)   44.2117, (double)   44.2117,
  (double)   81.5805, (double)   18.3372, (double)  130.4376, (double)  190.7749, (double)  262.5829,
  (double)  345.8500, (double)    3.9611, (double)  440.5620, (double)  546.7050, (double)   16.3876,
  (double)   16.2529, (double)  544.8628, (double)  438.7015, (double)    2.0843, (double)  343.9697,
  (double)  260.6826, (double)  188.8532, (double)  128.4921, (double)   16.3876, (double)   79.6070,
  (double)   42.2240, (double)   42.2001, (double)   79.5646, (double)  128.3978, (double)  188.7135,
  (double)  260.5011, (double)    0.0000, (double)   16.2529, (double)  343.7484, (double)  438.4418,
  (double)  544.5658, (double)    3.9611, (double)    2.0843, (double)    2.0843, (double)    3.9611,
  (double)    0.0000, (double)    2.0843, (double)   18.3372, (double)   16.3876, (double)   16.3876,
  (double)   18.3372, (double)   16.2529, (double)   16.3876, (double)   44.2117, (double)   42.2240,
  (double)   44.2117, (double)   42.2001, (double)   42.2240, (double)    2.0843, (double)   81.5805,
  (double)   79.5646, (double)   81.5805, (double)   79.6070, (double)   79.5646, (double)  130.4376,
  (double)  128.3978, (double)  130.4376, (double)  128.4921, (double)  128.3978, (double)  190.7749,
  (double)  188.7135, (double)  190.7749, (double)  188.8532, (double)  188.7135, (double)  262.5829,
  (double)  260.5011, (double)  262.5829, (double)  260.6826, (double)  260.5011, (double)  345.8500,
  (double)  343.7484, (double)  345.8500, (double)  343.9697, (double)  343.7484, (double)  440.5620,
  (double)  438.4418, (double)  440.5620, (double)  438.7015, (double)  438.4418, (double)  546.7050,
  (double)  544.5658, (double)  546.7050, (double)  544.8628, (double)  544.5658, (double)  662.1030,
  (double)  664.2610, (double)  662.4368, (double)  791.0344, (double)  793.2100, (double)  791.4045,
  (double)  931.3390, (double)  933.5330, (double)  931.7450, (double) 1082.9941, (double) 1085.2060,
  (double) 1083.4356, (double) 1245.9750, (double) 1248.2040, (double) 1246.4518, (double) 1420.2552,
  (double) 1422.5020, (double) 1420.7672, (double) 1605.8064, (double) 1608.0710, (double) 1606.3533
 };

static double o2plactel[O2_SIZE]=
 {
  (double) 3.630e-1, (double) 3.630e-1, (double) 3.637e-1, (double) 3.622e-1, (double) 3.622e-1,
  (double) 3.630e-1, (double) 3.613e-1, (double) 3.613e-1, (double) 3.622e-1, (double) 3.602e-1,
  (double) 3.602e-1, (double) 3.612e-1, (double) 3.589e-1, (double) 3.589e-1, (double) 3.601e-1,
  (double) 3.574e-1, (double) 3.574e-1, (double) 3.589e-1, (double) 3.556e-1, (double) 3.556e-1,
  (double) 3.573e-1, (double) 2.079e-3, (double) 3.533e-1, (double) 3.534e-1, (double) 3.555e-1,
  (double) 2.191e-3, (double) 2.597e-3, (double) 3.505e-1, (double) 3.506e-1, (double) 3.532e-1,
  (double) 2.755e-3, (double) 3.337e-3, (double) 3.468e-1, (double) 3.471e-1, (double) 3.504e-1,
  (double) 3.567e-3, (double) 4.444e-3, (double) 3.418e-1, (double) 3.422e-1, (double) 3.467e-1,
  (double) 4.800e-3, (double) 6.211e-3, (double) 3.348e-1, (double) 3.354e-1, (double) 3.416e-1,
  (double) 6.803e-3, (double) 9.288e-3, (double) 3.220e-1, (double) 3.251e-1, (double) 3.344e-1,
  (double) 1.127e-2, (double) 1.062e-3, (double) 1.387e-2, (double) 3.013e-1, (double) 3.077e-1,
  (double) 3.223e-1, (double) 1.979e-2, (double) 2.613e-2, (double) 2.544e-1, (double) 2.727e-1,
  (double) 3.020e-1, (double) 1.476e-3, (double) 4.342e-2, (double) 9.234e-2, (double) 6.596e-2,
  (double) 1.714e-1, (double) 2.571e-1, (double) 1.843e-2, (double) 1.615e-1, (double) 1.970e-3,
  (double) 2.445e-3, (double) 3.116e-3, (double) 1.077e-1, (double) 7.690e-2, (double) 4.105e-3,
  (double) 5.652e-3, (double) 8.271e-3, (double) 1.222e-2, (double) 2.842e-2, (double) 2.207e-2,
  (double) 1.470e-2, (double) 5.132e-2, (double) 8.256e-3, (double) 5.647e-3, (double) 4.103e-3,
  (double) 3.115e-3, (double) 2.308e-1, (double) 2.445e-3, (double) 1.970e-3, (double) 2.116e-3,
  (double) 3.810e-3, (double) 2.076e-3, (double) 2.593e-3, (double) 1.385e-1, (double) 3.329e-3,
  (double) 4.431e-3, (double) 6.184e-3, (double) 9.227e-3, (double) 3.991e-2, (double) 1.696e-2,
  (double) 1.867e-2, (double) 3.474e-2, (double) 1.078e-2, (double) 7.483e-3, (double) 5.200e-3,
  (double) 3.822e-3, (double) 5.383e-1, (double) 1.077e-1, (double) 2.927e-3, (double) 2.313e-3,
  (double) 1.874e-3, (double) 2.692e-1, (double) 1.843e-2, (double) 4.628e-1, (double) 4.000e-1,
  (double) 4.617e-1, (double) 9.234e-2, (double) 5.583e-2, (double) 1.476e-3, (double) 4.362e-1,
  (double) 4.286e-1, (double) 4.579e-1, (double) 3.193e-2, (double) 2.339e-2, (double) 4.214e-1,
  (double) 4.196e-1, (double) 4.352e-1, (double) 1.600e-2, (double) 1.911e-3, (double) 1.278e-2,
  (double) 4.130e-1, (double) 4.118e-1, (double) 4.210e-1, (double) 1.038e-2, (double) 7.519e-3,
  (double) 4.067e-1, (double) 4.060e-1, (double) 4.135e-1, (double) 6.803e-3, (double) 5.217e-3,
  (double) 4.021e-1, (double) 4.017e-1, (double) 4.070e-1, (double) 4.800e-3, (double) 3.831e-3,
  (double) 3.987e-1, (double) 3.985e-1, (double) 4.023e-1, (double) 3.567e-3, (double) 2.933e-3,
  (double) 3.961e-1, (double) 3.959e-1, (double) 3.988e-1, (double) 2.755e-3, (double) 2.317e-3,
  (double) 3.939e-1, (double) 3.938e-1, (double) 3.961e-1, (double) 2.191e-3, (double) 1.876e-3,
  (double) 3.922e-1, (double) 3.921e-1, (double) 3.940e-1, (double) 1.785e-3, (double) 3.908e-1,
  (double) 3.907e-1, (double) 3.922e-1, (double) 3.895e-1, (double) 3.895e-1, (double) 3.908e-1,
  (double) 3.885e-1, (double) 3.885e-1, (double) 3.896e-1, (double) 3.876e-1, (double) 3.876e-1,
  (double) 3.885e-1, (double) 3.868e-1, (double) 3.868e-1, (double) 3.876e-1, (double) 3.861e-1,
  (double) 3.861e-1, (double) 3.868e-1, (double) 3.855e-1, (double) 3.855e-1, (double) 3.861e-1
 };

static double o2deg[O2_SIZE]=
 {
  (double) 32., (double) 33., (double) 34., (double) 30., (double) 31., (double) 32., (double) 28.,
  (double) 29., (double) 30., (double) 26., (double) 27., (double) 28., (double) 24., (double) 25.,
  (double) 26., (double) 22., (double) 23., (double) 24., (double) 20., (double) 21., (double) 22.,
  (double) 19., (double) 18., (double) 19., (double) 20., (double) 18., (double) 17., (double) 16.,
  (double) 17., (double) 18., (double) 16., (double) 15., (double) 14., (double) 15., (double) 16.,
  (double) 14., (double) 13., (double) 12., (double) 13., (double) 14., (double) 12., (double) 11.,
  (double) 10., (double) 11., (double) 12., (double) 10., (double)  9., (double)  8., (double)  9.,
  (double) 10., (double)  8., (double)  4., (double)  7., (double)  6., (double)  7., (double)  8.,
  (double)  6., (double)  5., (double)  4., (double)  5., (double)  6., (double)  4., (double)  4.,
  (double)  2., (double)  3., (double)  3., (double)  4., (double)  2., (double)  2., (double) 19.,
  (double) 17., (double) 15., (double)  2., (double)  3., (double) 13., (double) 11., (double)  9.,
  (double)  7., (double)  5., (double)  5., (double)  7., (double)  3., (double)  9., (double) 11.,
  (double) 13., (double) 15., (double)  1., (double) 17., (double) 19., (double)  4., (double)  2.,
  (double) 18., (double) 16., (double)  2., (double) 14., (double) 12., (double) 10., (double)  8.,
  (double)  4., (double)  6., (double)  6., (double)  4., (double)  8., (double) 10., (double) 12.,
  (double) 14., (double)  0., (double)  2., (double) 16., (double) 18., (double) 20., (double)  1.,
  (double)  2., (double)  2., (double)  1., (double)  0., (double)  2., (double)  3., (double)  4.,
  (double)  4., (double)  3., (double)  2., (double)  4., (double)  5., (double)  6., (double)  5.,
  (double)  4., (double)  6., (double)  2., (double)  7., (double)  8., (double)  7., (double)  6.,
  (double)  8., (double)  9., (double) 10., (double)  9., (double)  8., (double) 10., (double) 11.,
  (double) 12., (double) 11., (double) 10., (double) 12., (double) 13., (double) 14., (double) 13.,
  (double) 12., (double) 14., (double) 15., (double) 16., (double) 15., (double) 14., (double) 16.,
  (double) 17., (double) 18., (double) 17., (double) 16., (double) 18., (double) 19., (double) 20.,
  (double) 19., (double) 18., (double) 20., (double) 22., (double) 21., (double) 20., (double) 24.,
  (double) 23., (double) 22., (double) 26., (double) 25., (double) 24., (double) 28., (double) 27.,
  (double) 26., (double) 30., (double) 29., (double) 28., (double) 32., (double) 31., (double) 30.,
  (double) 34., (double) 33., (double) 32.
 };

// -----------------------------------------------------------------------------
// FUNCTION      raman_o2
// -----------------------------------------------------------------------------
// PURPOSE       O2 Raman function
//
// INPUT         temp : temperature
//
// OUTPUT        N2 cross section
// -----------------------------------------------------------------------------

void raman_o2(double temp,double *o2xsec)
 {
  // Declarations and initializations

  double pi,c2,emult,qo2,prefix,o2frac;
  int i;

  pi = (double) 3.14159265358979;
  c2 = (double) 1.438769;
  prefix = (double) 256. * pow(pi,(double)5.) * 0.21 / 27.;

  emult = -c2 / temp;

  // Calculate partition function

  for (qo2=(double)0.,i=0;i<O2_STAT;i++)
   qo2+=(double)(o2stat_1[i]*2.+1.)*exp(emult*o2stat_2[i]);

  // Calculate population fractions for rotational Raman lines and
  // the cross sections (in cm2 * 1.e48), except for gamma**2/lambda**4

  for (i=0;i<O2_SIZE;i++)
   {
    o2frac=(o2deg[i]*2.+1.)*exp(emult*o2term[i])/qo2;
    o2xsec[i]=prefix*o2frac*o2plactel[i];
   }
 }

// =============
// MAIN FUNCTION
// =============

static double n2pos[N2_SIZE]=
 {
  (double) -194.3015, (double) -186.4226, (double) -178.5374, (double) -170.6459, (double) -162.7484,
  (double) -154.8453, (double) -146.9368, (double) -139.0233, (double) -131.1049, (double) -123.1819,
  (double) -115.2547, (double) -107.3235, (double)  -99.3886, (double)  -91.4502, (double) -83.5086,
  (double)  -75.5642, (double)  -67.6171, (double)  -59.6676, (double)  -51.7162, (double) -43.7629,
  (double)  -35.8080, (double)  -27.8519, (double)  -19.8950, (double)  -11.9373, (double)  11.9373,
  (double)   19.8950, (double)   27.8519, (double)   35.8080, (double)   43.7629, (double)  51.7162,
  (double)   59.6676, (double)   67.6171, (double)   75.5642, (double)   83.5086, (double)  91.4502,
  (double)   99.3886, (double)  107.3235, (double)  115.2547, (double)  123.1819, (double) 131.1049,
  (double)  139.0233, (double)  146.9368, (double)  154.8453, (double)  162.7484, (double) 170.6459,
  (double)  178.5374, (double)  186.4226, (double)  194.3015
 };

static double o2pos[O2_SIZE]=
 {
  (double) -185.5861, (double) -185.5690, (double) -185.5512, (double) -174.3154, (double) -174.2980,
  (double) -174.2802, (double) -163.0162, (double) -162.9980, (double) -162.9809, (double) -151.6906,
  (double) -151.6730, (double) -151.6551, (double) -140.3405, (double) -140.3230, (double) -140.3046,
  (double) -128.9677, (double) -128.9490, (double) -128.9314, (double) -117.5740, (double) -117.5560,
  (double) -117.5372, (double) -108.2632, (double) -106.1613, (double) -106.1430, (double) -106.1240,
  (double) -104.3008, (double)  -96.8136, (double)  -94.7318, (double)  -94.7120, (double)  -94.6934,
  (double)  -92.8515, (double)  -85.3489, (double)  -83.2871, (double)  -83.2671, (double)  -83.2473,
  (double)  -81.3868, (double)  -73.8694, (double)  -71.8294, (double)  -71.8080, (double)  -71.7876,
  (double)  -69.9077, (double)  -62.3771, (double)  -60.3611, (double)  -60.3373, (double)  -60.3157,
  (double)  -58.4156, (double)  -50.8730, (double)  -48.8851, (double)  -48.8571, (double)  -48.8332,
  (double)  -46.9116, (double)  -40.1158, (double)  -39.3565, (double)  -37.4069, (double)  -37.3688,
  (double)  -37.3406, (double)  -35.3953, (double)  -27.8241, (double)  -25.9472, (double)  -25.8745,
  (double)  -25.8364, (double)  -25.8125, (double)  -23.8629, (double)  -16.2529, (double)  -16.2529,
  (double)  -14.3761, (double)  -14.3033, (double)  -14.1686, (double)  -12.2918, (double)  -2.1392,
  (double)   -2.1202, (double)   -2.1016, (double)   -2.0843, (double)   -2.0843, (double)   -2.0818,
  (double)   -2.0614, (double)   -2.0398, (double)   -2.0159, (double)   -2.0116, (double)   -1.9877,
  (double)   -1.9735, (double)   -1.9496, (double)   -1.9455, (double)   -1.9217, (double)   -1.9003,
  (double)   -1.8803, (double)   -1.8768, (double)   -1.8605, (double)   -1.8422, (double)   -0.1347,
  (double)    0.1347, (double)    1.8422, (double)    1.8605, (double)    1.8768, (double)    1.8803,
  (double)    1.9003, (double)    1.9217, (double)    1.9455, (double)    1.9496, (double)    1.9735,
  (double)    1.9877, (double)    2.0116, (double)    2.0159, (double)    2.0398, (double)    2.0614,
  (double)    2.0818, (double)    2.0843, (double)    2.0843, (double)    2.1016, (double)    2.1202,
  (double)    2.1392, (double)   12.2918, (double)   14.1686, (double)   14.3033, (double)   14.3761,
  (double)   16.2529, (double)   16.2529, (double)   23.8629, (double)   25.8125, (double)   25.8364,
  (double)   25.8745, (double)   25.9472, (double)   27.8241, (double)   35.3953, (double)   37.3406,
  (double)   37.3688, (double)   37.4069, (double)   39.3565, (double)   40.1158, (double)   46.9116,
  (double)   48.8332, (double)   48.8571, (double)   48.8851, (double)   50.8730, (double)   58.4156,
  (double)   60.3157, (double)   60.3373, (double)   60.3611, (double)   62.3771, (double)   69.9077,
  (double)   71.7876, (double)   71.8080, (double)   71.8294, (double)   73.8694, (double)   81.3868,
  (double)   83.2473, (double)   83.2671, (double)   83.2871, (double)   85.3489, (double)   92.8515,
  (double)   94.6934, (double)   94.7120, (double)   94.7318, (double)   96.8136, (double)   104.3008,
  (double)  106.1240, (double)  106.1430, (double)  106.1613, (double)  108.2632, (double)  115.7318,
  (double)  117.5372, (double)  117.5560, (double)  117.5740, (double)  119.6952, (double)  128.9314,
  (double)  128.9490, (double)  128.9677, (double)  140.3046, (double)  140.3230, (double)  140.3405,
  (double)  151.6551, (double)  151.6730, (double)  151.6906, (double)  162.9809, (double)  162.9980,
  (double)  163.0162, (double)  174.2802, (double)  174.2980, (double)  174.3154, (double)  185.5512,
  (double)  185.5690, (double)  185.5861, (double)  196.7919, (double)  196.8100, (double)  196.8269
 };

// -----------------------------------------------------------------------------
// FUNCTION      RingHeader
// -----------------------------------------------------------------------------
// PURPOSE       Write options in the file header
//
// INPUT         fp    : pointer to the output file
//               order : order of the ring cross section (obsolete)
// -----------------------------------------------------------------------------

#if defined(__BC32_) && __BC32_
#pragma argsused
#endif
void RingHeader(FILE *fp,int order)
 {
  // Declaration

  UCHAR   fileName[MAX_ITEM_TEXT_LEN+1];
  INT     slitType;                                                 // type of the slit function

  // Header

  fprintf(fp,";\n");
  fprintf(fp,"; RING CROSS SECTION\n");
  fprintf(fp,"; Temperature : %d °K\n",RING_options.temperature);
  fprintf(fp,"; High resolution Kurucz file : %s\n",FILES_RebuildFileName(fileName,RING_options.kuruczFile,1));
  fprintf(fp,"; Calibration file : %s\n",FILES_RebuildFileName(fileName,RING_options.calibrationFile,1));

  if (strlen(RING_options.crossFile))
   fprintf(fp,"; Cross section file : %s\n",FILES_RebuildFileName(fileName,RING_options.crossFile,1));

  fprintf(fp,"; Slit function type : %s\n",XSCONV_slitTypes[(slitType=RING_options.slitConv.slitType)]);

  if ((slitType==SLIT_TYPE_FILE) || (slitType==SLIT_TYPE_GAUSS_FILE) || (slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE))
   fprintf(fp,"; Slit function file : %s\n",FILES_RebuildFileName(fileName,RING_options.slitConv.slitFile,1));
  if ((slitType==SLIT_TYPE_GAUSS) || (slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_ERF))
   fprintf(fp,"; Gaussian FWHM : %lf\n",RING_options.slitConv.slitParam);
  if ((slitType==SLIT_TYPE_INVPOLY) || (slitType==SLIT_TYPE_INVPOLY_FILE))
   fprintf(fp,"; Polynomial degree : %d\n",(int)RING_options.slitConv.slitParam2);
  if ((slitType==SLIT_TYPE_ERF) || (slitType==SLIT_TYPE_ERF_FILE))
   fprintf(fp,"; Boxcar width : %lf\n",RING_options.slitConv.slitParam2);

  if (slitType==SLIT_TYPE_VOIGT)
   {
    fprintf(fp,"; Gaussian FWHM (L) : %.3f\n",RING_options.slitConv.slitParam);
    fprintf(fp,"; Gaussian/Lorentz ratio (L) : %.3f\n",RING_options.slitConv.slitParam2);
    fprintf(fp,"; Gaussian FWHM (R) : %.3f\n",RING_options.slitConv.slitParam3);
    fprintf(fp,"; Gaussian/Lorentz ratio (R) : %.3f\n",RING_options.slitConv.slitParam4);
   }

  fprintf(fp,"; column 1 : wavelength\n");
  fprintf(fp,"; column 2 : ring (raman/solar spectrum)\n");
  fprintf(fp,"; column 3 : interpolated raman\n");
  fprintf(fp,"; column 4 : convoluted solar spectrum\n");

  fprintf(fp,";\n");
 }

// -----------------------------------------------------------------------------
// FUNCTION      Ring
// -----------------------------------------------------------------------------
// PURPOSE       Build a ring cross section
//
// INPUT         ringArg : handle of the Ring dialog box
// -----------------------------------------------------------------------------

void Ring(void *ringArg)
 {
  // Declarations

  UCHAR   ringFileName[MAX_ITEM_TEXT_LEN+1];                                    // name of the output ring file
  double *n2xref,*o2xref,*n2pos2,                                               // rotational Raman spectra
          gamman2,sigprimen2,sign2,distn2,n2xsec,sign2k,                        // n2 working variables
          gammao2,sigprimeo2,sigo2,disto2,o2xsec,sigo2k,                        // o2 working variables
          sigsq,lambda1e7,solar,n2posj,o2posj,                                  // other working variables
          lambda,lambdaMin,lambdaMax,                                           // range of wavelengths covered by slit function
          slitWidth,                                                            // width of the slit function
         *raman,*raman2,*ramanint,*ringEnd,                                     // output ring cross section
         *solarLambda,*solarVector,                                             // substitution vectors for solar spectrum
         *slitLambda,*slitVector,*slitDeriv2,                                   // substitution vectors for slit function
         *ringLambda,*ringVector,                                               // substitution vectors for ring cross section
          temp,                                                                 // approximate average temperature in °K for scattering
          slitParam,                                                            // gaussian full width at half maximum
          slitParam2,slitParam3,slitParam4,                                     // other slit function parameters
          a,invapi,delta,sigma,                                                 // parameters to build a slit function
          fact;

  XS      xsSolar,xsSlit,xsRing,xsO3;                                           // solar spectrum and slit function
  INT     nsolar,nslit,nring,                                                   // size of previous vectors
          slitType;                                                             // type of the slit function
  INDEX   i,j,k,indexMin,khi,klo;                                               // indexes for loops and arrays
  #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
  HWND    hwndRing;                                                             // handle of parent window
  #endif
  FILE   *fp;                                                                   // output file pointer
  RC      rc;                                                                   // return code

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Start("Ring.dbg","Ring",DEBUG_FCTTYPE_APPL,5,DEBUG_DVAR_YES,1);
  #endif

  // Initializations

  #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
  hwndRing=(HWND)ringArg; // QDOAS ???
  #endif

  memset(&xsSolar,0,sizeof(XS));
  memset(&xsSlit,0,sizeof(XS));
  memset(&xsRing,0,sizeof(XS));
  memset(&xsO3,0,sizeof(XS));

  slitWidth=(double)RING_SLIT_WIDTH;   // NB : force slit width to 6 because of convolutions
  slitType=RING_options.slitConv.slitType;
  slitParam2=RING_options.slitConv.slitParam2;
  slitParam3=RING_options.slitConv.slitParam3;
  slitParam4=RING_options.slitConv.slitParam4;
  o2xref=n2pos2=raman=raman2=ramanint=ringEnd=NULL;
  temp=(double)RING_options.temperature; // (double)250.;   May 2005/05/31
  fp=NULL;

  #if defined(__DEBUG_) && __DEBUG_
  {
  	// Declarations

   time_t today;                                                                // current date and time as a time_t number
   char datetime[20];                                                           // current date and time as a string

   // Get the current date and time

   today=time(NULL);

   // Convert into a string

   strftime(datetime,20,"%d/%m/%Y %H:%M:%S",localtime(&today));
   DEBUG_Print("Start at %s\n",datetime);
  }
  #endif

  // Buffers allocation

  if (((n2xref=(double *)MEMORY_AllocDVector("Ring ","n2xref",0,O2_SIZE-1 /* !!! N2_SIZE instead of O2_SIZE for computations optimization */))==NULL) ||
      ((o2xref=(double *)MEMORY_AllocDVector("Ring ","o2xref",0,O2_SIZE-1))==NULL) ||
      ((n2pos2=(double *)MEMORY_AllocDVector("Ring ","n2pos2",0,O2_SIZE-1))==NULL))

   rc=ERROR_ID_ALLOC;
                                                                                // Load the final wavelength calibration

  else if (!(rc=XSCONV_LoadCalibrationFile(&xsRing,RING_options.calibrationFile,0)) &&

  // Load slit function from file or pre-calculate the slit function

           !(rc=XSCONV_LoadSlitFunction(&xsSlit,&RING_options.slitConv,&slitParam,&slitType)) &&

  // Load solar spectrum in the range of wavelengths covered by the end wavelength scale corrected by the slit function

           !(rc=XSCONV_LoadCrossSectionFile(&xsSolar,RING_options.kuruczFile,(double)xsRing.lambda[0]-slitWidth-1.,
                                     (double)xsRing.lambda[xsRing.NDET-1]+slitWidth+1.,(double)0.,CONVOLUTION_CONVERSION_NONE)) ) // &&

  /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)

  // Test : load O3 cross section for correcting solar spectrum

           ((RING_options.convolutionType!=RING_TYPE_MOLECULAR) ||
           !(rc=XSCONV_LoadCrossSectionFile(&xsO3,RING_options.crossFile,(double)xsRing.lambda[0]-slitWidth-1.,
                                     (double)xsRing.lambda[xsRing.NDET-1]+slitWidth+1.,(double)0.,CONVOLUTION_CONVERSION_NONE))))

//           !(rc=XSCONV_LoadCrossSectionFile(&xsO3,"e:\\data\\xs-hr\\o3\\o3_241.air",(double)xsRing.lambda[0]-slitWidth-1.,
//                                     (double)xsRing.lambda[xsRing.NDET-1]+slitWidth+1.,(double)0.,CONVOLUTION_CONVERSION_NONE)))

     --- */
   {
    for (i=0;i<N2_SIZE;i++)
     n2xref[i]=n2pos2[i]=(double)n2pos[i];
    for (;i<O2_SIZE;i++)
     n2xref[i]=n2pos2[i]=(double)0.;

    // Set up the rotational Raman spectra

    raman_n2(temp,n2xref);
    raman_o2(temp,o2xref);

    // Use substitution variables

    solarLambda=xsSolar.lambda;
    solarVector=xsSolar.vector;
    nsolar=xsSolar.NDET;

    slitLambda=xsSlit.lambda;
    slitVector=xsSlit.vector;
    slitDeriv2=xsSlit.deriv2;
    nslit=xsSlit.NDET;

    ringLambda=xsRing.lambda;
    ringVector=xsRing.vector;
    nring=xsRing.NDET;

    if (!nsolar ||
       ((raman=MEMORY_AllocDVector("Ring ","raman",0,nsolar-1))==NULL) ||
       ((raman2=MEMORY_AllocDVector("Ring ","raman2",0,nsolar-1))==NULL) ||
       ((ramanint=MEMORY_AllocDVector("Ring ","ramanint",0,nring-1))==NULL) || ((ringEnd=MEMORY_AllocDVector("Ring ","ringEnd",0,nring-1))==NULL))
     rc=ERROR_ID_ALLOC;
    else
     {
      if ((rc=XSCONV_TypeStandard(&xsRing,0,xsRing.NDET,&xsSolar,&xsSlit,&xsSolar,NULL,slitType,slitWidth,slitParam,slitParam2,slitParam3,slitParam4))!=ERROR_ID_NO)
       goto EndRing;

//      memcpy(ringVector,solarVector,sizeof(double)*nsolar);  // !!!!

      /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)

      // Test : reduce solar spectrum with O3 optical depth

      if ((RING_options.convolutionType==RING_TYPE_MOLECULAR))
       {
        if ((rc=SPLINE_Vector(xsO3.lambda,xsO3.vector,xsO3.deriv2,xsO3.NDET,solarLambda,raman,nsolar,SPLINE_CUBIC,"Ring "))!=ERROR_ID_NO)
         goto EndRing;

        for (i=0;i<nsolar;i++)
         solarVector[i]=solarVector[i]*raman[i];
//       solarVector[i]=solarVector[i]*exp(-2.5e19*raman[i]);
       }

        --- */

      VECTOR_Init(raman,(double)0.,nsolar);
      VECTOR_Init(raman2,(double)0.,nsolar);

      // Add up Ring contributions over wavelengths and lines; remember that
      // the change in photon energy is opposite that of the molecule

      for (i=0;(i<nsolar) && !rc;i++)
       {
       	#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
        SendMessage(hwndRing,WM_USER,(INT)(100*i/nsolar),0L);
        #endif

        solar=(double)solarVector[i];
        lambda=(double)solarLambda[i];
        lambda1e7=(double)1.e7/lambda;

        if (((slitType==SLIT_TYPE_GAUSS_FILE) || (slitType==SLIT_TYPE_INVPOLY_FILE) || (slitType==SLIT_TYPE_ERF_FILE)) &&
            ((rc=SPLINE_Vector(slitLambda,slitVector,slitDeriv2,nslit,&lambda,&slitParam,1,SPLINE_CUBIC,"Ring "))!=0))
         goto EndRing;

        sigma=slitParam*0.5;
        a=sigma/sqrt(log(2.));
        delta=slitParam2*0.5;
        invapi=(double)1./(a*sqrt(PI));

        sigsq = (double) 1.e6/(lambda*lambda);
        gamman2 = (double) -0.601466 + 238.557 / (186.099 - sigsq);
        gammao2 = (double) 0.07149 + 45.9364 / (48.2716 - sigsq);

        gamman2*=gamman2;   // gamman2 <- gamman2**2;
        gammao2*=gammao2;   // gammao2 <- gammao2**2;

        for (j=0;j<O2_SIZE;j++)
         {
          n2posj=n2pos2[j];
          o2posj=o2pos[j];

          sigprimen2=(double) lambda1e7-n2posj;
          sigprimen2 *= sigprimen2;       // **2
          sigprimen2 *= sigprimen2;       // **4

          sigprimeo2 = (double) lambda1e7-o2posj;
          sigprimeo2 *= sigprimeo2;       // **2
          sigprimeo2 *= sigprimeo2;       // **4

          n2xsec=n2xref[j]*sigprimen2*gamman2*solar;
          o2xsec=o2xref[j]*sigprimeo2*gammao2*solar;

          sign2=(double)1.e7/(-n2posj+lambda1e7);
          sigo2=(double)1.e7/(-o2posj+lambda1e7);

          lambdaMin=(double)min(sign2,sigo2)-4.*slitParam; // lambda-slitWidth;
          lambdaMax=(double)max(sign2,sigo2)+4.*slitParam; // lambda+slitWidth;

          for (indexMin=0,klo=0,khi=nsolar-1;khi-klo>1;)
           {
            indexMin=(khi+klo)>>1;

            if (solarLambda[indexMin]>lambdaMin)
             khi=indexMin;
            else
             klo=indexMin;
           }

          #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
          if (!WaitForSingleObject(RING_event,0))
           {
            rc=ERROR_ID_USERBREAK;
            goto EndRing;
           }
           #endif

          // Contributions from +- npix pixels away

          for (k=indexMin;(k<nsolar)&&(solarLambda[k]<=lambdaMax);k++)
           {
           	sign2k=sign2-solarLambda[k];
           	sigo2k=sigo2-solarLambda[k];

            if (slitType==SLIT_TYPE_GAUSS_FILE)
             {
              distn2=invapi*exp(-(sign2k*sign2k)/(a*a));
              disto2=invapi*exp(-(sigo2k*sigo2k)/(a*a));
             }
            else if (slitType==SLIT_TYPE_INVPOLY_FILE)
             {
              distn2=(double)pow(sigma,(double)slitParam2)/((pow(sign2k,(double)slitParam2)+pow(sigma,(double)slitParam2)));
              disto2=(double)pow(sigma,(double)slitParam2)/((pow(sigo2k,(double)slitParam2)+pow(sigma,(double)slitParam2)));
             }
            else if (slitType==SLIT_TYPE_ERF_FILE)
             {
              distn2=(double)(ERF_GetValue((sign2k+delta)/a)-ERF_GetValue((sign2k-delta)/a))/(4.*delta);
              disto2=(double)(ERF_GetValue((sigo2k+delta)/a)-ERF_GetValue((sigo2k-delta)/a))/(4.*delta);
             }
            else if ((slitType==SLIT_TYPE_FILE) &&
                   (((rc=SPLINE_Vector(slitLambda,slitVector,slitDeriv2,nslit,&sign2k,&distn2,1,SPLINE_CUBIC,"Ring "))!=0) ||
                    ((rc=SPLINE_Vector(slitLambda,slitVector,slitDeriv2,nslit,&sigo2k,&disto2,1,SPLINE_CUBIC,"Ring "))!=0)))

             goto EndRing;

             raman[k]+=(n2xsec*distn2+o2xsec*disto2)*(solarLambda[k]-solarLambda[k-1]);
           }
         }
       }

      // Check if file exist

      FILES_RebuildFileName(ringFileName,RING_options.path,1);

      #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
      if (((fp=fopen(ringFileName,"rt"))!=NULL) &&
           (MSG_MessageBox(hwndRing,TOOL_RING_OUTPUT_BUTTON1,MENU_WINDOWS_TOOL_RING,IDS_MSGBOX_FILE,
            MB_YESNO|MB_ICONQUESTION,ringFileName)==IDNO))

       rc=IDS_MSGBOX_FILE;

      if (fp!=NULL)
       {
        fclose(fp);
        fp=NULL;
       }
      #endif

      #if defined(__DEBUG_) && __DEBUG_
      DEBUG_PrintVar("High resolution vectors",solarLambda,0,nsolar-1,solarVector,0,nsolar-1,raman,0,nsolar-1,NULL);
      #endif

      if (!rc && !(rc=SPLINE_Deriv2(solarLambda,raman,raman2,nsolar,"Ring ")) &&
         !(rc=SPLINE_Vector(solarLambda,raman,raman2,nsolar,ringLambda,ramanint,nring,SPLINE_CUBIC,"Ring ")))
       {
        if ((fp=fopen(ringFileName,"w+t"))==NULL)
         #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
         MSG_MessageBox(hwndRing,TOOL_RING_OUTPUT_BUTTON1,MENU_WINDOWS_TOOL_RING,(rc=ERROR_ID_FILE_OPEN),
                         MB_OK|MB_ICONHAND,ringFileName);
         #else
         rc=ERROR_SetLast("Ring",ERROR_TYPE_WARNING,ERROR_ID_FILE_OPEN,ringFileName);
         #endif
        else
         {
          if (!RING_options.noComment)
           RingHeader(fp,1);

          // Output ring

          for (i=0;i<nring;i++)
           ringEnd[i]=((ramanint[i]>(double)0.) && (ringVector[i]>(double)0.))?
                                    (double)ramanint[i] /* ring effect source spectrum */ /ringVector[i] /* solar spectrum */:(double)0.;
// OLD           ringEnd[i]=((ramanint[i]>(double)0.) && (ringVector[i]>(double)0.))?
//                                    (double)log(ramanint[i] /* ring effect source spectrum */ /ringVector[i] /* solar spectrum */):(double)0.;

          if (!(rc=ANALYSE_NormalizeVector(ringEnd-1,nring,&fact,"Ring")))
           {
            for (i=0;i<nring;i++)
             fprintf(fp,"%.14le %.14le %.14le %.14le\n",ringLambda[i],ringEnd[i],ramanint[i],ringVector[i]);

            #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
            DRAW_Spectra(CHILD_WINDOW_SPECTRA,"Ring cross section",
                        "","Wavelength (nm)","",NULL,0,
                        (double)0.,(double)0.,(double)0.,(double)0.,
                         ringLambda,ringEnd,nring,DRAW_COLOR1,0,nring-1,PS_SOLID,"",
                         NULL,NULL,0,DRAW_COLOR1,0,0,PS_SOLID,"",
                         0,1,1,1);
            #endif

            if (strlen(RING_options.path2))
             {
              fclose(fp);

              FILES_RebuildFileName(ringFileName,RING_options.path2,1);

              #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
              if (((fp=fopen(ringFileName,"rt"))!=NULL) &&
                   (MSG_MessageBox(hwndRing,TOOL_RING_OUTPUT_BUTTON1,MENU_WINDOWS_TOOL_RING,IDS_MSGBOX_FILE,
                    MB_YESNO|MB_ICONQUESTION,ringFileName)==IDNO))

               rc=IDS_MSGBOX_FILE;
              else
              #endif
               {
                if (fp!=NULL)
                 fclose(fp);

                if ((fp=fopen(ringFileName,"w+t"))==NULL)
                 {
                 	#if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
                  MSG_MessageBox(hwndRing,TOOL_RING_OUTPUT_BUTTON2,MENU_WINDOWS_TOOL_RING,ERROR_ID_FILE_OPEN,
                                 MB_OK|MB_ICONHAND,ringFileName);
                  #endif
                 }
                else
                 {
                  // Second order

                  if (!RING_options.noComment)
                   RingHeader(fp,2);

                  for (i=0;i<nring;i++)
                   ringEnd[i]*=ringEnd[i];

                  if (!ANALYSE_NormalizeVector(ringEnd-1,nring,&fact,"Ring"))
                   for (i=0;i<nring;i++)
                    fprintf(fp,"%.14le %.14le %.14le %.14le\n",ringLambda[i],ringEnd[i],ramanint[i],ringVector[i]);
                 }
               }
             }
           }
         }
       }
     }
   }

  EndRing :

  if (fp!=NULL)
   fclose(fp);

  // Release allocated buffers

  if (n2xref!=NULL)
   MEMORY_ReleaseDVector("Ring ","n2xref",n2xref,0);
  if (o2xref!=NULL)
   MEMORY_ReleaseDVector("Ring ","o2xref",o2xref,0);
  if (n2pos2!=NULL)
   MEMORY_ReleaseDVector("Ring ","n2pos2",n2pos2,0);
  if (raman!=NULL)
   MEMORY_ReleaseDVector("Ring ","raman",raman,0);
  if (raman2!=NULL)
   MEMORY_ReleaseDVector("Ring ","raman2",raman2,0);
  if (ramanint!=NULL)
   MEMORY_ReleaseDVector("Ring ","ramanint",ramanint,0);
  if (ringEnd!=NULL)
   MEMORY_ReleaseDVector("Ring ","ringEnd",ringEnd,0);

  XSCONV_Reset(&xsSolar);
  XSCONV_Reset(&xsSlit);
  XSCONV_Reset(&xsRing);
  XSCONV_Reset(&xsO3);

  #if defined(__WINDOAS_GUI_) && __WINDOAS_GUI_
  SendMessage(hwndRing,WM_USER,(!rc)?(INT)100:1000,0L);
  #endif

  #if defined(__DEBUG_) && __DEBUG_
  {
  	// Declarations

   time_t today;                                                                // current date and time as a time_t number
   char datetime[20];                                                           // current date and time as a string

   // Get the current date and time

   today=time(NULL);

   // Convert into a string

   strftime(datetime,20,"%d/%m/%Y %H:%M:%S",localtime(&today));
   DEBUG_Print("End at %s\n",datetime);
  }
  #endif

  // Debugging

  #if defined(__DEBUG_) && __DEBUG_
  DEBUG_Stop("Ring");
  #endif

  // End the thread

// QDOAS ???  #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???  _endthread();
// QDOAS ???  #endif
 }

// QDOAS ??? // =====================================
// QDOAS ??? // RING EFFECT PANEL MESSAGES PROCESSING
// QDOAS ??? // =====================================
// QDOAS ???
// QDOAS ??? #if defined (__WINDOAS_GUI_) && __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      RingTypeMolecular
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       show/hide fields to create a molecular ring XS (obsolete function)
// QDOAS ??? //
// QDOAS ??? // INPUT         hwndRing : handle of the parent dialog box
// QDOAS ??? //               action   : show/hide fields related to a molecular ring cross section
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ???
// QDOAS ??? void RingTypeMolecular(HWND hwndRing,INT action)
// QDOAS ???  {
// QDOAS ???   ShowWindow(GetDlgItem(hwndRing,TOOL_RING_CROSS_BUTTON),action);
// QDOAS ???   ShowWindow(GetDlgItem(hwndRing,TOOL_RING_CROSS_FILE),action);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RingDlgInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_INIT message processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndRing : handle of the Ring dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RingDlgInit(HWND hwndRing)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR string[20];
// QDOAS ???   HWND hwndConv;
// QDOAS ???   INDEX indexItem;
// QDOAS ???   SLIT *pSlit;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   hwndConv=GetDlgItem(hwndRing,TOOL_RING_TYPE);
// QDOAS ???   memcpy(&RING_buffer,&RING_options,sizeof(XSCONV));
// QDOAS ???   pSlit=&RING_buffer.slitConv;
// QDOAS ???
// QDOAS ???   // Center on parent window
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndRing,GetWindow(hwndRing,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Rebuild files names
// QDOAS ???
// QDOAS ???   FILES_RebuildFileName(RING_buffer.calibrationFile,RING_options.calibrationFile,1);
// QDOAS ???   FILES_RebuildFileName(RING_buffer.path,RING_options.path,1);
// QDOAS ???   FILES_RebuildFileName(RING_buffer.path2,RING_options.path2,1);
// QDOAS ???   FILES_RebuildFileName(RING_buffer.crossFile,RING_options.crossFile,1);
// QDOAS ???   FILES_RebuildFileName(RING_buffer.kuruczFile,RING_options.kuruczFile,1);
// QDOAS ???   FILES_RebuildFileName(RING_buffer.slitConv.slitFile,RING_options.slitConv.slitFile,1);
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???
// QDOAS ???   // Set ring type
// QDOAS ???
// QDOAS ???   CheckRadioButton(hwndRing,
// QDOAS ???                    TOOL_RING_TYPE_SOLAR,
// QDOAS ???                    TOOL_RING_TYPE_MOLECULAR,
// QDOAS ???                    TOOL_RING_TYPE_SOLAR+RING_options.convolutionType);
// QDOAS ???
// QDOAS ???      --- */
// QDOAS ???
// QDOAS ???   // Fill edit controls
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_OUTPUT_FILE1),RING_buffer.path);
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_OUTPUT_FILE2),RING_buffer.path2);
// QDOAS ???      --- */
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_SOLAR_FILE),RING_buffer.kuruczFile);
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_CALIBRATION_FILE),RING_buffer.calibrationFile);
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_CROSS_FILE),RING_buffer.crossFile);
// QDOAS ???      --- */
// QDOAS ???
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_SLIT_FILE),pSlit->slitFile);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam);
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_GAUSS_WIDTH),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam2);
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_INVPOLY),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam3);
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_VOIGT_RGAUSS),string);
// QDOAS ???   sprintf(string,"%g",pSlit->slitParam4);
// QDOAS ???   SetWindowText(GetDlgItem(hwndRing,TOOL_RING_VOIGT_RRATIO),string);
// QDOAS ???
// QDOAS ???   // Fill combobox
// QDOAS ???
// QDOAS ???   for (indexItem=0;indexItem<SLIT_TYPE_GAUSS_T_FILE;indexItem++)
// QDOAS ???    SendMessage(hwndConv,CB_ADDSTRING,0,(LPARAM)XSCONV_slitTypes[indexItem]);
// QDOAS ???
// QDOAS ???   SendMessage(hwndConv,CB_SETCURSEL,(WPARAM)pSlit->slitType,0);
// QDOAS ???
// QDOAS ???   // Show/Hide File/Gauss fields
// QDOAS ???
// QDOAS ???   XSCONV_SlitType(hwndRing,TOOL_RING_TYPE,pSlit,&RING_buffer.slitDConv);
// QDOAS ???
// QDOAS ???   // Check boxes
// QDOAS ???
// QDOAS ???   CheckDlgButton(hwndRing,TOOL_RING_NOCOMMENT,(RING_buffer.noComment!=0)?BST_CHECKED:BST_UNCHECKED);
// QDOAS ???
// QDOAS ???   SetDlgItemInt(hwndRing,TOOL_RING_TEMP,RING_buffer.temperature,FALSE);
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???   RingTypeMolecular(hwndRing,(RING_buffer.convolutionType==RING_TYPE_SOLAR)?SW_HIDE:SW_SHOW);
// QDOAS ???      --- */
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RingOK
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         OK command processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndRing : handle of the Ring dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RingOK(HWND hwndRing)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR string[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   SLIT *pSlit;
// QDOAS ???   RC rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   pSlit=&RING_buffer.slitConv;
// QDOAS ???   rc=ERROR_ID_NO;
// QDOAS ???
// QDOAS ???   // Fields read out
// QDOAS ???
// QDOAS ???   memset(string,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   RING_buffer.temperature=(INT)GetDlgItemInt(hwndRing,TOOL_RING_TEMP,NULL,FALSE);
// QDOAS ???
// QDOAS ???   pSlit->slitType=(UCHAR)SendMessage(GetDlgItem(hwndRing,TOOL_RING_TYPE),CB_GETCURSEL,0,0);
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???   if (IsDlgButtonChecked(hwndRing,TOOL_RING_TYPE_SOLAR))
// QDOAS ???    RING_buffer.convolutionType=(INT)RING_TYPE_SOLAR;
// QDOAS ???   else if (IsDlgButtonChecked(hwndRing,TOOL_RING_TYPE_MOLECULAR))
// QDOAS ???    RING_buffer.convolutionType=(INT)RING_TYPE_MOLECULAR;
// QDOAS ???      --- */
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_OUTPUT_FILE1),RING_buffer.path,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_OUTPUT_FILE2),RING_buffer.path2,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_SOLAR_FILE),RING_buffer.kuruczFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_CALIBRATION_FILE),RING_buffer.calibrationFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_CROSS_FILE),RING_buffer.crossFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_GAUSS_WIDTH),string,MAX_ITEM_TEXT_LEN);
// QDOAS ???   pSlit->slitParam=(double)fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_INVPOLY),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   pSlit->slitParam2=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_VOIGT_RGAUSS),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   pSlit->slitParam3=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_VOIGT_RRATIO),string,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???   pSlit->slitParam4=fabs(atof(string));
// QDOAS ???   GetWindowText(GetDlgItem(hwndRing,TOOL_RING_SLIT_FILE),pSlit->slitFile,MAX_ITEM_TEXT_LEN);
// QDOAS ???
// QDOAS ???   RING_buffer.noComment=(UCHAR)(IsDlgButtonChecked(hwndRing,TOOL_RING_NOCOMMENT)==BST_CHECKED)?(UCHAR)1:(UCHAR)0;
// QDOAS ???
// QDOAS ???   // Check validity of fields
// QDOAS ???
// QDOAS ???   if (!strlen(RING_buffer.path))
// QDOAS ???    MSG_MessageBox(hwndRing,TOOL_RING_OUTPUT_BUTTON1,MENU_WINDOWS_TOOL_RING,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Output File");
// QDOAS ???
// QDOAS ???   /* --- Commented out on 10 November 2004 (do not account for molecular absorption anymore)
// QDOAS ???   else if ((RING_buffer.convolutionType==RING_TYPE_MOLECULAR) && !strlen(RING_buffer.crossFile))
// QDOAS ???    MSG_MessageBox(hwndRing,TOOL_RING_CROSS_BUTTON,MENU_WINDOWS_TOOL_RING,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Cross Section");
// QDOAS ???      ---*/
// QDOAS ???
// QDOAS ???   else if (!strlen(RING_buffer.kuruczFile))
// QDOAS ???    MSG_MessageBox(hwndRing,TOOL_RING_SOLAR_BUTTON,MENU_WINDOWS_TOOL_RING,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Solar File");
// QDOAS ???
// QDOAS ???   else if (!strlen(RING_buffer.calibrationFile))
// QDOAS ???    MSG_MessageBox(hwndRing,TOOL_RING_CALIBRATION_BUTTON,MENU_WINDOWS_TOOL_RING,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Calibration File");
// QDOAS ???
// QDOAS ???   else if (((pSlit->slitType==SLIT_TYPE_FILE) || (pSlit->slitType==SLIT_TYPE_GAUSS_FILE) || (pSlit->slitType==SLIT_TYPE_INVPOLY_FILE)) &&
// QDOAS ???             !strlen(pSlit->slitFile))
// QDOAS ???    MSG_MessageBox(hwndRing,TOOL_RING_SLIT_BUTTON,MENU_WINDOWS_TOOL_RING,(rc=IDS_MSGBOX_FIELDEMPTY),
// QDOAS ???                    MB_OK|MB_ICONHAND,"Slit Function");
// QDOAS ???
// QDOAS ???   if (!rc)
// QDOAS ???    {
// QDOAS ???     if (!TLBAR_bSaveFlag)
// QDOAS ???      TLBAR_Enable(TRUE);
// QDOAS ???
// QDOAS ???     memcpy(&RING_options,&RING_buffer,sizeof(XSCONV));
// QDOAS ???
// QDOAS ???     FILES_ChangePath(RING_options.calibrationFile,RING_buffer.calibrationFile,1);
// QDOAS ???     FILES_ChangePath(RING_options.path,RING_buffer.path,1);
// QDOAS ???     FILES_ChangePath(RING_options.path2,RING_buffer.path2,1);
// QDOAS ???     FILES_ChangePath(RING_options.crossFile,RING_buffer.crossFile,1);
// QDOAS ???     FILES_ChangePath(RING_options.kuruczFile,RING_buffer.kuruczFile,1);
// QDOAS ???     FILES_ChangePath(RING_options.slitConv.slitFile,RING_buffer.slitConv.slitFile,1);
// QDOAS ???
// QDOAS ???     EndDialog(hwndRing,TRUE);
// QDOAS ???
// QDOAS ???     DialogBox(DOAS_hInst,                                                  // current instance
// QDOAS ???               MAKEINTRESOURCE(DLG_PROGRESS),                               // panel identification
// QDOAS ???               DOAS_hwndMain,                                               // handle of parent windows
// QDOAS ???      (DLGPROC)RING_ProgressWndProc);                                       // procedure for messages processing
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RingCommand
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_COMMAND command processing
// QDOAS ??? //
// QDOAS ??? // SYNTAX          usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK RingCommand(HWND hwndRing,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch((ULONG)GET_WM_COMMAND_ID(mp1,mp2))
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_TYPE_SOLAR :
// QDOAS ???      RingTypeMolecular(hwndRing,SW_HIDE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_TYPE_MOLECULAR :
// QDOAS ???      RingTypeMolecular(hwndRing,SW_SHOW);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_OUTPUT_BUTTON1 :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.path,FILE_TYPE_ALL,FILE_MODE_SAVE,TOOL_RING_OUTPUT_FILE1,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_OUTPUT_BUTTON2 :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.path2,FILE_TYPE_ALL,FILE_MODE_SAVE,TOOL_RING_OUTPUT_FILE2,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_SOLAR_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.kuruczFile,FILE_TYPE_CALIB_KURUCZ,FILE_MODE_OPEN,TOOL_RING_SOLAR_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_CROSS_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.crossFile,FILE_TYPE_CROSS,FILE_MODE_OPEN,TOOL_RING_CROSS_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_CALIBRATION_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.calibrationFile,FILE_TYPE_CALIB,FILE_MODE_OPEN,TOOL_RING_CALIBRATION_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_SLIT_BUTTON :
// QDOAS ???      XSCONV_FileSelection(hwndRing,RING_buffer.slitConv.slitFile,FILE_TYPE_SLIT,FILE_MODE_OPEN,TOOL_RING_SLIT_FILE,1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case TOOL_RING_TYPE :
// QDOAS ???      XSCONV_SlitType(hwndRing,TOOL_RING_TYPE,&RING_buffer.slitConv,&RING_buffer.slitDConv);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDOK :
// QDOAS ???      RingOK(hwndRing);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDCANCEL :
// QDOAS ???      EndDialog(hwndRing,FALSE);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case IDHELP :
// QDOAS ???      WinHelp(hwndRing,DOAS_HelpPath,HELP_CONTEXT,HLP_TOOLS_RING);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RING_WndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from ring effect tool panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX          usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK RING_WndProc(HWND hwndRing,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      RingDlgInit(hwndRing);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      RingCommand(hwndRing,msg,mp1,mp2);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_HELP :
// QDOAS ???      WinHelp(hwndRing,DOAS_HelpPath,HELP_CONTEXT,HLP_TOOLS_RING);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // =======================
// QDOAS ??? // PROGRESS BAR PROCESSING
// QDOAS ??? // =======================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RingProgressInit
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_INITDIALOG command processing
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndProgress : handle of the progress dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RingProgressInit(HWND hwndProgress)
// QDOAS ???  {
// QDOAS ???   // Declaration
// QDOAS ???
// QDOAS ???   SECURITY_ATTRIBUTES securityAttributes;
// QDOAS ???
// QDOAS ???   // Reset progress position
// QDOAS ???
// QDOAS ???   RING_progress=0;
// QDOAS ???
// QDOAS ???   // Center on parent window
// QDOAS ???
// QDOAS ???   DOAS_CenterWindow(hwndProgress,GetWindow(hwndProgress,GW_OWNER));
// QDOAS ???
// QDOAS ???   // Set range and step
// QDOAS ???
// QDOAS ???   SendMessage(GetDlgItem(hwndProgress,PROGRESS_BAR),PBM_SETRANGE,0,MAKELONG(0,100));
// QDOAS ???   SendMessage(GetDlgItem(hwndProgress,PROGRESS_BAR),PBM_SETSTEP,1,0L);
// QDOAS ???
// QDOAS ???   // Create handle for event
// QDOAS ???
// QDOAS ???   securityAttributes.nLength=sizeof(SECURITY_ATTRIBUTES);    // the size in bytes of this structure
// QDOAS ???   securityAttributes.lpSecurityDescriptor=NULL;              // use default
// QDOAS ???   securityAttributes.bInheritHandle=TRUE;
// QDOAS ???
// QDOAS ???   RING_event=CreateEvent(&securityAttributes,FALSE,FALSE,NULL);
// QDOAS ???
// QDOAS ???   // Start ring thread
// QDOAS ???
// QDOAS ???   _beginthread(Ring,0x4000,(void *)hwndProgress);
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RingProgressUser
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         WM_USER message processing (update progress bar on user message)
// QDOAS ??? //
// QDOAS ??? // INPUT           hwndProgress : handle of the progress dialog box
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RingProgressUser(HWND hwndProgress,int mp1)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR str[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???
// QDOAS ???   if (mp1>100)
// QDOAS ???    EndDialog(hwndProgress,FALSE);
// QDOAS ???   else if (mp1!=RING_progress)
// QDOAS ???    {
// QDOAS ???     sprintf(str,"%d %%",(RING_progress=mp1));
// QDOAS ???     SendMessage(GetDlgItem(hwndProgress,PROGRESS_BAR),PBM_STEPIT,0,0L);
// QDOAS ???     SetWindowText(GetDlgItem(hwndProgress,PROGRESS_TEXT),str);
// QDOAS ???
// QDOAS ???     if (RING_progress==100)
// QDOAS ???      EndDialog(hwndProgress,TRUE);
// QDOAS ???    }
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION        RING_ProgressWndProc
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE         dispatch messages from progress bar panel
// QDOAS ??? //
// QDOAS ??? // SYNTAX          usual syntax for Windows message processing;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? LRESULT CALLBACK RING_ProgressWndProc(HWND hwndProgress,UINT msg,WPARAM mp1,LPARAM mp2)
// QDOAS ???  {
// QDOAS ???   switch (msg)
// QDOAS ???    {
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_INITDIALOG :
// QDOAS ???      RingProgressInit(hwndProgress);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_COMMAND :
// QDOAS ???      SetEvent(RING_event);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_USER :
// QDOAS ???      RingProgressUser(hwndProgress,(int)mp1);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???     case WM_DESTROY :
// QDOAS ???      CloseHandle(RING_event);
// QDOAS ???     break;
// QDOAS ???  // ---------------------------------------------------------------------------
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   return 0;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? #endif // __WINDOAS_GUI_
// QDOAS ???
// QDOAS ??? // =============================
// QDOAS ??? // CONFIGURATION FILE MANAGEMENT
// QDOAS ??? // =============================
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      RING_ResetConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       reset Ring options
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RING_ResetConfiguration(void)
// QDOAS ???  {
// QDOAS ???   memset(&RING_options,0,sizeof(XSCONV));
// QDOAS ???   RING_options.temperature=RING_TEMPERATURE;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      RING_LoadConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       load data and input files needed for building a ring cross section
// QDOAS ??? //
// QDOAS ??? // INPUT         fileLine : the line to process
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? RC RING_LoadConfiguration(UCHAR *fileLine)
// QDOAS ???  {
// QDOAS ???   // Declarations
// QDOAS ???
// QDOAS ???   UCHAR keyName[MAX_ITEM_NAME_LEN+1],     // key name for first part of project information
// QDOAS ???         text[MAX_ITEM_TEXT_LEN+1];
// QDOAS ???   RC    rc;
// QDOAS ???
// QDOAS ???   // Initializations
// QDOAS ???
// QDOAS ???   memset(keyName,0,MAX_ITEM_NAME_LEN+1);
// QDOAS ???   memset(text,0,MAX_ITEM_TEXT_LEN+1);
// QDOAS ???
// QDOAS ???   rc=0;
// QDOAS ???
// QDOAS ???   if ((sscanf(fileLine,"%[^'=']=%[^'\n']",keyName,text)>=2) && !STD_Stricmp(keyName,RING_SECTION))
// QDOAS ???    {
// QDOAS ???     memset(&RING_options,0,sizeof(XSCONV));
// QDOAS ???     rc=1;
// QDOAS ???
// QDOAS ???     // Get data part
// QDOAS ???
// QDOAS ???     if (sscanf(text,"%[^','],%[^','],%d,%lf,%lf,%[^','],%[^','],%d,%[^','],%[^','],%d,%lf,%lf,%d",
// QDOAS ???             (UCHAR *) RING_options.path,
// QDOAS ???             (UCHAR *) RING_options.calibrationFile,
// QDOAS ???               (INT *)&RING_options.slitConv.slitType,
// QDOAS ???            (double *)&RING_options.slitConv.slitParam,
// QDOAS ???            (double *)&RING_options.slitConv.slitParam2,
// QDOAS ???             (UCHAR *) RING_options.slitConv.slitFile,
// QDOAS ???             (UCHAR *)&RING_options.kuruczFile,
// QDOAS ???               (INT *)&RING_options.noComment,
// QDOAS ???             (UCHAR *) RING_options.path2,
// QDOAS ???             (UCHAR *) RING_options.crossFile,
// QDOAS ???               (INT *)&RING_options.convolutionType,
// QDOAS ???            (double *)&RING_options.slitConv.slitParam3,
// QDOAS ???            (double *)&RING_options.slitConv.slitParam4,
// QDOAS ???               (INT *)&RING_options.temperature)>=1)
// QDOAS ???      {
// QDOAS ???       FILES_CompactPath(RING_options.path,RING_options.path,1,1);
// QDOAS ???       FILES_CompactPath(RING_options.path2,RING_options.path2,1,1);
// QDOAS ???       FILES_CompactPath(RING_options.calibrationFile,RING_options.calibrationFile,1,1);
// QDOAS ???       FILES_CompactPath(RING_options.kuruczFile,RING_options.kuruczFile,1,1);
// QDOAS ???       FILES_CompactPath(RING_options.crossFile,RING_options.crossFile,1,1);
// QDOAS ???       FILES_CompactPath(RING_options.slitConv.slitFile,RING_options.slitConv.slitFile,1,1);
// QDOAS ???
// QDOAS ???       if (!RING_options.temperature)
// QDOAS ???        RING_options.temperature=RING_TEMPERATURE;
// QDOAS ???      }
// QDOAS ???    }
// QDOAS ???
// QDOAS ???   // Return
// QDOAS ???
// QDOAS ???   return rc;
// QDOAS ???  }
// QDOAS ???
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // FUNCTION      RING_SaveConfiguration
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ??? // PURPOSE       save data needed for building a ring effect cross section
// QDOAS ??? //
// QDOAS ??? // INPUT         fp          : pointer to the current configuration file;
// QDOAS ??? // -----------------------------------------------------------------------------
// QDOAS ???
// QDOAS ??? void RING_SaveConfiguration(FILE *fp)
// QDOAS ???  {
// QDOAS ???   fprintf(fp,"%s=%s,%s,%d,%.3f,%.3f,%s,%s,%d,%s,%s,%d,%.3f,%.3f,%d\n",
// QDOAS ???               RING_SECTION,
// QDOAS ???               RING_options.path,
// QDOAS ???               RING_options.calibrationFile,
// QDOAS ???         (INT) RING_options.slitConv.slitType,
// QDOAS ???               RING_options.slitConv.slitParam,
// QDOAS ???               RING_options.slitConv.slitParam2,
// QDOAS ???               RING_options.slitConv.slitFile,
// QDOAS ???               RING_options.kuruczFile,
// QDOAS ???         (INT) RING_options.noComment,
// QDOAS ???               RING_options.path2,
// QDOAS ???               RING_options.crossFile,
// QDOAS ???         (INT) RING_options.convolutionType,
// QDOAS ???               RING_options.slitConv.slitParam3,
// QDOAS ???               RING_options.slitConv.slitParam4,
// QDOAS ???               RING_options.temperature);
// QDOAS ???  }
