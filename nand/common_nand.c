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
#include "common_nand.h"


static char *common_nand_info[COMMON_NAND_INFO_NUM] = {
	"Name", // 0, skip
	"Block_Size(KB)", // 1
	"Page_Size(B)", // 2
	"Spare_Size(B)", // 3
	"Block_Num", // 4
	"ECC_Required", // 5
	"Bad_Block_Num", // 6
	"Weak_Block_NUM", // 7
	"Max_PE_Cycle", // 8
	"Weak_PE_Cycle", // 9
	"Temperature(C)", // 10
};

static int common_nand_value[COMMON_NAND_INFO_NUM] = {
	0xFFFFFFFF, // skip
	4096,
	16384,
	2048,
	2048,
	60,
	40,
	10,
	3000,
	2000,
	25,
};

static void common_nand_bad_block_alloc(struct common_nand *com_nand)
{
	int i, j, num;
	int block, block_num;
	int row;

	num = com_nand->bad_block_num + com_nand->weak_block_num;
	block_num = com_nand->base.block_num - 1;

	/* skip block 0 */
	srand(time(0));
	for (i = 0; i < num; i++) {
repeat:
		block = rand() % block_num + 1;
		/* repeat check */ 
		for (j = 0; j < i; j++) {
			if (block == com_nand->bad_block[j])
				goto repeat;
		}
		com_nand->bad_block[i] = block;
		row = block * com_nand->page_num_per_block;
		bitmap_set(com_nand->page_map, row);
		bitmap_set(com_nand->page_map, row + 1);
		bitmap_set(com_nand->page_map, row + com_nand->page_num_per_block - 1);
		if (i >= com_nand->bad_block_num) {
			com_nand->block_info[block].pe_cycle = com_nand->weak_pe_cycle;
			com_nand->block_info[block].read_count = 0;
		}
	}
}


static int common_nand_bad_block(struct common_nand *com_nand, int block_num)
{
	int i, row;

	row = block_num * com_nand->page_num_per_block;
	if ((bitmap_get(com_nand->page_map, row) == 1) &&
		(bitmap_get(com_nand->page_map, row + 1) == 1) &&
		(bitmap_get(com_nand->page_map, row + 2) == 0) &&
		(bitmap_get(com_nand->page_map, row + com_nand->page_num_per_block - 1) == 1))
			return 1;
	for (i = 0; i < com_nand->bad_block_num; i++) {
		if (block_num == com_nand->bad_block[i]) 
			return 1;
	}
	return 0;
}


static int common_nand_weak_block(struct common_nand *com_nand, int block_num)
{
	int i, num;

	num = com_nand->bad_block_num + com_nand->weak_block_num;
	for (i = com_nand->bad_block_num; i < num; i++) {
		if (block_num == com_nand->bad_block[i])
			return 1;
	}
	return 0;
}

/*
 * BER=-226.8+0.1432*pe_cycle+3.499*read_count-1.40e-05*pe_cycle^2
 *     -0.000953*pe_cycle*read_count+7.27e-10*pe_cycle^3+pe_cycle^2*read_count;
 * reference: Reserch on error characteristics modeling of nand flash and applications
 */
static int common_nand_err_bit_gen(struct common_nand *com_nand, int block)
{
	unsigned int pe_cycle, read_count;
	double err_bit = 0.0;

	pe_cycle = com_nand->block_info[block].pe_cycle;
	read_count = com_nand->block_info[block].read_count;

	switch (com_nand->base.temperature) {
	case TEMPERATURE_NORMAL:
		break;
	case TEMPERATURE_HIGH:
		err_bit = -2260.8 + 0.1432 * pe_cycle + 3.499 * read_count - 1.40E-05 * power(pe_cycle, 2)
				  -0.000953 * pe_cycle * read_count + 7.27E-10 * power(pe_cycle, 3) + power(pe_cycle, 2) * read_count;
		break;
	case TEMPERATURE_LOW:
		break;
	default:
		ASSERT(0);
	}
	
	LOG(LOG_WARN, "BER: %f\n", err_bit);

	return MAX((int)err_bit, 0);
}

/************************CALLBACK FUNCTION IMPLEMENT***************************/
static int common_nand_erase(struct nand_base *nand, int row)
{
	int first_row, i, block;
	struct common_nand *com_nand = (struct common_nand *)nand;

	com_nand->status = 0;
	first_row = row / com_nand->page_num_per_block * com_nand->page_num_per_block;
	block = first_row / com_nand->page_num_per_block;
	if (common_nand_bad_block(com_nand, block)) {
		LOG(LOG_WARN, "Erase fail at block %d", block);
		com_nand->status |= (1 << STATUS_FAIL);
		return -1;
	}
	
	for (i = 0; i < com_nand->page_num_per_block; i++) {
		bitmap_clear(com_nand->page_map, first_row + i);
	}
	com_nand->block_info[block].pe_cycle++;
	return 0;
}

