#pragma once

/*!
 * \file spec.h
 * @author PolyArch Research Group 
 * \brief The default tech spec of the reconfigurable accelerator.
 *        These macros can be override by the compiler flags.
 * \date 2020-12-26
 * @copyright Copyright (c) 2020
 */

#pragma once

/*!
 * \brief The max number of DSA lanes supported
 */
#ifndef DSA_XLEN
#define DSA_XLEN 64
#endif

/*!
 * \brief The max number of DSA ports across all the lanes supported
 */
#ifndef DSA_MAX_PORTS
#define DSA_MAX_PORTS 128
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
#define DSA_SUB_LANES 4
#endif

/*!
 * \brief The unit of addressable memory of the DSA
 */
#ifndef DSA_ADDRESSABLE_MEM
#define DSA_ADDRESSABLE_MEM 1
#endif
