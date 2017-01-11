/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                           EVENT MANIPULATIONS                             */
/*                                                                           */
/*                                 C CODE                                    */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse, Nicolas Produit              */
/*  Date:    1 October 2003                                                  */
/*  Version: 4.3.3                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  06.11.2001 V 3.5.6                                                       */
/*  ==================                                                       */
/*  05.11: SPR0725: Fixed a bug making "DAL3IBISfindEventGaps" read bad rows */
/*                                                                           */
/*  21.05.2001 V 3.5.0                                                       */
/*  ==================                                                       */
/*  17.05: SPR0444: Fix access problems to isolated data structures          */
/*                                                                           */
/*  08.04.2001 V 3.4.0                                                       */
/*  ==================                                                       */
/*  20.03: SPR0219: "DAL3IBISfindEventGaps" returns "GAPS_NO_OBT" if no PRP  */
/*                  data can be found.                                       */
/*  28.02: SPR0235: Fixed a bug in the determination of valid levels         */
/*                                                                           */
/*  23.12.2000 V 3.3.0                                                       */
/*  ==================                                                       */
/*  18.12: SPR0136: "DAL3IBISfindEventGaps" now returns "NO_EVENTS" when     */
/*                  called without event data                                */
/*                                                                           */
/*  26.09.2000 V 3.2.0                                                       */
/*  ==================                                                       */
/*  1. Implement the OBT selection optimization from DAL3GEN 3.2             */
/*  2. Fixed a bug in "DAL3IBISfindGapsOBT"                                  */
/*  3. Catches internal DAL3GEN error codes for events                       */
/*                                                                           */
/*  11.05.2000 V 3.0.0                                                       */
/*  ==================                                                       */
/*  1. Removed the event treatment from DAL3IBIS and put it in DAL3GEN       */
/*  3. For consistency, the "IBIS_evpar" enumaration is now called           */
/*     "IBIS_params"                                                         */
/*  4. Uses now "DAL3 Lists" to speed up processing                          */
/*  7. Added a parameter to the C version of "DAL3IBISgetEventPackets"       */
/*                                                                           */
/*  21.02.2000 V 2.2.5                                                       */
/*  ==================                                                       */
/* 1. The library uses now Makefile-2.0.1                                    */
/* 2. The library requires now DAL3GEN 2.4                                   */
/* 5. Implements the new ISDCLevel concept from TN020                        */
/*                                                                           */
/*  29.10.1999 V 2.2.1                                                       */
/*  ==================                                                       */
/* 1. Fixed a bug that prevented the reading of some COMPTON event           */
/*    parameters.                                                            */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  2. Correct a small bug that prevents the correct reading of COMPTON mode */
/*     Delta-Times.                                                          */
/*                                                                           */
/*  22.08.1999 V 2.1.0                                                       */
/*  ==================                                                       */
/* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
/* 2. Changed variable and parameter names in accordance with the CTS.       */
/*    Defined new symbols for parameters.                                    */
/* 3. The APIs do not create automatically an IBIS_SUBSET anymore.           */
/* 4. Event table descriptions are now static global variables, and they     */
/*    are more widely used.                                                  */
/* 5. Better internal error handling. Now intermediate subsets are properly  */
/*    deleted in case of error.                                              */
/* 6. The event access APIs now work with a single data structure.           */
/* 7. Fixed a bug in "DAL3IBIScloseEvents" API, which tried to close non-    */
/*    existing data structures.                                              */
/* 8. Some bug fixes related to allocation of internal buffers.              */
/*                                                                           */
/*  18.06.1999 V 2.0.1                                                       */
/*  27.05.1999 V 2.0.0                                                       */
/*                                                                           */
/*  10.03.1999 V 1.0.0                                                       */
/*                                                                           */
/*****************************************************************************/

#include <dal3ibis.h>

/*****************************************************************************/
/*                                                                           */
/*  Static functions and variables                                           */
/*                                                                           */
/*****************************************************************************/

static int   DAL3IBISplageNum             = 0;
static int  *DAL3IBISplageChFirst         = NULL,    *DAL3IBISplageChLast      = NULL;
static long *DAL3IBISplageEvFirst         = NULL,    *DAL3IBISplageEvLast      = NULL;


