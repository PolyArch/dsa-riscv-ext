/*!
 * \file spec.h
 * @author PolyArch Research Group 
 * \brief The default tech spec of the reconfigurable accelerator.
 *        These macros can be override by the compiler flags.
 * \date 2020-12-26
 * @copyright Copyright (c) 2020
 */

#pragma once

#include <cstdint>

/*!
 * \brief The max number of DSA lanes supported
 */
#ifndef DSA_XLEN
#define DSA_XLEN 64
#endif

/*!
 * \brief The max number of bits to encode a port.
 */
#ifndef MAX_PORT_BITS
#define MAX_PORT_BITS 7
#endif

/*!
 * \brief The max number of ports that can be managed by a host core.
 */
#ifndef DSA_MAX_PORTS
#define DSA_MAX_PORTS (1 << 7)
#endif

/*!
 * \brief The max number of DSA input ports across all the lanes supported
 */
#ifndef DSA_MAX_IN_PORTS
#define DSA_MAX_IN_PORTS DSA_MAX_PORTS
#endif

/*!
 * \brief The max number of DSA output ports across all the lanes supported
 */
#ifndef DSA_MAX_OUT_PORTS
#define DSA_MAX_OUT_PORTS DSA_MAX_PORTS
#endif

/*!
 * \brief The bit granularity of the DSA
 */
#ifndef DSA_GRANULARITY
#define DSA_GRANULARITY 8
#endif

/*!
 * \brief The power of two of composabilities.
 */
#ifndef DSA_SUB_LANES
#define DSA_SUB_LANES 3
#endif

/*!
 * \brief The fixed digital point of port repeat.
 */
#ifndef DSA_REPEAT_DIGITAL_POINT
#define DSA_REPEAT_DIGITAL_POINT 4
#endif

/// {

typedef uint64_t addr_t;

#define SBDT uint64_t           //cgra datatype
#define SSWORD uint8_t          //dgra datatype
#define DATA_WIDTH sizeof(SBDT)
#define SCRATCH_SIZE (16384) //size in bytes -- 16KB
// #define SCRATCH_SIZE (32768) //size in bytes -- 16KB
#define SPU_NET_PACKET_SIZE 64
#define NUM_SCRATCH_BANKS 64

#define LSCRATCH_SIZE (1 << 20) //size in bytes -- 16KB
#define NUM_SCRATCH_BANKS 64
#define MAX_BANK_BUFFER_SIZE 64 // 8
// #define NUM_SPU_CORES 64 // for global address space

#define SB_TIMING

#define DEFAULT_FIFO_LEN 15
#define DEFAULT_IND_ROB_SIZE 8


#define SD_TRANSFERS_ALLOWED 22

#define MEM_WIDTH (64)
#define MEM_MASK ~(MEM_WIDTH-1)

#define SCR_WIDTH (64)
#define SCR_MASK ~(SCR_WIDTH-1)

#define PORT_WIDTH (64)
#define VP_LEN (64)

#define MAX_MEM_REQS (100)
#define CGRA_FIFO_LEN (32)

#define NUM_IN_PORTS  (32)
#define NUM_OUT_PORTS (32)

#define NET_ADDR_PORT (21)
#define NET_VAL_PORT (22)

#define ATOMIC_ADDR_PORT (27)
#define ATOMIC_BYTES_PORT (28)
#define BYTES_PORT_DATA_WIDTH (2)
#define ATOMIC_ADDR_DATA_WIDTH (2)
#define MAX_ATOM_REQ_QUEUE_SIZE (64)

// #define NET_ADDR_PORT (25)
// #define NET_VAL_PORT (32)

//Convenience ports for these functions
// #define MEM_SCR_PORT (23)
// #define SCR_MEM_PORT (24)

#define REPEAT_FXPNT (3)

#define MAX_WAIT (1000) //max cycles to wait for forward progress

#define MEM_WR_STREAM (127)
#define CONFIG_STREAM (128)

#define NUM_GROUPS (8)
#define NUM_TASK_DEP_CHARAC 6
#define NUM_TASK_TYPE_CHARAC 4

//bit std::vectors for sb_wait
#define WAIT_SCR_WR       1 //wait for just scratch
#define WAIT_CMP          2 //wait for everything to complete
#define WAIT_SCR_RD       4 //wait for all reads to complete (not impl)
#define WAIT_SCR_RD_Q     8 //wait for all reads to be de-queued (not impl)
#define WAIT_MEM_WR       16//wait for all writes to complete (not impl)
#define WAIT_SCR_ATOMIC   32//wait for all atomics to be done, delay the core
#define WAIT_SCR_WR_DF    64//wait for N remote writes to be done, delay the core
#define GLOBAL_WAIT       128//wait for all cores (threads) to be done
#define STREAM_WAIT       66//wait only for streams to be done


//fill modes
#define NO_FILL                0
#define POST_ZERO_FILL         1
#define PRE_ZERO_FILL          2
#define STRIDE_ZERO_FILL       3
#define STRIDE_DISCARD_FILL    4

//datatype encodings
#define T64 0
#define T32 1
#define T16 2
#define T08 3

#define NO_PADDING (~0ull)

/// }

namespace dsa {

struct Specification {
#define SPEC_ATTR(TY, ID, VAL) TY ID{VAL};
#include "./spec.attr"
#undef SPEC_ATTR
};

}
