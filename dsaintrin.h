/*!
 * \file dsaintrin.h
 * \author PolyArch Research Lab
 * \copyright Copyright (c) 2020
 */

#pragma once

#include "dsa/spec.h"
#include "dsa/rf.h"

// Magic sentinal for matching
#define SENTINAL (((uint64_t)1)<<63)
#define SENTINAL16 (((uint16_t)1)<<15)
#define SENTINAL32 (((uint32_t)1)<<31)


/*!
 * \brief The immediate value version of SS_CONTEXT.
 * \param bitmask: The bit mask of configuration.
 *                 We can use environment variable LANES=n to configure the number of lanes.
 *                 By default, it is 8.
 */
#define SS_CONTEXT_I(bitmask) \
  __asm__ __volatile__("ss_ctx x0, x0, %0" : : "i"(bitmask))

//Mask for accessing shared scratchpad
#define SHARED_SP 0x100
#define SHARED_SP_INDEX 8

/*!
 * \brief Configure the predicate of control code broadcasting as well as the offset of
 *        the SIMT like offset. When instantiating a memory stream, the starting address
 *        will be (address+offset*#lane).
 * \param bitmask: The bit mask of broadcasting.
 * \param offset: The memory offset.
 */
#define SS_CONTEXT_OFFSET(bitmask,offset) \
  __asm__ __volatile__("ss_ctx x0, %0, %1" : : "r"(offset), "i"(bitmask))

#define _CONFIG_PARAM_IMPL(idx1, val1, s1, idx2, val2, s2)                               \
  do {                                                                                   \
    int mask = (idx1) | ((int)(idx2) << 5) | ((s1) << 10) | ((s2 << 11));                \
    __asm__ __volatile__("ss_cfg_param %0, %1, %2" : : "r"(val1), "r"(val2), "i"(mask)); \
  } while (false)

/*! brief log of 2. Do give a perfect power of 2. */
#define _LOG2(x) ((x) ? ((31) - __builtin_clz((uint32_t)(x))) : 0)

/*! \brief Config the data type of the on coming stream. */
#define CONFIG_DTYPE(direct, indirect)                       \
  do {                                                       \
    int direct_ = _LOG2((direct) / DSA_ADDRESSABLE_MEM);     \
    int indirect_ = _LOG2((indirect) / DSA_ADDRESSABLE_MEM); \
    uint64_t value = (direct_) | ((indirect_) << 2);         \
    _CONFIG_PARAM_IMPL(DSARF::CSR, value, 1, 0, 0, 0);       \
  } while (false)

/*!
 * \brief Configure the predicate of control code broadcasting.
 * \param bitmask: The bit mask of configuration.
 *                 We can use environment variable LANES=n to configure the number of lanes.
 *                 By default, it is 8.
 * \note The number of each lane is 0-based, i.e. 0..(n-1). The n-th lane is the shared
 *       scratchpad.
 */
#define SS_CONTEXT(bitmask) \
  _CONFIG_PARAM_IMPL(DSARF::TBC, bitmask, 1, DSARF::ZERO, 0)

/*!
 * \brief A wrapper of SS_CONTEXT to configure the predicate of a specific lane.
 * \param core_id: The number of the lane to be enabled
 */
#define SS_SET_ACCEL(core_id) \
  SS_CONTEXT(1 << (core_id))

/*!
 * \brief Configure the spatial architecture.
 *        This is generated by the spatial scheduler.
 * \param addr: The array of bitstream of spatial architecture configuration.
 * \param size: The size of the configuration array in bytes.
 */
#define SS_CONFIG(addr, size) \
  _CONFIG_PARAM_IMPL(DSARF::CSA, addr, 0, DSARF::CFS, size, 0)

/*!
 * \brief Drop all the ongoing data request while retaining the configuration.
 */
#define SS_RESET() SS_CONFIG(0, 0)

/*!
 * \brief Reset all live streams after finishing all ports
 */
#define SS_STREAM_RESET() SS_CONFIG(0, 1)


/*!
 * \brief Set the registers that related to 1-d stream.
 * \param start Register SAR
 * \param length Register L1D
 */
#define _CONFIG_1D_STREAM(start, length) \
  _CONFIG_PARAM_IMPL(DSARF::SAR, start, 0, DSARF::L1D, length, 0)

/*!
 * \brief Instantiate a linear stream
 * \param port The source/destination port
 * \param padding The mode of padding. Refer rf.h:Padding for more details.
 *                If it is a write stream, this is useless. Use 0 as a placeholder.
 * \param action 0: access; 1: generate the affine linear value sequence to the port.
 * \param dimension (d+1)=the number of dimensions of the stream.
 *        For now, 1,2,and 3-d are supported.
 * \param operation 0: read, 1: write, 2-7: atomic +, -, *, /, min, and max.
 * \param memory 0: memory, 1: spad.
 */
#define _LINEAR_STREAM_MASK(port, padding, action, dimension, operation, memory, signal)  \
  ((((port) & 127) << 12) | (((padding) & 1) << 9) | (((action) & 1) << 8) |              \
   (((dimension) & 3) << 6) | (((operation) & 7) << 3) | ((memory) & 1) << 2) | ((signal) & 3)

