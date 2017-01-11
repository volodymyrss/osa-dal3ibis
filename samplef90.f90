!*****************************************************************************/
!*                                                                           */
!*                       INTEGRAL SCIENCE DATA CENTRE                        */
!*                                                                           */
!*                           DAL3  IBIS  LIBRARY                             */
!*                                                                           */
!*                            F90 SAMPLE PROGRAM                             */
!*                                                                           */
!*  Authors: Stéphane Paltani, Laurent Lerusse                               */
!*  Date:    23 December 2000                                                */
!*  Version: 3.3.0                                                           */
!*                                                                           */
!*  Revision history                                                         */
!*                                                                           */
!*  11.05.2000 V 3.0.0                                                       */
!*  ==================                                                       */
!*  6. Removed DAL3 subsets from the sample programs                         */
!*                                                                           */
!*  21.02.2000 V 2.2.5                                                       */
!*  ==================                                                       */
!* 1. The library uses now Makefile-2.0.1                                    */
!* 2. The library requires now DAL3GEN 2.4                                   */
!* 3. The library now installs "dal3ibis_f90_api_local.mod" for some non-SUN */
!*    F90 compilers                                                          */
!* 5. Implements the new ISDCLevel concept from TN020                        */
!*                                                                           */
!*  22.08.1999 V 2.1.0                                                       */
!*  ==================                                                       */
!* 1. The library now uses DAL 1.3 and Makefile-1.3.1                        */
!* 9. Changed "samplec" and "samplef90" so that the user can really "play"   */
!*    with it.                                                               */
!*                                                                           */
!*  18.06.1999 V 2.0.1                                                       */
!*  27.05.1999 V 2.0.0                                                       */
!*                                                                           */
!*  10.03.1999 V 1.0.0                                                       */
!*                                                                           */
!*****************************************************************************/

PROGRAM SAMPLEF90

!* Only the following includes are necessary.                                */

!* ISDC standard libraries                                                   */
USE ISDC

!* <ISDC> could have been replaced by <DAL_F90_API>, as this samplef90       */
!* program does not use other ISDC libraries; However a real ISDC software   */
!* must always include <ISDC>                                                */

!* DAL3GEN library must be included                                          */

USE DAL3GEN_F90_API

!* DAL3IBIS library                                                          */

USE DAL3IBIS_F90_API

IMPLICIT NONE

integer :: status,i,column,OBT_num
integer(KIND=8):: OBTstart(10),OBTend(10)
integer :: DAL_DS
integer :: type
integer :: sel,evType
integer :: num_events,ib_ev(ISGRI_EVTS:COMPTON_MULE)
integer (kind=4), dimension(:), allocatable :: buffer_int
integer (kind=8), dimension(:), allocatable :: buffer_obt
integer :: which_alloc
character(len=50) :: obj_name,param


status=ISDC_OK

!* The first argument of the program is the DOL of the group; e.g. :         */
!* og_l2.fits\[GROUPING\] (backslashes are needed to protect from            */
!* interpretation by the UNIX shell)                                         */

call getarg(1,obj_name)
print*,'OBJECT            : ',obj_name

!* The second argument is the type of the events given as an integer;        */
!* 0 for ISGRI events, 1 for PICsIT single events, and so on                 */

call getarg(2,param)
read(param,*) evType
write (6,'(A,i5)')'TYPE              : ',evType

!* The third argument is the event property given as an integer;             */
!* 0 for DELTA_TIME, 1 for RISE_TIME, and so on                              */

call getarg(3,param)
read(param,*) column
write (6,'(A,i5)')'COLUMN            : ',column

!* The fourth argument defines whether one wants to perform OBT selection    */
!* 1 means that the OBT selection is performed                               */

call getarg(4,param)
read(param,*) sel
write (6,'(A,i5)')'SELECTION         : ',sel
print*

!* Do we make an OBT selection ?                                             */
!* If yes, we use a hard-coded selection, made with 2 OBT ranges             */
!* If OBT_num is 0, DAL3IBISselectEvents will ignore the OBT ranges          */ 
if (sel==1 .or. sel==11) then
   OBT_num=2
else
   OBT_num=0;
end if

OBTstart(1)=762
OBTend(1)=2341
OBTstart(2)=25000
OBTend(2)=100170

