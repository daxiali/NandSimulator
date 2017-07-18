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

#ifndef __NAND_H__
#define __NAND_H__
/*
 * Nand_Info: format
 * Name: COMMON_NAND
 * Block_Size: 4096 (KB)
 * Page_Size: 16384 (B)
 * Spare_Size: 2048 (B)
 * Block_Num: 2048
 * ECC_Required: 40
 * Bad_Block_Num: 100
 * Weak_Block_Num: 20
 * PE_Cycle: 3000
 * Weak_PE_Cycle: 2000
 * Temperature: 28
 * ... vendor defined
 */
#define NAND_INFO_FOLDER				"NandInfo"


#define CMD_READ_1ST					0x00
#define CMD_READ_2ND					0x30
#define CMD_READ_MULTI_PLANE_2ND		0x32
#define CMD_COPYBACK_READ_2ND			0x35
#define CMD_CHANGE_READ_COLUMN_1ST      0x05
#define CMD_CHANGE_READ_COLUMN_2ND		0xe0
#define CMD_CHANGE_READ_COLUMN_EX_1ST	0x06
#define CMD_READ_CACHE_RANDOM_2ND		0x31
#define CMD_READ_CACHE_SEQ				0x31
#define CMD_READ_CACHE_END				0x3f
#define CMD_ERASE_1ST					0x60
#define CMD_ERASE_2ND					0xd0
#define CMD_ERASE_MULTI_PLANE_2ND		0xd1
#define CMD_READ_STATUS					0x70
#define CMD_READ_STATUS_EX				0x78
#define CMD_PROGRAM_1ST					0x80
#define CMD_PROGRAM_2ND					0x10
#define CMD_PROGRAM_MULTI_PLANE_2ND		0x11
#define CMD_CACHE_PROGRAM_2ND			0x15
#define CMD_COPYBACK_PROGRAM_1ST		0x85
#define CMD_CHANGE_WRITE_COLUMN			0x85
#define CMD_CHANGE_ROW_ADDRESS			0x85
#define CMD_READ_ID						0x90
#define CMD_VOLUME_SELECT				0xe1
#define CMD_ODT_CONFIGURE				0xe2
#define CMD_READ_PARAMETER_PAGE			0xeC
#define CMD_UNIQUE_ID					0xed
#define CMD_GET_FEATURE					0xee
#define CMD_SET_FEATURE					0xef
#define CMD_LUN_GET_FEATURE				0xd4
#define CMD_LUN_SET_FEATURE				0xd5
#define CMD_ZQ_CALIBRATION_SHORT		0xd9
#define CMD_ZQ_CALIBRATION_LONG			0xf9
#define CMD_RESET_LUN					0xfA
#define CMD_SYNC_RESET					0xfc
#define CMD_RESET						0xff


#define FLASH_OK						(0)
#define FLASH_BITFLIP					(-1)
#define FLASH_ERROR						(-2)
#define FLASH_BAD						(-3)


enum flash_face {
	NONE,
	ONFI_4,
	TOGGLE_3,
	LBA
};


enum flash_type {
	COMMON,
	MICRON,
	TOSHIBA,
	SANDISK,
	HYNIX,
	SAMSUNG
};


struct nand_cmdq {
	int row;
	int cmd;
};

struct nand_ops {
	void *buffer;
	int cmd_num;
	struct nand_cmdq cmdq[];
};


struct nand_base {
	int block_size;
	int page_size;
	int spare_size;
	int block_num;
	int ecc_required;
	int max_pe_cycle;
	int temperature;
	/* private method start */
	int (*erase)(struct nand_base *nand, int row);
	int (*read)(struct nand_base *nand, int row, void *data); // read page
	int (*program)(struct nand_base *nand, int row, void *data); // program page
	int (*command)(struct nand_base *nand, int cmd, int addr, void *data);
	/* private method end */
};


/*
 * nand_init - init a nand object from Nand_Info folder
 * @nand_type: customized nand type of info & operations
 * @nand_name: Nand_Info name
 *
 * Returns nand_base pbject, otherwise NULL
 */
struct nand_base *nand_init(enum flash_type nand_type, char *nand_name);


/*
 * nand_deinit - remove nand_base object
 * @nand_type: customized nand type of info & operations
 * @nand: nand_base object
 */
void nand_deinit(enum flash_type nand_type, struct nand_base *nand);


struct nand_ops *nand_ops_alloc(int cmd_num, int buf_size);
void nand_ops_free(struct nand_ops *ops);
/*
 * nand_cmd - send cmd to nand
 * @nand: created nand_base object
 * @ops: nand command operation sequence
 *
 * Returns zero if success, otherwise non-zero
 */
int nand_cmd(struct nand_base *nand, struct nand_ops *ops);


int nand_read_page(struct nand_base *nand, int row, int col, void *data, void *oob);
int nand_write_page(struct nand_base *nand, int row, int col, void *data, void *oob);
int nand_erase_block(struct nand_base *nand, int row);
int nand_bad_block(struct nand_base *nand, int row);
void nand_mark_block(struct nand_base *nand, int row);
#endif