enum MemoryOperation {
  Read,
  Write,
  AtomAdd,
  AtomSub,
  AtomMul,
  AtomDiv,
  AtomMax,
  AtomMin,
};

enum StreamAction {
  Access,
  Generate
};

enum MemoryType {
  DMA,
  SPAD
};

#define _INSTANTIATE_1D_STREAM(addr, bytes, port, padding, action, operation, memory) \
  do {                                                                                \
    int64_t bytes_ = bytes;                                                           \
    int signal = bytes_ < 0 ? -1 : bytes_ > 0;                                        \
    bytes_ = bytes_ < 0 ? -bytes_ : bytes_;                                           \
    CONFIG_DTYPE(DSA_ADDRESSABLE_MEM, 0);                                             \
    _CONFIG_1D_STREAM(addr, bytes_ / DSA_ADDRESSABLE_MEM);                            \
    uint64_t value =                                                                  \
       _LINEAR_STREAM_MASK(port, padding,                                             \
                           action, /*1d*/0, operation, memory, signal);               \
    __asm__ __volatile__("ss_lin_strm %0" : : "r"(value));                            \
  } while (false)

/*!
 * \brief addr[0:bytes] -> port
 */
#define SS_DMA_1D_READ(addr, bytes, port, padding)     \
    _INSTANTIATE_1D_STREAM(addr, bytes, port, padding, \
                           StreamAction::Access,       \
                           MemoryOperation::Read,      \
                           MemoryType::DMA);

/*!
 * \brief port -> addr[0:bytes]
 */
#define SS_DMA_1D_WRITE(port, addr, bytes)       \
  _INSTANTIATE_1D_STREAM(addr, bytes, port,      \
                         Padding::NoPadding,     \
                         StreamAction::Access,   \
                         MemoryOperation::Write, \
                         MemoryType::DMA)


#define _CONFIG_2D_STREAM(addr, stride, bytes, stretch, n) \
  do {                                                     \
    auto bytes_ = (bytes);                                 \
    auto stride_ = (stride) / DSA_ADDRESSABLE_MEM;         \
    auto stretch_ = (stretch) / DSA_ADDRESSABLE_MEM;       \
    _CONFIG_1D_STREAM(addr, bytes_);                       \
  	_CONFIG_PARAM_IMPL(DSARF::E2D, stretch_, 0,            \
                       DSARF::L2D, n, 0);                  \
  	_CONFIG_PARAM_IMPL(DSARF::I2D, stride_, 0,             \
                       0, 0, 0);                           \
  } while (false)

#define _INSTANTIATE_2D_STREAM(addr, stride, bytes, stretch, n, port, padding, action, op, mem) \
  do {                                                                                          \
    _CONFIG_2D_STREAM(addr, stride, bytes, stretch, n);                                         \
    int64_t bytes_ = bytes;                                                                     \
    int signal = bytes_ < 0 ? -1 : bytes_ > 0;                                                  \
    uint64_t value = _LINEAR_STREAM_MASK(port, padding,                                         \
                                         action, /*2d*/1, op, mem, signal);                     \
    __asm__ __volatile__("ss_lin_strm %0" : : "r"(value));                                      \
  } while (false)

/*!
 * \brief Instantiate a 2-d DMA read stream.
 */
#define SS_DMA_2D_READ(addr, stride, bytes, stretch, n, port, padding)   \
  _INSTANTIATE_2D_STREAM(addr, stride, bytes, stretch, n, port, padding, \
                         StreamAction::Access,                           \
                         MemoryOperation::Read,                          \
                         MemoryType::DMA)
/*!
 * \brief Legacy wrapper of a 2-d stream with stretch.
 */
#define SS_DMA_READ_STRETCH(addr, stride, acc_size, stretch, n, port ) \
  SS_DMA_2D_READ(addr, stride, acc_size, stretch, n, port, 0)

/*!
 * \brief Legacy wrapper of a 2-d stream without stretch.
 */
#define SS_DMA_READ(addr, stride, bytes, n, port) \
  SS_DMA_2D_READ(addr, stride, bytes, 0, n, port, 0)

/*!
 * \brief Instantiate a 2-d DMA write stream.
 */
#define SS_DMA_2D_WRITE(addr, stride, bytes, stretch, n, port)  \
  _INSTANTIATE_2D_STREAM(addr, stride, bytes, stretch, n, port, \
                         Padding::NoPadding,                    \
                         StreamAction::Access,                  \
                         MemoryOperation::Write,                \
                         MemoryType::DMA)

/*!
 * \brief This is a wrapper for DMA_WR_INNER and DMA_WR_OUTER to keep bardward compatibility.
 */
#define SS_DMA_WRITE(port, stride, bytes, n, addr) \
   SS_DMA_2D_WRITE(addr, stride, bytes, 0, n, port)


/*!
 * \brief Discard a num_elem*elem_size bytes of data from the specific port.
 * \param output_port: The port to discard the value.
 * \param num_elem: The number of elements to discard.
 * \param elem_size: The data size of each element.
 */
