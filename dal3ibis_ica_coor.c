/*****************************************************************************/
/*                                                                           */
/*                       INTEGRAL SCIENCE DATA CENTRE                        */
/*                                                                           */
/*                           DAL3  IBIS  LIBRARY                             */
/*                                                                           */
/*                              ICA COORDINATES                              */
/*                                                                           */
/*  Authors: Stéphane Paltani, Laurent Lerusse                            */
/*  Date:    01 July 2003                                                    */
/*  Version: 4.2.1                                                           */
/*                                                                           */
/*  Revision history                                                         */
/*                                                                           */
/*  01.07.2003 V 4.2.1                                                       */
/*  ==================                                                       */
/*  SPR 3084: The algorithm previously implemented was wrong.                */
/*            I took this one from Aymeric Sauvageon and IBIS ISSW S/W       */
/*                                                                           */
/*  08.04.2001 V 3.4.0 First version                                         */
/*  ==================                                                       */
/*  08.04: SCR0180: The ICA APIs have been completely revised. This part of  */
/*                  DAL3IBIS is now isolated at the file level to ease the   */
/*                  concurrent development.                                  */
/*                                                                           */
/*  01.09.1999 V 2.2.0                                                       */
/*  ==================                                                       */
/*  1. New DAL3IBIS APIs for ICA are included (written by L.Lerusse)         */
/*                                                                           */
/*****************************************************************************/

#include "dal3ibis.h"
            
/****************************************************************************
                                                                          
***************************************************************************/

#define MAX_AXIS 127
#define MAX_MCE    7
#define MAX_LINE   7
#define MAX_ASIC  63
#define MAX_PIX    3
#define MAX_BIT    7

const int PosMatrix[4][4] = { {3300, 3201, 2310, 2211},
                              {1221, 1320,  231,  330},
                              {  33,  132, 1023, 1122},
                              {2112, 2013, 3102, 3003}, };

const int YZ_Matrix[4][4] = { {2000, 2202, 3111, 3313},
                              {2101, 2303, 3010, 3212},
                              {1232, 1030,  323,  121},
                              {1333, 1131,  222,   20}, };

int LAPdecode(unsigned short [], 
              unsigned short []);


/********************************************************************/
int LAPdecode(unsigned short posIn[], 
              unsigned short posOut[])
{
 int mce, posA, posP, nY, nZ;

 if (posIn[0]>MAX_MCE)  return(-1);
 if (posIn[1]>MAX_LINE) return(-2);
 if (posIn[2]>MAX_ASIC) return(-3);
 if (posIn[3]>MAX_PIX)  return(-4);
 for (mce=0; mce<4; mce++) posOut[mce+2] = posIn[mce] ;
 posOut[6] = posOut[4] + 64 * (posOut[3] + 8*posOut[5]);
 posOut[7] = 4*(63-posOut[4]) + posOut[5];
 posOut[8] = posOut[3];
 posOut[9] = 4 * ((63-posOut[4]) % 4) + posOut[5];
 mce = posIn[0];
 if (mce<4) {
   nY = 64;
   nZ = 32*(3-mce) + 4*(7-posIn[1]);
   nY += posIn[2] & 252;
 }
 else {
   nY = 0;
   nZ = 32*(7-mce) + 4*posIn[1];
   nY += (63-posIn[2]) & 252;
 }
 posA = posIn[2] & 3;
 posP = PosMatrix[posA][posIn[3]];
 if (mce<4) posP = posP % 100; else posP = (int)(posP/100);
 posOut[0] = nY + posP % 10;
 posOut[1] = nZ + (int)(posP/10);
 return(0);
}

/***************************************************************************/
                                                                          
int DAL3IBISicaTransfCoord(int   module,
                           int   offsetBytes,
                           int   bitsNum, 
                           int  *YPtr, 
                           int  *ZPtr,
                           int   status){
  unsigned short v[4]={0, 0, 0, 0}, res[10];

  v[0] = module;
  v[3] = offsetBytes & 3;
  offsetBytes /= 4;
  v[2] = 63 - offsetBytes;
  v[1] = bitsNum;
  status = LAPdecode(v, res);
  if(status != ISDC_OK){
    status += DAL3IBIS_ERROR_CODE_BASE-310;
    return(status);
  }
  *YPtr      = res[0];    
  *ZPtr      = res[1];   
  return(status);
}


