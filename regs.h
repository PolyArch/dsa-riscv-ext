#pragma once

/*!
 * \brief The register file of the accelerator control state.
 */
#define ZERO  0  // zero register 
#define TBC   1  // accelerators To Be Configured
#define CSA   2  // Configuration Starting Address
#define CFS   3  // ConFiguration Size in bytes
#define CSR   4  // Control State Register, encodes control states in 32 bits
#define SAR   5  // Starting AddRess of linear memory streams
#define L1D   6  // Length of a 1D stream
#define E2D   7  // length strEtch of a 2D stream 
#define I2D   8  // strIde of a 2D stream 
#define L2D   9  // Length (trip count) of a 2D stream's outer loop
#define DE2D  10 // Delta on strEtch 2D of a 3d stream
#define DI2D  11 // Delta on strIde 2D of a 3d stream
#define E3D1D 12 // strEtch of a 3D stream affects the 1st-Dimension
#define E3D2D 13 // strEtch of a 3D stream affects the 2nd-Dimension
#define I3D   14 // strIde of a 3D stream
#define L3D   15 // Length (trip count) of a 3D stream's outer-most loop
#define INDP  16 // INDirect Ports encoded compactly in 32 bits
#define ISL   17 // Index Stream Length
#define IL1D  18 // Indirect Length of 1D stream
#define IL2D  19 // Indirect Length of 2D stream
#define ILV   20 // Indirect Length of Value stream
#define BR    21 // allocated Buffet address Range encoded in 32 bits
#define BSR   22 // Buffet State Register, encodes buffet configuration info in 32 bits
#define CLS   23 // initial Const of a Linear numeric Stream
#define OFL   24 // OFfset List (up to 4) accessed by an indirect stream
#define VSR   25 // Vector State Register, encodes vector state
#define RPT   26 // RePeaT times of each port element
#define ERPT  27 // strEtch of RePeaT times
#define EPRD  28 // apply the strEtch according to the PeRioD