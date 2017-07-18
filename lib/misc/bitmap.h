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

#ifndef __BITMAP_H__
#define __BITMAP_H__

struct bitmap {
	unsigned int base;
	unsigned int size;
	unsigned char b[];
};


/*
 * bitmap_create - create bitmap array
 * @size: total bit for map
 * @base: based bit
 * Returns bitmap if success, otherwise return NULL
 */
struct bitmap *bitmap_create(unsigned int size, unsigned int base);


/*
 * bitmap_set - set bit of index to 1
 * @bm: bitmap object
 * @index: bit index
 */
void bitmap_set(struct bitmap *bm, unsigned int index);


/*
 * bitmap_set - set bit of index to 0
 * @bm: bitmap object
 * @index: bit index
 */
void bitmap_clear(struct bitmap *bm, unsigned int index);

/*
 * bitmap_get - get bit value of index
 * @bm: bitmap object
 * @index: bit index
 *
 * Returns bit value 0 or non-zero
 */
int bitmap_get(struct bitmap *bm, unsigned int index);


/*
 * bitmap_delete - destory the bitmap
 * @bm: created bitmap
 */
void bitmap_delete(struct bitmap *bm);

#endif