!* We shall transfer all types into DAL_INT, as we do not have memory        */
!* limitations in this sample program                                        */
type=DAL_INT
  
!* Opening the Group ...                                                     */
print*,'DALobjectOpen'
status=DAL_OBJECT_OPEN(obj_name,DAL_DS,status);
write (6,'(A,i5)')'Status: ',status

!* Getting the total number of events ...                                    */
!* No selection is made with this function; it really returns every event    */
!* that i spresent in the group.                                             */
print*,'DAL3IBISshowAllEvents'
status=DAL3IBIS_SHOW_ALL_EVENTS(DAL_DS,ib_ev,status)
write (6,'(A,i5)')'ISGRI Events            : ',ib_ev(ISGRI_EVTS)
write (6,'(A,i5)')'PICsIT Single Events    : ',ib_ev(PICSIT_SGLE)
write (6,'(A,i5)')'PICsIT Multiple Events  : ',ib_ev(PICSIT_MULE)
write (6,'(A,i5)')'COMPTON Single Events   : ',ib_ev(COMPTON_SGLE)
write (6,'(A,i5)')'COMPTON Multiple Events : ',ib_ev(COMPTON_MULE)
write (6,'(A,i5)')'Status: ',status
print*

!* Event selection ...                                                      */
!* Again, as there is no memory issue here, all event parameters (ALL_PAR)  */
!* are copied into the selected events table                                */
  
print*,'DAL3IBISselectEvents'
if (sel>=10) then
   status=DAL3IBIS_SELECT_EVENTS(DAL_DS,evType,ALL_PAR,OBT_num,OBTstart,OBTend,'ISGRI_PHA<100',status)
else
   status=DAL3IBIS_SELECT_EVENTS(DAL_DS,evType,ALL_PAR,OBT_num,OBTstart,OBTend,'&',status)
end if
write (6,'(A,i5)')'Status: ',status
print*

!* Getting the number of selected events ...                                 */
print*,'DAL3IBISgetNumEvents'
status=DAL3IBIS_GET_NUM_EVENTS(num_events,status)
write (6,'(A,i5)')'Number of Events: ',num_events
write (6,'(A,i5)')'Status: ',status
print*


print*,'Reading all the events at once...'

!* A buffer is allocated for the requested properties. In F90, one cannot    */
!* allocate memory for one type and write another type; thus we must make    */
!* separate allocations                                                      */

if (column==OB_TIME) then
   which_alloc=1
   allocate(buffer_obt(1:num_events))
else
   which_alloc=2
   allocate(buffer_int(1:num_events))
end if

!* Getting the properties of the selected events ...                        */
!* The returned type (type) is set to DAL_INT. This paremeter is ignored    */
!* for the OB_TIME column. Thus the resulting type is translated to         */
!* DAL_INT, unless one reads the OB_TIME column, where INTEGER*8 is used    */

print*,'DAL3IBISgetEvents'
if (column==OB_TIME) then
   status=DAL3IBIS_GET_EVENTS(column,type,ADDROF(buffer_obt(1)),status)
else
   status=DAL3IBIS_GET_EVENTS(column,type,ADDROF(buffer_int(1)),status)
end if
write (6,'(A,i5)')'Status: ',status

!* Printing the result ...                                                   */
if (status==ISDC_OK) then
   if (column==OB_TIME) then
      do i=1,num_events
         write (6,'(A,i5,A,I10)')'Value ',i,' : ',buffer_obt(i)
      end do
    else
      do i=1,num_events
         write (6,'(A,i5,A,I10)')'Value ',i,' : ',buffer_int(i)
      end do
   end if
end if
print*

!* The intermediate buffer is not needed anymore.                            */
if (which_alloc==1) deallocate(buffer_obt)
if (which_alloc==2) deallocate(buffer_int)

!* Closing the selected events table ...                                     */
!* This step is not mandatory. It is useful if the application still         */
!* important ressources once the events have been processed.                 */

print*,'DAL3IBIScloseEvents'
status=DAL3IBIS_CLOSE_EVENTS(status)
write (6,'(A,i5)')'Status: ',status
print*
    
!* Closing the input data structures ...                                     */
print*,'DALobjectClose'

status=DAL_OBJECT_CLOSE(DAL_DS,DAL_SAVE,status)
write (6,'(A,i5)')'Status: ',status


END PROGRAM
