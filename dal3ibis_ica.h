/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                                  ICA                                      */
/*                                                                           */
/*                              C PROTOTYPE                                  */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                               */
/*  Date:    30 Jan 2012                                                     */
/*  Version: 5.5                                                             */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  30.01.2012 V  5.5                                                        */
/*  ==================                                                       */
/*  30.01: SPR 5011: protect  OBTime     (*eff_time)[IBIS_NUM_BLOCK+1],      */
/*                   of function DAL3IBISicaIsgriNoisEff for ROOT            */
/*                                                                           */
/*                                                                           */
/*  02.05.2001 V 3.4.2                                                       */
/*  ==================                                                       */
/*  01.05: SPR0392: Implemented a multipart SPR: Fixed wrong extension name, */
/*                  missing symbols, treatment of NO_OBT, missing "break" in */
/*                  switch, insufficient alloc size, wrong selection, bad    */
/*                  pixel position                                           */
/*                                                                           */
/*  08.04.2001 V 3.4.0 First version                                         */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*****************************************************************************/

 /******************************************************************************

 Error Code : 

    name                       value     description
   --------------------------|---------|-------------------------------------
   DAL3IBIS_NOT_VALID_DS       	-26302	The DS is not the exepected one
   DAL3IBIS_NOT_VALID_INDEX   	-26303	The DS is not an index or not the expected one
   DAL3IBIS_INDEX_EMPTY	      	-26304	The selection inside the function return 0 DS.
   DAL3IBIS_ICA_PIX_POS_ERR    	-26305	Error in the pixel position < 0 or > detector size
   DAL3IBIS_BAD_CTXT_STRUCT    	-26306  The structure of the context is not as expected.
   DAL3IBIS_BAD_CTXT_PARAMETER 	-26307  The dataKind pameter has a wrong value.
   DAL3IBIS_SW_LIST_NOT_EMPTY   -26308	The pixel switches list DS is not empty, 
   					The function wil not write in this table.
   DAL3IBIS_NOT_ENOUGH_SW 	-26309  the number of allocated switches is less then the number trying to be written.
******************************************************************************/

#define DAL3IBIS_NOT_VALID_DS 	      DAL3IBIS_ERROR_CODE_BASE	-302
#define DAL3IBIS_NOT_VALID_INDEX      DAL3IBIS_ERROR_CODE_BASE	-303
#define	DAL3IBIS_INDEX_EMPTY	      DAL3IBIS_ERROR_CODE_BASE	-304
#define DAL3IBIS_ICA_PIX_POS_ERR      DAL3IBIS_ERROR_CODE_BASE	-305
#define	DAL3IBIS_BAD_CTXT_STRUCT      DAL3IBIS_ERROR_CODE_BASE	-306
#define	DAL3IBIS_BAD_CTXT_PARAMETER   DAL3IBIS_ERROR_CODE_BASE	-307
#define DAL3IBIS_SW_LIST_NOT_EMPTY    DAL3IBIS_ERROR_CODE_BASE	-308
#define DAL3IBIS_NOT_ENOUGH_SW        DAL3IBIS_ERROR_CODE_BASE	-309
#define DAL3IBIS_UNEXPECTED_SWITCH_FLAG DAL3IBIS_ERROR_CODE_BASE -310
/* extname for the ISGRI context */
#define	ICA_EXTNAME_ISGR_CTXT_GRP   	"ISGR-CTXT-GRP"
#define	ICA_EXTNAME_ISGR_CTXT_MODP  	"ISGR-MODP-CFG"
#define	ICA_EXTNAME_ISGR_CTXT_ASIP  	"ISGR-ASIP-CFG"
#define	ICA_EXTNAME_ISGR_CTXT_PXLP  	"ISGR-PXLP-CFG"