#define SS_GARBAGE_GENERAL(output_port, num_elem, elem_size) \
  do { \
    int imm = (output_port) << 1; \
    __asm__ __volatile__("ss_wr_dma %0, %1, %2" : : "r"(0), "r"((num_elem) * (elem_size)), "i"(imm)); \
  } while (false)

/*!
 * \brief This is a wrapper for GARBAGE_GENERAL to keep the backward compatibility with
 *        the decomposability.
 * \param num_elem: The number of q-word to discard.
 */
#define SS_GARBAGE(output_port, num_elem) \
  SS_GARBAGE_GENERAL(output_port, num_elem, 8)

/*! \brief Insert a barrier for the accelerator. Refer rf.h to see the masks. */
#define SS_WAIT(mask) __asm__ __volatile__("ss_wait x0, %0" : : "i"(mask))

/*! \brief Block the control host and wait everything done on the accelerator. */
#define SS_WAIT_ALL() SS_WAIT(~0ull)

/*!
 * \brief Configure repeat register of the next instantiated input stream.
 * \param port The port to be configured.
 * \param stretch The field of the port to be configured.
 * \param value The value to the field to be set
 */
#define SS_CONFIG_PORT(port, field, value)                                \
  do {                                                                    \
    uint64_t mask = port;                                                 \
    mask <<= 1;                                                           \
    mask = (mask << 4) | (field);                                         \
    __asm__ __volatile__("ss_cfg_port %0, %1" : : "r"(value), "i"(mask)); \
  } while (false)

#define PORT_BROADCAST      0
#define PORT_REPEAT         1
#define PORT_REPEAT_STRETCH 2
#define PORT_STRETCH_PERIOD 3

/*! \brief The next stream instantiated from this port will be repeated n times. */
#define SS_REPEAT_PORT(port, n) SS_CONFIG_PORT(port, PORT_REPEAT, n)


/*!
 * \brief The semantics is similar to DMA_READ_STRETCH but for scratchpad read.
 */
#define SS_SCR_PORT_STREAM_STRETCH(scr_addr,stride,acc_size,stretch,n_strides, port) \
  do {                                                                               \
    if (acc_size > 0) {                                                              \
      SS_SCR_RD_OUTER(stride, n_strides, stretch);                                   \
      SS_SCR_RD_INNER(scr_addr, acc_size, port);                                     \
    } else {                                                                         \
      int _addr = scr_addr + acc_size;                                               \
      int _outer_cnt = n_strides;                                                    \
      SS_SCR_RD_OUTER(stride, n_strides, stretch);                                   \
      SS_SCR_RD_INNER(_addr, -acc_size, port);                                       \
    }                                                                                \
  } while (false)

/*!
 * \brief The semantics is similar to DMA_READ but for scratchpad read.
 */
#define SS_SCR_PORT_STREAM(scr_addr,stride,acc_size,n_strides, port) \
   SS_SCR_PORT_STREAM_STRETCH(scr_addr,stride,acc_size,0,n_strides, port) 

/*!
 * \brief This is a wrapper for SCR_PORT_STREAM to keep backward compatibility.
 */
#define SS_SCRATCH_READ(scr_addr, n_bytes, port) \
  SS_SCR_PORT_STREAM_STRETCH(scr_addr,0,n_bytes,0,1, port) 

/*!
 * \brief This is similar to DMA_RD_INNER but for scratchpad write.
 */
#define SS_SCR_WR_INNER(addr, acc_size, port) \
  __asm__ __volatile__("ss_wr_scr %0, %1, %2" : : "r"(addr), "r"(acc_size), "i"((port) << 2))

/*!
 * \brief This is similar to DMA_RD_OUTER but for scratchpad write.
 */
#define SS_SCR_WR_OUTER(stride, n, stretch) \
  __asm__ __volatile__("ss_wr_scr %0, %1, %2" : : "r"(stride), "r"(n), "i"((stretch) << 2 | 1))

/*!
 * \brief This is a wrapper for SCR_WR_INNER and SCR_WR_OUTER to keep backward compatibility.
 * \param output_port: The data source of the output port.
 * \param num_bytes: The number of bytes from the port to be written to the scratchpad.
 * \param scr_addr: The starting address on the scratchpad.
 */
#define SS_SCR_WRITE(output_port, num_bytes, scr_addr) \
  SS_SCR_WR_INNER(scr_addr, num_bytes, output_port)

/*!
 * \brief Configure the value of the iteration register.
 * \param n: The number to be set.
 *           This will be used to pad recurrence streams, and configure the iterations of 2D_CONST (see below).
 */
#define SS_SET_ITER(n) \
  __asm__ __volatile__("ss_set_iter %0 " : : "r"(n))

/*!
 * \brief This is a legacy wrapper to read data from the memory to the scratchpad.
 *        We used to have dedicated implementation for memory-scratchpad communication,
 *        but now we use a accelerator port (MEM_SCR_PORT) to coordinate the data transfer.
 */
// TODO(@were): What's the `shr' for?
#define SS_DMA_SCRATCH_LOAD_GENERAL(mem_addr, stride, acc_size, stretch, n_strides, scr_addr, shr) \
  SS_DMA_READ_STRETCH(mem_addr, stride, acc_size, stretch, n_strides, MEM_SCR_PORT);               \
  SS_SCR_WRITE(MEM_SCR_PORT, acc_size * n_strides, scr_addr)

