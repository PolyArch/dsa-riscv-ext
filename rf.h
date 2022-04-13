#pragma once

#include <stdint.h>

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

const int64_t REG_STICKY[] = {
0, // ZERO
1, // TBC
1, // CSA
1, // CFS
1, // CSR
0, // SAR
0, // I1D
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
0, // VSR
0, // RPT
0, // ERPT
0, // EPRD
0, // RESERVED0
0, // RESERVED1
0, // RESERVED2
0, // RESERVED3
0, // TOTAL_REG
};

const int REG_DEFAULT[] = {
0, // ZERO
1, // TBC
0, // CSA
0, // CFS
0, // CSR
0, // SAR
0, // I1D
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
-1, // BR
0, // BSR
0, // OFL
0, // VSR
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
  DP_PostStreamZero,    	// Pad at the end of whole stream with zero
  DP_PostStreamPredOff, 	// Pad at the end of whole stream with invalid value
  DP_Post2DStreamZero,  	// Pad at the end of 2D stream with zero
  DP_Post2DStreamPredOff,	// Pad at the end of 2D stream with invalid value
  DP_PostStrideZero,		// Pad at the end of 1D stream with zero
  DP_PostStridePredOff,		// Pad at the end of 1D stream with invalid value
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
  DMO_Add,
  DMO_Sub,
  DMO_Mul,
  DMO_Min,
  DMO_Max,
  DMO_Unkown,
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
