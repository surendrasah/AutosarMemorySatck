/*
 * NvM.c
 *
 *
 *      Author: Sahar
 */


/*****************************************************************************************/
/*                                   Include headers                                     */
/*****************************************************************************************/

#include "NvM.h"
#include "Det.h"

/*****************************************************************************************/
/*                                   Local types Definition                              */
/*****************************************************************************************/

/*Jobs type*/
typedef uint8 JobRequestType ;

/*Module state type*/
typedef uint8 ModuleStateType;

/* [SWS_NvM_00134]
 * Administrative block Type
 * The Administrative block shall be located in RAM
 * shall contain a block index which is used in association with Data set NV blocks.
 * error/status information of the corresponding NVRAM block shall be contained
 * use state information of the permanent RAM block (valid / Invalid)
 * The Administrative block shall be invisible for the application
 * shall use an error/status field to manage the error/status value of the last request
 */
typedef struct
{
    uint8 DataSetIndex ;
    uint8 BlockStatus ;
    uint8 PRAMStatus ;
    uint8 LastRequestStatus ;
}AdministrativeBlockType;

/*****************************************************************************************/
/*                                 JobRequestType  Local Macro Definition                */
/*****************************************************************************************/


/*Request jobs possible values*/
#define NO_JOB                      ((JobRequestType)0U)
#define READ_BLOCK                  ((JobRequestType)1U)
#define WRITE_BLOCK                 ((JobRequestType)2U)
#define RESTORE_BLOCK               ((JobRequestType)3U)
#define INVALIDATE_BLOCK            ((JobRequestType)4U)
#define READ_ALL                    ((JobRequestType)5U)
#define WRITE_ALL                   ((JobRequestType)6U)

/*Empty Queue size*/
#define EMPTY_QUEUE                 (0U)

/*Module States*/
#define MODULE_UNINITIALIZED        (0U)
#define INIT_DONE                   (1U)

/*****************************************************************************************/
/*                                   Local variables Definition                          */
/*****************************************************************************************/

/*Administrative block for each NVRAM block
 *The �Administrative Block� is a �Basic Storage Object�.
 *It resides in RAM. The  �Administrative Block� is a mandatory part of a �NVRAM Block�.
 */
static AdministrativeBlockType AdministrativeBlock[NUMBER_OF_NVM_BLOCKS];

/*Variable to save module state*/
static ModuleStateType ModuleState = MODULE_UNINITIALIZED ;

/*****************************************************************************************/
/*                                   extern variables                                    */
/*****************************************************************************************/

/*Blocks configurations */
extern NvMBlockDescriptorType NvMBlockDescriptor[NUMBER_OF_NVM_BLOCKS] ;

/*****************************************************************************************/
/*                                   Local Functions Prototypes                          */
/*****************************************************************************************/

//static void Init_Queue(void) ;


/*****************************************************************************************/
/*                                   Global Function Definition                          */
/*****************************************************************************************/


/****************************************************************************************/
/*    Function Name           : NvM_Init                                                */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00447                                           */
/*    Notes                   :[SWS_NvM_00881]The Configuration pointer ConfigPtr shall */
/*                             always have a NULL_PTR value.                            */
/****************************************************************************************/

void NvM_Init(const NvM_ConfigType* ConfigPtr )
{
    /*counter to loop blocks*/
    uint32 counter = 0 ;

    /*[SWS_NvM_00399] The function NvM_Init shall reset all internal variables,
     * e.g. the queues, request flags, state machines, to their initial values.
     * It shall signal �INIT DONE� internally, e.g. to enable job processing
     * and queue management. (SRS_BSW_00101, SRS_BSW_00406)
     */

    /*Initialize Queue*/
 //   Init_Queue() ;

    /*[SWS_NvM_00192]The function NvM_Init shall set the data set index
     * of all NVRAM blocks of type NVM_BLOCK_DATASET to zero.
     */
    for(counter = 0 ; counter < NUMBER_OF_NVM_BLOCKS ; counter++)
    {
        /*check if block type is data set*/
        if(NvMBlockDescriptor[counter].NvMBlockManagement == NVM_BLOCK_DATASET )
        {
            AdministrativeBlock[counter].DataSetIndex = 0 ;
        }
    }

    /*signal �INIT DONE� internally*/
    ModuleState = INIT_DONE ;
}


/****************************************************************************************/
/*    Function Name           : NvM_SetDataIndex                                        */
/*    Function Description    : Service for resetting all internal variables.           */
/*    Parameter in            : BlockId , DataIndex                                     */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00448                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_SetDataIndex(NvM_BlockIdType BlockId, uint8 DataIndex )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized
     *[SWS_NvM_00707] The NvM module�s environment shall have initialized
     *the NvM module before it calls the function NvM_SetDataIndex.
     */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_SET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /*Check if block management type not data set
     *[SWS_NvM_00264] For blocks with block management different from NVM_BLOCK_DATASET,
     *NvM_SetDataIndex shall return without any effect in production mode.
     *Further, E_NOT_OK shall be returned
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
        AdministrativeBlock[BlockId].DataSetIndex = DataIndex ;
    }

    return rtn_val ;
}


/****************************************************************************************/
/*    Function Name           : NvM_GetDataIndex                                        */
/*    Function Description    : Service for getting the currently set DataIndex of      */
/*                              a data set NVRAM block                                  */
/*    Parameter in            : BlockId                                                 */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : DataIndexPtr                                            */
/*    Return value            : none                                                    */
/*    Requirement             : SWS_NvM_00449                                           */
/*    Notes                   : none                                                    */
/****************************************************************************************/
Std_ReturnType NvM_GetDataIndex(NvM_BlockIdType BlockId, uint8* DataIndexPtr )
{
    /*Variable to save return value*/
    Std_ReturnType rtn_val = E_OK ;

    /*Check if Module not internally initialized */
    if(ModuleState != INIT_DONE)
    {
        #if(NVM_DEV_ERROR_DETECT == STD_ON)
            Det_ReportError(NVRAM_MANAGER_ID, NVRAM_MANAGER_INSTANCE, NVM_GET_DATAINDEX_API_ID ,NVM_E_NOT_INITIALIZED) ;
        #endif
        rtn_val = E_NOT_OK ;
    }
    /* [SWS_NvM_00265] For blocks with block management different from NVM_BLOCK_DATASET,
     * NvM_GetDataIndex shall set the index pointed by DataIndexPtr to zero. Further,
     * E_NOT_OK shall be returned.
     */
    else if(NvMBlockDescriptor[BlockId].NvMBlockManagement != NVM_BLOCK_DATASET )
    {
        rtn_val = E_NOT_OK ;
    }
    else
    {
        /*Save data index in the administrative block*/
         *DataIndexPtr = AdministrativeBlock[BlockId].DataSetIndex ;
    }

    return rtn_val ;
}


/*****************************************************************************************/
/*                                   Local Function Definition                           */
/*****************************************************************************************/



/****************************************************************************************/
/*    Function Name           : Init_Queue                                              */
/*    Function Description    : put queue in it's initialized state                     */
/*    Parameter in            : none                                                    */
/*    Parameter inout         : none                                                    */
/*    Parameter out           : none                                                    */
/*    Return value            : none                                                    */
/****************************************************************************************/