int DAL3IBIScalculateAllEventGaps(dal_element *InputDS,
				  IBIS_type    EvType,
				  int         *NumChildren,
				  int         *NumMissPack,
				  int         *NumIntervals,
				  int          current_status)
  /*
    "DAL3IBIScalculateAllEventGaps" finds the possible gaps in the telemetry or in the
    data itself
    and returns the location of the events at the boundaries
  */
{
  int status = ISDC_OK;
  dal_element **listDS=NULL;
  dal_element **listDS1=NULL;
  char name[DAL_MED_STRING],tmpErt[DAL_MED_STRING];
  double ijdErt,oldErt;
  int firstPacket,lastPacket,oldPacket;
  int numAlloc,i;
  /* SPR 2676, duh, numval is used for two disjoint things */
  long j,numVal, numVal1 ,numRows,firstEvt,totEvt,nEvt;
  dal_dataType dtype;
  int *ssc=NULL;
  int *firstdn=NULL;
  int *secdn=NULL;
  int *part_lobt=NULL;
  int searchdummy=0;
  int nsrw;
  int fd;
  long idum;
  long offset,offset1;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Select the type of events                                             */
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-PRW");searchdummy=1;     break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-PRW");searchdummy=2;     break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-PRW");searchdummy=2;     break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-PRW");     break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-PRW");     break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if ( status != ISDC_OK ) continue;
    status=DAL3GENbuildList(InputDS,name,&listDS,NumChildren,status);
    if ( status != ISDC_OK ) continue;
    if (*NumChildren==0){
      status=DAL3IBIS_NO_IBIS_EVENTS;
      continue;
    }
    if (*NumChildren>1){
      status=DAL3IBIS_TOO_MANY_CHILDREN;
      continue;
    }
    if (searchdummy>0){
      name[10]='S';
      status=DAL3GENbuildList(InputDS,name,&listDS1,&nsrw,status);
      if (nsrw==0){/* there can be no SRW structures if absolutely no dummy*/
	searchdummy=0;
      }
      else{
	if (*NumChildren!=nsrw) status=DAL3IBIS_MISMATCH_SRW;
	if ( status != ISDC_OK ) continue;
      }
    }
    if ( status != ISDC_OK ) continue;
    /* Initialize the "plage" list                                           */
    DAL3IBISplageNum=0;
    *NumMissPack=0;
    oldPacket=-2;
    oldErt=-1.0;
    numAlloc=1;
    DAL3IBISplageChFirst=malloc(100*numAlloc*sizeof(int));
    DAL3IBISplageChLast =malloc(100*numAlloc*sizeof(int));
    DAL3IBISplageEvFirst=malloc(100*numAlloc*sizeof(long));
    DAL3IBISplageEvLast =malloc(100*numAlloc*sizeof(long));
    /* Process all the children                                              */
    for (i=1; i<=*NumChildren; i++) {
      /* Read the first and last SSC, and the ERT of the first packet        */
      status=DALattributeGet(listDS[i-1],"SSC_BEG",DAL_INT,&firstPacket,NULL,NULL,status);
      status=DALattributeGet(listDS[i-1],"SSC_END",DAL_INT,&lastPacket,NULL,NULL,status);
      status=DALattributeGet(listDS[i-1],"ERTFIRST",DAL_CHAR,tmpErt,NULL,NULL,status);
      status=DAL3GENconvertUTC2IJD(tmpErt,&ijdErt,status);
      if ( status != ISDC_OK ) break;
      /* Get the number of events in the Science Window                      */
      status=DALtableGetNumRows(listDS[i-1],&numRows,status);
      dtype=DAL_LONG;
      status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,numRows,numRows,&numVal,&firstEvt,status);
      status=DALtableGetColBins(listDS[i-1],"ROW_NUM",0,&dtype,numRows,numRows,&numVal,&nEvt,status);
      totEvt=firstEvt+nEvt-1;
      if ( status != ISDC_OK ) break;
      /* Checks whether there is any discontinuity between Science Windows   */
      /* There is discontinuity either if SSC is not incremented by 1        */
      /* Or if the time gaps between the ERT is larger than ~500s~=5e-3 days */
      /* This figure is the time needed for the SSC to cycle if all the      */
      /* telemetry is sent through one APID                                  */
      if ( firstPacket!=oldPacket+1 || 
	  (firstPacket==oldPacket+1 && ijdErt-oldErt> 5e-3) ) {
	/* There is a discontinuity: the number of gaps is incremented       */
	DAL3IBISplageNum++;
	if (DAL3IBISplageNum/100 == numAlloc ) {
	  /* Pointers are reallocated                                        */
	  numAlloc++;
	  DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
	  DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
	  DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
	  DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
	}
	/* The "plage" is identified                                         */
	DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
	DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=1;
	DAL3IBISplageEvLast [DAL3IBISplageNum-1]=firstEvt+nEvt-1;
	
      } else {
	/* There is no discontinuity: the last "plage" is extended           */
	DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	DAL3IBISplageEvLast [DAL3IBISplageNum-1]=firstEvt+nEvt-1;
      }
      oldPacket=lastPacket;
      oldErt=ijdErt;
      /* Check whether there are discontinuities in the Science Window       */
      ssc=malloc(numRows*sizeof(int));
      dtype=DAL_INT;
      status=DALtableGetCol(listDS[i-1],"SSC",0,&dtype,&numVal,ssc,status);
      if ( status != ISDC_OK ) break;
      if (searchdummy>0){
	firstdn=malloc(numRows*sizeof(int));
	secdn=malloc(numRows*sizeof(int));
	part_lobt=malloc(numRows*sizeof(int));
	status=DALtableGetCol(listDS[i-1],"FIRST_DUMMY_NUM",0,&dtype,&numVal,firstdn,status);
	status=DALtableGetCol(listDS[i-1],"SECOND_DUMMY_NUM",0,&dtype,&numVal,secdn,status);
	firstdn[0]+=secdn[0];
	status=DALtableGetCol(listDS[i-1],"PART_LOBT",0,&dtype,&numVal,part_lobt,status);
	if ( status != ISDC_OK ) break;
      }
      if ( status != ISDC_OK ) break;
      for (j=1;j<numVal;j++) {/* cannot do anything about first event */
	if (searchdummy>0){
	  firstdn[j]+=(firstdn[j-1]+secdn[j]);/* to find diretely the good one in SRW */
	}
	/* There is discontinuity if SSC is not incremented by 1             */
	/* SPR 2174 */
	if (! ((ssc[j]==ssc[j-1]+1)  ||
	       (DAL3IBIS_MAX_SSC==ssc[j-1] && 0==ssc[j]))) {
	  
	  /* There is a discontinuity: the number of gaps is incremented     */
	  /* SPR 4531, Don't get negative NumMissPack when the ssc wraps */
	  if ( (ssc[j]-(ssc[j-1]+1)) < 0) {
	    *NumMissPack += (DAL3IBIS_MAX_SSC - (ssc[j-1]+1)) + ssc[j];
	  } else {
	    *NumMissPack+=ssc[j]-(ssc[j-1]+1);
	  }
	  DAL3IBISplageNum+=1;
	  if (DAL3IBISplageNum/100 == numAlloc ) {
	    /* Pointers are reallocated                                      */
	    numAlloc+=1;
	    DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
	    DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
	    DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
	    DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
	  }

	  /* SPR 3822, reorder this */
	  /* Get the location of the last event                              */
	  dtype=DAL_LONG;
	  /* spr 2676, thanks to RR for finding this silly error, numVal is 
	     used twice */
	  status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,j+1,j+1,
				    &numVal1,&firstEvt,status);
	  if ( status != ISDC_OK ) break;
	  /* New plage is inserted at the before-last position               */
	  /* SPR 3822 test was wrong before */
	  
	  if ((DAL3IBISplageNum > 1) &&
	      DAL3IBISplageEvFirst[DAL3IBISplageNum-2] >= firstEvt) {
	    /* SPR 3503, in a few cases we have an end before a begin, 
	       don't add. This happens because we found the end before */
	    DAL3IBISplageNum--;
	    
	  }
	  DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
	  DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	  /* was firstEvt+nEvt-1, sadly firstEvt is updated a few lines
	     further down - SPR 2684; */
	  DAL3IBISplageEvLast[DAL3IBISplageNum-1]=
	    DAL3IBISplageEvLast[DAL3IBISplageNum-2];
	  
	  DAL3IBISplageEvLast [DAL3IBISplageNum-2]=firstEvt-1;
	  DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=firstEvt;
	}
	if (searchdummy>0){
	  if (part_lobt[j]==0){ /*SPR 3223 */
	    dtype=DAL_LONG;
	    status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,j+1,j+1,&numVal1,&firstEvt,status);
	    if ( status != ISDC_OK ) break;
	    if (firstEvt<=totEvt){
	      DAL3IBISplageNum++;
	      if (DAL3IBISplageNum/100 == numAlloc ) {
		/* Pointers are reallocated                                      */
		numAlloc++;
		DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
		DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
		DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
		DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
	      }
	      /* SPR 4440, this was probably never right before so let's do a good comparision */
	      /* Basically the same fix as SPR 3822 above */
	      if ((DAL3IBISplageNum > 1) &&
                  (DAL3IBISplageEvFirst[DAL3IBISplageNum-2] >= firstEvt)) {
		/* SPR 3503, in a few cases we have an end before a begin, don't add.
		   This happens because we found the end before */
		DAL3IBISplageNum--;
	      } else {
		/* New plage is inserted at the before-last position               */ 
		DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
		DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
		DAL3IBISplageEvLast[DAL3IBISplageNum-1]=
		  DAL3IBISplageEvLast[DAL3IBISplageNum-2];
		DAL3IBISplageEvLast [DAL3IBISplageNum-2]=firstEvt-1;
		DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=firstEvt;
	      }
	      
	    }
	    else{
	      if (i!=*NumChildren){
		status=DAL3IBIS_MISMATCH_CHILDREN;
	      }
	    }
	  }
	  for (idum=firstdn[j-1];idum<firstdn[j];idum++){
	    dtype=DAL_INT;
	    status=DALtableGetColBins(listDS1[i-1],"FIRST_DUMMY",0,&dtype,idum+1,idum+1,&numVal1,&fd,status);	      
	    if (fd==254){
	      dtype=DAL_LONG;
	      status=DALtableGetColBins(listDS1[i-1],"POSITION",0,&dtype,idum+1,idum+1,&numVal1,&offset,status);
	      if ( status != ISDC_OK ) break;
	      if (idum+1<firstdn[j]){
		dtype=DAL_LONG;
		status=DALtableGetColBins(listDS1[i-1],"POSITION",0,&dtype,idum+2,idum+2,&numVal1,&offset1,status);	      
		if ( status != ISDC_OK ) break;
		if (offset1==offset){
		  offset--;
		}
	      }
	      /* Get the location of the last event                              */
	      dtype=DAL_LONG;
	      status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,j+1,j+1,&numVal1,&firstEvt,status);
	      if ( status != ISDC_OK ) break;
	      if (!(offset==0&&part_lobt[j]==0)){
		/* There is the case where the last even start the gap ! scw 004000030010 */
		if (firstEvt+offset<=totEvt){
		  /* There is a discontinuity: the number of gaps is incremented     */
		  DAL3IBISplageNum++;
		  if (DAL3IBISplageNum/100 == numAlloc ) {
		    /* Pointers are reallocated                                      */
		    numAlloc++;
		    DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
		    DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
		    DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
		    DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
		  }
		  if (DAL3IBISplageEvFirst[DAL3IBISplageNum-2] > firstEvt+offset-1) {
		    /* SPR 3160, in a few cases we have an end before a begin, don't add.
		       This happens because we found the end above */
		    DAL3IBISplageNum--;
		    
		  } else {
		    
		      
		  
		    /* New plage is inserted at the before-last position               */ 
		    DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
		    DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
		    DAL3IBISplageEvLast[DAL3IBISplageNum-1]=
		      DAL3IBISplageEvLast[DAL3IBISplageNum-2];
		    DAL3IBISplageEvLast [DAL3IBISplageNum-2]=firstEvt+offset-1;
		    DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=firstEvt+offset;
		  }
		  
		}
		  
		else{
		  if (i!=*NumChildren){
		    status=DAL3IBIS_MISMATCH_CHILDREN;
		  }
		}
	      }
	      /*
		else{
		this gap is already treated by resync check
		}
	      */
	    }
	    if (status!=ISDC_OK) break;
	  }
	  if (status!=ISDC_OK) break;
	}
      }
      if (status!=ISDC_OK) break;
      if (ssc!=NULL){
	free(ssc);
	ssc=NULL;
      }
      if (firstdn!=NULL){
	free(firstdn);
	firstdn=NULL;
      }
      if(secdn!=NULL){
	free(secdn);
	secdn=NULL;
      }
      if (part_lobt!=NULL){
	free(part_lobt);
	part_lobt=NULL;
      }
    }
    
    if ( status != ISDC_OK) continue;
    
    /* SPR 3822 */
    if (DAL3IBISplageEvFirst[DAL3IBISplageNum-1] > 
	DAL3IBISplageEvLast[DAL3IBISplageNum-1]) {
      DAL3IBISplageNum--;
    }
    *NumIntervals=DAL3IBISplageNum;
    
  } while (0);
  
  free(listDS);

  return(status);
}

