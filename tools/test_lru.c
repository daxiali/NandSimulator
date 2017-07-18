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

#define MAX_KEY_NUM		(100)

int remove_key(void *data, void **userdata)
{
	struct lru_node *node;

	node = (struct lru_node *)data;
	printf("[CB]remove key: %d, content: %d\n", node->key, node->buffer[0]);
	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	int size, num, key;
	struct lru_cache *test_cache;
	unsigned char *buf;

	if (argc != 3) {
		printf("[Usage]:%s [size] [num]\n", argv[0]);
		return 0;
	}

	size = atoi(argv[1]);
	num = atoi(argv[2]);

	printf("size:%d num:%d\n", size, num);
	test_cache = lru_create(size, num);
	if (!test_cache) {
		printf("alloc lru cache fail!\n");
		return -1;
	}

	for (i = 1; i <= num; i++) {
		printf("key: %d\n", i);
		buf = lru_set(test_cache, i, &remove_key, NULL);
		buf[0] = i;
		buf = lru_get(test_cache, i);
		if (buf[0] != i)
			printf("[1]error get at key %d\n", i);
	}

	srand(time(0));
	for (i = 0; i < num; i++) {
		key = rand() % MAX_KEY_NUM;
		printf("key: %d\n", key);
		buf = lru_get(test_cache, key);
		if (!buf)
			printf("[2]key %d buffer not found\n", key);
		buf = lru_set(test_cache, key, &remove_key, NULL);
		if (!buf)
			printf("[3]key %d buffer cannot set\n", key);
		else
			buf[0] = key;
	}

	printf("\nClear:\n");
	lru_for_each(test_cache, &remove_key, NULL);
	lru_delete(test_cache);
	return 0;
}