/*!
 * \brief This is a wrapper for SS_DMA_SCRATCH_LOAD_GENERAL to keep backward compatibility.
 */
#define SS_DMA_SCRATCH_LOAD_STRETCH(mem_addr, stride, acc_size, stretch, n_strides, scr_addr) \
  SS_DMA_SCRATCH_LOAD_GENERAL(mem_addr, stride, acc_size, stretch, n_strides, scr_addr, 0)

/*!
 * \brief This is a wrapper for SS_DMA_SCRATCH_LOAD_STRETCH to keep backward compatibility.
 */
#define SS_DMA_SCRATCH_LOAD(mem_addr, stride, acc_size, n_strides, scr_addr) \
  SS_DMA_SCRATCH_LOAD_STRETCH(mem_addr,stride, acc_size, 0, n_strides, scr_addr)

/*!
 * \brief This is a legacy wrapper to write data from the scratchpad to the memory.
 *        We used to have dedicated implementation for memory-scratchpad communication,
 *        but now we use a accelerator port (MEM_SCR_PORT) to coordinate the data transfer.
 */
// TODO(@were): What's the `shr' for?
#define SS_SCRATCH_DMA_STORE_GENERAL(scr_addr, stride, acc_size, num_strides, mem_addr, shr) \
  SS_SCR_PORT_STREAM_STRETCH(scr_addr,stride,acc_size,0,num_strides, SCR_MEM_PORT); \
  SS_DMA_WRITE(SCR_MEM_PORT, acc_size*num_strides, acc_size*num_strides, 1, mem_addr)

/*!
 * \brief This is a wrapper for SCRATCH_DMA_STORE_GENERAL to keep backward compatibility.
 */
#define SS_SCRATCH_DMA_STORE(scr_addr, stride, access_size, num_strides, mem_addr) \
  SS_SCRATCH_DMA_STORE_GENERAL(scr_addr, stride, access_size, num_strides, mem_addr, 0)

/*!
 * \brief This is a wrapper for SCRATCH_DMA_STORE_GENERAL to keep backward compatibility.
 */
// TODO(@were): What's the `shr' for?
#define SS_SCRATCH_STORE_REMOTE(scr_addr, stride, access_size, num_strides, mem_addr) \
  SS_SCRATCH_DMA_STORE_GENERAL(scr_addr, stride, access_size, num_strides, mem_addr, 1)

// TODO(@were): Confirm the semantics with @vidushi.
#define SS_SCRATCH_LOAD_REMOTE(remote_scr_addr, stride, acc_size, stretch, n_strides, scr_addr) \
  SS_DMA_SCRATCH_LOAD_GENERAL(remote_scr_addr, stride, acc_size, stretch, n_strides, scr_addr, 1)

// TODO(@were): Fix the buffet functionality.
#define SS_BUFFET_ALLOCATE(start, buffer_size, total_bytes, port) \
  __asm__ __volatile__("ss_wr_scr %0, %1, %2" : : "r"(((uint64_t)start << 32ull) | buffer_size), "r"(total_bytes), "i"((port) << 2 | 2))

// TODO(@were): Fix the buffet functionality.
#define SS_SCR_RD_INNER_BUFFET(size, inport, inport_dtype, shadowport, shadow_dtype) \
  __asm__ __volatile__("ss_scr_rd %0, %1, %2" : : "r"(shadowport | ((shadow_dtype + 1) << 5) | ((inport_dtype + 1) << 8)), "r"(size), "i"(inport << 2 | 2))

/*!
 * \brief Write several consts to initialize the scratchpad.
 * \param scr_addr: The starting address on the scratchpad.
 * \param val: The value to be fed on the scratchpad.
 * \param num_elements: The number of elements to be written onto the scratchpad.
 * \param data_width: The data type of the value.
 */
// TODO(@were): Can we make it a wrapper of DCONST and SCRATCH_WRITE?
#define SS_CONST_SCR(scr_addr, val, num_elements, data_width)                                     \
  do {                                                                                            \
    __asm__ __volatile__("ss_set_iter %0 " : : "r"(num_elements));                                \
    __asm__ __volatile__("ss_const_scr %0, %1, %2" : : "r"(scr_addr), "r"(val), "i"(data_width)); \
  } while (false)

// TODO(@were): Confirm the semantics with @vidushi.
#define SS_ATOMIC_DFG_CONFIG(dfg_addr_cons, dfg_val_cons, dfg_val_out) \
  __asm__ __volatile__("ss_atom_op %0, %1, %2" : : "r"(dfg_addr_cons), "r"(dfg_val_cons), "i"(dfg_val_out << 1 | 0))

/*!
 * \brief Write a value from CGRA to the register file.
 * \param out_port: The source port.
 * \param val: A lvalue reference where the value is written to.
 */
#define SS_RECV(out_port, val) \
  __asm__ __volatile__("ss_recv %0, a0, %1 " : "=r"(val) : "i"(out_port))

/*!
 * \brief Feed several consts to a port.
 * \param port: The destination port.
 * \param val: The value to be fed. The data type is 8-byte.
 * \param num_elements: The number of elements to be fed to the port.
 */