int DAL3IBIScalculateEventGaps(dal_element *InputDS,
			       IBIS_type    EvType,
			       int         *NumChildren,
			       int         *NumMissPack,
			       int         *NumIntervals,
			       int          current_status)
  /*
    "DAL3IBIScalculateEventGaps" finds the possible gaps in the telemetry 
    and returns the location of the events at the boundaries
  */
{
  int status = ISDC_OK;
  dal_element **listDS=NULL;
  char name[DAL_MED_STRING],tmpErt[DAL_MED_STRING];
  double ijdErt,oldErt;
  int firstPacket,lastPacket,oldPacket;
  int numAlloc,i;
  /* SPR 2676, duh, numval is used for two disjoint things */
  long j,numVal, numVal1 ,numRows,firstEvt,nEvt;
  dal_dataType dtype;
  int *ssc=NULL;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Select the type of events                                             */
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-PRW");     break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-PRW");     break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-PRW");     break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-PRW");     break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-PRW");     break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if ( status != ISDC_OK ) continue;
    status=DAL3GENbuildList(InputDS,name,&listDS,NumChildren,status);
    if (*NumChildren==0) status=DAL3IBIS_NO_IBIS_EVENTS;
    if ( status != ISDC_OK ) continue;
    /* Initialize the "plage" list                                           */
    DAL3IBISplageNum=0;
    *NumMissPack=0;
    oldPacket=-2;
    oldErt=-1.0;
    numAlloc=1;
    DAL3IBISplageChFirst=malloc(100*numAlloc*sizeof(int));
    DAL3IBISplageChLast =malloc(100*numAlloc*sizeof(int));
    DAL3IBISplageEvFirst=malloc(100*numAlloc*sizeof(long));
    DAL3IBISplageEvLast =malloc(100*numAlloc*sizeof(long));
    /* Process all the children                                              */
    for (i=1; i<=*NumChildren; i++) {
      /* Read the first and last SSC, and the ERT of the first packet        */
      status=DALattributeGet(listDS[i-1],"SSC_BEG",DAL_INT,&firstPacket,NULL,NULL,status);
      status=DALattributeGet(listDS[i-1],"SSC_END",DAL_INT,&lastPacket,NULL,NULL,status);
      status=DALattributeGet(listDS[i-1],"ERTFIRST",DAL_CHAR,tmpErt,NULL,NULL,status);
      status=DAL3GENconvertUTC2IJD(tmpErt,&ijdErt,status);
      if ( status != ISDC_OK ) break;
      /* Get the number of events in the Science Window                      */
      status=DALtableGetNumRows(listDS[i-1],&numRows,status);
      dtype=DAL_LONG;
      status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,numRows,numRows,&numVal,&firstEvt,status);
      status=DALtableGetColBins(listDS[i-1],"ROW_NUM",0,&dtype,numRows,numRows,&numVal,&nEvt,status);
      if ( status != ISDC_OK ) break;
      /* Checks whether there is any discontinuity between Science Windows   */
      /* There is discontinuity either if SSC is not incremented by 1        */
      /* Or if the time gaps between the ERT is larger than ~500s~=5e-3 days */
      /* This figure is the time needed for the SSC to cycle if all the      */
      /* telemetry is sent through one APID                                  */
      if ( firstPacket!=oldPacket+1 || 
	  (firstPacket==oldPacket+1 && ijdErt-oldErt> 5e-3) ) {
	/* There is a discontinuity: the number of gaps is incremented       */
	DAL3IBISplageNum+=1;
	if (DAL3IBISplageNum/100 == numAlloc ) {
	  /* Pointers are reallocated                                        */
	  numAlloc+=1;
	  DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
	  DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
	  DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
	  DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
	}
	/* The "plage" is identified                                         */
	DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
	DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=1;
	DAL3IBISplageEvLast [DAL3IBISplageNum-1]=firstEvt+nEvt-1;
      } else {
	/* There is no discontinuity: the last "plage" is extended           */
	DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	DAL3IBISplageEvLast [DAL3IBISplageNum-1]=firstEvt+nEvt-1;
      }
      oldPacket=lastPacket;
      oldErt=ijdErt;
      /* Check whether there are discontinuities in the Science Window       */
      ssc=malloc(numRows*sizeof(int));
      dtype=DAL_INT;
      status=DALtableGetCol(listDS[i-1],"SSC",0,&dtype,&numVal,ssc,status);
      if ( status != ISDC_OK ) break;
      for (j=1;j<numVal;j++) {
	/* There is discontinuity if SSC is not incremented by 1             */
	/* SPR 2174 */
	if (! ((ssc[j]==ssc[j-1]+1)  ||
	       (DAL3IBIS_MAX_SSC==ssc[j-1] && 0==ssc[j]))) {
	  
	  /* There is a discontinuity: the number of gaps is incremented     */
	  *NumMissPack+=ssc[j]-(ssc[j-1]+1);
	  DAL3IBISplageNum+=1;
	  if (DAL3IBISplageNum/100 == numAlloc ) {
	    /* Pointers are reallocated                                      */
	    numAlloc+=1;
	    DAL3IBISplageChFirst=realloc(DAL3IBISplageChFirst,100*numAlloc*sizeof(int));
	    DAL3IBISplageChLast =realloc(DAL3IBISplageChLast ,100*numAlloc*sizeof(int));
	    DAL3IBISplageEvFirst=realloc(DAL3IBISplageEvFirst,100*numAlloc*sizeof(long));
	    DAL3IBISplageEvLast =realloc(DAL3IBISplageEvLast ,100*numAlloc*sizeof(long));
	  }
	  /* New plage is inserted at the before-last position               */ 
	  DAL3IBISplageChFirst[DAL3IBISplageNum-1]=i;
	  DAL3IBISplageChLast [DAL3IBISplageNum-1]=i;
	  /* was firstEvt+nEvt-1, sadly firstEvt is updated a few lines
	     further down - SPR 2684; */
	  DAL3IBISplageEvLast[DAL3IBISplageNum-1]=
	    DAL3IBISplageEvLast[DAL3IBISplageNum-2];
	  /* Get the location of the last event                              */
	  dtype=DAL_LONG;
	  /* spr 2676, thanks to RR for finding this silly error, numVal is 
	     used twice */
	  status=DALtableGetColBins(listDS[i-1],"ROW_BEGIN",0,&dtype,j+1,j+1,&numVal1,&firstEvt,status);
	  if ( status != ISDC_OK ) break;
	  DAL3IBISplageEvLast [DAL3IBISplageNum-2]=firstEvt-1;
	  DAL3IBISplageEvFirst[DAL3IBISplageNum-1]=firstEvt;
	}
      }
      if (status!=ISDC_OK) break;
    }
    free(ssc);

    if ( status != ISDC_OK) continue;

    *NumIntervals=DAL3IBISplageNum;
  
  } while (0);

  free(listDS);

  return(status);
}

