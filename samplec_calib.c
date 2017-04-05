/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                            C SAMPLE PROGRAM                               */
/*                                                                           */
/*  Authors: Volodymyr Savchenko                                             */
/*  Date:    23 January 2017                                                 */
/*  Version: 3.3.1                                                           */
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
#include <dal3ibis_calib.h>


int main(int arg, char *argv[]) {

  int status,i,column,OBT_num;
  OBTime OBTstart[10],OBTend[10];
  dal_element *DAL_DS;
  dal_dataType type;
  int sel,evType;
  long num_events,ib_ev[5];
  void *buffer;

  char DOL[255];
  int chatter=10;

  LogFile     fileRef;

  status=ISDC_OK;

  TRY_BLOCK_BEGIN

      TRY(RILinit(&fileRef, "", OUT_ALL, "Default"),status,"RILinit");

      printf("OBJECT            : %s\n",argv[1]);

      C256_setup_E_bands(chatter);

      OBT_num=0;
      type=DAL_LONG;
      
      printf("DALobjectOpen\n");
      TRY(DAL_GC_objectOpen(argv[1],&DAL_DS,status,"input object"),status,"opening input %s",argv[1]);
      
      IBIS_events_struct IBIS_events;
      ISGRI_energy_calibration_struct ISGRI_energy_calibration;
      PICsIT_energy_calibration_struct PICsIT_energy_calibration;
      ISGRI_efficiency_struct ISGRI_efficiency;
      ISGRI_efficiency.LT_approximation=0.001;

      TRY( DAL3IBIS_read_IBIS_events(DAL_DS,ISGRI_EVTS,&IBIS_events,1,chatter,status), 0, "reading events"); 

      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &ISGRI_efficiency, DS_ISGR_EFFC,  &DAL3IBIS_open_EFFC, &DAL3IBIS_read_EFFC,chatter,status), status, "efficiency" );

      dal_double **LowThreshMap= NULL;
      if((LowThreshMap=(dal_double **)calloc(ISGRI_SIZE, sizeof(dal_double *)))==NULL)
        FAIL(status, "Error in allocating memory for LowThreshold image  map.");
      for(i=0; i<ISGRI_SIZE; i++)
        if((LowThreshMap[i]=(dal_double *)calloc(ISGRI_SIZE, sizeof(dal_double )))==NULL)
          FAIL(status,"Error in allocating memory for LowThreshold image map : i = %d.", i);
      dal_int **ONpixelsREVmap= NULL;
      if((ONpixelsREVmap= (dal_int**)calloc(ISGRI_SIZE, sizeof(dal_int*)))==NULL) FAIL(status,"");
      for(i=0; i<ISGRI_SIZE; i++)
        if((ONpixelsREVmap[i]= (dal_int*)calloc(ISGRI_SIZE, sizeof(dal_int)))==NULL) FAIL(status,"")


      TRY( DAL3IBIS_init_ISGRI_energy_calibration(&ISGRI_energy_calibration,status), status, "initializing ISGRI energy calibration");
      TRY( DAL3IBIS_init_PICsIT_energy_calibration(&PICsIT_energy_calibration,status), status, "initializing PICsIT energy calibration");

      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &ISGRI_energy_calibration, DS_ISGR_LUT1, &DAL3IBIS_open_LUT1, &DAL3IBIS_read_LUT1,chatter,status), status, "reading LUT1" );
      TRY( DAL3IBIS_correct_LUT1_for_temperature_bias(DAL_DS,&ISGRI_energy_calibration,&IBIS_events,chatter,status), status, "correcting for LUT1 temperature bias");
      
      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &ISGRI_energy_calibration, DS_ISGR_MCEC, &DAL3IBIS_open_MCEC, &DAL3IBIS_read_MCEC, chatter,status), status, "loading MCE evolution correction");
      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &ISGRI_energy_calibration, DS_ISGR_LUT2, &DAL3IBIS_open_LUT2, &DAL3IBIS_read_LUT2, chatter,status), status, "loading LUT2" );
      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &ISGRI_energy_calibration, DS_ISGR_L2RE, &DAL3IBIS_open_L2RE, &DAL3IBIS_read_L2RE, chatter,status), status, "loading LUT2 rapid evolution" );
      
      TRY( DAL3IBIS_populate_newest_DS(IBIS_events.ijdStart,IBIS_events.ijdStop, &PICsIT_energy_calibration, DS_PICS_GO, &DAL3IBIS_open_PICsIT_GO, &DAL3IBIS_read_PICsIT_GO, chatter,status), status, "PICsIT GO" );
      
      TRY( DAL3IBIS_reconstruct_ISGRI_energies(&ISGRI_energy_calibration,&IBIS_events,chatter,status), status, "reconstructing energies");
      

  TRY_BLOCK_END
  
  printf("Final status: %d\n",status);

  return(ISDC_OK);
}
