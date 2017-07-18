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
#ifndef __LRU_H__
#define __LRU_H__

#define LRU_DEFAULT_KEY     0xffffffff

typedef int (*LRU_CALLBACK)(void *data, void **userdata);

struct lru_node {
	struct lru_node *prev;
	struct lru_node *next;
	struct lru_node *h_next;
	unsigned int key;
	char buffer[];
};


struct lru_cache {
	unsigned int size;
	unsigned int num;
	struct lru_node **table;
	struct lru_node *head; // lru list
	struct lru_node *free;  // free list
	struct lru_node cache[];
};


/*
 * lru_create - create the lru cache object.
 * @size: single cache size
 * @num: total cache number
 *
 * Returns lru object if success, otherwise NULL
 */
struct lru_cache *lru_create(unsigned int size, unsigned int num);


/*
 * lru_get - get the buffer of key
 * @lru: allocated lru cache object
 * @key: key number of buffer
 *
 * Returns buffer address if success, otherwise NULL
 */
void *lru_get(struct lru_cache *lru, unsigned int key);


/*
 * lru_set - set key to a buffer
 * @lru: allocated lru cache object
 * @key: key number of buffer
 * @lru_cb: lru callback function for last node
 * @userdata: import user info
 *
 * Returns buffer address of key, otherwise NULL
 */
void* lru_set(struct lru_cache *lru, unsigned int key, LRU_CALLBACK lru_cb, void **userdata);


/*
 * lru_for_each - iterate lru list node for each
 * @lru: allocated lru cache object
 * @lru_cb: lru callback function for each node
 * @userdata: import user info
 *
 * Returns zero if success, otherwise non-zero
 */
int lru_for_each(struct lru_cache *lru, LRU_CALLBACK lru_cb, void **userdata);


/*
 * lru_delete - delete allocated lru cache object
 * @lru: allocated lru cache object
 */
void lru_delete(struct lru_cache *lru);

#endif // __LRU_H__
