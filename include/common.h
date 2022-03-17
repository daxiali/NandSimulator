/*
 * BeanSim : Nand Flash Simulator by File Operations
 * Authors: Bean.Li<lishizelibin@163.com>
 *
 * This file is part of BeanSim.
 *
 * BeanSim is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * BeanSim is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with BeanSim.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#define BEAN_DEBUG

#define PLATFORM_X86
//#define PLATFORM_ARM

#ifdef PLATFORM_X86
#include <stdio.h> // for printf
#include <stdlib.h>
#include <assert.h> // for assert
#include <stdbool.h>
#include <string.h>
#include <dirent.h>
//#include <io.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#elif defined(PLATFORM_ARM)
// ToDo
#else
// ToDO
#endif // PLATFORM

#ifndef TRUE
#define TRUE	true
#endif
#ifndef FALSE
#define FALSE	false
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

#define HASH_SIZE(n) ((size_t)1 << (n))
#define HASH_MASK(n) (HASH_SIZE(n) - 1)

#define roundup(x,n) (((x)+((n)-1))&(~((n)-1))) // power 2


enum LOG_LEVEL {
	LOG_ALL,
	LOG_WARN,
	LOG_ERR
};

#ifdef BEAN_DEBUG
static enum LOG_LEVEL log_level_stat = LOG_ALL;
#define LOG(level, fmt, ...) \
	do { \
		if ((level) >= log_level_stat) { \
			switch (level) { \
			case LOG_WARN: \
				printf("[WARN at %s %d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
				break; \
			case LOG_ERR: \
				printf("[ERROR at %s %d]: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); \
				break; \
			} \
		} \
	} while(0)

#define ASSERT(expr) \
	do { \
		if (!(expr)) { \
			LOG(LOG_ERR, "Bean assert fail"); \
			assert(0); \
		} \
	} while(0)
#else
#define LOG(level, fmt, ...)
#define ASSERT(expr)
#endif

static inline unsigned short calc_crc16(unsigned short crc, unsigned char data) {
    unsigned short rc;
    static const unsigned short crc16_table[] = { 
        0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
        0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
        0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
        0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
        0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
        0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
        0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
        0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
        0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
        0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
        0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
        0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
        0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
        0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
        0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
        0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
        0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
        0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
        0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
        0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
        0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
        0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
        0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
        0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
        0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
        0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
        0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
        0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
        0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
        0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
        0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
        0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0,
    };

    rc = (crc << 8) ^ crc16_table[(int)(((crc >> 8) ^ data) & 0xff)];
    return rc;
}

static inline int fls32(unsigned int x)
{
	int r = 32;

 	if (!x)
 		return 0;
 	if (!(x & 0xffff0000u)) {
 		x <<= 16;
 		r -= 16;
 	}
 	if (!(x & 0xff000000u)) {
 		x <<= 8;
 		r -= 8;
 	}
 	if (!(x & 0xf0000000u)) {
 		x <<= 4;
 		r -= 4;
 	}
 	if (!(x & 0xc0000000u)) {
 		x <<= 2;
 		r -= 2;
 	}
 	if (!(x & 0x80000000u)) {
 		x <<= 1;
 		r -= 1;
 	}
 	return r;
}


static inline int fls64(unsigned long long x)
{
	unsigned int _x = x >> 32;

	if (_x)
		return fls32(_x) + 32;
	return fls32(x);
}


static inline bool is_power_of_2(unsigned long n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}


static inline unsigned long roundup_power2(unsigned long n)
{
	if (sizeof(n) == 4)
		return 1UL << fls32(n - 1);
	return 1UL << fls64(n - 1);
}


static inline unsigned long roundown_power2(unsigned long n)
{
	if (sizeof(n) == 4)
		return 1UL << (fls32(n) - 1);
	return 1UL << (fls64(n) - 1);
}


static inline int calc_msb_index(unsigned int n)
{
    unsigned int _n = 1;

    if (n == 0)
		return -1;

    if ((n >> 16) == 0) {
		_n += 16;
		n <<= 16;
	}

    if ((n >> 24) == 0) {
		_n += 8;
		n <<= 8;
	}

    if ((n >> 28) == 0) {
		_n += 4;
		n <<= 4;
	}

    if ((n >> 30) == 0) {
		_n += 2;
		n <<= 2;
	}
    _n -= (n >> 31);
    return 31 - _n;
}


static inline double power(double n, int p)
{
	int i;
	double pow = 1;

	for (i = 0; i < p; i++)
		pow *= n;

	return pow;
}

static inline void *mem_alloc(unsigned int size)
{
	void *buf;

	if (!size)
		return NULL;
	buf = malloc(size);
	if (!buf)
		ASSERT(0);
	memset(buf, 0, size);
	return buf;
}


static inline void mem_free(void *mem)
{
	if (mem != NULL)
		free(mem);
}

#endif // __COMMON_H__