static int common_nand_read_page(struct nand_base *nand, int row, void *data)
{
	unsigned char *buf;
	int block, size;
	struct common_nand *com_nand = (struct common_nand *)nand;

	com_nand->status = 0;
	block = row / com_nand->page_num_per_block;
	row = row % com_nand->page_num_per_block;

	if (common_nand_bad_block(com_nand, block)) {
		LOG(LOG_WARN, "read bad block %d", block);
		return -1;
	}

	size = nand->page_size + nand->spare_size;

	if (bitmap_get(com_nand->page_map, row) == 0) {
		memset(data, 0xFF, size);
		return 0;
	}
	buf = file_read(com_nand->block_file, block);
	memcpy(data, buf + row * size, size);
	com_nand->block_info[block].read_count++;
	return common_nand_err_bit_gen(com_nand, block);
}

static int common_nand_program_page(struct nand_base *nand, int row, void *data)
{
	unsigned char *buf;
	int block, size, offset;
	struct common_nand *com_nand = (struct common_nand *)nand;

	com_nand->status = 0;
	if (bitmap_get(com_nand->page_map, row) != 0) {
		LOG(LOG_WARN, "re-program page %d", row);
		com_nand->status |= (1 << STATUS_FAIL);
		return -1;
	}

	bitmap_set(com_nand->page_map, row);
	if (data == NULL) {
		LOG(LOG_WARN, "No data program");
		return 0;
	}

	block = row / com_nand->page_num_per_block;
	size = nand->page_size + nand->spare_size;
	offset = row % com_nand->page_num_per_block * size;

	if (com_nand->block_info[block].pe_cycle > nand->max_pe_cycle) {
		LOG(LOG_WARN, " %d", row);
		com_nand->status |= (1 << STATUS_FAIL);
		return -2;
	}

	buf = file_write_cache(com_nand->block_file, block);
	memcpy(buf + offset, data, size);
	// for test
	//file_write(com_nand->block_file, block);
	return 0;
}

static int common_nand_command(struct nand_base *nand, int cmd, int addr, void *data)
{
	int i;
	unsigned char *buf = data;
	struct common_nand *com_nand = (struct common_nand *)nand;

	switch (cmd) {
	case CMD_READ_STATUS:
	case CMD_READ_STATUS_EX:
		if (buf) 
			buf[0] = com_nand->status;
		com_nand->status = 0;
		break;
	case CMD_READ_ID:
		if (buf) {
			for (i = 0; i < 6; i++)
				buf[i] = 0xCC;
		}
		break;
	case CMD_GET_FEATURE:
	case CMD_LUN_GET_FEATURE:
		/* Todo */
		break;
	case CMD_SET_FEATURE:
	case CMD_LUN_SET_FEATURE:
		/* Todo */
		break;
	case CMD_RESET_LUN:
	case CMD_SYNC_RESET:
	case CMD_RESET:
		file_flush(com_nand->block_file);
		break;
	default:
		if (buf)
			buf[0] = 0xEE;
		break;
	}
	return 0;
}
/************************CALLBACK FUNCTION END***************************/


struct nand_base *common_nand_init(char *name)
{
	int i, len;
	FILE *fp;
	struct common_nand *com_nand;
	char buf[64] = {'\0'};
	int value[COMMON_NAND_INFO_NUM] = {-1};

	/* open nand info file */
	sprintf(buf, NAND_INFO_FOLDER"/%s.ini", name);
	fp = fopen(buf, "r");
	if (!fp) {
		LOG(LOG_WARN, NAND_INFO_FOLDER"/%s.ini not found!", name);
		if (access(NAND_INFO_FOLDER, 0))
			mkdir(NAND_INFO_FOLDER, 0777);
		fp = fopen(buf, "w");
		if (!fp)
			return NULL;
		fprintf(fp, "%s: %s\n", common_nand_info[0], COMMON_NAND_NAME);
		for (i = 1; i < COMMON_NAND_INFO_NUM; i++) {
			fprintf(fp, "%s: %d\n", common_nand_info[i], common_nand_value[i]);
			value[i] = common_nand_value[i];
		}
	} else {
		/* skip 1st info */
		memset(buf, 0, 64);
		fgets(buf, 64, fp);
		while (!feof(fp)) {
			memset(buf, 0, 64);
			fgets(buf, 64, fp);
			for (i = 1; i < COMMON_NAND_INFO_NUM; i++) {
				len = strlen(common_nand_info[i]);
				if (!strncmp(buf, common_nand_info[i], len)) {
					value[i] = atoi(buf + len + 1);
					LOG(LOG_WARN, "info:%s value:%d", buf, value[i]);
					break;
				}
			}
		}
	}
	fclose(fp);

