/* Copyright (c) 2013 Daniel YC Lin <dlin.tw at gmail>
 * All right reserved.
 *
 * @brief show source code file name and line number for debug
 */
#ifndef _DBG_H
#define _DBG_H

#define DBG0(a) printf("%s:%d " a "\n", __FILE__, __LINE__)
#define DBG1(a,b) printf("%s:%d " a "\n", __FILE__, __LINE__, (b))
#define DBG2(a,b,c) printf("%s:%d " a "\n", __FILE__, __LINE__, (b), (c))
#define DBG3(a,b,c,d) printf("%s:%d " a "\n", __FILE__, __LINE__, (b), (c), (d))
#define DBG4(a,b,c,d,e) printf("%s:%d " a "\n", __FILE__, __LINE__, \
		(b), (c), (d), (e))

#endif
