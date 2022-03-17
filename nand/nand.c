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

#include <common.h>
#include <time.h>
#include "nand.h"

#define STORE_CMD_TO_FILE
#define CMDQ_FILE_NAME					"commandq.log"

#define CMD_READ_ERR		1
#define CMD_PROGRAM_ERR		2
#define CMD_ERASE_ERR		3

/********************External Function**********************/
extern struct nand_base *common_nand_init(char *name);
extern void common_nand_deinit(struct nand_base *nand);
extern struct nand_base *micron_nand_init(char *name);
extern void micron_nand_deinit(struct nand_base *nand);
/***********************************************************/

static FILE *cmd_record_fp = NULL;

#ifdef STORE_CMD_TO_FILE
static void store_cmdq(int cmd_num, struct nand_cmdq *cmdq)
{
	int i, size;
	char *buf;
	time_t t;
	struct tm *p;

	if (!cmd_record_fp)
		return;
	buf = mem_alloc((cmd_num + 2) * 24);
	time(&t);
	p = localtime(&t);
	// 22
	sprintf(buf, "[%04d-%02d-%02d %02d:%02d:%02d]\n",
					1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
					p->tm_hour, p->tm_min, p->tm_sec);


	for (i = 0; i < cmd_num; i++) {
		// 23
		sprintf(buf + strlen(buf), "  %08x %02x\n", cmdq[i].row, cmdq[i].cmd);
	}

	fwrite(buf, 1, strlen(buf), cmd_record_fp);
	mem_free(buf);
}
#else
#define  store_cmdq(cmd_num, cmdq)
#endif


struct nand_ops *nand_ops_alloc(int cmd_num, int buf_size)
{
	struct nand_ops *ops;

	ops = (struct nand_ops *)mem_alloc(sizeof(struct nand_ops) +
								cmd_num * sizeof(struct nand_cmdq));
	ops->cmd_num = cmd_num;
	ops->buffer = mem_alloc(buf_size);
	return ops;
}


void nand_ops_free(struct nand_ops *ops)
{
	mem_free(ops->buffer);
	mem_free(ops);
}

int nand_cmd(struct nand_base *nand, struct nand_ops *ops)
{
	int i, ret;
	char *buf;
	int rcount, wcount, ecount;
	int cc_read, cc_write;

	rcount = wcount = ecount = 0;
	cc_read = cc_write = 0;

	ret = 0;
	for (i = 0; i < ops->cmd_num; i++) {
		if (ret) {
			LOG(LOG_WARN, "command fail %d\n", ret);
			return ret;
		}

		buf = ops->buffer;

		switch (ops->cmdq[i].cmd) {
		case CMD_READ_1ST:
			rcount++;
			break;

		case CMD_READ_CACHE_SEQ:
		case CMD_READ_CACHE_END:
			buf = (char *)ops->buffer + cc_read * (nand->page_size + nand->spare_size);
			cc_read++;
		case CMD_COPYBACK_READ_2ND:
		case CMD_READ_2ND:
		case CMD_READ_MULTI_PLANE_2ND:
			if (rcount == 1) {
				rcount = 0;
				ret = nand->read(nand, ops->cmdq[i].row, buf);
			} else
				ret = -CMD_READ_ERR;
			break;


		case CMD_ERASE_1ST:
			ecount++;
			break;

		case CMD_ERASE_2ND:
		case CMD_ERASE_MULTI_PLANE_2ND:
			if (ecount == 1) {
				ecount = 0;
				ret = nand->erase(nand, ops->cmdq[i].row);
			} else
				ret = -CMD_ERASE_ERR;
			break;

		case CMD_COPYBACK_PROGRAM_1ST:
		case CMD_PROGRAM_1ST:
			wcount++;
			break;

		case CMD_CACHE_PROGRAM_2ND:
			buf = (char *)ops->buffer + cc_write * (nand->page_size + nand->spare_size);
			cc_write++;
		case CMD_PROGRAM_2ND:
		case CMD_PROGRAM_MULTI_PLANE_2ND:
			if (wcount == 1) {
				wcount = 0;
				ret = nand->program(nand, ops->cmdq[i].row, buf);
			} else
				ret = -CMD_PROGRAM_ERR;
			break;

		case CMD_READ_STATUS:
		case CMD_READ_STATUS_EX:
		case CMD_READ_ID:
		case CMD_VOLUME_SELECT:
		case CMD_ODT_CONFIGURE:
		case CMD_READ_PARAMETER_PAGE:
		case CMD_UNIQUE_ID:
		case CMD_GET_FEATURE:
		case CMD_SET_FEATURE:
		case CMD_LUN_GET_FEATURE:
		case CMD_LUN_SET_FEATURE:
		case CMD_ZQ_CALIBRATION_SHORT:
		case CMD_ZQ_CALIBRATION_LONG:
		case CMD_RESET_LUN:
		case CMD_SYNC_RESET:
		case CMD_RESET:
			ret = nand->command(nand, ops->cmdq[i].cmd, ops->cmdq[i].row, buf);
			break;

		default:
			LOG(LOG_ERR, "error command!!!");
			break;
		}
	}
	store_cmdq(ops->cmd_num, ops->cmdq);
	return ret;
}

