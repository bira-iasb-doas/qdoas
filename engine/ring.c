
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

#include "engine_xsconv.h"

// =================
// N2 RAMAN FUNCTION
// =================

// Statistical parameters, for partition function calculation

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

double n2pos[N2_SIZE]=
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

double o2pos[O2_SIZE]=
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



