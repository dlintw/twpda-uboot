#include <stdio.h>

#include "string.h"
#include "dbg.h"

char *SWLOADADDR;

#define SUCCESS_RET 0
#define FAILURE_RET 1

/* @brief parse uboot.env file format
 *
 * TODO: trim off space
 */
int parse_uboot_env()
{
	char *p, *name, *value, *pEndOfLine;
	int lineNo=1;

	DBG1("== /uboot.env ==\n%s\n== end of /uboot.env ==", SWLOADADDR);
	printf("buf len=%d\n", strlen(SWLOADADDR));
	for (p=SWLOADADDR; *p; ) {
		if (*p == '#') { // skip comment
			while (*p && *p != '\n' && *p != '\r') p++;
			if (*p == '\r') p++;
			if (*p == '\n') p++;
			lineNo++;
		} else if (*p == '\r' || *p == '\n') { // empty line
				if (*p == '\r') p++;
				if (*p == '\n') p++;
				lineNo++;
				continue;
		} else {
			name = p;
			while (*p != ' ' && *p != '\t'
					&& *p && *p != '\n' && *p != '\r')
				p++;
			if (*p != ' ' && *p != '\t') {
				DBG1("Err:/boot.env line:%d lack space", lineNo);
				return FAILURE_RET;
			}
			*p++ = '\0';
			for (value = p; *p && *p != '\n' && *p != '\r'; p++);
			if (p == value) {
				DBG1("Err:/boot.env line:%d no value", lineNo);
				return FAILURE_RET;
			}
			if (*p == '\n' || *p == '\r') {
				pEndOfLine = p;
				if (*p == '\r') p++;
				if (*p == '\n') p++;
				*pEndOfLine = '\0';
			}
			printf("n_name=%d n_value=%d\n",
					strlen(name),
					strlen(value));
			printf("%d:name=[%s] value=%s\n", lineNo, name, value);
			lineNo++;
		}
	}

	return SUCCESS_RET;
}

int main() {
	char *s0="# boot parameters for external USB partition 2\
\n\
\nbootargs console=ttyAS0,115200 root=/dev/sdb2  rootfstype=ext3 rw  nwhwconf=device:eth0,hwaddr:10:08:E2:12:06:BD phyaddr:0,watchdog:5000 mem=256M bigphysarea=2048\
\nbootcmd ext2load usb 0:2 80000000 vmlinux.ub; bootm 80000000\
\n";
	char s[1024];
	strcpy(s, s0);
	SWLOADADDR=s;
	printf("return:%d\n", parse_uboot_env());
	return 0;
}
