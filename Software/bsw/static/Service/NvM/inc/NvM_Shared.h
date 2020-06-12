#ifndef NVM_SHARED_H_
#define NVM_SHARED_H_

/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/

#include "NvM.h"
#include "Det.h"

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

/*Module state type*/
typedef uint8 ModuleStateType ;

/*Permanent RAM Status type*/
typedef uint8 PRamStatusType ;

/* [SWS_NvM_00134]
 * Administrative block Type
 * The Administrative block shall be located in RAM
 * shall contain a block index which is used in association with Data set NV blocks.
 * error/status information of the corresponding NVRAM block shall be contained
 * use state information of the permanent RAM block (valid / Invalid)
 * The Administrative block shall be invisible for the application
 * shall use an error/status field to manage the error/status value of the last request
 * shall use an attribute field to manage the NV block write protection in order to
 * protect/unprotect a NV block data field
 */
typedef struct
{
    uint8 DataSetIndex ;
    NvM_RequestResultType BlockStatus ;
    PRamStatusType PRAMStatus ;
    uint32 PrevCRCVal ;

}AdministrativeBlockType ;

/* struct to hold the parameters for the job request*/
typedef struct
{
   uint8 ServiceId ;
   NvM_BlockIdType Block_Id ;
   void* RAM_Ptr ;

}Job_Parameters ;

/*struct to hold the indices which point to the queue
 *Used for queue implementation
 */
typedef struct{
  uint16 Head ;
  uint16 Tail ;
}Queue_Indices_Struct ;

/*****************************************************************************************/
/*                               Local Macros Definition                                 */
/*****************************************************************************************/

/*Size of the largest block in our module*/
#define LARGEST_BLOCK_SIZE          (100U)

/*Empty Queue size*/
#define EMPTY_QUEUE                 (0U)

/*Module States*/
#define MODULE_UNINITIALIZED        ((ModuleStateType)0U)
#define INIT_DONE                   ((ModuleStateType)1U)

/*Permanent Ram Status*/
#define INVALID                     ((PRamStatusType)0U)
#define VALID                       ((PRamStatusType)1U)


/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The �Administrative Block� is a �Basic Storage Object�.
 *It resides in RAM. The  �Administrative Block� is a mandatory part of a �NVRAM Block�.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];



/*****************************************************************************************/
/*                                   Local Functions Prototypes                          */
/*****************************************************************************************/

 Std_ReturnType Job_Enqueue(Job_Parameters Job) ;
 Std_ReturnType Job_Dequeue(Job_Parameters* Job) ;
 void Init_Queue(void) ;
 Std_ReturnType Search_Queue(NvM_BlockIdType BlockId) ;
 void NvM_Main_Write(void) ;

/*****************************************************************************************/
/*                                   external variables                                  */
/*****************************************************************************************/

/*Blocks configurations */
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS];

/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The �Administrative Block� is a �Basic Storage Object�.
 *It resides in RAM. The  �Administrative Block� is a mandatory part of a �NVRAM Block�.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];

/*[SWS_NvM_00195] The function NvM_ReadBlock shall take over the given parameters,
 *queue the read request in the job queue and return
 *Hint: this requirement is the same for all asynchronous single block requests.
 */
// standard job queue
static Job_Parameters Standard_Job_Queue[NVM_SIZE_STANDARD_JOB_QUEUE];
static Queue_Indices_Struct Stand_Queue_Indeces = {0, 0};

/*[SWS_NvM_00378]
 * In case of priority based job processing order,
 * the NvM module shall use two queues, one for immediate write jobs (crash data) another for all other jobs.
 */
// immediate job queue
#if (NVM_JOB_PRIORITIZATION == STD_ON)
  static Job_Parameters Immediate_Job_Queue[NVM_SIZE_IMMEDIATE_JOB_QUEUE];
  static Queue_Indices_Struct Immed_Queue_Indeces = {0, 0};
#endif

/*Variable to save module state*/
static ModuleStateType ModuleState = MODULE_UNINITIALIZED ;

/*Variable to hold the current job being processed by the main function*/
static Job_Parameters Current_Job ;

/*Temporary variable to store the NV data combined with CRC value */
static uint8 TempBuffer[LARGEST_BLOCK_SIZE] ;

/*****************************************************************************************/
/*                                   Queue Flags                                         */
/*****************************************************************************************/
static boolean Standard_Queue_Empty = TRUE;
static boolean Standard_Queue_FULL = FALSE;

#if (NVM_JOB_PRIORITIZATION == STD_ON)
  static boolean Immediate_Queue_Empty = TRUE;
  static boolean Immediate_Queue_FULL = FALSE;
#endif


#endif
