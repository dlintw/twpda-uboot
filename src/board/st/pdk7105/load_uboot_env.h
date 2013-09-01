/* Copyright (c) 2013 Daniel YC Lin <dlin.tw at gmail>
 * All right reserved.
 *
 * @brief load uboot.env in first partition to setup environment variable
 */
#ifndef _LOAD_UBOOT_ENV_H
#define _LOAD_UBOOT_ENV_H

#include "swUpdate.h"
int load_uboot_env(int part);

#endif