int DAL3IBISfindGapsOBT(dal_element *InputDS,
			IBIS_type    EvType,
			int          NumChildren,
			OBTime      *OBTstart,
			OBTime      *OBTstop,
			int          current_status)
  /*
    "DAL3IBISfindGapsOBT" calculates the OBTs corresponding to the beginnings
    and ends of the continuous "plages"
  */
{
  int status=ISDC_OK;
  char name[DAL_MED_STRING];
  dal_element **listDS=NULL;
  int i,numChildrenPrp;
  long numVal;

  if ( current_status!=ISDC_OK ) return(current_status);
  
  do {
    /* Select the type of events                                             */
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-PRP");     break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-PRP");     break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-PRP");     break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-PRP");     break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-PRP");     break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if (status!=ISDC_OK) continue;
    /* Create a list the Packet Raw Data                                     */
    status=DAL3GENbuildList(InputDS,name,&listDS,&numChildrenPrp,status);
    if (status!=ISDC_OK) continue;
    /* SCREW 1388 */
    if ( 0 == numChildrenPrp ) {
      status = ISDC_OK;
      /* Select the type of events                                          */
      switch (EvType) {
      case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-ALL");     break;
      case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-ALL");     break;
      case PICSIT_MULE  : strcpy(name,"PICS-MULE-ALL");     break;
      case COMPTON_SGLE : strcpy(name,"COMP-SGLE-ALL");     break;
      case COMPTON_MULE : strcpy(name,"COMP-MULE-ALL");     break;
      default           : status=DAL3GEN_INVALID_PARAMETER; break;
      }
      /* Create a list the Packet Raw Data                                   */
      status=DAL3GENbuildList(InputDS,name,&listDS,&numChildrenPrp,status);
      if ( status != ISDC_OK ) continue;
    }
    
    if        (numChildrenPrp==0) {
      status=DAL3IBIS_GAPS_NO_OBT;
    } else if (NumChildren==numChildrenPrp) {
      for (i=0;i<DAL3IBISplageNum;i++) {
	
	status=DAL3GENtableGetOBTBins(listDS[DAL3IBISplageChFirst[i]-1],"OB_TIME",0,DAL3IBISplageEvFirst[i],
				      DAL3IBISplageEvFirst[i],&numVal,OBTstart+i,status);
	status=DAL3GENtableGetOBTBins(listDS[DAL3IBISplageChLast[i]-1] ,"OB_TIME",0,DAL3IBISplageEvLast[i],
				      DAL3IBISplageEvLast[i],&numVal,OBTstop+i,status);
	if ( status != ISDC_OK ) break;
      }
    } else
      status=DAL3IBIS_INVALID_GROUP;

  } while (0);

  /* Delete the PRP list                                                     */
  free(listDS);

  return(status);
}



