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
#include "nand.h"

/* need set common_nand.c
 *com_nand->block_file = file_create(name, (com_nand->base.page_size +
 *							com_nand->base.spare_size) * com_nand->page_num_per_block,
 *							4, COMPRESS_BROTLI); // FILE_MAX_HANDLER
 */
#define TEST_NUM	5

static int g_first_row = -1;

int main(int argc, char *argv[])
{
	int ret, i, j, row;
	char *data, *oob;
	char *rb_data, *rb_oob;
	struct nand_base *nand;
	int page_num_per_block;

	if (argc != 2) {
		printf("[Usage]: %s [nand_name]\n", argv[0]);
		return 0;
	}

	nand = nand_init(COMMON, argv[1]);
	if (!nand) {
		printf("Nand init fail, please check the config file\n");
		return -1;
	}

	printf("block_size: %d\n", nand->block_size);
	printf("page_size%: %d\n", nand->page_size);
	printf("spare_size: %d\n", nand->spare_size);
	printf("block_num: %d\n", nand->block_num);
	printf("ecc_required: %d\n", nand->ecc_required);
	printf("max_pe_cycle: %d\n", nand->max_pe_cycle);
	printf("temperature: %d\n", nand->temperature);

	page_num_per_block = nand->block_size / nand->page_size;
	data = mem_alloc(nand->page_size);
	oob = mem_alloc(nand->spare_size);
	rb_data = mem_alloc(nand->page_size);
	rb_oob = mem_alloc(nand->spare_size);

	printf("=====Start Test=====\n");
	srand(0);
	for (i = 0; i < TEST_NUM; i++) {
		row = rand() % nand->block_num * page_num_per_block;
		if (g_first_row == -1)
			g_first_row = row;
		printf("======Opration row: %d======\n", row);
		ret = nand_erase_block(nand, row);
		if (ret) {
			printf("Erase fail at %d\n", row);
			continue;
		}
		for (j = row; j < row + page_num_per_block; j++) {
			printf("write page %d\n", j);
			memset(data, j, nand->page_size);
			memset(oob, j, nand->spare_size);
			memset(rb_data, 0, nand->page_size);
			memset(rb_oob, 0, nand->spare_size);

			ret = nand_write_page(nand, j, 0, data, oob);
			if (ret) {
				printf("Program fail at %d\n", j);
				nand_mark_block(nand, row);
				break;
			}

			printf("read page %d\n", j);
			ret = nand_read_page(nand, j, 0, rb_data, rb_oob);
			if ((ret == FLASH_ERROR) || (ret == FLASH_BAD)) {
				printf("read fail at %d\n", j);
				nand_mark_block(nand, row);
				break;
			}

			ret = memcmp(data, rb_data, nand->page_size);
			if (ret) {
				printf("[Error data]read != write %d\n", j);
				break;
			}

			ret = memcmp(oob, rb_oob, nand->spare_size);
			if (ret) {
				printf("[Error oob]read != write %d\n", j);
				break;
			}
		}
	}

	printf("\nread back flush block:\n");
	printf("enter any key to continue......\n");
	getchar();
	for (j = g_first_row; j < g_first_row + page_num_per_block; j++) {
		memset(data, j, nand->page_size);
		memset(oob, j, nand->spare_size);
		memset(rb_data, 0, nand->page_size);
		memset(rb_oob, 0, nand->spare_size);

		printf("read back page %d\n", j);
		ret = nand_read_page(nand, j, 0, rb_data, rb_oob);
		if ((ret == FLASH_ERROR) || (ret == FLASH_BAD)) {
			printf("read fail at %d\n", j);
			nand_mark_block(nand, row);
			break;
		}

		ret = memcmp(data, rb_data, nand->page_size);
		if (ret) {
			printf("[Error data]read != write %d\n", j);
			break;
		}

		ret = memcmp(oob, rb_oob, nand->spare_size);
		if (ret) {
			printf("[Error oob]read != write %d\n", j);
			break;
		}
	}
	printf("=====End Test=====\n");
	mem_free(data);
	mem_free(oob);
	mem_free(rb_data);
	mem_free(rb_oob);
	nand_deinit(COMMON, nand);
	return 0;
}