/*! \brief The signal of the given number. */
#define _SIGNAL(x) ((x) < 0 ? -1 : (x) > 0)


/*! \brief log of 2. Do give a perfect power of 2. */
#define _LOG2(x) ((x) ? ((31) - __builtin_clz((uint32_t)(x))) : 0)


/*! \brief Configure the state register of the DSA. */
inline void CONFIG_PARAM(int idx1, REG val1, bool s1,
                         int idx2, REG val2, bool s2) {
  int s2_ = (s2) ? ~((1 << 11) - 1) : 0;
  int mask = (idx1) | ((int)(idx2) << 5) | ((s1) << 10) | s2_;
  INTRINSIC_RRI("ss_cfg_param", val1, val2, (uint64_t) mask);
}


/*! \brief Configure the state register of the DSA. */
inline void CONFIG_PARAM(int idx1, REG val1, bool s1) {
  int mask = (idx1) | ((s1) << 10);
  INTRINSIC_RRI("ss_cfg_param", val1, (uint64_t) 0, (uint64_t) mask);
}


/*!
 * \brief Configure the predicate of control code broadcasting.
 * \param bitmask: The bit mask of configuration.
 *                 We can use environment variable LANES=n to configure the number of lanes.
 *                 By default, it is 8.
 * \note The number of each lane is 0-based, i.e. 0..(n-1). The n-th lane is the shared
 *       scratchpad.
 */
inline void SS_CONTEXT(REG bitmask) {
  CONFIG_PARAM(DSARF::TBC, bitmask, 1);
}


/*!
 * \brief Configure the spatial architecture.
 *        This is generated by the spatial scheduler.
 * \param addr: The array of bitstream of spatial architecture configuration.
 * \param size: The size of the configuration array in bytes.
 */
inline void SS_CONFIG(REG addr, REG size) {
  CONFIG_PARAM(DSARF::CSA, addr, 0, DSARF::CFS, size, 0);
}


/*!
 * \brief Drop all the ongoing data request while retaining the configuration.
 */
inline void SS_RESET() {
  SS_CONFIG((uint64_t)0, (uint64_t)0);
}


/*!
 * \brief Reset all live streams after finishing all ports
 */
inline void SS_STREAM_RESET() {
  SS_CONFIG((uint64_t)0, (uint64_t)1);
}


/*!
 * \brief Concatenate the data type bitmask.
 * \param dtype The direct memory access data type. [0:1]
 * \param const_type The const stream data type. [2:3]
 * \param index_type The indirect index data type. [4:5]
 * \param offset_type The indirect starting address data type. [6:7]
 * \param l1d_type The indirect inner dimension length data type. [8:9]
 */
inline uint64_t DTYPE_MASK(int dtype = 0,
                           int const_type = 0,
                           int index_type = 0,
                           int offset_type = 0,
                           int l1d_type = 0) {
  int direct_ = _LOG2(dtype);
  int const_ = _LOG2(const_type);
  int index_ = _LOG2(index_type);
  int offset_ = _LOG2(offset_type);
  int l1d_ = _LOG2(l1d_type);
  uint64_t value = 0;
  value = (value << 2) | l1d_;
  value = (value << 2) | offset_;
  value = (value << 2) | index_;
  value = (value << 2) | const_;
  value = (value << 2) | direct_;
  return value;
}


/*!
 * \brief Configure repeat register of the next instantiated input stream.
 * \param port The port to be configured.
 * \param field The field of the port to be configured.
 * \param value The value to the field to be set
 */
inline void SS_CONFIG_PORT(int port, int field, REG value) {
  uint64_t mask = port;
  mask <<= 1;
  mask = (mask << 4) | (field);
  INTRINSIC_RI("ss_cfg_port", value, mask);
}

/*! \brief The next stream instantiated from this port will be repeated n times. */
inline void SS_REPEAT_PORT(int port, REG n) {
  SS_CONFIG_PORT(port, DPF_PortRepeat, MUL(n, 1 << DSA_REPEAT_DIGITAL_POINT));
}


/*! \brief A delta will be applied on the times of repeat after each time the repeat is done. */
inline void SS_REPEAT_STRETCH(int port, REG n) {
  SS_CONFIG_PORT(port, DPF_PortRepeatStretch, MUL(n, 1 << DSA_REPEAT_DIGITAL_POINT));
}


