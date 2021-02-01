#pragma once

enum DSARF {
#define MACRO(x) x,
#include "./rf.def"
#undef MACRO
};

const char* const REG_NAMES[] = {
#define MACRO(x) #x,
#include "./rf.def"
#undef MACRO
};

const int REG_STICKY[] = {
0, // ZERO
1, // TBC
1, // CSA
1, // CFS
1, // CSR
0, // SAR
0, // L1D
0, // E2D
0, // I2D
0, // L2D
0, // DE2D
0, // DI2D
0, // E3D1D
0, // E3D2D
0, // I3D
0, // L3D
0, // INDP
0, // ISL
0, // IL1D
0, // IL2D
0, // ILV
0, // BR
0, // BSR
0, // OFL
1, // VSR
0, // RPT
0, // ERPT
0, // EPRD
0, // RESERVED0
0, // RESERVED1
0, // RESERVED2
0, // RESERVED3
0, // TOTAL_REG
};

enum Padding {
  DP_NoPadding,
  DP_PostStrideZero,
  DP_PreStrideZero,
  DP_PostStridePredOff,
  DP_PreStridePredOff,
};

enum BarrierFlag {
  DBF_DMAStreams,
  DBF_SPadStreams,
  DBF_RecurStreams,
  DBF_ReadStreams,
  DBF_WriteStreams,
  DBF_AtomicStreams,
  DBF_ComputStreams,
};

enum MemoryOperation {
  DMO_Read,
  DMO_Write,
  DMO_AtomAdd,
  DMO_AtomSub,
  DMO_AtomMul,
  DMO_AtomDiv,
  DMO_AtomMax,
  DMO_AtomMin,
};

enum StreamAction {
  DSA_Access,
  DSA_Generate
};

enum PortField {
  DPF_PortBroadcast,
  DPF_PortRepeat,
  DPF_PortRepeatStretch,
  DPF_PortPeriod,
};

enum MemoryType {
  DMT_DMA,
  DMT_SPAD
};

