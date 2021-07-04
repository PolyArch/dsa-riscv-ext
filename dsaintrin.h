/*!
 * \file dsaintrin.h
 * \author PolyArch Research Lab
 * \copyright Copyright (c) 2020
 */

#pragma once

#include "dsa-ext/spec.h"
#include "dsa-ext/rf.h"

// Magic sentinal for matching
#define SENTINAL (((uint64_t)1)<<63)
#define SENTINAL16 (((uint16_t)1)<<15)
#define SENTINAL32 (((uint32_t)1)<<31)

//Mask for accessing shared scratchpad
#define SHARED_SP 0x100
#define SHARED_SP_INDEX 8

struct REG {
  uint64_t value;
  inline operator uint64_t&() { return value; }
  REG() {}
  REG(uint64_t value_) : value(value_) {}
  REG(void *value_) : value((uint64_t)(value_)) {}
};

#define INTRINSIC_RRI(mn, a, b, c) \
  __asm__ __volatile__(mn " %0, %1, %2" : : "r"(a), "r"(b), "i"(c))

#define INTRINSIC_RI(mn, a, b) __asm__ __volatile__(mn " %0, %1" : : "r"(a), "i"(b))

#define INTRINSIC_R(mn, a) __asm__ __volatile__(mn " %0" : : "r"(a))

#define INTRINSIC_DI(mn, a, b) \
   __asm__ __volatile__(mn " %0, %1" : "=r"(a) : "i"(b));

#define INTRINSIC_DRI(mn, a, b, c) \
   __asm__ __volatile__(mn " %0, %1, %2" : "=r"(a) : "r"(b), "i"(c));

#define DIV(a, b) ((a) / (b))
#define SUB(a, b) ((a) - (b))
#define SHL(a, b) ((a) << (b))
#define MUL(a, b) ((a) * (b))

#include "intrin_impl.h"

#undef INTRINSIC_RRI
#undef INTRINSIC_RI
#undef INTRINSIC_R

#undef DIV
#undef SUB
#undef SHL
#undef MUL

/*!
 * \brief addr[0:bytes] -> port
 */
inline void SS_1D_READ(REG addr,
                       REG bytes,
                       int port,
                       Padding padding,
                       MemoryType source,
                       int wbytes = 1) {
  INSTANTIATE_1D_STREAM(addr, wbytes, bytes / wbytes, port, padding,
                        /*Stream Action*/DSA_Access,
                        /*Memory Operation*/DMO_Read, /*Data Source*/source,
                        /*Word Bytes*/wbytes, /*Const Bytes*/0);
}


/*!
 * \brief port -> addr[0:bytes]
 */
inline void SS_1D_WRITE(int port,
                        REG addr,
                        REG bytes,
                        MemoryType source,
                        int wbytes = 1) {
  INSTANTIATE_1D_STREAM(addr, wbytes, bytes / wbytes, port, DP_NoPadding,
                        DSA_Access, DMO_Write,
                        source, wbytes, 0);
}

/*!
 * \brief The semantics is similar to DMA_READ_STRETCH but for scratchpad read.
 */
#define SS_SCR_PORT_STREAM_STRETCH(addr, stride, bytes, stretch, n, port) \
    SS_2D_READ(addr, stride, bytes, stretch, n, port, 0, DMT_SPAD)


/*!
 * \brief The semantics is similar to DMA_READ but for scratchpad read.
 */
#define SS_SCR_PORT_STREAM(scr_addr,stride,acc_size,n_strides, port) \
   SS_SCR_PORT_STREAM_STRETCH(scr_addr,stride,acc_size,0,n_strides, port) 


/*!
 * \brief This is a wrapper for SCR_PORT_STREAM to keep backward compatibility.
 */
#define SS_SCRATCH_READ(scr_addr, n_bytes, port) \
  SS_SCR_PORT_STREAM_STRETCH(scr_addr, 0, n_bytes, 0, 1, port) 


/*!
 * \brief Instantiate a 2-d DMA read stream.
 */
inline void SS_2D_READ(REG addr, REG stride, REG bytes, REG stretch, REG n, int port, int padding, int source) {
  INSTANTIATE_2D_STREAM(addr, 1, bytes, stride,
                        stretch, n, port, padding,
                        DSA_Access, DMO_Read,
                        source, 1, 0);
}


