/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_


//
//-------Nodes for the Queue-----------
//
typedef struct _queue_node_
{
	//Pointer to the Object in the Node
	void *m_object;
	//Pointer to the Next node in the queue
	struct _queue_node_ *m_next;
} node_t;

node_t* new_node(void *object_ptr);
/**
  Priqueue Data Structure
*/
typedef struct priqueue_t
{
	//Current size of the queue
	int m_size;
	//Pointer to the node at the head of the queue
	node_t *m_head;
	//Function that'll be used to compare pointers upon insert
	int (*comparer)(const void *, const void *);
} priqueue_t;


void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
