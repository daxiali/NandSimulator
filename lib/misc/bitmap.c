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


struct bitmap *bitmap_create(unsigned int size, unsigned int base)
{
	struct bitmap *bm;

	LOG(LOG_WARN, "size:%u base:%d", (roundup(size, 8) >> 3), sizeof(struct bitmap));
	bm = (struct bitmap *)mem_alloc(sizeof(struct bitmap) + (roundup(size, 8) >> 3));
	bm->base = base;
	bm->size = size;

	return bm;
}


void bitmap_set(struct bitmap *bm, unsigned int index)
{
	unsigned int start, len;

	if (index > (bm->base + bm->size))
		return;
	start = (index - bm->base) >> 3;
	len = (index - bm->base) & 0x7;
	bm->b[start] |= (1 << len);
}

void bitmap_clear(struct bitmap *bm, unsigned int index)
{
	unsigned int start, len;

	if (index > (bm->base + bm->size))
		return;
	start = (index - bm->base) >> 3;
	len = (index - bm->base) & 0x7;
	bm->b[start] &= ~(1 << len);
}

int bitmap_get(struct bitmap *bm, unsigned int index)
{
	unsigned int start, len;

	if (index > (bm->base + bm->size))
		return -1;
	start = (index - bm->base) >> 3;
	len = (index - bm->base) & 0x7;
	return bm->b[start] & (1 << len);
}


void bitmap_delete(struct bitmap *bm)
{
	mem_free(bm);
}