/*!
 * \brief Set the registers that related to 1-d stream.
 * \param start Register SAR
 * \param length Register L1D
 */
inline void CONFIG_1D_STREAM(REG start, REG stride1d, REG length, int dtype, int ctype) {
  CONFIG_PARAM(DSARF::SAR, (REG)(start), 0, DSARF::L1D, length, 0);
  CONFIG_PARAM(DSARF::CSR, DTYPE_MASK(dtype, ctype, 0), 1, DSARF::I1D, stride1d, 0);
}

/*! \brief Concatenate the given values in a bitmask. */
inline uint64_t LINEAR_STREAM_MASK(int port, int padding, int action, int dimension,
                                   int operation, int memory) {
  uint64_t res = dimension & 3;
  res = (res << 1) | (action & 1);
  res = (res << 3) | (padding & 7);
  res = (res << 1) | (memory & 1);
  res = (res << 3) | (operation & 7);
  res = (res << 7) | (port & 127);
  return res;
}

/*!
 * \brief Mask wrapper for indirect memory streams.
 * \param port The destination port.
 * \param memory The type of the memory, 0: dma, 1: spad.
 * \param ind_mode a 3-bit hot vector. xx1: if use index from port, ow 0;
 *                 x1x: if use offset address from a port, ow the stride2d * outer;
 *                 1xx: if use length from a port, ow the l1d register.
 * \param lin_mode 0: 1d indirect stream; 2: 2d indirect stream
 */
inline uint64_t INDIRECT_STREAM_MASK(int port,
                                     int memory,
                                     int ind,
                                     int dim,
                                     MemoryOperation operation,
                                     bool penetrate,
                                     bool associate) {
  uint64_t value = associate;
  value = (value << 1) | dim;
  value = (value << 1) | penetrate;
  value = (value << 3) | ind;
  value = (value << 1) | memory;
  value = (value << 3) | ((int) operation);
  value = (value << 7) | port;
  return value;
}

/*!
 * \brief Instantiate a 1d linear stream. (dtype*)(a + i*stride1d)
 * \param addr The initial value of the state machine.
 * \param stride The stride after accessing.
 * \param length The number of trip counts.
 * \param port The source/destination port.
 * \param padding The mode of padding. Refer rf.h:Padding for more details.
 *                If it is a write stream, this is useless. Use 0 as a placeholder.
 * \param action 0: access; 1: generate the affine linear value sequence to the port.
 * \param dimension (d+1)=the number of dimensions of the stream.
 *        For now, 1,2,and 3-d are supported.
 * \param operation 0: read, 1: write, 2-7: atomic +, -, *, /, min, and max.
 * \param memory 0: memory, 1: spad.
 * \param dtype The data type of this stream.
 */
inline void INSTANTIATE_1D_STREAM(REG addr, REG stride, REG length,
                                  int port, int padding, int action, int operation,
                                  int memory, int dtype, int ctype) {
  CONFIG_1D_STREAM(addr, stride, length, dtype, ctype);
  auto value = LINEAR_STREAM_MASK(port, padding, action, /*1d*/0, operation, memory);
  INTRINSIC_R("ss_lin_strm", value);
}

/*!
 * \brief Feed several consts to a port.
 * \param port: The destination port.
 * \param val: The value to be fed. The data type is 8-byte.
 * \param n: The number of elements to be fed to the port.
 * \param cbyte: The data type of the constant.
 */
inline void SS_CONST(int port, REG value, REG n, int cbyte = 8) {
  INSTANTIATE_1D_STREAM(/*Initial*/value, /*Stride*/(uint64_t) 0, /*N*/n,
                        port, DP_NoPadding, DSA_Generate,
                        /*Memory Operation*/ 0,
                        /*Memory Source*/ 0,
                        /*Word Byte*/ 1,
                        cbyte);
}


/*! \brief Insert a barrier for the accelerator. Refer rf.h to see the masks. */
inline void SS_WAIT(REG mask) {
  INTRINSIC_RI("ss_wait", mask, (uint64_t) 0);
}


/*! \brief Block the control host and wait everything done on the accelerator. */
inline void SS_WAIT_ALL() {
  REG all_ones(~0ull);
  SS_WAIT(all_ones);
}