/* extname for the PICsIT context */
#define	ICA_EXTNAME_PICS_CTXT_GRP   	"PICS-CTXT-GRP"
#define	ICA_EXTNAME_PICS_CTXT_DETP  	"PICS-DETP-CFG"
#define	ICA_EXTNAME_PICS_CTXT_MODP  	"PICS-MODP-CFG"
#define	ICA_EXTNAME_PICS_CTXT_PXLP  	"PICS-PXLP-CFG"

/* extname for the ISGRI pixel switches list */
#define	ICA_EXTNAME_ISGR_SW_LIST   	"ISGR-SWIT-STA"

/* extname for the PICsIT faulty pixels list */
#define	ICA_EXTNAME_PICS_FALT_LIST   	"PICS-FALT-STA"

/* extname for the ISGRI noisy pixel map */
#define ICA_EXTNAME_ISGR_NOISY_MAP	"ISGR-NOIS-CPR"

/* OBT keyword */
#define ICA_OBT_FIRST_KEY	"OBTFIRST"
#define ICA_OBT_LAST_KEY	"OBTLAST"

#define SWITCH_START_TIME     	ICA_OBT_FIRST_KEY 
#define SWITCH_END_TIME		ICA_OBT_LAST_KEY 
	    
#define ICA_OBT_CTXT_REF_KEY	"CTXT_OBT"
#define ICA_ORG_CTXT_REF_KEY	"CTXT_ORG"

typedef enum { 	IBIS_INDEX	= 0,
		IBIS_DS		= 1 ,
		ISGRI_CTXT      = 10,
		ISGRI_PIX_GAIN  = 11, 
		ISGRI_PIX_LTHR  = 12, 
		ISGRI_PIX_STA   = 13,
		ISGRI_PIX_TEST  = 14,
		ISGRI_ASIC_GAIN = 15,
		ISGRI_ASIC_HTHR = 16,
		ISGRI_MODP      = 18, 
		PICSIT_CTXT     = 20, 
		PICSIT_PIX_STA  = 21,
		PICSIT_MODP     = 28,
		PICSIT_DETP     = 29 } IBISDS_Type;

/* typical quantities */
#define IBIS_NUM_BLOCK      	8
#define ISGRI_SIZE          	128
#define IBIS_IBLOCK_LENGTH  	256
#define PICSIT_SIZE         	64
#define SELECT_STRING_LENGTH   	1024

/* colname and data type of the ISGR-LIST-STA data structure */
#define SWITCH_Y_COLNAME      "ISGRI_Y" 
#define SWITCH_Y_COLTYPE      DAL_BYTE	
#define SWITCH_Z_COLNAME      "ISGRI_Z" 
#define SWITCH_Z_COLTYPE      DAL_BYTE
#define SWITCH_OBT1_COLNAME   "OBT_DETECT"
#define SWITCH_OBT2_COLNAME   "OBT_SWITCH"
#define SWITCH_FLAG_COLNAME   "NOIS_FLAG"
#define SWITCH_FLAG_COLTYPE   DAL_BYTE

/* Keywords in the  ISGR-LIST-STA data structure */
#define SWITCH_ORIGIN_KEYWORD	"FILE_ORG"
#define SWITCH_ORIGIN_NOISY_MAP	"NOISY_MAP"
#define SWITCH_ORIGIN_SC_EVENTS	"SC_EVENTS"
#define SWITCH_MCE_END_TIME_0	"OBT_MCE0"
#define SWITCH_MCE_END_TIME_1	"OBT_MCE1"
#define SWITCH_MCE_END_TIME_2	"OBT_MCE2"
#define SWITCH_MCE_END_TIME_3	"OBT_MCE3"
#define SWITCH_MCE_END_TIME_4	"OBT_MCE4"
#define SWITCH_MCE_END_TIME_5	"OBT_MCE5"
#define SWITCH_MCE_END_TIME_6	"OBT_MCE6"
#define SWITCH_MCE_END_TIME_7	"OBT_MCE7"

