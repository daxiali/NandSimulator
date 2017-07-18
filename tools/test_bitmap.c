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
#include "bitmap.h"

#define MAX_COUNT	2

int main(int argc, char *argv[])
{
	int i, j, count = 0;
	int size, base;
	struct bitmap *test_bm;

	if (argc != 3) {
		printf("[Usage]:%s [size] [base]\n", argv[0]);
		return 0;
	}

	size = atoi(argv[1]);
	base = atoi(argv[2]);
	printf("size:%d base:%d\n", size, base);
	test_bm = bitmap_create(size, base);

	do {
		printf("\nloop %d\n", count);
		for (i = base; i < base + size; i++) {
			if (bitmap_get(test_bm, i) != 0)
				printf("[1]Error at %d\n", i);

			bitmap_set(test_bm, i);
			for (j = 0; j < (roundup(size, 8) >> 3); j++) {
				printf("%2x ", test_bm->b[j]);
			}
			printf("\n");
			if (bitmap_get(test_bm, i) == 0)
				printf("[2]Error at %d\n", i);
			bitmap_clear(test_bm, i);
			if (bitmap_get(test_bm, i) != 0)
				printf("[3]Error at %d\n", i);
		}

		printf("\n");
		for (j = 0; j < (roundup(size, 8) >> 3); j++) {
			printf("%2x ", test_bm->b[j]);
		}
		printf("\n");
	} while (++count < MAX_COUNT);

	bitmap_delete(test_bm);
	return 0;
}