/*****************************************************************************/
/*                                                                           */
/*  External functions                                                       */
/*                                                                           */
/*****************************************************************************/

int DAL3IBISshowAllEvents(dal_element *InputDS,
			  long         IBISnum[5],
			  int          current_status)
  /*
    Given a DAL data structure, "DAL3IBISshowAllEvents" finds the number of all
    events of all types
  */

{
  int status=ISDC_OK;
  int i, j, found, numDS,numSec,numAllSec;
  long numRows;
  dal_element **dsList=NULL,**secList=NULL, **secAllList=NULL;
  char dsName[DAL_BIG_STRING];
  char *ibisDSnameList[]={"ISGR-EVTS-","PICS-\?\?LE-","COMP-\?\?LE-"};
  /* SCREW 1388 */
  char *ibisTypeList[]={"RAW","PRP","COR"};
 
  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Set the counters to 0                                                 */
    memset(IBISnum,0,5*sizeof(long));
    found=0;
    /* Loop on RAW, then PRP, then COR                                       */
    /* SCREW 1388.  */
    /* SPR 3565, only loop up to 3 */
    for (i=0; i<3 && found==0; i++) {
      /* Loop on the 5 types of IBIS events                                  */
      numDS=0;
      dsList=NULL;
      for (j=0;j<3;j++) {
	strcpy(dsName,ibisDSnameList[j]);
	strcat(dsName,ibisTypeList[i]);
	status=DAL3GENbuildList(InputDS,dsName,&secList,&numSec,status);
	strcpy(dsName,ibisDSnameList[j]);
	strcat(dsName,"ALL");
	status=DAL3GENbuildList(InputDS,dsName,&secAllList,&numAllSec,status);
	secList=realloc(secList,(numSec + numAllSec)*sizeof(dal_element *));
	status=DAL3GENmergeTwoLists(secList,&numSec,secAllList,&numAllSec,status);
	
	status=DAL3GENaddToList(&dsList,&numDS,secList,numSec,status);
      }
      secList=NULL;
      if (status!=ISDC_OK) break;
      for (j=0; j<numDS && status==ISDC_OK; j++) {
	status=DALelementGetName(dsList[j],dsName,status);
	status=DALtableGetNumRows(dsList[j],&numRows,status);
	if      (strncmp(dsName,"ISGR-EVTS-",10)==0) {
	  IBISnum[ISGRI_EVTS]+=numRows;   found=1;
	} else if (strncmp(dsName,"PICS-SGLE-",10)==0) {
	  IBISnum[PICSIT_SGLE]+=numRows;   found=1;
	} else if (strncmp(dsName,"PICS-MULE-",10)==0) {
	  IBISnum[PICSIT_MULE]+=numRows;   found=1;
	} else if (strncmp(dsName,"COMP-SGLE-",10)==0) {
	  IBISnum[COMPTON_SGLE]+=numRows;   found=1;
	} else if (strncmp(dsName,"COMP-MULE-",10)==0) {
	  IBISnum[COMPTON_MULE]+=numRows;   found=1;
	}
      }
    }
  } while (0);

  free(dsList);
  free(secList);

  return(status);
}


int DAL3IBISselectEvents(dal_element *InputDS,
			 IBIS_type    EvType,
			 ISDCLevel    ColSel,
			 int          NumIntervals,
			 OBTime      *gtiStart,
			 OBTime      *gtiStop,
			 const char  *RowFilter,
			 int          current_status)
  /*
    Given a DAL data structure, "DAL3IBISselectEvents" extracts
    selected events
  */

