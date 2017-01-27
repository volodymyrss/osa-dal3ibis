#include "dal3ibis.h"
#include "dal3hk.h"
#include "dal3aux.h"
#include "ril.h"


typedef struct DAL_GC_allocation_struct {
    char comment[DAL_MAX_STRING];
    void *ptr;
    DAL_GC_RESOURCE_KIND resource_kind;
} DAL_GC_allocation_struct;


static struct DAL_GC_struct {
    int n_entries;
    DAL_GC_allocation_struct allocations[DAL_GC_MAX_ALLOCATIONS];
} DAL_GC;

void DAL_GC_register_allocation(void *ptr,
        DAL_GC_RESOURCE_KIND resource_kind,
        char *comment);

void DAL_GC_print();

int DAL_GC_free_all(int chatter, int status);

int DAL_GC_allocateDataBuffer(void **buffer, 
                              long buffSize, 
                              int status,
                              char *comment);

int DAL_GC_freeDataBuffer(void *buffer,
                      int   status);


int DAL_GC_objectOpen(const char   *DOL,    
        dal_object   *object,
        int           status);