#define SS_CONST(port, val, num_elements) \
  __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val), "r"(num_elements), "i"(port|(0<<8)))

/*!
 * \brief Feed several consts to a port.
 * \param port: The destination port.
 * \param val: The value to be fed. The data type is 8-byte.
 * \param num_elements: The number of elements to be fed to the port.
 * \param const_width: The data type of the constant.
 */
#define SS_DCONST(port, val, num_elements, const_width) \
  __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val), "r"(num_elements), "i"(port | ((const_width + 1) << 9)))

/*!
 * \brief Periodically feed two consts to a port. [(val1 x v1_repeat), (val2 x v2_repeat)] x iters
 * \param port: The destination port.
 * \param val1: The first value to be fed. The data type is 8-byte.
 * \param v1_repeat: The repeat times of val1.
 * \param val2: The second value to be fed. The data type is 8-byte.
 * \param v2_repeat: The repeat times of val2.
 * \param iters: The times of repeating the periods.
 */
#define SS_2D_CONST(port, val1, v1_repeat, val2, v2_repeat, iters)                                \
  do {                                                                                            \
    __asm__ __volatile__("ss_set_iter %0 " : : "r"(iters));                                       \
    __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val1), "r"(v1_repeat), "i"(port|(1<<7))); \
    __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val2), "r"(v2_repeat), "i"(port|(1<<6))); \
  } while (false)

// TODO(@were): It seems to have some mal-function here. Confirm with @vidushi.
#define SS_2D_DCONST(port, val1, v1_repeat_port, val2, v2_repeat_port, iters)                                 \
  do {                                                                                                        \
    __asm__ __volatile__("ss_set_iter %0 " : : "r"(iters));                                                   \
    __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val1), "r"(v1_repeat_port), "i"(port|(1<<7)|(1<<8))); \
    __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val2), "r"(v2_repeat_port), "i"(port|(1<<6)|(1<<8))); \
  } while (false)

//Send a constant value, repetated num_elements times to a port
//Plain Write to Scratch
// #define SS_2D_DCONST(port, val1, v1_repeat, val2, v2_repeat, iters, dtype) \
//   __asm__ __volatile__("ss_set_iter %0 " : : "r"(iters)); \
//   __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val1), "r"(v1_repeat), "i"(port|(1<<7) | ((dtype+1) << 8))); \
//   __asm__ __volatile__("ss_const %0, %1, %2 " : : "r"(val2), "r"(v2_repeat), "i"(port|(1<<6) | ((dtype+1) << 8))); 

/*!
 * \brief Configure repeat register of the next instantiated input stream.
 *        Instead of having a constant repeating times, it comes from a port.
 * \param times_port: The source port of repeating times.
 */
#define SS_VREPEAT_PORT(times_port) \
  __asm__ __volatile__("ss_cfg_port %0, t0, %1" : : "r"(times_port), "i"(1))

/*!
 * \brief Forward value from output port to the input port.
 * \param output_port: The data source port.
 * \param input_port: The destination data port.
 * \param num_strides: The number of data forwarded.
 */
// TODO(@were):  I need clearer specification on the data type.
#define SS_RECURRENCE(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, zero, %1" : : "r"(num_strides), "i"((input_port<<6) | (output_port)))

/*!
 * \brief Forward value from output port to the input port with data padding.
 *        The padding stride is determined by the iteration register. See SET_ITER.
 * \param output_port: The data source port.
 * \param input_port: The destination data port.
 * \param num_strides: The number of data forwarded.
 */
#define SS_RECURRENCE_PAD(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, %1, %2" : : "r"(num_strides), "r"(4), "i"((input_port<<6) | (output_port)))

/*!
 * \brief Transfer value from current lanes' source port to the adjacent left lane's destination port.
 *        All the lanes are one a ring, i.e. the 8th lanes right is the 1st lane.
 * \param output_port: The source port in the current lane.
 * \param input_port: The destination port in the right lane.
 * \param num_strides: The number of elements to be transfered.
 */
#define SS_XFER_LEFT(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, %1, %2" : : "r"(num_strides), "r"(1), "i"((input_port<<6) | (output_port)))

/*!
 * \brief Transfer value from current lanes' source port to the adjacent right lane's destination port.
 *        All the lanes are one a ring, i.e. the 8th lanes right is the 1st lane.
 * \param output_port: The source port in the current lane.
 * \param input_port: The destination port in the right lane.
 * \param num_strides: The number of elements to be transfered.
 */
#define SS_XFER_RIGHT(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, %1, %2" : : "r"(num_strides), "r"(2), "i"((input_port<<6) | (output_port)))

/*!
 * \brief Transfer value from current lanes' source port to the adjacent left lane's destination port.
 *        All the lanes are one a ring, i.e. the 8th lanes right is the 1st lane. Padding will be injected
 *        with respect to FILL_MODE and ITER.
 * \param output_port: The source port in the current lane.
 * \param input_port: The destination port in the right lane.
 * \param num_strides: The number of elements to be transfered.
 */
#define SS_XFER_LEFT_PAD(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, %1, %2" : : "r"(num_strides), "r"(1 | 4), "i"((input_port<<6) | (output_port)))