{
  int status=ISDC_OK;
  int numChild;
  char nameExt[DAL_MED_STRING];
  dal_element **listDS=NULL;
  Event_type locType = IBIS_ISGRI_EVTS;
  int i,numSWG;
  OBTime *obtStart=NULL,*obtEnd=NULL;
  
  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    if (NumIntervals<0) NumIntervals=0;    
    /* Check the level                                                       */
    switch (ColSel) {
    case ALL_PAR:
    case RAW:
    case PRP:
    case COR:
    case PRP_NO_OBT:
    case COR_NO_OBT:
      break;
    default: 
      status=DAL3GEN_INVALID_PARAMETER;
    }
    /* Select the type of events                                             */    
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(nameExt,"ISGR-EVTS-"); locType=IBIS_ISGRI_EVTS;    break;
    case PICSIT_SGLE  : strcpy(nameExt,"PICS-SGLE-"); locType=IBIS_PICSIT_SGLE;   break;
    case PICSIT_MULE  : strcpy(nameExt,"PICS-MULE-"); locType=IBIS_PICSIT_MULE;   break;
    case COMPTON_SGLE : strcpy(nameExt,"COMP-SGLE-"); locType=IBIS_COMPTON_SGLE;  break;
    case COMPTON_MULE : strcpy(nameExt,"COMP-MULE-"); locType=IBIS_COMPTON_MULE;  break;
    default           : status=DAL3GEN_INVALID_PARAMETER;     break;
    }
    if (status!=ISDC_OK) continue;
    strcat(nameExt,"*");
    status=DAL3GENbuildList(InputDS,nameExt,&listDS,&numChild,status);
    if (status!=ISDC_OK) continue;
    if (numChild==0) {
      status=DAL3IBIS_NO_IBIS_EVENTS;
      continue;
    }
    /* Find the OBT boundaries of the Science Windows                        */
    numSWG=0;
    if (NumIntervals>0) {
      status=DAL3GENgetNumSWG(InputDS,&numSWG,status);
      if (status!=ISDC_OK && status!=DAL3GEN_NOT_AN_ISDC_GROUP) continue;
      if (status==DAL3GEN_NOT_AN_ISDC_GROUP) {
	status=ISDC_OK;
      } else {
	obtStart=malloc(numSWG*sizeof(OBTime));
	obtEnd=malloc(numSWG*sizeof(OBTime));
	status=DAL3GENgetSWGbounds(InputDS,1,0,&numSWG,obtStart,obtEnd,status);
	for (i=0;i<numSWG;i++) {
	  obtStart[i]-=DAL3GEN_SWG_BOUNDARY;
	  obtEnd  [i]+=DAL3GEN_SWG_BOUNDARY;
	}
      }
    }

    if (RowFilter!=NULL && RowFilter[0]=='&' && RowFilter[1]==0)
      RowFilter=NULL;
    /* SPR 1942, we can no longer assume that if we have 10 science window
       groups we also have 10 elements in our listDS since now we run science 
       window clean */

    status=DAL3GENselectEvents(listDS,numChild,obtStart,obtEnd,numSWG,locType,
			       ColSel,NumIntervals,gtiStart,gtiStop,RowFilter,
			       InputDS,status);

    if (status==DAL3GEN_INEXISTENT_PARAMETER) status=DAL3IBIS_INEXISTENT_PARAMETER;
    if (status==DAL3GEN_INVALID_GROUP       ) status=DAL3IBIS_INVALID_GROUP       ;
    if (status==DAL3GEN_NO_EVENTS           ) status=DAL3IBIS_NO_IBIS_EVENTS      ;
  } while (0);

  /* Delete the list                                                         */ 
  free(listDS);
  /* Delete the arrays                                                       */ 
  free(obtStart);
  free(obtEnd);

  return(status);
}


int DAL3IBISgetNumEvents(long *NumEvents,
			 int   current_status)
  /*
    Given a DAL data structure, "DAL3IBISgetNumEvents" retrieves the
    number of selected events
  */

{
  return(DAL3GENgetNumEvents(NumEvents,current_status));
}


int DAL3IBISgetEventsBins(IBIS_evpar    ColID,
			  dal_dataType *ColType,
			  long          StartBin,
			  long          EndBin,
			  void         *ColBuff,
			  int           current_status)
  /*
    Given a DAL data structure, "DAL3IBISgetEvents" retrieves the
    content of the column "colID" for the selected events from rows
    "startBin" to "endBin"
  */

{
  int status=ISDC_OK;

  if ( current_status!=ISDC_OK ) return(current_status);

  status=DAL3GENgetEventsBins(ColID,ColType,StartBin,EndBin,ColBuff,status);

  if (status==DAL3GEN_INEXISTENT_PARAMETER)   status=DAL3IBIS_INEXISTENT_PARAMETER;
  if (status==DAL3GEN_INCOMPATIBLE_PARAMETER) status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
  return(status);
}


int DAL3IBISgetEvents(IBIS_evpar    ColID,
		      dal_dataType *ColType,
		      void         *ColBuff,
		      int           current_status)
  /*
    Given a DAL data structure, "DAL3IBISgetEvents" retrieves the
    content of the column "colID" for the selected events
  */

{
  return(DAL3GENgetEventsBins(ColID,ColType,1,0,ColBuff,current_status));
}


int DAL3IBIScloseEvents(int current_status)

  /*
    "DAL3IBIScloseEvents" closes the IBIS related data structures
    and frees the associated memory
  */

{
  return(DAL3GENcloseEvents(current_status));
}

int DAL3IBISgetNumEventPackets(dal_element *InputDS,
			       IBIS_type    EvType,
			       int         *NumPackets,
			       int          current_status)
  /*
    "DAL3IBISgetNumEventPackets" returns the number of packets containing
    events found in the given Science Window.
  */
{
  int status=ISDC_OK;
  ds_type type;
  dal_element *curDS;
  char name[DAL_MED_STRING];
  long rows;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Determine the type of the data structure                              */
    status=DAL3GENobjectType(InputDS,&type,NULL,NULL,NULL,status);
    if ( status!=ISDC_OK ) continue;
    if (type!=SWG) {
      status=DAL3IBIS_NOT_A_SWG;
      continue;
    }
    /* Select the type of events                                             */    
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-");        break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-");        break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-");        break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-");        break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-");        break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if (status!=ISDC_OK) continue;

    strcat(name,"PRW");
    status=DALobjectFindElement(InputDS,name,&curDS,status);
    if (status==DAL_ELEMENT_NOT_FOUND) {
      *NumPackets=0;
    } else {
      status=DALtableGetNumRows(curDS,&rows,status);
      *NumPackets=rows;
    }

  } while (0);

  return(status);
}

