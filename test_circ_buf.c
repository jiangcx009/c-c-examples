/* circulate cache functions
 */
typedef struct ptcp_queue_node {
    list_entry entry;
    union {
        SSegment *ss;
        RSegment *rs;
    } seg;
};

typedef enum ptcp_queue_type {
    PTCP_SEND_Q,
    PTCP_RECV_Q
};

#define CIRC_SIZE (1024*16)
#define CIRC_MASK (CIRC_SIZE - 1)
#define circ_empty(circ)     ((circ)->head == (circ)->tail)
#define circ_free(circ)      CIRC_SPACE((circ)->head, (circ)->tail, CIRC_SIZE)
#define circ_cnt(circ)       CIRC_CNT((circ)->head, (circ)->tail, CIRC_SIZE)


typedef struct ptcp_queue {
    /* elements */
    char cache[CIRC_SIZE];
    ptcp_queue_node head;
    pthread_spin_lock_t lock;
    struct circ_buf circ_cache;
    ptcp_queue_type type;      
};


static bool ptcp_queue_init(ptcp_queue *q, ptcp_queue_type type)
{
    memset(q->cache, 0, sizeof(cache));
    
    LIST_HEAD_INIT(&q->head.entry);
    memset(&q->seg, 0);

    pthread_spin_init(&q->lock, 0);

    q->circ_cache.buf = q->cache;
    q->circ_cache.head = 0;
    q->circ_cache.tail = 0;

    q->type = type;
}

bool ptcp_queue_uninit(ptcp_queue *q)
{

}


bool ptcp_queue_add_tail(ptcp_queue *q, void *seg, char *data)
{
    int res = false;
    assert(sseg);

    pthread_spin_lock(&q->lock);
    ptcp_queue_node *node= (ptcp_queue_node *)malloc(sizeof(*node));
    assert(node);

    if (q->type == PTCP_SEND_Q) {
        if (ptcp_queue_free_cache(q) >= node->seg.ss->len) {
            node->seg.ss = (SSegment *)seg;
            res = true;
        }
    } else if (q->type == PTCP_RECV_Q) {
        if (ptcp_queue_free_cache(q) >= node->seg.rs->len) {
            node->seg.rs = (SSegment *)seg;
            res = true;
        } 
    }

    if (true == res) {
        list_add_tail(&node->list, &q->head.list);
        
        
    } else {
        free(node);
    }
    pthread_spin_unlock(&q->lock);

    return res;
}