/*!
 * \brief Write a value from CGRA to the register file.
 * \param out_port: The source port.
 * \param val: A lvalue reference where the value is written to.
 */
inline REG SS_RECV(int port, int dtype = 8) {
  int mask = port;
  mask <<= 1;
  mask <<= 2;
  mask |= _LOG2((int) dtype);
  mask <<= 1;
  REG res;
  REG x0((uint64_t) 0);
  INTRINSIC_DRI("ss_recv", res, x0, mask);
  return res;
}

/*!
 * \brief Forward value from output port to the input port.
 * \param output_port: The data source port.
 * \param input_port: The destination data port.
 * \param n: The number of data forwarded.
 * \param dtype: The data type of each element forwarded.
 */
inline void SS_RECURRENCE(int oport, int iport, REG n, int dtype = 8) {
  CONFIG_PARAM(DSARF::L1D, n, false, DSARF::CSR, _LOG2(dtype), false);
  REG port(iport | (oport << 7));
  INTRINSIC_R("ss_wr_rd", port);
}


/*!
 * \brief Set the registers that related to 2d stream.
 * \param start Register SAR
 * \param stride Register I2D
 * \param length Register L1D
 * \param stretch Register E2D
 * \param n Register L2D
 */
inline void CONFIG_2D_STREAM(REG addr, REG stride1d, REG length,
                             REG stride2d, REG stretch, REG n, int dtype, int ctype) {
  CONFIG_1D_STREAM(addr, stride1d, length, dtype, ctype);
  CONFIG_PARAM(DSARF::E2D, stretch, 0, DSARF::L2D, n, 0);
  CONFIG_PARAM(DSARF::I2D, stride2d, 0);
}


/*!
 * \brief Instantiate a 2d linear stream.
 */
inline void INSTANTIATE_2D_STREAM(REG addr, REG stride1d, REG l1d, REG stride2d, REG stretch, REG n,
                                  int port, int padding, int action, int op, int mem,
                                  int dtype, int ctype) {
  CONFIG_2D_STREAM(addr, stride1d, l1d, stride2d, stretch, n, dtype, ctype);
  auto value = LINEAR_STREAM_MASK(port, padding, action, /*2d*/1, op, mem);
  INTRINSIC_R("ss_lin_strm", value);
}


inline void CONFIG_3D_STREAM(REG addr, REG stride_1d, REG l1d, REG stride_2d, REG stretch_2d1d, REG n_2d,
                             REG delta_stretch_3d2d, REG delta_stride_3d2d,
                             REG delta_length_3d1d, REG delta_length_3d2d,
                             REG stride_3d, REG n_3d, int dtype, int ctype) {
  CONFIG_2D_STREAM(addr, stride_1d, l1d, stride_2d, stretch_2d1d, n_2d, dtype, ctype);
  CONFIG_PARAM(DSARF::DE2D, delta_stretch_3d2d, 0,
               DSARF::DI2D, delta_stride_3d2d, 0);
  CONFIG_PARAM(DSARF::E3D1D, delta_length_3d1d, 0,
               DSARF::E3D2D, delta_length_3d2d, 0);
  CONFIG_PARAM(DSARF::I3D, stride_3d, 0, DSARF::L3D, n_3d, 0);
}

/*!
 * \brief Instantiate a 2d linear stream.
 */
inline void INSTANTIATE_3D_STREAM(REG addr, REG stride_1d, REG l1d, REG stride_2d,
                                  REG stretch_2d1d, REG n_2d,
                                  REG delta_stretch_3d2d, REG delta_stride_3d2d,
                                  REG delta_length_3d1d, REG delta_length_3d2d,
                                  REG stride_3d, REG n_3d,
                                  int port, int padding, int action, int op, int mem,
                                  int dtype, int ctype) {
  CONFIG_3D_STREAM(addr, stride_1d, l1d, stride_2d, stretch_2d1d, n_2d,
                   delta_stretch_3d2d, delta_stride_3d2d,
                   delta_length_3d1d, delta_length_3d2d,
                   stride_3d, n_3d, dtype, ctype);
  auto value = LINEAR_STREAM_MASK(port, padding, action, /*3d*/2, op, mem);
  INTRINSIC_R("ss_lin_strm", value);
}


