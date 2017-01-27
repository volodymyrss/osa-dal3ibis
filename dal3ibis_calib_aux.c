#include "dal3ibis.h"
#include "dal3ibis_calib.h"
#include "dal3hk.h"
#include "dal3aux.h"
#include "ril.h"
#include "dal3ibis_calib_aux.h"


int explain_error(int status, char *errmsg) {
  /* If an error occurred, write a message describing the final status. */
 //  char errmsg[DAL_MAX_STRING];
    int saveStatus=status;

    char *ISDC_ENV = getenv("ISDC_ENV");
  
    if('\0' != *ISDC_ENV) {
      char msgFileName[DAL_FILE_NAME_STRING];

      strncpy(msgFileName, ISDC_ENV, DAL_FILE_NAME_STRING - 1);
      if(strlen("/share/isdc_errors.txt") <
          DAL_FILE_NAME_STRING - 1 - strlen(msgFileName)) {
        FILE* msgFile;

        strcat(msgFileName, "/share/isdc_errors.txt");
  
        /* Read through file, looking for the error. */
        msgFile = fopen(msgFileName, "r");
        if(NULL != msgFile) {
          char match = 0;
          while(NULL != fgets(errmsg, DAL_MAX_STRING, msgFile)) {
            if(strtol(errmsg, NULL, 10) == saveStatus) {
              match = 1;
              break;
            }
          }
  
          if(match) {
            char* tmp;
            char* tmp2;

            /* Skip initial whitespace. */
            for(tmp = errmsg;
                (' ' == *tmp || '\t' == *tmp) && '\0' != *tmp; ++tmp);

            /* Skip the number itself, as this is printed below. */
            for(; ' ' != *tmp && '\t' != *tmp && '\0' != *tmp; ++tmp);

            /* Skip whitespace after the number. */
            for(; (' ' == *tmp || '\t' == *tmp) && '\0' != *tmp; ++tmp);

            /* Remove newline which fgets probably added. */
            for(tmp2 = tmp; '\n' != *tmp2 && '\0' != *tmp2; ++tmp2);
            *tmp2 = '\0';

            /* Print the message if it contains anything useful. */
            if('\0' != *tmp)
              ;//  RILlogMessage(NULL, Error_2, tmp);
            else
                RILlogMessage(NULL, Error_2, "unable to interpret error code!");
          }
          fclose(msgFile);
        }
      }
    } else {
        RILlogMessage(NULL, Error_2, "unable to interpret error code %i",status);
    }
}

int print_error(int status) {
    char message[DAL_MAX_STRING];

    explain_error(status, message);
    RILlogMessage(NULL, Error_2, "%s at %s:%d", message,__FILE__, __LINE__);
}

void report_try_error(int status, int fail_status, char message[], char filename[], int line) { 
    char error[DAL_MAX_STRING];

    explain_error(status,error);
    RILlogMessage(NULL,Error_1,"ERROR %s: %s ; %s:%i",message, error, filename, line);

    if (fail_status!=status) {
        char error[DAL_MAX_STRING];
        explain_error(fail_status,error);
        RILlogMessage(NULL,Error_1,"ERROR %s: new status: %s",message, error);
    }
}