/*!
 * \brief Transfer value from current lanes' source port to the adjacent right lane's destination port.
 *        All the lanes are one a ring, i.e. the 8th lanes right is the 1st lane. Padding will be injected
 *        with respect to FILL_MODE and ITER.
 * \param output_port: The source port in the current lane.
 * \param input_port: The destination port in the right lane.
 * \param num_strides: The number of elements to be transfered.
 */
#define SS_XFER_RIGHT_PAD(output_port, input_port, num_strides) \
  __asm__ __volatile__("ss_wr_rd %0, %1, %2" : : "r"(num_strides), "r"(2 | 4), "i"((input_port<<6) | (output_port)))

// TODO(@were): Confirm the semantics with @vidushi
#define SS_REM_PORT(output_port, num_elem, mask, remote_port) \
  __asm__ __volatile("ss_rem_port %0, %1, %2" : : "r"(num_elem), "r"(mask), "i"(((output_port<15?output_port:output_port-32)<<7) | (0<<6) | (remote_port<<1) | (0)))

// TODO(@were): Confirm the semantics with @vidushi
#define SS_IND_REM_SCRATCH(val_port, addr_port, num_elem, scr_base_addr, scratch_type); \
  __asm__ __volatile("ss_rem_port %0, %1, %2" : : "r"(num_elem), "r"(scr_base_addr), "i"((val_port<<7) | (scratch_type<<6) | (addr_port<<1) | (1)))

// TODO(@were): Confirm the semantics with @vidushi
#define SS_REM_SCRATCH(scr_base_addr, stride, access_size, num_strides, val_port, scratch_type) \
  __asm__ __volatile__("ss_stride   %0, %1, 0" : : "r"(stride), "r"(access_size)); \
  __asm__ __volatile("ss_rem_port %0, %1, %2" : : "r"(num_strides), "r"(scr_base_addr), "i"((val_port<<7) | (scratch_type<<6) | (0<<1) | (1)))

// TODO(@were): Confirm the semantics with @vidushi
// banked scratchpad: scr->port, port->remote scr
// TODO(@vidushi): remove scratch_type later (for now, I make the immediate negative)
#define SS_SCR_REM_SCR(src_scr_base_addr, stride, access_size, num_strides, dest_scr_base_addr, scratch_type) \
  SS_SCR_PORT_STREAM(src_scr_base_addr, stride, access_size, num_strides, SCR_SCR_PORT); \
  SS_REM_SCRATCH(dest_scr_base_addr, stride, access_size, num_strides, (SCR_SCR_PORT-32), scratch_type)

// TODO(@were): Confirm the semantics with @vidushi
#define SS_SCR_REM_PORT(scr_base_addr, num_strides, mask, remote_port) \
  SS_SCRATCH_READ(scr_base_addr, num_strides, SCR_REM_PORT); \
  SS_REM_PORT((SCR_REM_PORT-32), num_strides, mask, remote_port)

// Datatype Encodings
#define T64 0
#define T32 1
#define T16 2
#define T08 3
#define TS 4

// new val_type is val_type*val_num bytes
// #define SS_CONFIG_ATOMIC_SCR_OP(addr_type, val_type, output_type, val_num) \
//   __asm__ __volatile__("ss_cfg_atom_op %0, t0, %1" : : "r"(val_num), "i"( ((val_type<<4)&0x1ADB0 | (output_type<<2)&0x44C | (addr_type)&0x3)))

/*!
 * \brief Configure the indirect atomic operation. Something like a[b[i]] += c[i].
 * \param addr_type: The data type of the address value (b)
 * \param val_type: The data type of the operand (c)
 * \param output_type: The data type of the result (a)
 * \param val_num: sizeof(val_type) * val_num together determine the number of bytes to read.
 * \param num_dates: The number of outputs
 * \param is_update_port: Where the operation happens, 0: near storage units, 1: CGRA.
 */
#define SS_CONFIG_ATOMIC_SCR_OP(addr_type, val_type, output_type, val_num, num_updates, is_update_port)      \
  do {                                                                                                       \
    if (is_update_port == 1) {                                                                               \
      auto __cfg_imm__ = ((val_type<<4)&0x1ADB0 | (output_type<<2)&0x44C | (addr_type)&0x3);                 \
      auto __update__  = num_updates | 1<<16;                                                                \
      __asm__ __volatile__("ss_cfg_atom_op %0, %1, %2" : : "r"(val_num), "r"(__update__), "i"(__cfg_imm__)); \
    } else {                                                                                                 \
      auto __update__ = num_updates | 0<<16;                                                                 \
      auto __cfg_imm__ = ((val_type<<4)&0x1ADB0 | (output_type<<2)&0x44C | (addr_type)&0x3);                 \
      __asm__ __volatile__("ss_cfg_atom_op %0, %1, %2" : : "r"(val_num), "r"(__update__), "i"(__cfg_imm__)); \
    }                                                                                                        \
  } while(false)


