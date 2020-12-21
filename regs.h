#pragma once

enum DSARF {
  ZERO = 0, // zero register 
  TBC,      // accelerators To Be Configured
  CSA,      // Configuration Starting Address
  CFS,      // ConFiguration Size in bytes
  CSR,      // Control State Register, encodes control states in 32 bits
  SAR,      // Starting AddRess of linear memory streams
  L1D,      // Length of a 1D stream
  E2D,      // length strEtch of a 2D stream 
  I2D,      // strIde of a 2D stream 
  L2D,      // Length (trip count) of a 2D stream's outer loop
  DE2D,     // Delta on strEtch 2D of a 3d stream
  DI2D,     // Delta on strIde 2D of a 3d stream
  E3D1D,    // strEtch of a 3D stream affects the 1st-Dimension
  E3D2D,    // strEtch of a 3D stream affects the 2nd-Dimension
  I3D,      // strIde of a 3D stream
  L3D,      // Length (trip count) of a 3D stream's outer-most loop
  INDP,     // INDirect Ports encoded compactly in 32 bits
  ISL,      // Index Stream Length
  IL1D,     // Indirect Length of 1D stream
  IL2D,     // Indirect Length of 2D stream
  ILV,      // Indirect Length of Value stream
  BR,       // allocated Buffet address Range encoded in 32 bits
  BSR,      // Buffet State Register, encodes buffet configuration info in 32 bits
  OFL,      // OFfset List (up to 4) accessed by an indirect stream
  VSR,      // Vector State Register, encodes vector state
  RPT,      // RePeaT times of each port element
  ERPT,     // strEtch of RePeaT times
  EPRD,     // apply the strEtch according to the PeRioD
  RESERVED0,
  RESERVED1,
  RESERVED2,
  RESERVED3,
  RESERVED4,
  TOTAL_REG,
};
