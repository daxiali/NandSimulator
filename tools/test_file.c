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

#include "common.h"
#include "file.h"

static char data_pattern[] =  {
	0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
	0x1F, 0x2E, 0x3D, 0x4C, 0x5B, 0x6A, 0x79, 0x88, 0x97, 0xA6, 0xB5, 0xC4, 0xD3, 0xE2, 0xF1,
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
};

int main(int argc, char *argv[])
{
	int i;
	int size, num, len;
	struct file_info *test_file;
	unsigned char *buf;

	if (argc != 4) {
		printf("[Usage]: %s [name] [size] [num]\n", argv[0]);
		return 0;
	}

	size = atoi(argv[2]);
	num = atoi(argv[3]);
	printf("size:%d num:%d\n", size, num);
	test_file = file_create(argv[1], size, num, COMPRESS_NONE);
	if (!test_file) {
		printf("[Error] create file fail!\n");
		return -1;
	}

	len = MIN(size, sizeof(data_pattern) / sizeof(char));
	printf("len:%d\n", len);
	for (i = 0; i < num; i++) {
		buf = file_write_cache(test_file, i);
		printf("1\n");
		memcpy(buf, data_pattern, len);
		buf[0] = '0' + i;
		printf("2\n");
		file_write(test_file, i);
	}
	printf("3\n");
	buf = NULL;
	buf = file_write_cache(test_file, i);
	memcpy(buf, data_pattern, len);
	buf[0] = '0' + i;
	printf("4\n");
	buf = file_read(test_file, 0);
	printf("buf[0]=%02x, buf[1]=%02x, buf[2]=%02x\n", buf[0], buf[1], buf[2]);
	printf("5\n");
	file_flush(test_file);
	file_delete(test_file);
	return 0;
}