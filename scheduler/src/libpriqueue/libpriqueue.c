/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"

node_t* new_node(void* object_ptr){
  node_t * p = (node_t*)malloc(sizeof(node_t));
  p->m_object = object_ptr;
  p->m_next = NULL;
  return p;
}


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
  q->m_size = 0;
  q->m_head = NULL;
  q->comparer = comparer;
}


/**
  Inserts the specified element into this priority queue.
  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
	node_t* node_ptr = new_node(ptr);
  int i;
  if(q->m_size ==0){
    q->m_head = node_ptr;
    q->m_size ++;
    return 0;
  }

  if(q->comparer(node_ptr->m_object,q->m_head->m_object) < 0)
  {
    node_ptr->m_next = q->m_head;
    q->m_head = node_ptr;
    q->m_size ++;
    return 0;
  }

  node_t* traverse_node = q->m_head;
  for (i = 0; i < q->m_size; ++i)
  {
    if(traverse_node->m_next == NULL || q->comparer(node_ptr->m_object,traverse_node->m_next->m_object)<0){
      node_ptr->m_next = traverse_node->m_next;
      traverse_node->m_next = node_ptr;
      q->m_size ++;
      return i+1;
    }
    else traverse_node = traverse_node -> m_next;
  }

  return -1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
	if(q->m_size == 0) return NULL;
  return q->m_head->m_object;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
	if(q->m_size == 0) return NULL;

  node_t * node_ptr = q->m_head;
  q->m_head = node_ptr->m_next;
  q->m_size --;

  void* p = node_ptr->m_object;
  free(node_ptr);
  return p;

}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
	if(index < 0){
    return NULL;
  }
  if(index >= q->m_size) return NULL;
  if(index == 0) return q->m_head->m_object;

  int i;
  node_t * traverse_at = q->m_head;
  for(i=1;i<= index;++i)
    traverse_at = traverse_at -> m_next;
  return traverse_at -> m_object;

}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
  int num_elements_deleted = 0;

  //
  // Check at the first index if the size is greater than 0
  //
  while(q->m_size >0 && ptr == q->m_head->m_object) {
    priqueue_poll(q);
    num_elements_deleted ++;
  }

  //
  // Use two nodes to traverse
  //
  node_t *find_remove;
  node_t *del_node;
  if(q->m_size > 0)
  {
    find_remove = q->m_head;
    while(find_remove->m_next != NULL ){
      if(ptr == find_remove->m_next->m_object){
        del_node = find_remove -> m_next;
        find_remove -> m_next = del_node -> m_next;
        q->m_size --;

        free(del_node);

        num_elements_deleted ++;
      }else{
        find_remove = find_remove -> m_next;
      }
    }
  }

  return num_elements_deleted;
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	if(index < 0){
    return NULL;
  }
  if(index >= q->m_size) return NULL;
  if(index == 0) return priqueue_poll(q);

  int i;
  node_t *traverse_at = q->m_head;
  node_t *node_ptr;
  for(i=1;i<= index -1 ;++i)
    traverse_at = traverse_at -> m_next;

  node_ptr = traverse_at -> m_next;
  traverse_at -> m_next = node_ptr -> m_next;
  q-> m_size --;
  void* obj_ptr = node_ptr->m_object;
  free(node_ptr);
  return obj_ptr;

}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->m_size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{
  while(q->m_size >0) priqueue_poll(q);
}
