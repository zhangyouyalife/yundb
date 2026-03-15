#ifndef _BLOCK_
#define _BLOCK_

#include <stdint.h>

#include "block/block_types.h"

int blk_types_init();

struct __attribute__((packed)) blk_header
{
    uint8_t     type;
};

#define BLK_TYPE(b) (((struct blk_header*) (b))->type)

#define BLK_OPS_NOT_SUPPORTED   -1
#define BLK_OPS_FAIL            -2

/*
 * For all operations,
 * if not supported, return BLK_OPS_NOTSUPPORTED.
 */
struct blk_ops
{
    /*
     * Initialize block according to type.
     *
     * Return
     *  Always return 0
     */
    int     (*blk_init)(char *blk, int type);

    /*
     * Create a tuple entry in data block.
     *
     * Return
     *  If success, the newly created entry index.
     *  otherwise BLK_OPS_FAIL
     */
    int     (*blk_entry_create)(char *blk, int sz);

    /*
     * Delete a tuple entry in data block.
     *
     * Return
     *  If success, 0
     *  otherwise BLK_OPS_FAIL
     */
    int     (*blk_entry_delete)(char *blk, int n);

    /*
     * Update a tuple entry in data block.
     *
     * Return
     *  If success, 0.
     *  otherwise BLK_OPS_FAIL
     */
    int     (*blk_entry_update)(char *blk, int n, int sz); 

    /*
     * Get nth tuple entry in data block.
     *
     * Return
     *  If success, address of the entry.
     *  otherwise 0
     */
    char*   (*blk_entry_get)(char *blk, int n);

    /*
     * Get nth record address in data block.
     *
     * Return
     *  If success, address of the record.
     *  otherwise 0
     */
    char*   (*blk_record)(char *blk, int n);
};

int     blk_init(char *blk, int type);
int     blk_entry_create(char *blk, int sz);
int     blk_entry_delete(char *blk, int n);
int     blk_entry_update(char *blk, int n, int sz); 
char*   blk_entry_get(char *blk, int n);
char*   blk_record(char *blk, int n);

#endif

