/*******************************************************************************
 * Copyright (c) 2013 Advanced Micro Devices, Inc.
 *
 * RESTRICTED RIGHTS NOTICE (DEC 2007)
 * (a)     This computer software is submitted with restricted rights under
 *     Government Contract No. DE-AC52-8MA27344 and subcontract B600716. It
 *     may not be used, reproduced, or disclosed by the Government except as
 *     provided in paragraph (b) of this notice or as otherwise expressly
 *     stated in the contract.
 *
 * (b)     This computer software may be -
 *     (1) Used or copied for use with the computer(s) for which it was
 *         acquired, including use at any Government installation to which
 *         the computer(s) may be transferred;
 *     (2) Used or copied for use with a backup computer if any computer for
 *         which it was acquired is inoperative;
 *     (3) Reproduced for safekeeping (archives) or backup purposes;
 *     (4) Modified, adapted, or combined with other computer software,
 *         provided that the modified, adapted, or combined portions of the
 *         derivative software incorporating any of the delivered, restricted
 *         computer software shall be subject to the same restricted rights;
 *     (5) Disclosed to and reproduced for use by support service contractors
 *         or their subcontractors in accordance with paragraphs (b)(1)
 *         through (4) of this notice; and
 *     (6) Used or copied for use with a replacement computer.
 *
 * (c)     Notwithstanding the foregoing, if this computer software is
 *         copyrighted computer software, it is licensed to the Government with
 *         the minimum rights set forth in paragraph (b) of this notice.
 *
 * (d)     Any other rights or limitations regarding the use, duplication, or
 *     disclosure of this computer software are to be expressly stated in, or
 *     incorporated in, the contract.
 *
 * (e)     This notice shall be marked on any reproduction of this computer
 *     software, in whole or in part.
 ******************************************************************************/

#ifndef _VALIDATE_CONFIG_H_
#define _VALIDATE_CONFIG_H_

#include <stdint.h>
#include "pim.h"
#include "pim-private.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#define snprintf _snprintf
#endif


/*****************************************************************************************
* This function accepts as input a emulator data structure that has been previously
* parsed by xml_parser and validates the configuration.
* Arguments
* 	pim_emu_configuration_t : A pointer to the emulator structure.
* Return:
* An integer value whioh returns one of the following
* 	PIM_INVALID_CONFIGRUATION : This value is returned if the configuration is invalid
* 	PIM_SUCCESS : This value is returned if the configuration is valid.
*****************************************************************************************/

int validate_configuration(pim_emu_configuration_t *);

#ifdef __cplusplus
}
#endif
#endif
