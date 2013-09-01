/* Copyright (c) 2013 Daniel YC Lin <dlin.tw at gmail>
 * All right reserved.
 *
 * @brief load uboot.sh in first partition to setup environment variable
 */
#include <malloc.h>
#include <common.h>
#include <hush.h>
#include "dbg.h"
#include "load_uboot_env.h"

extern int ext2fs_open (char *filename);
extern int ext2fs_set_blk_dev (block_dev_desc_t * rbdd, int part);
extern int ext2fs_close (void);
extern int ext2fs_mount (unsigned part_length);
extern int usb_fatload(char *file_name,unsigned int LoadSize);
extern int usb_ext2load(char *file_name,int partition);

/* @brief parse uboot.sh file format
 */
int parse_uboot_env(void)
{
	int len = strlen((char *)SWLOADADDR);
	char *cmd;
	int rc;

	if ((cmd = malloc (len + 1)) == NULL) {
		return ABNORMITY_RET;
	}
	memmove(cmd, (char *)SWLOADADDR, len+1);
	DBG1("= uboot.sh content=\n=%s\n= end of uboot.sh =\n",cmd);
	rc = parse_string_outer (cmd, FLAG_PARSE_SEMICOLON);
	free(cmd);
	return (rc) ? FAILURE_RET : SUCCESS_RET;
}


/* @brief load uboot.sh in first parition
 * @param part parition number defined in .h
 * @return SUCCESS_RET, FAILURE_RET(wrong format), ABNORMITY_RET(not found)
 */
int load_uboot_env(int part)
{
	block_dev_desc_t *dev_desc=NULL;
	char *envFile = "uboot.sh";
	int maxFileLen=1024;
	char *buf = (char *)SWLOADADDR;
	int partLen;

	dev_desc = get_dev("usb",0);  //first partition
	if (dev_desc == NULL) {
		DBG0("get_dev first partition failed");
		return ABNORMITY_RET;
	}
	//DBG1("load_uboot_env(partition=%d)", part);
	partLen = ext2fs_set_blk_dev(dev_desc, part);
	if (partLen  == 0) {
		DBG0("ext2fs_set_blk_dev partLen == 0");
		ext2fs_close();
		return ABNORMITY_RET;
	}
	if (!ext2fs_mount(partLen)) {   // not ext2
		ext2fs_close();
		memset(buf, 0, maxFileLen);
		if (SUCCESS_RET  == usb_fatload(envFile, maxFileLen)) {
			buf[maxFileLen-1] = '\0';
			return parse_uboot_env();
		}
		return ABNORMITY_RET;
	}
	if (ext2fs_open(envFile) < 0)
	{
		DBG2("Warn:%s not found on partition %d\n", envFile, part);
		ext2fs_close();
		return ABNORMITY_RET;
	}
	if (SUCCESS_RET  == usb_ext2load(envFile, part)) {
		ext2fs_close();
		buf[maxFileLen-1] = '\0';
		return parse_uboot_env();
	}
	ext2fs_close();
	return ABNORMITY_RET;
}
