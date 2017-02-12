#include <stdio.h>
#include <stdlib.h>

#ifdef __CINT__
  #include "isdc_isdcroot.h"
#else
  #include "isdc.h"
  #include "icaccess.h"
#endif

int icaccess_test(int argc = 0, char** argv = 0) {
  int status = ISDC_OK;
  int saveStatus = ISDC_OK;
  int num = 0;
  char **outdols=NULL;
  char outdol[PIL_LINESIZE];
  *outdol = '\0';


  status = CommonRILinit("icaccess_test", "1.0", status);
  if(ISDC_OK != status) { printf("Unable to initialize RIL!"); return -1; }

  char *DS=argv[1]; // oh!!
  double ijd=atof(argv[2]);
  char alias[]="OSA";

  char icmaster[DAL_BIG_STRING];
  sprintf(icmaster,"%s/idx/ic/ic_master_file.fits[1]",getenv("CURRENT_IC"));
  status=ICsubIndexGetDOLs(icmaster,
                          alias,
                          DS,
                          "",
                          "VSTART",
                          DAL_SORT_DESCENDING,
                          &outdols,
			  &num,
                          status);
  saveStatus = (ISDC_OK == saveStatus) ? status : saveStatus;

  printf("%i %i\n",status,num);

  for (int i=0; i<num; i++) {
      printf("%s \n",outdols[i]);
  }
  status=DALfreeStringBuffer(outdols,num,status);
  status=ISDC_OK;

  /*status=ICgetOneDOL(icmaster,
		     alias,
		     DS,
		     "VSTART<7000",
		     outdol,status);
  saveStatus = (ISDC_OK == saveStatus) ? status : saveStatus;

  printf("%i %s \n",status,outdol);*/
  status=ISDC_OK;
  status=ICgetNewestDOL(icmaster,
			alias,
			DS,
			"",
			ijd, // omg!!
		        outdol,status);
  saveStatus = (ISDC_OK == saveStatus) ? status : saveStatus;

  printf("%i %s \n",status,outdol);

  CommonRILclose(ISDC_OK);

  if(ISDC_OK != saveStatus) return -1; else return 0;
}

int main(int argc, char** argv){
  return icaccess_test(argc, argv);
}
