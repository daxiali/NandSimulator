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
#include "brotli/encode.h"
#include "brotli/decode.h"

#define COMPRESS_SIZE	(4*1024*1024)


int main(int argc, char *argv[])
{
	int rn = 0;
	bool compress;
	BROTLI_BOOL ret;
	size_t in_size, out_size;
	char *in_buf, *out_buf;
	FILE *fp_in, *fp_out;

	if (argc != 4) {
		printf("[Usage]: %s -C[/D] in_file out_file\n", argv[0]);
		return 0;
	}

	fp_in = fopen(argv[2], "rb");
	if (!fp_in) {
		printf("[Error]: open read file %s fail!\n", argv[1]);
		return -1;
	}

	fp_out = fopen(argv[3], "wb");
	if (!fp_out) {
		printf("[Error]: open write file %s fail!\n", argv[2]);
		fclose(fp_in);
		return -2;
	}

	fseek(fp_in, 0, SEEK_END);
	in_size = ftell(fp_in);
	rewind(fp_in);

	if (in_size != COMPRESS_SIZE) {
		printf("[Warning]: file size %d must be align with %d\n \t else would be forced align\n",
					in_size, COMPRESS_SIZE);
	}

	if (in_size > COMPRESS_SIZE)
		in_size = COMPRESS_SIZE;
	out_size = COMPRESS_SIZE;

	in_buf = mem_alloc(in_size);
	out_buf = mem_alloc(out_size);

	fread(in_buf, 1, in_size, fp_in);

	if (!strcmp(argv[1], "-C")) {
		ret = BrotliEncoderCompress(BROTLI_DEFAULT_QUALITY, BROTLI_DEFAULT_WINDOW,
					BROTLI_DEFAULT_MODE, in_size, in_buf, &out_size, out_buf);
		compress = true;
	} else if (!strcmp(argv[1], "-D")) {
		ret = BrotliDecoderDecompress(in_size, in_buf, &out_size, out_buf);
		compress = false;
	} else {
		printf("[Error]: invalid argument %s\n", argv[1]);
		rn = -3;
		goto out;
	}

	if (ret == BROTLI_FALSE) {
		printf("[Error]: %s fail!\n", compress ? "compress" : "decompress");
		rn = -4;
		goto out;
	}

	fwrite(out_buf, 1, out_size, fp_out);

out:
	fclose(fp_in);
	fclose(fp_out);
	mem_free(in_buf);
	mem_free(out_buf);
	return rn;
}