int DAL3IBISgetEventPackets(dal_element    *InputDS,
			    IBIS_type       EvType,
			    int            *SSC,
			    int            *PartLOBT,
			    OBTime         *LOCAL_OBT,
			    int            *RowBegin,
			    int            *RowNum,
			    int            *FirstNum,
			    int            *SecondNum,
			    int            *CovflwNum,
			    int             current_status)
  /*
    "DAL3IBISgetEventPackets" returns the properties of the packets found
    in the given Science Window.
  */
{
  int status=ISDC_OK;
  ds_type type;
  dal_dataType ds;
  long numRows;
  dal_element *curDS;
  char name[DAL_MED_STRING];

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Determine the type of the data structure                              */
    status=DAL3GENobjectType(InputDS,&type,NULL,NULL,NULL,status);
    if ( status!=ISDC_OK ) continue;
    if (type!=SWG) {
      status=DAL3IBIS_NOT_A_SWG;
      continue;
    }
    /* Select the type of events                                             */    
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-");        break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-");        break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-");        break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-");        break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-");        break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if (status!=ISDC_OK) continue;

    strcat(name,"PRW");
    status=DALobjectFindElement(InputDS,name,&curDS,status);
    if (status!=ISDC_OK) continue;
    /* To make stupid F90 language happy, everything is patched into INTEGER */
    ds=DAL_INT;
    /* Read the individual columns                                           */
    /* Source Sequence Count                                                 */
    if (SSC!=NULL) {
      status=DALtableGetCol(curDS,"SSC",0,&ds,&numRows,SSC,status);
      if (status!=ISDC_OK) continue;
    }
    /* Partial Local-On Board time in the data field header                  */
    if (PartLOBT!=NULL) {
      status=DALtableGetCol(curDS,"PART_LOBT",0,&ds,&numRows,PartLOBT,status);
      if (status!=ISDC_OK) continue;
    }
    /* (Full!) Local-On Board time in the data field header                  */
    if (PartLOBT!=NULL) {
      status=DAL3GENtableGetOBT(curDS,"LOCAL_OBT",0,&numRows,LOCAL_OBT,status);
      if (status!=ISDC_OK) continue;
    }
    /* Events from this packet are beginning at this row                     */
    if (RowBegin!=NULL) {
      status=DALtableGetCol(curDS,"ROW_BEGIN",0,&ds,&numRows,RowBegin,status);
      if (status!=ISDC_OK) continue;
    }
    /* Number of true events contained in this packet                        */
    if (RowNum!=NULL) {
      status=DALtableGetCol(curDS,"ROW_NUM",0,&ds,&numRows,RowNum,status);
      if (status!=ISDC_OK) continue;
    }
    /* Number of first dummy events (not COMPTON!)                           */
    if (FirstNum!=NULL) {
      if (EvType==COMPTON_SGLE || EvType==COMPTON_MULE) {
	status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      } else {
	status=DALtableGetCol(curDS,"FIRST_DUMMY_NUM",0,&ds,&numRows,FirstNum,status);
      }
      if (status!=ISDC_OK) continue;
    }
    /* Number of second dummy events (not COMPTON!)                          */
    if (SecondNum!=NULL) {
      if (EvType==COMPTON_SGLE || EvType==COMPTON_MULE) {
	status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      } else {
	status=DALtableGetCol(curDS,"SECOND_DUMMY_NUM",0,&ds,&numRows,SecondNum,status);
      }
      if (status!=ISDC_OK) continue;
    }
    /* Number of Counter Overflow events (COMPTON only!)                     */
    if (CovflwNum!=NULL) {
      if (EvType==COMPTON_SGLE || EvType==COMPTON_MULE) {
	status=DALtableGetCol(curDS,"COUNTER_OVFLW_NUM",0,&ds,&numRows,CovflwNum,status);
      } else {
	status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      }
    }

  } while (0);

  return(status);
}

int DAL3IBISgetEventPacketsF90(dal_element *InputDS,
			       IBIS_type    EvType,
			       int         *SSC,
			       int         *PartLOBT,
			       int         *RowBegin,
			       int         *RowNum,
			       int         *FirstNum,
			       int         *SecondNum,
			       int         *CovflwNum,
			       void        *DAL3_NULL,
			       int          current_status)
  /*
    "DAL3IBISgetEventPacketsF90" is an intermediate functions that is not
    supposed to be used directly, but from the DAL3IBIS_GET_EVENT_PACKETS
    F90 function
  */
{
  int status;
  int *tempSSC;
  int *tempPLOBT;
  int *tempBeg;
  int *tempNum;
  int *tempFirst;
  int *tempSecond;
  int *tempCount;

  if (SSC==DAL3_NULL) {
    tempSSC=NULL;
  } else {
    tempSSC=SSC;
  }
  if (PartLOBT==DAL3_NULL) {
    tempPLOBT=NULL;
  } else {
    tempPLOBT=PartLOBT;
  }
  if (RowBegin==DAL3_NULL) {
    tempBeg=NULL;
  } else {
    tempBeg=RowBegin;
  }
  if (RowNum==DAL3_NULL) {
    tempNum=NULL;
  } else {
    tempNum=RowNum;
  }
  if (FirstNum==DAL3_NULL) {
    tempFirst=NULL;
  } else {
    tempFirst=FirstNum;
  }
  if (SecondNum==DAL3_NULL) {
    tempSecond=NULL;
  } else {
    tempSecond=SecondNum;
  }
  if (CovflwNum==DAL3_NULL) {
    tempCount=NULL;
  } else {
    tempCount=CovflwNum;
  }

  status=DAL3IBISgetEventPackets(InputDS,EvType,tempSSC,tempPLOBT,NULL,tempBeg,tempNum,tempFirst,
				 tempSecond,tempCount,current_status);

  return(status);
}


int DAL3IBISgetNumDummy(dal_element *InputDS,
			IBIS_type    EvType,
			int         *NumDummy,
			int          current_status)
  /*
    "DAL3IBISgetNumDummy" returns the number of dummy events of all kinds
    found in the given Science Window.
  */
{
  int status=ISDC_OK;
  ds_type type;
  dal_element *curDS;
  char name[DAL_MED_STRING];
  long rows;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Determine the type of the data structure                              */
    status=DAL3GENobjectType(InputDS,&type,NULL,NULL,NULL,status);
    if ( status!=ISDC_OK ) continue;
    if (type!=SWG) {
      status=DAL3IBIS_NOT_A_SWG;
      continue;
    }
    /* Select the type of events                                             */    
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-");        break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-");        break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-");        break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-");        break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-");        break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if (status!=ISDC_OK) continue;

    strcat(name,"SRW");
    status=DALobjectFindElement(InputDS,name,&curDS,status);
    if (status==ISDC_OK) status=DALtableGetNumRows(curDS,&rows,status);
    if (status==ISDC_OK) *NumDummy=rows;
    if (status==DAL_ELEMENT_NOT_FOUND) {
      *NumDummy=0;
      status=ISDC_OK;
    }

  } while (0);

  return(status);
}

