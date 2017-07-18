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
#ifndef __FILE_H__
#define __FILE_H__

#define FILE_COMPRESS_BROTLI
#define FILE_MAX_HANDLER	(32)


enum file_compress {
	COMPRESS_NONE,
	COMPRESS_BROTLI
};


struct file_info {
	char name[16];
	int compress;
	int size;
	int num;
	struct lru_cache *cache;
};


/*
 * file_create - create file object
 * @size: file size
 * @num: file handler number, max:FILE_MAX_HANDLER
 * @comp: compress type
 *
 * Returns file object if success, otherwise NULL
 */
struct file_info *file_create(char name[], int size, int num, enum file_compress comp);


/*
 * file_read - read file of id to cache
 * @info: file object
 * @id: file id
 *
 * Returns data address if success, otherwise NULL
 */
void *file_read(struct file_info *info, int id);


/*
 * file_write - write file data of id to cache or file
 * @info: file object
 * @id: file id
 * @direct: if TRUE, write to file
 *
 * Returns zero if success, otherwise non-zero
 */
int file_write(struct file_info *info, int id);


/*
 * file_flush - flush cache to file
 * @info: file object
 *
 * Returns zero if success, otherwise non-zero
 */
int file_flush(struct file_info *info);


/*
 * file_write_cache - get cache address of id to write
 * @info: file object
 * @id: file id
 *
 * Returns cache address if success, otherwise NULL
 */
void *file_write_cache(struct file_info *info, int id);


/*
 * file_delete - delete file object
 * @info: file object
 */
void file_delete(struct file_info *info);


/*
 * data_compress - compress data from a buffer to another
 * @in_data: origin data buffer
 * @in_size: origin data buffer size
 * @out_data: compressed data buffer
 * @out_size: compressed data buffer size
 */
void data_compress(char *in_data, int in_size, char *out_data, int *out_size, int compress_type);


/*
 * data_decompress - decompress data from a buffer to another
 * @in_data: compressed data buffer
 * @in_size: compressed data buffer size
 * @out_data: decompressed data buffer
 * @out_size: decompressed data buffer size
 */
void data_decompress(char *in_data, int in_size, char *out_data, int *out_size, int compress_type);

#endif // __FILE_H__