/*!
 * \brief Periodically feed two consts to a port. [(val1 x v1_repeat), (val2 x v2_repeat)] x iters
 * \param port: The destination port.
 * \param val1: The first value to be fed. The data type is 8-byte.
 * \param v1_repeat: The repeat times of val1.
 * \param val2: The second value to be fed. The data type is 8-byte.
 * \param v2_repeat: The repeat :qtimes of val2.
 * \param iters: The times of repeating the periods.
 * \param ctype: The data type of this const stream.
 */
inline void SS_2D_CONST(int port, REG v1, REG r1, REG v2, REG r2, REG iters, int cbyte = 8) {
  REG stride_2d = SUB(v2, v1);
  REG stretch_2d1d = SUB(r2, r1);
  INSTANTIATE_3D_STREAM(v1, (uint64_t) 0, r1, stride_2d, /*stretch 2d1d*/stretch_2d1d,
                        /*n2d*/(uint64_t) 2,
                        /*delta stretch 3d2d*/(uint64_t) 0,
                        /*delta stride 3d2d*/(uint64_t) 0,
                        /*delta length 3d1d*/(uint64_t) 0,
                        /*delta length 3d2d*/(uint64_t) 0,
                        /*stride 3d*/(uint64_t) 0,
                        /*n3d*/iters, port, 0, DSA_Generate, 0, 0, 1, cbyte);
}

/*!
 * \brief Instantiate a 1d indirect stream a[b[i]]
 */
inline void INSTANTIATE_1D_INDIRECT(int target_port, int target_type, int idx_port, int index_type,
                                    REG start, REG stride1d, REG len, int memory,
                                    MemoryOperation operation, bool penetrate, bool associate = false) {
  CONFIG_PARAM(DSARF::INDP, idx_port, 0, DSARF::SAR, start, 0);
  CONFIG_PARAM(DSARF::L1D, len, 0, DSARF::CSR, DTYPE_MASK(target_type, 0, index_type), 0);
  CONFIG_PARAM(DSARF::I1D, stride1d, 0);
  auto value = INDIRECT_STREAM_MASK(target_port, memory, 1, 0, operation, penetrate, associate);
  INTRINSIC_R("ss_ind_strm", value);
}

/*!
 * \brief Allocate [start, end) on the spad to be buffet buffer.
 * \param start The close set of the starting address.
 * \param end The open set of the end address.
 */
inline void SS_BUFFET_ALLOC(int start, int end) {
  int64_t mask = end;
  mask <<= 16;
  mask |= start;
  CONFIG_PARAM(DSARF::BR, mask, 0);
}

/*!
 * \brief Allocate [start, end) on the spad to be buffet buffer.
 * \param start The close set of the starting address.
 * \param end The open set of the end address.
 */
inline void SS_BUFFET_DEALLOC() {
  SS_BUFFET_ALLOC(-1, -1);
}

/*!
 * \brief Instantiate a 2-d indirect stream.
 * \param ind_mode a 3-bit hot vector. xx1: if use index from port, ow 0;
 *                 x1x: if use offset address from a port, ow the stride2d * outer;
 *                 1xx: if use length from a port, ow the l1d register.
 */
inline void SS_INDIRECT_2D_READ(int in_port, int dtype, int start_port, REG start,
                                int idx_port, int idx_type, int l1d_port, REG l1d, REG stretch,
                                int memory, bool penetrate = false, bool associate = false) {
  int ind_mode = (idx_port != -1) | (start_port != -1) * 2 | (l1d_port != -1) * 4;
  idx_port = idx_port == -1 ? 0 : idx_port;
  l1d_port = l1d_port == -1 ? 0 : l1d_port;
  start_port = start_port == -1 ? 0 : start_port;
  int port_mask = (idx_port) | (start_port << 7) | (l1d_port << 14);
  CONFIG_PARAM(DSARF::INDP, port_mask, 0, DSARF::L1D, l1d, 0);
  CONFIG_PARAM(DSARF::E2D, stretch, 0, DSARF::CSR, DTYPE_MASK(dtype, 0, idx_type), 0);
  CONFIG_PARAM(DSARF::SAR, start, 0);
  auto value = INDIRECT_STREAM_MASK(in_port, memory, ind_mode, 1, DMO_Read, penetrate, associate);
  INTRINSIC_R("ss_ind_strm", value);
}