/*!
 * \brief Legacy wrapper of a 2-d stream without stretch.
 */
#define SS_DMA_READ(addr, stride, bytes, n, port) \
  SS_DMA_READ_STRETCH(addr, stride, bytes, (uint64_t) 0, n, port)


/*!
 * \brief Instantiate a 2-d DMA write stream.
 */
inline void SS_DMA_2D_WRITE(REG addr, REG stride, REG bytes, REG stretch, REG n, int port, int dtype = 1) {
  INSTANTIATE_2D_STREAM(addr, 1, bytes,
                        stride,
                        stretch, n, port,
                        DP_NoPadding, DSA_Access, DMO_Write,
                        DMT_DMA, dtype, 0);
}


/*!
 * \brief Legacy wrapper of a 2-d stream with stretch.
 */
#define SS_DMA_READ_STRETCH(addr, stride, acc_size, stretch, n, port) \
  SS_2D_READ(addr, stride, acc_size, stretch, n, port, 0, DMT_DMA)


/*!
 * \brief This is a wrapper for DMA_WR_INNER and DMA_WR_OUTER to keep bardward compatibility.
 */
#define SS_DMA_WRITE(port, stride, bytes, n, addr) \
   SS_DMA_2D_WRITE(addr, stride, bytes, (uint64_t) 0, n, port)


/*!
 * \brief Discard a num_elem*elem_size bytes of data from the specific port.
 * \param output_port: The port to discard the value.
 * \param num_elem: The number of elements to discard.
 * \param elem_size: The data size of each element.
 */
#define SS_GARBAGE_GENERAL(output_port, num_elem, elem_size)   \
  do {                                                         \
    auto bytes_ = bytes;                                       \
    INSTANTIATE_1D_STREAM(0, elem_size, num_elem, port,        \
                          DP_NoPadding, DSA_Access, DMO_Write, \
                          DMT_DMA, elem_size, 0);              \
  } while (false)

/*!
 * \brief This is a wrapper for GARBAGE_GENERAL to keep the backward compatibility with
 *        the decomposability.
 * \param num_elem: The number of q-word to discard.
 */
#define SS_GARBAGE(output_port, num_elem) \
  SS_GARBAGE_GENERAL(output_port, num_elem, 8)

/*!
 * \brief This is a wrapper for SCR_WR_INNER and SCR_WR_OUTER to keep backward compatibility.
 * \param port: The data source of the output port.
 * \param bytes: The number of bytes from the port to be written to the scratchpad.
 * \param addr: The starting address onsdlkfjjkljlk; the scratchpad.
 */
#define SS_SCR_WRITE(port, bytes, addr) SS_1D_WRITE(port, addr, bytes, DMT_SPAD) 

/*!
 * \brief Indirect 1d read.
 */
#define SS_INDIRECT_READ(in_port, dtype, idx_port, itype, start, stride, len, memory) \
  INSTANTIATE_1D_INDIRECT(in_port, dtype, idx_port, itype, start, stride, len, memory, DMO_Read)

/*!
 * \brief Indirect 1d atomic operation.
 */
#define SS_INDIRECT_ATOMIC(operand_port, otype, idx_port, itype, start, stride, len, memory, operation) \
  INSTANTIATE_1D_INDIRECT(operand_port, otype, idx_port, itype, start, stride, len, memory, operation)

// ==================== Above are implemented ====================

// TODO(@were): Confirm the semantics with @vidushi.
#define SS_ATOMIC_DFG_CONFIG(dfg_addr_cons, dfg_val_cons, dfg_val_out) \
  __asm__ __volatile__("ss_atom_op %0, %1, %2" : : "r"(dfg_addr_cons), "r"(dfg_val_cons), "i"(dfg_val_out << 1 | 0))

/*!
 * \brief Configure repeat register of the next instantiated input stream.
 *        Instead of having a constant repeating times, it comes from a port.
 * \param times_port: The source port of repeating times.
 */
#define SS_VREPEAT_PORT(times_port) \
  __asm__ __volatile__("ss_cfg_port %0, t0, %1" : : "r"(times_port), "i"(1))

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