/* colname and data type of the ISGR-FALT-STA data structure */
#define FAULT_Y_COLNAME      "PICSIT_Y" 
#define FAULT_Y_COLTYPE      DAL_BYTE	
#define FAULT_Z_COLNAME      "PICSIT_Z" 
#define FAULT_Z_COLTYPE      DAL_BYTE
#define FAULT_OBT1_COLNAME   "OBT_DETECT"
#define FAULT_OBT2_COLNAME   "OBT_FAULT"
#define FAULT_FLAG_COLNAME   "FALT_FLAG"
#define FAULT_FLAG_COLTYPE   DAL_BYTE

/* Keywords in the  ISGR-FALT-STA data structure */
#define FAULT_OBT_START_KEY	"OBTSTART"
#define FAULT_OBT_END_KEY	"OBTEND"
#define FAULT_OBT_CTXT_KEY	"OBT_CTXT"
#define FAULT_OBT_SC_KEY	"OBT_SC"

/* colname and data type of the ISGR-NOIS-CRW and CRP data structure */
#define NOISY_MAP_PACK_TIME_COLNAME "PACK_TIME"
#define NOISY_MAP_OBT_COLNAME       "OB_TIME"
#define NOISY_MAP_MCEID_COLNAME     "MCE_ID"
#define NOISY_MAP_PERIOD_ON_COLNAME "PERIOD_ON"         
#define NOISY_MAP_ON_RANGE_COLNAME  "ON_RANGE"
#define NOISY_MAP_BLOCKS_COLNAME    "PIXEL_STATUS"

#define DAL3IBIS_IPIX_STABLE      	0 
#define DAL3IBIS_IPIX_ON            	0 
#define DAL3IBIS_IPIX_OFF            	1 
#define DAL3IBIS_IPIX_DUBIOUS        	2 
#define DAL3IBIS_SWITCH_ON_OFF	     	3
#define DAL3IBIS_SWITCH_OFF_ON       	4
#define DAL3IBIS_IPIX_UNKNOWN           255

/* SPR 2202 */
#define DAL3IBIS_ISGRI_CTXT_STATUS_ON           1
#define DAL3IBIS_ISGRI_CTXT_STATUS_DUBIOUS      0
#define DAL3IBIS_PICSIT_CTXT_STATUS_ON          0
#define DAL3IBIS_PICSIT_CTXT_STATUS_DUBIOUS     1

/* Specific value for the low thresholds */
#define DAL3IBIS_THRES_OFF_VALUE	63
#define DAL3IBIS_THRES_NO_VALUE		255
/* meaning of the value of the status in the context table */
#define DAL3IBIS_CTXT_STATUS_ON		0
#define DAL3IBIS_CTXT_STATUS_DUBIOUS   	1
/* meaning of the value of the status in the noisy pixel blocks Hk H3 */
#define DAL3IBIS_BLK_STATUS_ON		1
#define DAL3IBIS_BLK_STATUS_OFF   	0

/* 
 *
 */

