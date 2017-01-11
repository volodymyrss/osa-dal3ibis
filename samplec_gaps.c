/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                            C SAMPLE PROGRAM  for event gaps               */
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

int main(int argc, char *argv[]) {

  int status;
  dal_element *DAL_DS;
  int NumMissPack;
  int NumIntervals;
  int Numintervalsall;
  long long gapsSum = 0;
  long long allGapsSum = 0;
  int stopBeforeStart;
  OBTime *gtiStart;
  OBTime *gtiStop;
  int ii;
  
    

  status=ISDC_OK;

  if (argc !=  3) {
    puts("Need a science window/obs group as first argument and a boolean for printing as the second argument");
    return 1;
  }
  
  printf("%s",argv[1]);

  
  /* Opening the Group ...                                                   */
  status=DALobjectOpen(argv[1],&DAL_DS,status);

  status = DAL3IBISfindEventGaps(DAL_DS,/*PICSIT_MULE,*/ISGRI_EVTS,
				 &NumMissPack,
				 &NumIntervals,
				 &gtiStart,
				 &gtiStop,
				 status);

  if (ISDC_OK == status) {

    if (*argv[2] == 'y') printf("\n");
    if (*argv[2] == 'y') DAL3IBISprintPlages();
    
    if (*argv[2] == 'y') printf("Missed packets %d Intervals %d\n",
				NumMissPack, NumIntervals);
    for (ii=0;ii<NumIntervals;ii++) {
      if (*argv[2] == 'y')  printf("%3d %14llu %14llu %14llu %14llu %14lld\n",
				   ii,
				   gtiStart[ii],gtiStop[ii],
				   gtiStart[ii] - gtiStart[0],
				   gtiStop[ii]-gtiStart[0],
				   gtiStop[ii] - gtiStart[ii]);
      if (gtiStop[ii] - gtiStart[ii] < 0) stopBeforeStart = 1;
      gapsSum += gtiStop[ii] - gtiStart[ii];
      
    }
  }
  
  
  

  status = DAL3IBISfindAllEventGaps(DAL_DS,/* PICSIT_MULE, */ISGRI_EVTS,
				    &NumMissPack,
				    &Numintervalsall,
				    &gtiStart,
				    &gtiStop,
				    status);
  
  if (ISDC_OK == status) {

    if (*argv[2] == 'y') DAL3IBISprintPlages();
    
    if (*argv[2] == 'y') printf("All Missed packets %d Intervals %d\n",
				NumMissPack, Numintervalsall);
    for (ii=0;ii<Numintervalsall;ii++) {
      if (*argv[2] == 'y') printf("%3d %14llu %14llu %14llu %14llu %14lld\n",
				  ii,
				  gtiStart[ii],gtiStop[ii],
				  gtiStart[ii] - gtiStart[0],
				  gtiStop[ii]-gtiStart[0],
				  gtiStop[ii] - gtiStart[ii]);
      if (gtiStop[ii] - gtiStart[ii] < 0) stopBeforeStart = 1;
      allGapsSum += gtiStop[ii] - gtiStart[ii];
    }
    
  }
  
  /* Closing the input data structures ...                                   */

  status=DALobjectClose(DAL_DS,DAL_SAVE,status);

  if (status != ISDC_OK &&
      DAL3IBIS_NO_IBIS_EVENTS  != status) printf("ERROR: %d\n",status);
  if (Numintervalsall >= NumIntervals &&
      !stopBeforeStart &&
      gapsSum >= allGapsSum) {
    puts (" -- OK");
  } else {
    puts (" -- WRONG");
  }

  if (*argv[2] == 'y') printf("Numintervalsall %d NumIntervals %d Stopbeforestop %d \nallGapsSum %lld gapsSum %lld diff %lld \n",
			      Numintervalsall,NumIntervals,stopBeforeStart,
			      allGapsSum,gapsSum,gapsSum-allGapsSum);

  return(ISDC_OK);
}