int nand_read_page(struct nand_base *nand, int row, int col, void *data, void *oob)
{
	int ret;
	struct nand_ops *ops;

	ops = nand_ops_alloc(2, nand->page_size + nand->spare_size);
	ops->cmdq[0].row = -1;
	ops->cmdq[0].cmd = CMD_READ_1ST;
	ops->cmdq[1].row = row;
	ops->cmdq[1].cmd = CMD_READ_2ND;
	ret = nand_cmd(nand, ops);
	if (ret >= 0 && ret <= nand->ecc_required) {
		memcpy(data, (char *)ops->buffer + col, nand->page_size - col);
		memcpy(oob, (char *)ops->buffer + nand->page_size, nand->spare_size);
		ret = ret > 0 ? FLASH_BITFLIP : FLASH_OK;
	} else if (ret > nand->ecc_required) {
		LOG(LOG_ERR, "Uncorrect error bit %d at page %d", ret, row);
		ret = FLASH_ERROR;
	} else {
		ret = FLASH_BAD;
	}
	nand_ops_free(ops);
	return ret;
}


int nand_write_page(struct nand_base *nand, int row, int col, void *data, void *oob)
{
	int ret;
	struct nand_ops *ops;

	ops = nand_ops_alloc(2, nand->page_size + nand->spare_size);
	ops->cmdq[0].row = -1;
	ops->cmdq[0].cmd = CMD_PROGRAM_1ST;
	ops->cmdq[1].row = row;
	ops->cmdq[1].cmd = CMD_PROGRAM_2ND;
	memcpy((char *)ops->buffer + col, data, nand->page_size - col);
	memcpy((char *)ops->buffer + nand->page_size, oob, nand->spare_size);
	ret = nand_cmd(nand, ops);
	nand_ops_free(ops);
	return (ret < 0 ? FLASH_BAD : FLASH_OK);
}

int nand_erase_block(struct nand_base *nand, int row)
{
	int ret;
	struct nand_ops *ops;

	ops = nand_ops_alloc(2, 0);
	ops->cmdq[0].row = -1;
	ops->cmdq[0].cmd = CMD_ERASE_1ST;
	ops->cmdq[1].row = row;
	ops->cmdq[1].cmd = CMD_ERASE_2ND;
	ret = nand_cmd(nand, ops);
	nand_ops_free(ops);
	return (ret < 0 ? FLASH_BAD : FLASH_OK);
}

int nand_bad_block(struct nand_base *nand, int row)
{
	int ret;
	struct nand_ops *ops;

	ops = nand_ops_alloc(2, nand->page_size + nand->spare_size);
	ops->cmdq[0].row = -1;
	ops->cmdq[0].cmd = CMD_READ_1ST;
	ops->cmdq[1].row = row;
	ops->cmdq[1].cmd = CMD_READ_2ND;
	ret = nand_cmd(nand, ops);
	nand_ops_free(ops);
	return (ret < 0 ? FLASH_BAD : FLASH_OK);
}

void nand_mark_block(struct nand_base *nand, int row)
{
	int ret;
	struct nand_ops *ops;
	int page_num_per_block = nand->block_size / nand->page_size;

	row = row / page_num_per_block * page_num_per_block;
	ret = nand_erase_block(nand, row);
	if (ret == 0) {
		ops = nand_ops_alloc(2, 0);
		ops->cmdq[0].row = -1;
		ops->cmdq[0].cmd = CMD_PROGRAM_1ST;
		ops->cmdq[1].row = row;
		ops->cmdq[1].cmd = CMD_PROGRAM_2ND;
		nand_cmd(nand, ops);
		ops->cmdq[1].row = row + 1;
		nand_cmd(nand, ops);
		ops->cmdq[1].row = row + page_num_per_block - 1;
		nand_cmd(nand, ops);
		nand_ops_free(ops);
	}
}

struct nand_base *nand_init(enum flash_type nand_type, char *nand_name)
{
	struct nand_base *nand;

	switch (nand_type) {
	case COMMON:
		nand = common_nand_init(nand_name);
		break;

	case MICRON:
		nand = micron_nand_init(nand_name);
		break;

	case TOSHIBA:
	case SANDISK:
	case HYNIX:
	case SAMSUNG:
	default:
		LOG(LOG_WARN, "not found nand type %d", nand_type);
		nand = NULL;
		break;
	}
	if (!cmd_record_fp)
		cmd_record_fp = fopen(CMDQ_FILE_NAME, "w");
	return nand;
}


void nand_deinit(enum flash_type nand_type, struct nand_base *nand)
{
	switch (nand_type) {
	case COMMON:
		common_nand_deinit(nand);
		break;

	case MICRON:
		micron_nand_deinit(nand);
		break;

	case TOSHIBA:
	case SANDISK:
	case HYNIX:
	case SAMSUNG:
	default:
		LOG(LOG_WARN, "not found nand type %d", nand_type);
		break;
	}
	if (cmd_record_fp)
		fclose(cmd_record_fp);
}