/*!
 * \brief Call after CONFIG_ATOMIC_SCR_OP to instantiate an atomic update stream.
 *        Something like a[b[i]] += c[i].
 * \param addr_port: The data source of address of indices (b).
 * \param val_port: The data source of operands (c).
 * \param offset: TODO(@were)
 * \param iters: The number of iterations.
 * \param opcode: The operation performed in the near storage FU.
 *                TODO(@were): What happens when something happens in CGRA (is_update).
 */
#define SS_ATOMIC_SCR_OP(addr_port, val_port, offset, iters, opcode) \
  __asm__ __volatile__("ss_atom_op %0, %1, %2" : : "r"(offset | addr_port << 24), "r"(iters), "i"((val_port<<2) | opcode))

/*!
 * \brief Configure an indirect read stream. Something like a[b[i]*k].
 * \param itype: The data type of the indices (b).
 * \param dtype: The data type of the value (a).
 * \param mult: The coefficient of the indices (k).
 * \param num_elem: num_elem * sizeof(dtype) together determine how many continous bytes to read each time.
 * \param offset_list: A list (up to 4) of memory offset of accessing data.
 */
#define SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem,offset_list)  \
  __asm__ __volatile__("ss_cfg_ind %0, %1, %2" : : "r"(offset_list), "r"(num_elem), "i"( (mult<<4) | (itype<<2)  |  (dtype<<0) )  )

#define SS_CONFIG_INDIRECT(itype,dtype,mult,num_elem) \
  SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem,0)
#define SS_CONFIG_INDIRECT1(itype,dtype,mult,num_elem,o1) \
  SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem, (o1))
#define SS_CONFIG_INDIRECT2(itype,dtype,mult,num_elem,o1,o2) \
  SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem, (o1) | (o2) << 8)
#define SS_CONFIG_INDIRECT3(itype,dtype,mult,num_elem,o1,o2,o3) \
  SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem, (o1) | (o2) << 8 | (o3) << 16)
#define SS_CONFIG_INDIRECT4(itype,dtype,mult,num_elem,o1,o2,o3,o4) \
  SS_CONFIG_INDIRECT_GENERAL(itype,dtype,mult,num_elem, (o1) | (o2) << 8 | (o3) << 16 | (o4) << 24)

/*!
 * \brief Call after CONFIG_INDIRECT to instantiate an indirect stream.
 * \param ind_port: The source port of indices.
 * \param addr_offset: The starting address of the array.
 * \param num_elem: The length of the address stream.
 * \param input_port: The destination port of this stream.
 */
