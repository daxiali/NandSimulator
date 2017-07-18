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
 
#ifndef __COMMON_NAND_H__
#define __COMMON_NAND_H__

#include "bitmap.h"
#include "file.h"


#define PAGE_MAP_FILE_NAME				"page_map.bin"
#define BLOCK_INFO_FILE_NAME			"block_info.bin"
#define BAD_BLOCK_FILE_NANE				"bad_block.bin"
#define COMMON_NAND_INFO_NUM			11

#define COMMON_NAND_NAME				"COMMON_NAND"

enum nand_temprature {
	TEMPERATURE_LOW,		//(<20)
	TEMPERATURE_NORMAL,		//(20~60)
	TEMPERATURE_HIGH		//(>60)
};

enum nand_status {
	STATUS_FAIL,
	STATUS_FAILC,
	STATUS_CSP = 3
};


struct nand_block {
	unsigned int pe_cycle;
	unsigned int read_count;
};


struct common_nand {
	struct nand_base base;
	struct bitmap *page_map;
	struct file_info *block_file;
	struct nand_block *block_info;
	int page_num_per_block;
	int bad_block_num;
	int weak_block_num;
	int weak_pe_cycle;
	unsigned int status;
	int bad_block[];
};

 #endif