#ifndef __CINT__
#ifdef __cplusplus
extern "C" {
#endif
#endif

/*****************************************************************************

 Function :  DAL3IBISindexOrMember                                          

 Description :
 	Determine the input type. It can be the data structure or 
	the index of it 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   dsPtr	  dal_element*	  I   Pointer to the table index or to the 
   				      table itself.
   extnameRef	  char * 	  I   Extname of the expected data structure
   input	  IBISDS_Type*	  O   Input type
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISindexOrMember(DAL_ELEMENTP dsPtr,
			  char        *extnameRef,
			  IBISDS_Type *input,
			  int          status);

/*****************************************************************************

 Function :  DAL3IBISicaGetSize                                          

 Description :
 	Determine the maximum size needed to read tables. 
	The input can be an index of those tables.
	The selection will be : 
	 " limStart <= endDS_key && startDS_key <= limEnd "

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   dsPtr	  dal_element*	  I   Pointer to the table index or to the 
   				      table itself.
   input	  IBISDS_Type 	  I   Input type
   startDs_key	  char*		  I   keyword onto the selection will be made
   endDs_key	  char*		  I   keyword onto the selection will be made
   limStart	  OBTime	  I   limit OBT for the selection
   limEnd	  OBTime	  I   limit OBT for the selection
   size		  long *	  O   Number of rows find in the selected 
   				      tables.
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISicaGetSize(DAL_ELEMENTP dsPtr,
  		       IBISDS_Type  input,
		       char 	   *startDs_key,
		       char 	   *endDs_key,
  		       OBTime       limStart, 
		       OBTime       limEnd, 
		       long        *size, 
		       int          status);

/*****************************************************************************

 Function :  DAL3IBISicaTransfCoord                                          

 Description :
 	This function transform the bit position in the block in 
 	the Y, Z coordinate.      
	       
 Parameter argument :

    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   module	  int		  I  Module Number
   offsetBytes	  int		  I  offset in byte max value == 255
   bitsNum	  int		  I  bit number 0 == LSB, 7 = MSB
   YPtr           int*		  O  Y coordinate
   ZPtr           int*		  O  Z coordinate
   status         int            I/O Error code

*****************************************************************************/
int DAL3IBISicaTransfCoord(int   module,
	       	    	   int   offsetBytes,
		  	   int   bitsNum, 
		 	   int  *Y, 
		 	   int  *Z,
		 	   int   status);
  
/*****************************************************************************

 Function :   DAL3IBISgetSizeNoisyMaps                                         

 Description :
 	Get the size needed to allocate the memory space for the data. 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                             
    mapPtr	  dal_element*	  I  Pointer to the ISGRI noisy map index or 
   				     to the noisy map DS itself.
   obtStart	  OBTime	  I  Start time of the considered interval
   obtEnd	  OBTime	  I  End time of the considered interval
   noisylength	  long*		  O  Number of block to allocate.
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISgetSizeNoisyMaps(DAL_ELEMENTP mapPtr, 
			     OBTime       obtStart, 
			     OBTime       obtEnd, 
			     long        *noisylength, 
			     int          status);
			      			  

/*****************************************************************************

 Function : DAL3IBISallocateNoisyMap                                           

 Description :
 	Allocate the memory space for the ISGRI noisy maps.

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   blockTime	  OBTime**	 I/O Time of the block
   mceId	  DAL3_Byte**	 I/O Mce Id of the block
   periodOn	  DAL3_Byte**	 I/O Period On for the block
   onRange	  DAL3_Byte**	 I/O On Range for the block
   blockStatus	  DAL3_Byte** [] I/O Pixel status map
   noisylength 	  long		  I  Number of blocks to allocate.
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISallocateNoisyMaps(OBTime **blockTime, 
			      DAL3_Byte **mceId, 
			      DAL3_Word **periodOn, 
			      DAL3_Byte **onRange,
#if defined(__CINT__) || defined(COMPILING_CINT_FILES)
			      DAL3_Byte ***blockStatus,
#else
			      DAL3_Byte (**blockStatus)[IBIS_IBLOCK_LENGTH],
#endif
			      long noisylength,
			      int status);

/*****************************************************************************

 Function : DAL3IBISfreeNoisyMap                                           

 Description :
 	Dellocates the memory space for the ISGRI noisy maps.

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   blockTime	  OBTime*	 I/O Time of the block
   mceId	  DAL3_Byte*	 I/O Mce Id of the block
   periodOn	  DAL3_Byte*	 I/O Period On for the block
   onRange	  DAL3_Byte*	 I/O On Range for the block
   blockStatus	  DAL3_Byte** []  I/O Pixel status map
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISfreeNoisyMaps(OBTime     *blockTime, 
			  DAL3_Byte  *mceId, 
			  DAL3_Word  *periodOn, 
			  DAL3_Byte  *onRange, 
			  DAL3_Byte (*blockStatus)[IBIS_IBLOCK_LENGTH], 
			  int         status);

/*****************************************************************************

 Function : DAL3IBISgetNoisyMaps                                            

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
 
    mapPtr	  dal_element*	  I  Pointer to the ISGRI noisy map index or 
   				     to the noisy map DS itself.
   obtStart	  OBTime	  I  Start time of the considered interval
   obtEnd	  OBTime	  I  End time of the considered interval
   blockTime	  OBTime*	  O  Time of the block
   mceId	  DAL3_Byte*	  O  Mce Id of the block
   periodOn	  DAL3_Byte*	  O  Period On for the block
   onRange	  DAL3_Byte*	  O  On Range for the block
   blockStatus	  DAL3_Byte**	  O  Pixel status map
   noisylength 	  long*		  O  Number of blocks read.
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISgetNoisyMaps(DAL_ELEMENTP  mapPtr, 
			 OBTime        obtStart, 
			 OBTime        obtEnd, 
			 OBTime       *blockTime, 
			 DAL3_Byte    *mceId, 
			 DAL3_Word    *periodOn, 
			 DAL3_Byte    *onRange, 
#if defined(__CINT__) || defined(COMPILING_CINT_FILES)
			 DAL3_Byte   **blockStatus, 
#else
			 DAL3_Byte   (*blockStatus)[IBIS_IBLOCK_LENGTH], 
#endif
			 long         *noisylength, 
			 int           status);

/*****************************************************************************

 Function : DAL3IBISgetSizeSwitchList                                           

 Description :
 	Get the size needed to allocate the memory space for the data. 

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
    mapPtr	  dal_element*	  I  Pointer to the ISGRI pixels switches 
    				     list index or to the list DS itself.
   obtStart	  OBTime	  I  Start time of the considered interval
   obtEnd	  OBTime	  I  End time of the considered interval
   noisylength	  long*		  O  Number of switch to allocate.
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISgetSizeSwitchList(DAL_ELEMENTP listPtr, 
			      OBTime       obtStart, 
			      OBTime       obtEnd, 
			      long        *listlength, 
			      int          status);	

/*****************************************************************************

 Function : DAL3IBISallocateSwitchList                                           

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   status         int            I/O Error code.
                           

*****************************************************************************/
int DAL3IBISallocateSwitchList(DAL3_Byte **Y_switch, 
			       DAL3_Byte **Z_switch, 
			       OBTime    **timeDetect, 
			       OBTime    **timeSwitch, 
			       DAL3_Byte **flag_switch, 
			       long        listlength, 
			       int         status);
/*****************************************************************************

 Function : DAL3IBISfreeSwitchList                                           

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   status         int            I/O Error code.
                              

*****************************************************************************/
int DAL3IBISfreeSwitchList(DAL3_Byte *Y_switch, 
			   DAL3_Byte *Z_switch, 
			   OBTime    *timeDetect, 
			   OBTime    *timeSwitch, 
			   DAL3_Byte *flag_switch, 
			   int         status);

/*****************************************************************************

 Function : DAL3IBISgetSwitchList                                           

 Description : DAL3IBISgetSwitchList

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISgetSwitchList(DAL_ELEMENTP listPtr, 
			  OBTime       obtStart, 
			  OBTime       obtEnd, 
			  DAL3_Byte   *Y_switch, 
			  DAL3_Byte   *Z_switch, 
			  OBTime      *timeDetect, 
			  OBTime      *timeSwitch, 
			  DAL3_Byte   *flag_switch, 
			  long        *listlength, 
			  int          status);	
			  		       
/*****************************************************************************

 Function : DAL3IBISgetSwitchListMceTime                                           

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
                              
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISgetSwitchListMceTime( DAL_ELEMENTP listPtr, 
				  OBTime obtStart, 
				  OBTime obtEnd, 
				  OBTime *mceTime, 
				  int status);

/*****************************************************************************

 Function : DAL3IBISputSwitchList                                           

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|---------------------------------------- 
                             
   status         int            I/O Error code.

*****************************************************************************/
int DAL3IBISputSwitchList( DAL_ELEMENTP  listPtr,
		           DAL3_Byte    *Y_switch,
			   DAL3_Byte    *Z_switch,
			   OBTime       *timeDetect,
  		       	   OBTime       *timeSwitch,
  		           DAL3_Byte    *flag_switch,
  		           long          numSwitch,
			   OBTime	*mceTime,
			   char 	*origin,
		           int           status);

/*****************************************************************************

 Function : DAL3IBIselectIsgriCtxt                                           

 Description :

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   ctxtPtr	  dal_element *   I   pointer to the inde of context or to 
   				      the context itself                    
   limTime	  OBTime	  I   OBT used to select in the index.
   dataKind	  IBISDS_Type	  I   parameter which define where the function
   				      is expected to read
   slctCtxtPtr	  dal_element **  O   pointer to the selected context DS.
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISselectCtxt(DAL_ELEMENTP  ctxtPtr, 
		       OBTime       *limTime,
 		       IBISDS_Type   dataKind, 
		       DAL_ELEMENTP *slctCtxtPtr,
		       int           status);

/*****************************************************************************

 Function :  DAL3IBISctxtGetPixAsicPar                                          

 Description : Read the IMAGE part (pixels, ASICs parameters) 
               of the IBIS context (ISGRI or PICsIT)

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   ctxtPtr	  dal_element *   I   pointer to the inde of context or to 
   				      the context itself                    
   limTime	  OBTime	  I   OBT used to select in the index.
   dataKind	  IBISDS_Type	  I   parameter which define where the function
   				      is expected to read
   dataBuff	  DAL_VOIDP	  O   data buffer
   status         int            I/O  Error code.

*****************************************************************************/
int DAL3IBISctxtGetImaPar(DAL_ELEMENTP ctxtPtr, 
			  OBTime      *limTime, 
			  IBISDS_Type  dataKind, 
			  DAL_VOIDP    dataBuff, 
			  int          status);

/*****************************************************************************

 Function : DAL3IBISctxtGetMcePar                                           

 Description : Read the TABLE part (module, detector parameters) 
               of the IBIS context (ISGRI or PICsIT)

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|--------------|---|----------------------------------------
   ctxtPtr	  dal_element *   I   pointer to the inde of context or to 
   				      the context itself                    
   limTime	  OBTime	  I   OBT used to select in the index.
   dataKind	  IBISDS_Type	  I   parameter which define where the function
 				      is expected to read
   colName	  const char *	  I   column name 
   dataType	  dal_dataType	  I   data type of the column
   dataBuff	  void *	  O   data buffer
   status         int            I/O  Error code.
                              

*****************************************************************************/
int DAL3IBISctxtGetTblPar(DAL_ELEMENTP ctxtPtr, 
			  OBTime      *limTime, 
			  IBISDS_Type  dataKind, 
			  const char  *colName, 
			  dal_dataType dataType, 
			  DAL_VOIDP    dataBuff, 
			  int          status);
			

/*****************************************************************************

 Function : DAL3IBISicaIsgriNoisEff                                           

 Description :	This function give some statistic on the noisy pixels:      
   	        - The percent of time the pixels are off and 		    
	        - The number of pixels which are off at a certain time.     
	       The statistics are build for a science window group 	    
	       or an observation group. The calculation are limited to the  
	       "good" time interval.	

 Parameter argument :                                                      
    name           type          I/O  description
   --------------|------------|---|------------------------------------------
   idxListPtr	  dal_element*	I     - Pointer to the INDEX of ISGRI pixel 
   				        switches list    
   numGti   	  int		I     - number of uninterupted "good"        
   				        time intervals writtewn to 'gti'     
   gtiStart 	  OBTime*	I     - array of OBTs giving the starts of   
				        the GTIs. 			     	
   gtiStop        OBTime*	I     - array of OBTs giving the ends of     
				        the GTIs. 			     
   pixPercOffTime double** 	O     - array (1 cell per pixel) The value   
   				        in the cell is the percentage of     
				        time, the pixel was off. 	     
   numChg	  long*   	O     - array of long, 1 value per MCE and   
   				        1 for the detector. 		     
				        give the number of changes that has  
 				        been observed.			     	     
   effTime        OBTime**	O     - array of OBTs. 1 array per MCE and   
   				        1 for the detector.		     
				        OBT from which the number of pixel   
				        off is valid.			     
   numPixOff	  int**		O     - array of int. 1 array per MCE and       				        1 for the detector.		     
				        number of pixel off.		     
   status         int          I/O    - Error code                           
                             

*****************************************************************************/
int DAL3IBISicaIsgriNoisEff(DAL_ELEMENTP idxListPtr,
			    int          numGti,  /* min == 1 */
			    OBTime      *gtiStart,
			    OBTime      *gtiStop,
			    double       pixPercOffTime[ISGRI_SIZE][ISGRI_SIZE],
			    long         num_change[IBIS_NUM_BLOCK+1], 
#if defined(__CINT__) || defined(COMPILING_CINT_FILES)   /* SPR 5011  */
             OBTime     *eff_time,
             int        *num_pixOff,
#else
			    OBTime     (*eff_time)[IBIS_NUM_BLOCK+1],
			    int        (*num_pixOff)[IBIS_NUM_BLOCK+1],
#endif
			    int          status);


/*****************************************************************************

 Function : DAL3IBISGetlowthresholdKev              

 Description :	This function retrun all the low threshold valid at time limTime


 Parameter argument :                                                      
    name           type        I/O  description
   --------------|------------|---|------------------------------------------
   ctxtPtr	  dal_element*	I     - Pointer to the INDEX of ISGRI context
   limTime        OBTime        I     - Time at which we want to know the low threshold
   dataBuff       128*128floats O     - the low Threshold in Kev
                                        pixel z y is dataBuff[z*ISGRI_SIZE+y]
   status         int          I/O    - Error code                           
                             

*****************************************************************************/
int DAL3IBISGetlowthresholdKev( dal_element *ctxtPtr, 
				OBTime      limTime, 
				float        *dataBuff, 
				int          status);

/*****************************************************************************

 Function : DAL3IBISTransformISGRIEnergy

 Description :	

 Parameter argument :                                                      
    name           type        I/O  description
   --------------|------------|---|------------------------------------------
   ctxtPtr	  dal_element*	I     - Pointer to the INDEX of ISGRI context
   limTime        OBTime        I     - Time at which we want to know the low threshold
   dataBuff       128*128floats O     - the low Threshold in Kev
                                        pixel z y is dataBuff[z*ISGRI_SIZE+y]
   status         int          I/O    - Error code                           
                             

*****************************************************************************/
int DAL3IBISTransformISGRIEnergy(dal_element 
				OBTime      limTime, 
				float        *dataBuff, 
				int          status);

/*****************************************************************************

 Function : DAL3IBISGetISGRIEfficiency

 Description :	ISGRI efficiency per pixel and energy

 Parameter argument :                                                      
    name           type        I/O  description
   --------------|------------|---|------------------------------------------
   ctxtPtr	  dal_element*	I     - 
   limTime        OBTime        I   
   dataBuff       128*128floats O   
                                    
   status         int          I/O  
                             

*****************************************************************************/
int DAL3IBISGetISGRIEfficiency(dal_element 
				OBTime      limTime, 
				float        *dataBuff, 
				int          status);

#ifndef __CINT__
#ifdef __cplusplus
}
#endif
#endif
