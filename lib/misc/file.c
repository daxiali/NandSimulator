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
#include "lru.h"
#include "file.h"
#include "brotli/encode.h"
#include "brotli/decode.h"


void data_compress(char *in_data, int in_size, char *out_data, int *out_size, int compress_type)
{
	BROTLI_BOOL ret;

	ASSERT(out_data);
	switch (compress_type) {
	case COMPRESS_NONE:
		memcpy(out_data, in_data, *out_size);
		break;
	case COMPRESS_BROTLI:
		ret = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW,
				BROTLI_DEFAULT_MODE, in_size, in_data, (size_t *)out_size, out_data);
		if (ret == BROTLI_FALSE)
			memcpy(out_data, in_data, *out_size);
		break;
	default:
		ASSERT(0);
	}
}


void data_decompress(char *in_data, int in_size, char *out_data, int *out_size, int compress_type)
{
	BROTLI_BOOL ret;

	ASSERT(out_data);
	switch (compress_type) {
	case COMPRESS_NONE:
		memcpy(out_data, in_data, *out_size);
		break;
	case COMPRESS_BROTLI:
		ret = BrotliDecoderDecompress(in_size, in_data, (size_t *)out_size, out_data);
		if (ret == BROTLI_FALSE)
			memcpy(out_data, in_data, *out_size);
		break;
	default:
		ASSERT(0);
	}
}


static int file_lru_cb(void *data, void **userdata)
{
	char *buf, *wbuf;
	int out_size;
	FILE *fp = NULL;
	struct lru_node *node;
	struct file_info *info;
	unsigned long value;

	node = (struct lru_node *)data;
	info = (struct file_info *)(*userdata);

	wbuf = buf = node->buffer;
	value = *(unsigned long *)(buf + info->size);
	fp = (FILE *)value;
	if (!fp) {
		LOG(LOG_WARN, "file handler is NULL");
		return 0;
	}
	out_size = info->size;
	if (info->compress) {
		//wbuf = mem_alloc(out_size);
		data_compress(buf, info->size, wbuf, &out_size, info->compress);
	}
	fwrite(wbuf, 1, out_size, fp);
	memset(buf, 0, info->size);
	//if (info->compress)
	//	mem_free(wbuf);
	fclose(fp);
	*(unsigned long *)(buf + info->size) = (unsigned long)NULL;
	return 0;
}


struct file_info *file_create(char name[], int size, int num, enum file_compress comp)
{
	int sz;
	struct file_info *info;

	sz = MIN(strlen(name), 15);
	info = mem_alloc(sizeof(struct file_info));
	memcpy(info->name, name, sz);
	info->compress = comp;
	info->size = size;
	num = roundup_power2(num);
	info->num = MIN(FILE_MAX_HANDLER, num);
	printf("num: %d", info->num);
	info->cache = lru_create(size + sizeof(FILE *), info->num);

	if (access(info->name, 0))
		mkdir(info->name, 0777);

	return info;
}


void *file_read(struct file_info *info, int id)
{
	int ret, size, out_size;
	FILE *fp = NULL;
	char *buf, *rbuf;
	char name[16] = {'\0'};

	buf = lru_get(info->cache, id);
	if (buf)
		return buf;

	sprintf(name, "%s/%d", info->name, id);
	fp = fopen(name, "rb");
	if (!fp) {
		LOG(LOG_WARN, "Cannot read file %s", name);
		return NULL;
	}

	rbuf = buf = lru_set(info->cache, id, file_lru_cb, (void **)&info);

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);

	ASSERT(size <= info->size);
	if (info->compress)
		rbuf = mem_alloc(size);

	fread(rbuf, 1, size, fp);
	fclose(fp);
	fp = fopen(name, "wb+");

	if (info->compress) {
		out_size = info->size;
		data_decompress(rbuf, size, buf, &out_size, info->compress);
		mem_free(rbuf);
	}

	ASSERT(fp);
	*(unsigned long *)(buf + info->size) = (unsigned long)fp;

	return buf;
}


int file_write(struct file_info *info, int id)
{
	FILE *fp = NULL;
	char *buf, *wbuf;
	int out_size;
	unsigned long value;

	wbuf = buf = lru_get(info->cache, id);
	if (!buf)
		return -1;

	value = *(unsigned long *)(buf + info->size);
	fp = (FILE *)value;
	out_size = info->size;
	if (info->compress) {
		//wbuf = mem_alloc(out_size);
		data_compress(buf, info->size, wbuf, &out_size, info->compress);
	}
	fwrite(wbuf, 1, out_size, fp);
	//if (info->compress)
		//mem_free(wbuf);
	fclose(fp);
	*(unsigned long *)(buf + info->size) = (unsigned long)NULL;
	return 0;
}


int file_flush(struct file_info *info)
{
	return lru_for_each(info->cache, file_lru_cb, (void **)&info);
}


void *file_write_cache(struct file_info *info, int id)
{
	FILE *fp = NULL;
	unsigned long value;
	unsigned char *buf;
	char name[16] = {'\0'};

	buf = lru_set(info->cache, id, file_lru_cb, (void **)&info);
	value = *(unsigned long *)(buf + info->size);
	fp = (FILE *)value;
	if (fp == NULL) {
		sprintf(name, "%s/%d", info->name, id);
		fp = fopen(name, "wb+");
		ASSERT(fp);
		*(unsigned long *)(buf + info->size) = (unsigned long)fp;
	}
	return buf;
}


void file_delete(struct file_info *info)
{
	lru_delete(info->cache);
	mem_free(info);
}