	com_nand = (struct common_nand *)mem_alloc(sizeof(struct common_nand) +
										(value[7] + value[8]) * sizeof(int));

	com_nand->base.block_size = value[1] << 10;
	com_nand->base.page_size = value[2];
	com_nand->base.spare_size = value[3];
	com_nand->base.block_num = value[4];
	com_nand->base.ecc_required = value[5];
	com_nand->base.max_pe_cycle = value[8];
	com_nand->base.temperature = value[10] > 20 ? (value[10] > 60 ?
									TEMPERATURE_HIGH : TEMPERATURE_NORMAL) : TEMPERATURE_LOW;
	com_nand->page_num_per_block = com_nand->base.block_size / com_nand->base.page_size;
	com_nand->bad_block_num = value[6];
	com_nand->weak_block_num = value[7];
	com_nand->weak_pe_cycle = value[9];
	com_nand->base.erase = common_nand_erase;
	com_nand->base.read = common_nand_read_page;
	com_nand->base.program = common_nand_program_page;
	com_nand->base.command = common_nand_command;

	LOG(LOG_WARN, "block_size: %d", com_nand->base.block_size);
	LOG(LOG_WARN, "page_size: %d", com_nand->base.page_size);
	LOG(LOG_WARN, "spare_size: %d", com_nand->base.spare_size);
	LOG(LOG_WARN, "block_num: %d", com_nand->base.block_num);
	LOG(LOG_WARN, "ecc_required: %d", com_nand->base.ecc_required);
	LOG(LOG_WARN, "max_pe_cycle: %d", com_nand->base.max_pe_cycle);
	LOG(LOG_WARN, "temperature: %d", com_nand->base.temperature);
	LOG(LOG_WARN, "page_num_per_block: %d", com_nand->page_num_per_block);
	LOG(LOG_WARN, "bad_block_num: %d", com_nand->bad_block_num);
	LOG(LOG_WARN, "weak_block_num: %d", com_nand->weak_block_num);
	LOG(LOG_WARN, "weak_pe_cycle: %d", com_nand->weak_pe_cycle);
	
	com_nand->page_map = bitmap_create(
							com_nand->page_num_per_block * com_nand->base.block_num, 0);

	fp = fopen(NAND_INFO_FOLDER"/"PAGE_MAP_FILE_NAME, "rb");
	if (fp) {
		fread(com_nand->page_map, 1,
				com_nand->page_num_per_block * com_nand->base.block_num >> 3, fp);
		fclose(fp);
	}

	com_nand->block_file = file_create(name, (com_nand->base.page_size +
							com_nand->base.spare_size) * com_nand->page_num_per_block,
							4, COMPRESS_BROTLI); // FILE_MAX_HANDLER

	com_nand->block_info = (struct nand_block *)mem_alloc(com_nand->base.block_num *
															sizeof(struct nand_block));

	fp = fopen(NAND_INFO_FOLDER"/"BLOCK_INFO_FILE_NAME, "rb");
	if (fp) {
		fread(com_nand->block_info, sizeof(struct nand_block),
				com_nand->base.block_num, fp);
		fclose(fp);
	}

	fp = fopen(NAND_INFO_FOLDER"/"BAD_BLOCK_FILE_NANE, "rb");
	if (fp) {
		fread(com_nand->bad_block, sizeof(int),
				com_nand->bad_block_num + com_nand->weak_block_num, fp);
		fclose(fp);
	} else {
		common_nand_bad_block_alloc(com_nand);
	}

	return (struct nand_base *)com_nand;
}


void common_nand_deinit(struct nand_base *nand)
{
	FILE *fp;
	struct common_nand *com_nand;

	com_nand = (struct common_nand *)nand;
	fp = fopen(NAND_INFO_FOLDER"/"PAGE_MAP_FILE_NAME, "wb");
	if (fp) {
		fwrite(com_nand->page_map, 1,
				com_nand->page_num_per_block * com_nand->base.block_num >> 3, fp);
		fclose(fp);
	}
	fp = fopen(NAND_INFO_FOLDER"/"BLOCK_INFO_FILE_NAME, "wb");
	if (fp) {
		fwrite(com_nand->block_info, 1,
				com_nand->base.block_num * sizeof(struct nand_block), fp);
		fclose(fp);
	}
	mem_free(com_nand->block_info);
	bitmap_delete(com_nand->page_map);
	file_flush(com_nand->block_file);
	file_delete(com_nand->block_file);
	mem_free(com_nand);
}





