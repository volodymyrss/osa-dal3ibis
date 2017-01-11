/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                            C SAMPLE PROGRAM                               */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    23 December 2000                                                */
/*  Version: 3.3.0                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  11.05.2000 V 3.0.0                                                       */
/*  ==================                                                       */
/*  6. Removed DAL3 subsets from the sample programs                         */
/*                                                                           */
/*  21.02.2000 V 2.2.5                                                       */
/*  ==================                                                       */
/* 1. The library uses now Makefile-2.0.1                                    */
/* 2. The library requires now DAL3GEN 2.4                                   */
/* 5. Implements the new ISDCLevel concept from TN020                        */
/*                                                                           */
/*  22.08.1999 V 2.1.0                                                       */
/*  ==================                                                       */
/* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
/* 9. Changed "samplec" and "samplef90" so that the user can really "play"   */
/*    with it.                                                               */
/*                                                                           */
/*  18.06.1999 V 2.0.1                                                       */
/*  27.05.1999 V 2.0.0                                                       */
/*                                                                           */
/*  10.03.1999 V 1.0.0                                                       */
/*                                                                           */
/*****************************************************************************/

/* Only the following includes are necessary.                                */

/* ISDC standard libraries                                                   */
#include <isdc.h>

/* <isdc.h> could have been replaced by <dal.h>, as this samplec program     */
/* does not use other ISDC libraries; However a real ISDC software must      */
/* always include <isdc.h>                                                   */

/* DAL3IBIS optional library                                                 */
#include <dal3ibis.h>

int main(int arg, char *argv[]) {

  int status,i,column,OBT_num;
  OBTime OBTstart[10],OBTend[10];
  dal_element *DAL_DS;
  dal_dataType type;
  int sel,evType;
  long num_events,ib_ev[5];
  void *buffer;

  status=ISDC_OK;

  /* The first argument of the program is the DOL of the group; e.g. :       */
  /* og_l2.fits\[GROUPING\] (backslashes are needed to protect from          */
  /* interpretation by the UNIX shell)                                       */

  printf("OBJECT            : %s\n",argv[1]);

  /* The second argument is the type of the events given as an integer;      */
  /* 0 for ISGRI events, 1 for PICsIT single events, and so on               */

  sscanf(argv[2],"%d",&evType);
  printf("TYPE              : %d\n",evType);

  /* The third argument is the event property given as an integer;           */
  /* 0 for DELTA_TIME, 1 for RISE_TIME, and so on                            */

  sscanf(argv[3],"%d",&column);
  printf("COLUMN            : %d\n",column);

  /* The fourth argument defines whether one wants to perform a selection    */
  /* 1  means that the OBT selection is performed                            */
  /* 10 means that the CFITSIO selection is performed                        */

  sscanf(argv[4],"%d",&sel);
  printf("SELECTION         : %d\n\n",sel);

  /* Do we make an OBT selection ?                                           */
  /* If yes, we use a hard-coded selection, made with 2 OBT ranges           */
  /* If OBT_num is 0, DAL3IBISselectEvents will ignore the OBT ranges        */ 
  if (sel==1 || sel==11) {
    OBT_num=2;
  } else {
    OBT_num=0;
  }
  OBTstart[0]=762;
  OBTend[0]=2341;
  OBTstart[1]=25000;
  OBTend[1]=100170;

  /* We shall transfer all types into DAL_LONG, as we do not have memory     */
  /* limitations in this sample program                                      */
  type=DAL_LONG;
  
  /* Opening the Group ...                                                   */
  printf("DALobjectOpen\n");
  status=DALobjectOpen(argv[1],&DAL_DS,status);
  printf("Status: %d\n\n",status);

  /* Getting the total number of events ...                                  */
  /* No selection is made with this function; it really returns every event  */
  /* that i spresent in the group.                                           */
  printf("DAL3IBISshowAllEvents\n");
  status=DAL3IBISshowAllEvents(DAL_DS,ib_ev,status);
  printf("ISGRI Events            : %ld\n",ib_ev[ISGRI_EVTS]);
  printf("PICsIT Single Events    : %ld\n",ib_ev[PICSIT_SGLE]);
  printf("PICsIT Multiple Events  : %ld\n",ib_ev[PICSIT_MULE]);
  printf("COMPTON Single Events   : %ld\n",ib_ev[COMPTON_SGLE]);
  printf("COMPTON Multiple Events : %ld\n",ib_ev[COMPTON_MULE]);
  printf("Status: %d\n\n",status);

  /* Event selection ...                                                     */
  /* Again, as there is no memory issue here, all event parameters (ALL_PAR) */
  /* are copied into the selected events table                               */
  
  printf("DAL3IBISselectEvents\n");
  if (sel>=10) {
    status=DAL3IBISselectEvents(DAL_DS,evType,ALL_PAR,OBT_num,OBTstart,OBTend,"ISGRI_PHA<100",status);
  } else {
    status=DAL3IBISselectEvents(DAL_DS,evType,ALL_PAR,OBT_num,OBTstart,OBTend,NULL,status);
  }
  printf("Status: %d\n\n",status);

  /* Getting the number of selected events ...                               */
  printf("DAL3IBISgetNumEvents\n");
  status=DAL3IBISgetNumEvents(&num_events,status);
  printf("Number of Events: %ld\n",num_events);
  printf("Status: %d\n\n",status);

  printf("Reading all the events at once...\n");

  /* A buffer is allocated for the requested properties. We use the type     */
  /* OBTime, because we are sure it is large enough to contain any parameter */

  buffer=malloc(sizeof(OBTime)*num_events);

  /* Getting the properties of the selected events ...                       */
  /* The returned type (type) is set to DAL_LONG. This paremeter is ignored  */
  /* for the OB_TIME column. Thus the resulting type is translated to        */
  /* DAL_LONG, unless one reads the OB_TIME column, where OBTime is used     */

  printf("DAL3IBISgetEvents\n");
  status=DAL3IBISgetEvents(column,&type,buffer,status);
  printf("Status: %d\n",status);

  /* Printing the result ...                                                 */
  if (status==ISDC_OK) {
    if (column==OB_TIME) {
      for (i=1; i<=num_events; i++) printf("Value %d : %llu\n",i,((OBTime *)buffer)[i-1]);
    } else {
      for (i=1; i<=num_events; i++) printf("Value %d : %ld\n",i,((long *)buffer)[i-1]);
    }
  }
  printf("\n");

  /* The intermediate buffer is not needed anymore.                          */
  free(buffer);

  /* Closing the selected events table ...                                   */
  /* This step is not mandatory. It is useful if the application still       */
  /* important ressources once the events have been processed.               */

  printf("DAL3IBIScloseEvents\n");
  status=DAL3IBIScloseEvents(status);
  printf("Status: %d\n\n",status);
    
  /* Closing the input data structures ...                                   */
  printf("DALobjectClose\n");

  status=DALobjectClose(DAL_DS,DAL_SAVE,status);
  printf("Status: %d\n",status);

  return(ISDC_OK);
}
