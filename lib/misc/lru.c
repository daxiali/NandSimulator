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


static unsigned int DJBhash(char *str, unsigned int len)
{
	unsigned int i;
	unsigned int hash = 5381;

	for (i = 0; i < len; str++, i++)
		hash = ((hash << 5) + hash) + (*str);

	return hash;
}


static void hash_insert(struct lru_cache *lru, struct lru_node *node)
{
	unsigned int index, size;

	size = calc_msb_index(roundup_power2(lru->size));
	index = DJBhash((char *)&node->key, sizeof(unsigned int)) & HASH_MASK(size);

	node->h_next = lru->table[index];
	lru->table[index] = node;
}


static struct lru_node *hash_find(struct lru_cache *lru, unsigned int key)
{
	unsigned int index, size;
	struct lru_node *node;

	size = calc_msb_index(roundup_power2(lru->size));
	index = DJBhash((char *)&key, sizeof(unsigned int)) & HASH_MASK(size);

	node = lru->table[index];
	while (node) {
		if (node->key == key)
			return node;
		node = node->h_next;
	}

	return NULL;
}


static void hash_delete(struct lru_cache *lru, unsigned int key)
{
	struct lru_node *last;
	struct lru_node *tmp;
	unsigned int index, size;

	size = calc_msb_index(roundup_power2(lru->size));
	index = DJBhash((char *)&key, sizeof(unsigned int)) & HASH_MASK(size);

	//LOG(LOG_WARN, "delete key:%d", key);
	last = tmp = lru->table[index];
	while (tmp && (key != tmp->key)) {
		last = tmp;
		tmp = tmp->h_next;
	}

	ASSERT(tmp);
	if (last == tmp) {
		lru->table[index] = NULL;
	} else {
		last->h_next = tmp->h_next;
		tmp->h_next = NULL;
	}
}


static void lru_push_head(struct lru_node **head, struct lru_node *node)
{
	if (*head == node) {
		//LOG(LOG_WARN, "head1:%x key:%d", *head, node->key);
		return;
	}
	if (*head) {
		//LOG(LOG_WARN, "head2:%x key:%d", *head, node->key);
		if (node->prev) {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}
		node->prev = (*head)->prev;
		node->next = (*head);
		node->prev->next = node;
		(*head)->prev = node;
	} else {
		node->prev = node;
		node->next = node;
	}
	*head = node;
}


static void lru_delete_node(struct lru_node **head, struct lru_node **free,
							struct lru_node *node)
{
	if (*head == (*head)->next) {
		ASSERT(*head == node);
		*head = NULL;
	} else {
		node->next->prev = node->prev;
		node->prev->next = node->next;
		if (*head == node)
			*head = node->next;
	}

	node->prev = node->next = NULL;
	node->key = LRU_DEFAULT_KEY;
	if (free)
		lru_push_head(free, node);
}


static void lru_remove_tail(struct lru_node **head, struct lru_node **free)
{
	lru_delete_node(head, free, (*head)->prev);
}


struct lru_cache *lru_create(unsigned int size, unsigned int num)
{
	int i, node_size;
	struct lru_cache *lru;
	struct lru_node *node;

	node_size = size + sizeof(struct lru_node);
	lru = mem_alloc(sizeof(struct lru_cache) + node_size * num);

	lru->size = size;
	lru->num = num;
	lru->head = NULL;

	lru->table = mem_alloc(roundup_power2(size) * sizeof(void *));

	/* initial lru node */
	lru->free = NULL;
	for (i = num - 1; i >= 0; i--) {
		node = (struct lru_node *)((char *)lru->cache + i * node_size);
		node->key = LRU_DEFAULT_KEY;
		lru_push_head(&lru->free, node);
	}
	return lru;
}


void* lru_get(struct lru_cache *lru, unsigned int key)
{
	struct lru_node *node;

	node = hash_find(lru, key);
	if (!node)
		return NULL;

	lru_push_head(&lru->head, node);
	return node->buffer;
}


void* lru_set(struct lru_cache *lru, unsigned int key, LRU_CALLBACK lru_cb, void **userdata)
{
	struct lru_node *node;

	node = hash_find(lru, key);
	if (node) {
		lru_push_head(&lru->head, node);
		return node->buffer;
	}

	if (lru->free) {
		node = lru->free;
		lru_delete_node(&lru->free, NULL, node);
		node->key = key;
		lru_push_head(&lru->head, node);
		hash_insert(lru, node);
		return node->buffer;
	}

	node = lru->head->prev;
	if (lru_cb)
		lru_cb(node, userdata);

	hash_delete(lru, node->key);
	lru_remove_tail(&lru->head, NULL);
	
	node->key = key;
	lru_push_head(&lru->head, node);
	//LOG(LOG_WARN, "head key:%d, %d", lru->head->key, key);
	hash_insert(lru, node);
	return node->buffer;
}


int lru_for_each(struct lru_cache *lru, LRU_CALLBACK lru_cb, void **userdata)
{
	int ret = 0;
	struct lru_node *node;

	if (lru_cb) {
		node = lru->head;
		while (node) {
			ret = lru_cb(node, userdata);
			if (ret)
				break;
			node = node->next;
			if (node == lru->head)
				break;
		}
	}
	return ret;
}


void lru_delete(struct lru_cache *lru)
{
	mem_free(lru->table);
	mem_free(lru);
}
