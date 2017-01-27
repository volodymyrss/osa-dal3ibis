#include "dal3ibis_calib_gc.h"



void DAL_GC_register_allocation(void *ptr, DAL_GC_RESOURCE_KIND resource_kind, char *comment) {
    DAL_GC.allocations[DAL_GC.n_entries].ptr=ptr;
    DAL_GC.allocations[DAL_GC.n_entries].resource_kind=resource_kind;
    strncpy(DAL_GC.allocations[DAL_GC.n_entries].comment,comment,DAL_MAX_STRING);
    DAL_GC.n_entries++;
}

void DAL_GC_print() {
    int i;

    for (i=0;i<DAL_GC.n_entries;i++) {
        if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC resource memory: %s",DAL_GC.allocations[i].comment);
        } else if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            RILlogMessage(NULL,Log_0,"GC resource DAL object: %s",DAL_GC.allocations[i].comment);
        }
    }
}

int DAL_GC_free_all(int chatter, int status) {
    int i;

    for (i=DAL_GC.n_entries-1;i>=0;i--) {
        if (DAL_GC.allocations[i].resource_kind == DAL_GC_MEMORY_RESOURCE) {
            if (chatter>6)
                RILlogMessage(NULL,Log_0,"GC to free resource memory: %s",DAL_GC.allocations[i].comment);
            free(DAL_GC.allocations[i].ptr);
        } else if (DAL_GC.allocations[i].resource_kind == DAL_GC_DAL_OBJECT_RESOURCE) {
            if (chatter>6)
                RILlogMessage(NULL,Log_0,"GC to free resource DAL object: %s",DAL_GC.allocations[i].comment);
            status=DALobjectClose((dal_object)(DAL_GC.allocations[i].ptr), DAL_SAVE, ISDC_OK);
        } else {
        };
    }

    return status;
}

int DAL_GC_allocateDataBuffer(void **buffer, 
                              long buffSize, 
                              int status,
                              char *comment)
{
    *buffer=NULL;
    status=DALallocateDataBuffer(buffer, 
                                 buffSize, 
                                 status);
    DAL_GC_register_allocation(*buffer, DAL_GC_MEMORY_RESOURCE, comment);
    return status;
}

int DAL_GC_freeDataBuffer(void *buffer,
                      int   status)
{
    int i;
    int found=0;

    status=DALfreeDataBuffer(buffer,status);

    for (i=0;i<DAL_GC.n_entries;i++) {
        if (DAL_GC.allocations[i].ptr == buffer) found=1;
        if ( (found==1) && (i<DAL_GC.n_entries-1) )
            DAL_GC.allocations[i]=DAL_GC.allocations[i+1];
    }
    if ( found==1 ) 
        DAL_GC.n_entries--;

    return status;
}


int DAL_GC_objectOpen(const char   *DOL,    /* I DOL of object to open          */
        dal_object   *object, /* O DAL element pointer            */
        int           status) {
    status=DALobjectOpen(DOL,object,status);

    if (status == ISDC_OK)
    DAL_GC_register_allocation((void*)object, DAL_GC_DAL_OBJECT_RESOURCE,(char *)DOL);

    return status;
}