/* SPR 2710, change SecondDummy to unsigned */
/* SPR 2710, now change it to unsigned long */
int DAL3IBISgetDummy(dal_element *InputDS,
		     IBIS_type    EvType,
		     int         *SSC,
		     int         *PartLOBT,
		     int         *Position,
		     int         *FirstDummy,
		     unsigned long *SecondDummy,
		     int         *CounterOvflw,
		     int          current_status)
  /*
    "DAL3IBISgetDummy" returns the number of dummy events of all kinds
    found in the given Science Window.
  */
{
  int status=ISDC_OK;
  ds_type type;
  dal_element *curDS;
  char name[DAL_MED_STRING];
  dal_dataType ds;
  long numRows;
  int num_2;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {
    /* Determine the type of the data structure                              */
    status=DAL3GENobjectType(InputDS,&type,NULL,NULL,NULL,status);
    if ( status!=ISDC_OK ) continue;
    if (type!=SWG) {
      status=DAL3IBIS_NOT_A_SWG;
      continue;
    }
    /* Select the type of events                                             */    
    switch (EvType) {
    case ISGRI_EVTS   : strcpy(name,"ISGR-EVTS-");        break;
    case PICSIT_SGLE  : strcpy(name,"PICS-SGLE-");        break;
    case PICSIT_MULE  : strcpy(name,"PICS-MULE-");        break;
    case COMPTON_SGLE : strcpy(name,"COMP-SGLE-");        break;
    case COMPTON_MULE : strcpy(name,"COMP-MULE-");        break;
    default           : status=DAL3GEN_INVALID_PARAMETER; break;
    }
    if (status!=ISDC_OK) continue;

    strcat(name,"SRW");
    status=DALobjectFindElement(InputDS,name,&curDS,status);
    if (status!=ISDC_OK) continue;
    /* To make stupid F90 language happy, everything is patched into INTEGER */
    ds=DAL_INT;
    /* Read the individual columns                                           */
    num_2=0;
    status=DAL3IBISgetNumDummy(InputDS,EvType,&num_2,status);
    if (num_2==0 || status!=ISDC_OK) continue;
    numRows=num_2;
    /* Source Sequence Count                                                 */
    /* Check if one must use a temporary SSC storage                         */
    if (SSC!=NULL) {
      status=DALtableGetCol(curDS,"SSC",0,&ds,&numRows,SSC,status);
      if (status!=ISDC_OK) continue;
    }
    /* Time in the data field header                                         */
    if (PartLOBT!=NULL) {
      status=DALtableGetCol(curDS,"PART_LOBT",0,&ds,&numRows,PartLOBT,status);
      if (status!=ISDC_OK) continue;
    }
    /* Position of the dummy events in the packet                            */
    if (Position!=NULL) {
      status=DALtableGetCol(curDS,"POSITION",0,&ds,&numRows,Position,status);
      if (status!=ISDC_OK) continue;
    }
    /* Value of the first dummy event                                        */
    if (FirstDummy!=NULL) {
      if (EvType==COMPTON_SGLE || EvType==COMPTON_MULE) status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      status=DALtableGetCol(curDS,"FIRST_DUMMY",0,&ds,&numRows,FirstDummy,status);
      if (status!=ISDC_OK) continue;
    }
    /* Value of the second dummy event                                       */
    if (SecondDummy!=NULL) {
      if (EvType==COMPTON_SGLE || EvType==COMPTON_MULE) status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      /* SPR 2710, change second dummy to unsigned */
      /* SPR 2710, change second dummy to unsigned long now*/
      ds=DAL_ULONG;
      status=DALtableGetCol(curDS,"SECOND_DUMMY",0,&ds,&numRows,SecondDummy,status);
      ds=DAL_INT;
      if (status!=ISDC_OK) continue;
    }
    /* Value of the second dummy event                                       */
    if (CounterOvflw!=NULL) {
      if (EvType!=COMPTON_SGLE && EvType!=COMPTON_MULE) status=DAL3IBIS_INCOMPATIBLE_PARAMETER;
      status=DALtableGetCol(curDS,"COUNTER_OVFLW",0,&ds,&numRows,CounterOvflw,status);
      if (status!=ISDC_OK) continue;
    }

  } while (0);

  return(status);
}

void DAL3IBISprintPlages()
{
  int i;
  
  puts ("CHFirst, CHLast, EvFirst, EvLast");
  for (i=0;i<DAL3IBISplageNum;i++) {
    printf("%7d %7d %7d %7d\n",DAL3IBISplageChFirst[i],
	   DAL3IBISplageChLast [i],
	   DAL3IBISplageEvFirst[i],
	   DAL3IBISplageEvLast [i]);
  }
}


int DAL3IBISfindEventGaps(dal_element *InputDS,
			  IBIS_type    EvType,
			  int         *NumMissPack,
			  int         *NumIntervals,
			  OBTime     **gtiStart,
			  OBTime     **gtiStop,
			  int          current_status)
  /*
    "DAL3IBISfindEventGaps" finds the possible gaps in the telemetry 
    and returns the OBT of the last event before the gaps and of the first event
    after the gap
  */
{
  int status = ISDC_OK;
  int numChildren;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {

    /* Call to the function that actually does the job                       */
    status=DAL3IBIScalculateEventGaps(InputDS,EvType,&numChildren,NumMissPack,NumIntervals,status);
    if (status!=ISDC_OK) continue;
    /* printPlages(); BEO */
    /* Do we also request the OBTs of the boundary events ?                  */
    if (gtiStart!=NULL && gtiStop!=NULL) {

      /* Allocating the memory for the two vectors                           */
      *gtiStart=malloc(DAL3IBISplageNum*sizeof(OBTime));
      *gtiStop =malloc(DAL3IBISplageNum*sizeof(OBTime));

      /* Call to the function that actually does the job                     */
      status=DAL3IBISfindGapsOBT(InputDS,EvType,numChildren,*gtiStart,*gtiStop,status);

    }

  } while (0);

  return(status);

}

int DAL3IBISfindAllEventGaps(dal_element *InputDS,
			     IBIS_type    EvType,
			     int         *NumMissPack,
			     int         *NumIntervals,
			     OBTime     **gtiStart,
			     OBTime     **gtiStop,
			     int          current_status)
  /*
    "DAL3IBISfindEventGaps" finds the possible gaps in the telemetry or in the 
    data itself
    and returns the OBT of the last event before the gaps and of the first event
    after the gap
  */
{
  int status = ISDC_OK;
  int numChildren;

  if ( current_status!=ISDC_OK ) return(current_status);

  do {

    /* Call to the function that actually does the job                       */
    status=DAL3IBIScalculateAllEventGaps(InputDS,EvType,&numChildren,
					 NumMissPack,NumIntervals,status);
    if (status!=ISDC_OK) continue;
    /* printPlages(); BEO */

    /* Do we also request the OBTs of the boundary events ?                  */
    if (gtiStart!=NULL && gtiStop!=NULL) {

      /* Allocating the memory for the two vectors                           */
      *gtiStart=malloc(DAL3IBISplageNum*sizeof(OBTime));
      *gtiStop =malloc(DAL3IBISplageNum*sizeof(OBTime));

      /* Call to the function that actually does the job                     */
      status=DAL3IBISfindGapsOBT(InputDS,EvType,numChildren,*gtiStart,*gtiStop,status);

    }

  } while (0);

  return(status);

}