#define SS_INDIRECT(ind_port, addr_offset, num_elem, input_port)                    \
  __asm__ __volatile__("ss_stride   %0, %1, %2" : : "r"(8), "r"(8), "i"((0<<10)));  \
  __asm__ __volatile__("ss_ind    %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),  \
                                                  "i"((input_port<<5) | (ind_port)))

/*!
 * \brief Call after CONFIG_INDIRECT to instantiate an indirect stream with unfixed length.
 * \param ind_port: The source port of indices.
 * \param addr_offset: The starting address of the array.
 * \param stride: The stride after reading access_size bytes.
 * \param access_size: The continous bytes to read.
 * \param num_elem_port: The port generates the length of each stream.
 * \param input_port: The destination of this stream.
 */
#define SS_INDIRECT_2D(ind_port, addr_offset, num_elem, stride, access_size, num_elem_port, input_port)           \
  __asm__ __volatile__("ss_stride   %0, %1, %2" : : "r"(stride), "r"(access_size), "i"(num_elem_port | (1<<10))); \
  __asm__ __volatile__("ss_ind    %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),                                \
                                                  "i"((input_port<<5) | (ind_port)))

/*! \brief This is similar to INDIRECT_2D but for scratchpad read. */
#define SS_INDIRECT_SCR_2D(ind_port, addr_offset, num_elem, stride, access_size, num_elem_port, input_port)       \
  __asm__ __volatile__("ss_stride   %0, %1, %2" : : "r"(stride), "r"(access_size), "i"(num_elem_port | (1<<10))); \
  __asm__ __volatile__("ss_ind    %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),                                \
                                                  "i"((1<<10) | (input_port<<5) | (ind_port)));

/*! \brief This is similar to INDIRECT_2D but for scratchpad write. */
#define SS_INDIRECT_2D_WR(ind_port, addr_offset, num_elem, stride, access_size, num_elem_port, output_port) \
  __asm__ __volatile__("ss_stride   %0, %1, %2" : : "r"(stride), "r"(access_size), "i"(num_elem_port | (1<<10))); \
  __asm__ __volatile__("ss_ind_wr    %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),\
                                                  "i"((output_port<<5) | (ind_port)));
 
/*! \brief This is similar to INDIRECT but for memory write. */
#define SS_INDIRECT_WR(ind_port, addr_offset, num_elem, output_port) \
  __asm__ __volatile__("ss_ind_wr %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),\
                                                  "i"((output_port<<5) | (ind_port)));

/*! \brief This is similar to INDIRECT_WR but for scratchpad write. */
#define SS_INDIRECT_WR_SCR(ind_port, addr_offset, num_elem, output_port) \
  __asm__ __volatile__("ss_ind_wr %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),\
                                                  "i"((1<<10) | (output_port<<5) | (ind_port)));


/*! \brief This is similar to INDIRECT but for scratchpad read. */
#define SS_INDIRECT_SCR(ind_port, addr_offset, num_elem, input_port) \
  __asm__ __volatile__("ss_ind    %0, %1, %2" : : "r"(addr_offset), "r"(num_elem),\
                                                  "i"((1<<10) | (input_port<<5) | (ind_port)));


#define SS_CONFIG_MEM_MAP(part_size, active_core_bv, map_type) \
  __asm__ __volatile__("ss_cfg_mmap %0, %1, %2" : : "r"(part_size), "r"(active_core_bv), "i"(map_type));


#define PART_CORE_BANK_REST 0
#define PART_CORE_REST_BANK 1
#define CORE_PART_BANK_REST 2 // this one works only for power of 2 data-structure sizes


/*!
 * \brief Wait for several elements write to the sratchpad.
 * \param num_rem_writes: 
 * \param scratch_type: 
 */
#define SS_WAIT_DF(num_rem_writes, scratch_type) \
  __asm __volatile__("ss_wait_df %0, %1" : : "r"(num_rem_writes), "i"(scratch_type));

/*!
 * \brief All the sratch operations will not be issued until all the scratch write before this 
 *        fence retire.
 */
#define SS_WAIT_SCR_WR() \
  __asm__ __volatile__("ss_wait t0, t0, 1"); \

/*!
 * \brief All the operations will not be issued until all the computations on the accelerator retire.
 */
#define SS_WAIT_COMPUTE() \
  __asm__ __volatile__("ss_wait t0, t0, 2" : : : "memory"); \

/*!
 * \brief All the sratch operations will not be issued until all the scratch read before this 
 *        fence retire.
 */
#define SS_WAIT_SCR_RD() \
  __asm__ __volatile__("ss_wait t0, t0, 4"); \

// TODO(@were): Confirm this with vidushi.
//wait for all prior scratch reads to be complete (128*8?)
#define SS_GLOBAL_WAIT(num_threads) \
  __asm__ __volatile__("ss_wait %0, t0, 128" :: "r"(num_threads)); \

//wait for all prior scratch reads to be complete (NOT IMPLEMENTED IN SIMULTOR YET)
#define SS_WAIT_SCR_RD_QUEUED() \
  __asm__ __volatile__("ss_wait t0, t0, 8"); \

//wait for all prior scratch reads to be complete (NOT IMPLEMENTED IN SIMULTOR YET)
#define SS_WAIT_MEM_WR() \
  __asm__ __volatile__("ss_wait t0, t0, 16"); \

#define SS_WAIT_SCR_ATOMIC() \
  __asm__ __volatile__("ss_wait t0, t0, 32"); \

// TODO(@were): Confirm this with vidushi.
// wait on all threads -- stall core
#define SS_WAIT_STREAMS() \
  __asm__ __volatile__("ss_wait t0, t0, 66"); \


//Indirect Ports
#define P_IND_1 (31)
#define P_IND_2 (30)
#define P_IND_3 (29)
#define P_IND_4 (28)
#define P_IND_5 (27)

//Convenience ports for these functions
#define MEM_SCR_PORT (23)
#define SCR_MEM_PORT (24)
#define SCR_SCR_PORT (25)
#define SCR_REM_PORT (26)

// TODO(@were): Confirm if this is used.
#define SS_SCR_WR_PART(addr, acc_size, port, part_size) \
  __asm__ __volatile__("ss_wr_scr %0, %1, %2" : : "r"(addr), "r"(acc_size), "i"((port) << 2))

#define SS_DMA_READ_SIMP(mem_addr, num_strides, port ) \
  __asm__ __volatile__("ss_dma_rd    %0, %1, %2" : : "r"(mem_addr), "r"(num_strides), "i"(port))

#define SS_GARBAGE_SIMP(output_port, num_elem) \
  __asm__ __volatile__("ss_wr_dma   %0, %1, %2"   : : "r"(0), "r"(num_elem), "i"(output_port|0x100))

#define SS_GARBAGE_BEFORE_STRIDE(num_garb) \
  __asm__ __volatile__("ss_garb   %0, %1, 0" : : "r"(num_garb), "r"(num_garb)); \

#define SS_DMA_WRITE_SIMP(output_port, num_strides, mem_addr) \
  __asm__ __volatile__("ss_wr_dma   %0, %1, %2"   : : "r"(mem_addr), "r"(num_strides), "i"(output_port))

#define SS_DMA_WRITE_SHF16(output_port, stride, access_size, num_strides, mem_addr) \
  __asm__ __volatile__("ss_stride   %0, %1, 0" : : "r"(stride), "r"(access_size));  \
  __asm__ __volatile__("ss_wr_dma   %0, %1, %2"   : : "r"(mem_addr),  "r"(num_strides), "i"(output_port|0x40))

#define SS_DMA_WRITE_SHF32(output_port, stride, access_size, num_strides, mem_addr) \
  __asm__ __volatile__("ss_stride   %0, %1, 0" : : "r"(stride), "r"(access_size));  \
  __asm__ __volatile__("ss_wr_dma   %0, %1, %2"   : : "r"(mem_addr),  "r"(num_strides), "i"(output_port|0x80))
