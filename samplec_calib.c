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
#include <dal3ibis_calib.h>



/*int doICgetNewestDOL(char * category,char * filter, double valid_time, char * DOL,int status) {
    char ic_group[255];
    snprintf(ic_group,255,"%s/idx/ic/ic_master_file.fits[1]",getenv("CURRENT_IC"));
    status=ICgetNewestDOL(ic_group,
            "OSA",
            category,filter,valid_time,DOL,status);
    return status;
}*/


int main(int arg, char *argv[]) {

  int status,i,column,OBT_num;
  OBTime OBTstart[10],OBTend[10];
  dal_element *DAL_DS;
  dal_dataType type;
  int sel,evType;
  long num_events,ib_ev[5];
  void *buffer;

  char DOL[255];
  int chatter=5;

  LogFile     fileRef;

  status=ISDC_OK;

  TRY_BLOCK_BEGIN
      setup_E_bands();

      double x;

      for (x=0;x<3;x+=0.1) {
        double en=pow(10,x);
        int ch=get_channel(en);
        double en1=E_band_min[ch];
        double en2=E_band_max[ch];
        printf("en:%.5lg ch:%i er: %g - %g\n",en,ch,get_E_min(ch),get_E_max(ch));
      }

      TRY(RILinit(&fileRef, "", OUT_ALL, "Default"),status,"RILinit");

      printf("OBJECT            : %s\n",argv[1]);

      OBT_num=0;
      type=DAL_LONG;
      
      printf("DALobjectOpen\n");
      TRY(DAL_GC_objectOpen(argv[1],&DAL_DS,status,"input object"),status,"opening input %s",argv[1]);
      
      IBIS_events_struct IBIS_events;
      ISGRI_energy_calibration_struct ISGRI_energy_calibration;
      PICsIT_energy_calibration_struct PICsIT_energy_calibration;
      ISGRI_efficiency_struct ISGRI_efficiency;


      TRY( DAL3IBIS_read_IBIS_events(DAL_DS,COMPTON_SGLE,&IBIS_events,1,chatter,status), 0, "reading Compton events"); 
      //TRY( DAL3IBIS_read_IBIS_events(DAL_DS,ISGRI_EVTS,&IBIS_events,1,chatter,status), 0, "reading ISGRI events"); 

      TRY( DAL3IBIS_init_ISGRI_energy_calibration(&ISGRI_energy_calibration,status), status, "initializing ISGRI energy calibration");
      TRY( DAL3IBIS_init_PICsIT_energy_calibration(&PICsIT_energy_calibration,status), status, "initializing PICsIT energy calibration");
      
      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &PICsIT_energy_calibration, DS_PICS_GO, &DAL3IBIS_open_PICsIT_GO, &DAL3IBIS_read_PICsIT_GO, chatter,status), status, "PICsIT GO" );

      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &ISGRI_energy_calibration, DS_ISGR_LUT1, &DAL3IBIS_open_LUT1, &DAL3IBIS_read_LUT1,chatter,status), status, "reading LUT1" );
      TRY( DAL3IBIS_correct_LUT1_for_temperature_bias(DAL_DS,&ISGRI_energy_calibration,&IBIS_events,chatter,status), status, "correcting for LUT1 temperature bias");
      
      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &ISGRI_energy_calibration, DS_ISGR_MCEC, &DAL3IBIS_open_MCEC, &DAL3IBIS_read_MCEC, chatter,status), status, "loading MCE evolution correction");
      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &ISGRI_energy_calibration, DS_ISGR_LUT2, &DAL3IBIS_open_LUT2, &DAL3IBIS_read_LUT2, chatter,status), status, "loading LUT2" );
      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &ISGRI_energy_calibration, DS_ISGR_L2RE, &DAL3IBIS_open_L2RE, &DAL3IBIS_read_L2RE, chatter,status), status, "loading LUT2 rapid evolution" );
      
      TRY( DAL3IBIS_reconstruct_ISGRI_energies(&ISGRI_energy_calibration,&IBIS_events,chatter,status), status, "reconstructing energies");

      TRY( DAL3IBIS_populate_newest_DS(&IBIS_events, &ISGRI_efficiency, DS_ISGR_EFFC,  &DAL3IBIS_open_EFFC, &DAL3IBIS_read_EFFC,chatter,status), status, "efficiency" );
      
      TRY( DAL3IBIS_reconstruct_ISGRI_energies(&ISGRI_energy_calibration,&IBIS_events,chatter,status), status, "reconstructing energies");
      TRY( DAL3IBIS_reconstruct_Compton_energies(&ISGRI_energy_calibration,&PICsIT_energy_calibration,&IBIS_events,chatter,status), status, "reconstructing energies");
      

  TRY_BLOCK_END
  
  //DAL_GC_print();
  DAL_GC_free_all();

//status=DALobjectClose(DAL_DS,DAL_SAVE,status);
  printf("Final status: %d\n",status);

  return(ISDC_OK);
}
