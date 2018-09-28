/*
 * -------------------------------------------
 *    CC3220 SDK - v0.10.00.00 
 * -------------------------------------------
 *
 *   Copyright (C) 2015 Texas Instruments Incorporated
 *
 *   All rights reserved. Property of Texas Instruments Incorporated.
 *   Restricted rights to use, duplicate or disclose this code are
 *   granted through contract.
 *
 *   The program may not be used without the written permission of
 *   Texas Instruments Incorporated or against the terms and conditions
 *   stipulated in the agreement under which this program has been supplied,
 *   and under no circumstances can it be used with non-TI connectivity device.
 *   
 */
//*******************************************************
//                  INCLUDES
//*******************************************************
/* Standard includes */
#include "FreeRTOS.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Board.h"
/* TI-DRIVERS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include <ti/drivers/I2C.h>
#include <ti/display/Display.h>

#include <time.h>
#include <sys/time.h>
#include <ti/drivers/Power.h>
#include <ti/drivers/net/wifi/simplelink.h>
#include "pthread.h"
#include <ti/utils/json/json.h>
#include "uart_term.h"
#include "task.h"


//*******************************************************
//                  DEFINES - CONSTS
//*******************************************************
#define APPLICATION_NAME                        "JSON"
#define APPLICATION_VERSION                     "1.0.0"
#define SL_STOP_TIMEOUT                         (200)
#define SPAWN_TASK_PRIORITY                     (9)
#define TASKSTACKSIZE                           (4096)
#define TEMPLATE_FILENAME                       "template1"
#define JSON_FILENAME                           "json1"

#define ASCI_0                                  48
#define ASCI_9                                  57
#define OBJ_BUFFER_SIZE                         6
#define CMD_BUFFER_SIZE                         100
#define SELECT_BUFFER_SIZE                      2
//*******************************************************
//                  STRUCTs
//*******************************************************

typedef struct
{
    char *fileBuffer;
} Json_Filename_t;



typedef enum
{
    JSON_CREATE_TEMPLATE                    = 0 ,
    JSON_CREATE_OBJECT                      = 1 ,
    JSON_PARSE                              = 2 ,
    JSON_GET_VALUE                          = 3 ,
    JSON_SET_VALUE                          = 4 ,
    JSON_GET_ARRAY_MEMBER_COUNT             = 5 ,
    JSON_BUILD                              = 6 ,
    JSON_DESTROY_TEMPLATE                   = 7 ,
    JSON_DESTROY_JSON_OBJECT                = 8
}json_action;

typedef enum
{
    INT32                                   = 0 ,
    STRING_RAW                              = 1 ,
    BOOLEAN                                 = 2
}json_value_t;
//*******************************************************
//                     GLOBAL VARIABLES
//*******************************************************
Json_Handle     templateHandle;
Json_Handle h;
Json_Handle     jsonObjHandle;
Json_Filename_t templateBuff;
Json_Filename_t jsonBuffer;
int16_t         templateSize;
uint16_t        objSize;

//*******************************************************
//                      LOCAL FUNCTION PROTOTYPES
//*******************************************************
int16_t readFile(Json_Filename_t  * pBufferFile, char *FileName);
void removeUnwantedChars(char * pBuf);
void validateForPrint(char *pBuf);
void createTemplate(void);
void createObject(void);
void parse(void);
void getValue(void);
void setValue(void);
void destroyJsonObject(void);
void destroyTemplate(void);
void build(void);
void getArrayMemberCount(void);


//void SimpleLinkNetAppRequestMemFreeEventHandler (uint8_t *buffer)
//{
  // do nothing...
//}

//void SimpleLinkNetAppRequestEventHandler (SlNetAppRequest_t *pNetAppRequest, SlNetAppResponse_t *pNetAppResponse)
//{
  // do nothing...
//}

//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
//void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
//{
    // do nothing...
//}

//*****************************************************************************
//
//! \brief The Function Handles the Fatal errors
//!
//! \param[in]  slFatalErrorEvent - Pointer to Fatal Error Event info
//!
//! \return None
//!
//*****************************************************************************
//void SimpleLinkFatalErrorEventHandler(SlDeviceFatal_t *slFatalErrorEvent)
//{
    // do nothing...
//}

//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
//void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
//{
    // do nothing...
//}

//*****************************************************************************
//
//! \brief This function handles HTTP server events
//!
//! \param[in]  pServerEvent - Contains the relevant event information
//! \param[in]    pServerResponse - Should be filled by the user with the
//!                                      relevant response information
//!
//! \return None
//!
//****************************************************************************
//void SimpleLinkHttpServerEventHandler(SlNetAppHttpServerEvent_t *pHttpEvent,
        //                            SlNetAppHttpServerResponse_t *pHttpResponse)
//{
    // Unused in this application
//}

//*****************************************************************************
//
//! \brief This function handles General Events
//!
//! \param[in]     pDevEvent - Pointer to General Event Info
//!
//! \return None
//!
//*****************************************************************************
//void SimpleLinkGeneralEventHandler(SlDeviceEvent_t *pDevEvent)
//{
    // do nothing...
//}

//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
//void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
//{
    // do nothing...

//}

//*****************************************************************************
//
//! \brief This function reads a file from the file system and stores it
//!        in a buffer.
//! \param[out]    pBufferFile - pointer to a buffer which will be filled with a file
//! \param[in]     FileName    - pointer to filename which needs to be
//!                              read.
//!
//! \return on success number of bytes read from file system
//!         on failure error code.
//****************************************************************************
int16_t readFile(Json_Filename_t *pBufferFile, char *FileName)
{
    int32_t FileHdl = 0;
    int32_t Status = 0;
    SlFsFileInfo_t *FsFileInfo;
    uint32_t FileSize;
    int16_t retVal = 0;

    FsFileInfo = malloc(sizeof(SlFsFileInfo_t));

    if (FsFileInfo)
    {
        /* Get the file size */
        Status = sl_FsGetInfo((unsigned char *)FileName,0,FsFileInfo);
        if (Status <0)
        {
            UART_PRINT("FS - Couldn't get info on file. error status %d \n\r",Status);
            return Status;
        }
        FileSize = FsFileInfo->Len;
        free(FsFileInfo);

        FileHdl = sl_FsOpen((unsigned char *)FileName, SL_FS_READ,0);
        if( FileHdl < 0 )
        {
            UART_PRINT("FS - Couldn't open file. error status %d \n\r",FileHdl);
            return FileHdl;
        }
        else
        {
            pBufferFile->fileBuffer = malloc(FileSize+1);
            if (pBufferFile->fileBuffer != NULL)
            {
                memset(pBufferFile->fileBuffer,'\0',FileSize+1);
                /* Read the entire file */
                Status = sl_FsRead(FileHdl , 0, (unsigned char *)pBufferFile->fileBuffer, FileSize);
                if(Status < 0)
                {
                    UART_PRINT("FS - Couldn't read file. error status %d \n\r",Status);
                    return Status;
                }
                retVal = sl_FsClose(FileHdl,NULL,NULL,0);
                if (retVal < 0)
                {
                    UART_PRINT("FS - Couldn't close file. error status %d \n\r",retVal);
                    return retVal;
                }
                return Status;
            }
            else
            {
                UART_PRINT("Couldn't allocate memory \n\r");
                return JSON_RC__MEMORY_ALLOCATION_ERROR;
            }
         }
    }
    else
    {
        UART_PRINT("Couldn't allocate memory \n\r");
        return JSON_RC__MEMORY_ALLOCATION_ERROR;
    }
}
//*****************************************************************************
//
//! \brief This function removes from the buffer '\n' and ' '
//!
//! \param[inout]      pBuf pointer to a buffer
//!
//! \return none
//****************************************************************************
void removeUnwantedChars(char *pBuf)
{
    char * str_tmp;
    uint16_t i=0,j=0;
    str_tmp = pBuf;

    for(i = 0; str_tmp[i] != '\0'; ++i)
    {
        while ( (!(str_tmp[i] != '\n' ) || !(str_tmp[i] != ' ' ))&& (str_tmp[i] != '\0') )
       // while  (!(str_tmp[i] != '\n' ))
        {
            for(j = i; str_tmp[j] != '\0'; ++j)
            {
                str_tmp[j] = str_tmp[j+1];
            }
            str_tmp[j] = '\0';
        }
    }
}
//*****************************************************************************
//
//! \brief This function validates and changes the Json text into readable
//!        Json convention
//! \param[inout]      pBuf pointer to a buffer
//!
//! \return none
//****************************************************************************
void validateForPrint(char *pBuf)
{
    char * str_tmp = pBuf;
    char * pre = NULL;
    uint16_t i=0,j=0;
    int16_t ident = 0;

    for(i = 0; str_tmp[i] != '\0'; ++i)
    {
        if ((str_tmp[i] == ']')|| (str_tmp[i] == '}')) ident--;
        if(pre != NULL)
        {
            if ((*pre == '[' )|| (*pre == '{' )|| (*pre == ',' )|| (str_tmp[i] == ']' )||( str_tmp[i] == '}'))
            {
                UART_PRINT("\n\r");
                for (j=0;j<ident; j++ )
                    UART_PRINT(" ");
            }
        }
        UART_PRINT("%c",str_tmp[i]);
        if ((str_tmp[i] == '[')||(str_tmp[i] == '{')) ident++;
        pre = &str_tmp[i];
    }
}

void createTemplate(void)
{
    int16_t retVal;


         char *templatestr = "{"
                    "\"ID\":int32,"
                    "\"ID_measure\":int32,"
                 "\"Data\":string,"
                 "\"timestamp\":string}";
   /* char *templatestr = "{"
              "\"ID\":int32,"
              "\"ID_measure\":int32,"
             "\"Data\":string,"
            "\"Timestamp\":string}";*/







    retVal = Json_createTemplate(&templateHandle, templatestr, strlen(templatestr));
    if (retVal<0)
    {
        UART_PRINT("Error: %d, Couldn't create template \n\r",retVal);
    }
    else
    {
        UART_PRINT("Template object created successfully. \n\n\r");
    }
}



/*char *struct2str (templatestr tp)
{
    // get lenght of string required to hold struct values

    size_t len =0;
    len = snprintf (NULL, len , "%d,%d,%s,%s", tp.ID, tp.ID_measure, tp.Data, tp.Timestamp);

    // allocate/validate string to hold all values (+1 to null-terminate)
    char *tpstr = calloc (1, sizeof *tpstr * len + 1);


    return tpstr;
}*/
void createObject(void)
{
    int16_t retVal;

    //char objSizeBuffer[OBJ_BUFFER_SIZE];
    /* initialize object size buffer */
  //  memset(objSizeBuffer,'\0',OBJ_BUFFER_SIZE);
   // UART_PRINT("Please enter object size in bytes [0 - default size ]\n\r");
  //  retVal = GetCmd((char *)objSizeBuffer, OBJ_BUFFER_SIZE);
  /*  if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    /* convert object size received from uart into integer */
  //  objSize = atoi(objSizeBuffer);



    retVal = Json_createObject(&h,templateHandle,1024);
    if (retVal < 0)
    {
        UART_PRINT("Error: %d  , Couldn't create json object \n\r", retVal);
    }
    else
    {
        UART_PRINT("Json object created successfully. \n\n\r");
    }
}
void parse(void)
{
    int16_t retVal;


       char *jsonBuf = "{\"ID\":\"1\","
               "\"ID_measure\":1,"
     "\"Data\":30}";



       retVal = Json_parse(h, jsonBuf, strlen(jsonBuf));

  //  retVal = Json_parse(jsonObjHandle,jsonBuffer.fileBuffer,strlen(jsonBuffer.fileBuffer));
    if (retVal<0)
    {
        UART_PRINT("Error: %d  , Couldn't parse the Json file \n\r", retVal);
    }
    else
    {
        UART_PRINT("Json was parsed successfully \n\n\r");
    }
}
void getValue(void)
{
    char     getBuffer[CMD_BUFFER_SIZE];
    char     keyBuffer[CMD_BUFFER_SIZE];
    char     valueType[SELECT_BUFFER_SIZE];
    uint16_t valueSize = CMD_BUFFER_SIZE;
    int16_t  retVal;
    int32_t  numValue;
    /* initialize key and set buffers to null terminated chars */
    memset(keyBuffer,'\0',CMD_BUFFER_SIZE);
    memset(getBuffer,'\0',CMD_BUFFER_SIZE);

    UART_PRINT("Please enter a key to the value\n\r");
    retVal = GetCmd((char *)keyBuffer, CMD_BUFFER_SIZE);
    if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    UART_PRINT("Please choose value type [0 - Int32, 1 - String / RAW , 2 - Boolean ]. \n\r");
    retVal = GetCmd((char *)valueType, SELECT_BUFFER_SIZE);
    if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    UART_PRINT("Processing the request ... \n\r");
    if (atoi(valueType) == INT32)
    {
        retVal = Json_getValue(jsonObjHandle,keyBuffer,&numValue,&valueSize);
        if (retVal == JSON_RC__VALUE_IS_NULL)
        {
            UART_PRINT("The value is null\n\r");
            return;
        }
        else if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't get the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value is : %d \n\r",numValue);
    }
    else if (atoi(valueType) == STRING_RAW)
    {
        retVal = Json_getValue(jsonObjHandle,keyBuffer,getBuffer,&valueSize);
        if (retVal == JSON_RC__VALUE_IS_NULL)
        {
            UART_PRINT("The value is null\n\r");
            return;
        }
        else if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't get the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value is : %s \n\r",getBuffer);
    }
    else if (atoi(valueType) == BOOLEAN)
    {
        retVal = Json_getValue(jsonObjHandle,keyBuffer,&numValue,&valueSize);
        if (retVal == JSON_RC__VALUE_IS_NULL)
        {
            UART_PRINT("The value is null\n\r");
            return;
        }
        else if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't get the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value is : %s \n\r",((uint8_t)numValue == 0)?"false":"true");
    }
    else
    {
        UART_PRINT("Invalid value type.\n\r");
    }
}
void setValue(void)
{
    char     setBuffer[CMD_BUFFER_SIZE];
    char     keyBuffer[CMD_BUFFER_SIZE];
   // char     valueType[SELECT_BUFFER_SIZE];
 //   uint16_t valueSize = CMD_BUFFER_SIZE;
 //   int16_t  retVal;
   // int32_t  numValue;
    /* initialize key and set buffers to null terminated chars */
    memset(keyBuffer,'\0',100);
    memset(setBuffer,'\0',100);
    uint32_t ret2;
     char *key =  "\"ID\"";
     char *key1 = "\"ID_measure\"";
     char *key2 = "\"Data\"";
     char *key3 = "\"Timestamp\"";
      uint32_t value = 1;
      uint32_t valueSize = sizeof(value);
      uint32_t value1 = 2;
      uint32_t valueSize1 = sizeof(value1);
      char value2[] = "28 C";
      uint16_t valueSize2 = sizeof(value2);
      char value3[] = "timestamp" ;
      uint16_t valueSize3 = sizeof(value3);


      char builtBuff[100];
     uint16_t builtBuffSize = 100;
      ret2 = Json_setValue(h, key, value, valueSize);
      ret2 = Json_setValue(h, key1, value1, valueSize1);
      ret2 = Json_setValue(h, key2, value2, valueSize2);
      ret2 = Json_setValue(h, key3, value3, valueSize3);
      ret2 = Json_build(h, builtBuff, &builtBuffSize);

    }
  /*  UART_PRINT("\n");
    UART_PRINT("Please choose value type [0 - Int32, 1 - String / RAW , 2 - Boolean ]. \n\r");
    retVal = GetCmd((char *)valueType, SELECT_BUFFER_SIZE);
    if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    UART_PRINT("Please enter a value to set \n\r");
    retVal = GetCmd((char *)setBuffer, CMD_BUFFER_SIZE);
    if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    UART_PRINT("Processing the request ... \n\r");
    if (atoi(valueType)==INT32)
    {
        numValue = atoi(setBuffer);
        valueSize = sizeof(numValue);
        retVal = Json_setValue(jsonObjHandle, keyBuffer,&numValue,valueSize);
        if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't set the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value has been set: %d\n\r",numValue);
    }
    else if (atoi(valueType)==STRING_RAW)
    {
        valueSize = strlen(setBuffer);
        retVal = Json_setValue(jsonObjHandle, keyBuffer,setBuffer,valueSize);
        if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't set the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value has been set: %s\n\r",setBuffer);
    }
    else if (atoi(valueType)==BOOLEAN)
    {
        numValue = atoi(setBuffer);
        valueSize = sizeof(uint16_t);*/
        /* verify that the value is valid boolean */
     /*   if ((numValue != 0) && (numValue != 1))
        {
            UART_PRINT("Wrong boolean value. 0 - false , 1 - true \n\r ");
            return;
        }
        retVal = Json_setValue(jsonObjHandle,keyBuffer,&numValue,valueSize);
        if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't set the data \n\r",retVal);
            return;
        }
        UART_PRINT("The value has been set: %s\n\r",((uint8_t)numValue == 0)?"false":"true");
    }
    else
    {
        UART_PRINT("Invalid value type.\n\r");
    }
}*/
void getArrayMemberCount(void)
{
    char     keyBuffer[CMD_BUFFER_SIZE];
    int16_t  retVal;
    /* Initialize the key buffer to null terminated chars */
    memset(keyBuffer,'\0',CMD_BUFFER_SIZE);
    UART_PRINT("Please enter a key to the array? \n\r");
    retVal = GetCmd((char *)keyBuffer, CMD_BUFFER_SIZE);
    if (retVal <= 0)
    {
        UART_PRINT("Buffer length exceeded\n\r");
        return;
    }
    UART_PRINT("\n");
    UART_PRINT("Processing the request ... \n\r");
    retVal = Json_getArrayMembersCount(jsonObjHandle,keyBuffer);
    if (retVal<0)
    {
        UART_PRINT("Error: %d  , Couldn't get array member count.  \n\r", retVal);
        return;
    }

    UART_PRINT("Number of members in array %d \r\n",retVal);
}
void build(void)
{
    char        *builtText;
    int16_t     retVal;
    uint16_t    builtTextSize;
    /* set object size to default size if zero was chosen */
    builtTextSize = (objSize==0)?JSON_DEFAULT_SIZE:objSize;
    /* creates buffer for building the json */
    builtText = (char *)malloc(builtTextSize);
    if (builtText)
    {
        retVal = Json_build(h,builtText,&builtTextSize);
       // retVal = Json_build(jsonObjHandle,builtText,&builtTextSize);
        if (retVal<0)
        {
            UART_PRINT("Error: %d  , Couldn't build the json.  \n\r", retVal);
            free(builtText);
            return;
        }
        removeUnwantedChars(builtText);
        /* prints json according to json convention */
        validateForPrint(builtText);
        free(builtText);
    }
    else
    {
        UART_PRINT("Couldn't allocate memory \n\r");
    }
}
void destroyTemplate(void)
{
    int16_t retVal;

    retVal = Json_destroyTemplate(h);
    if (retVal<0)
    {
        UART_PRINT("Error: %d  , Couldn't destroy the template.  \n\r", retVal);
        return;
    }
    UART_PRINT("Template was destroyed successfully.  \n\r", retVal);
}
void destroyJsonObject(void)
{
    int16_t retVal;

    retVal = Json_destroyObject(h);
    if (retVal<0)
    {
        UART_PRINT("Error: %d  , Couldn't destroy the json.  \n\r", retVal);
        return;
    }
    UART_PRINT("Json was destroyed successfully.  \n\r", retVal);
}
//*****************************************************************************
//
//! \brief Display Application Banner
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
int32_t displayBanner(void)
{
    int32_t     status = -1;
    uint8_t     macAddress[SL_MAC_ADDR_LEN];
    uint16_t    macAddressLen = SL_MAC_ADDR_LEN;
    uint16_t    configSize = 0;
    uint8_t     configOpt = SL_DEVICE_GENERAL_VERSION;
    SlDeviceVersion_t ver = {0};

    configSize = sizeof(SlDeviceVersion_t);
    status = sl_Start(0, 0, 0);

    /* Print device version info. */
    status = sl_DeviceGet(SL_DEVICE_GENERAL, &configOpt, &configSize, (uint8_t*)(&ver));


    /* Print device Mac address */
    status = sl_NetCfgGet(SL_NETCFG_MAC_ADDRESS_GET, 0, &macAddressLen, &macAddress[0]);

    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\t    %s Example Ver: %s\n\r",APPLICATION_NAME, APPLICATION_VERSION);
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\t CHIP: 0x%x",ver.ChipId);
    UART_PRINT("\n\r");
    UART_PRINT("\t MAC:  %d.%d.%d.%d",ver.FwVersion[0],ver.FwVersion[1],ver.FwVersion[2],ver.FwVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t PHY:  %d.%d.%d.%d",ver.PhyVersion[0],ver.PhyVersion[1],ver.PhyVersion[2],ver.PhyVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t NWP:  %d.%d.%d.%d",ver.NwpVersion[0],ver.NwpVersion[1],ver.NwpVersion[2],ver.NwpVersion[3]);
    UART_PRINT("\n\r");
    UART_PRINT("\t ROM:  %d",ver.RomVersion);
    UART_PRINT("\n\r");
    UART_PRINT("\t HOST: %s", SL_DRIVER_VERSION);
    UART_PRINT("\n\r");
    UART_PRINT("\t MAC address: %02x:%02x:%02x:%02x:%02x:%02x", macAddress[0], macAddress[1], macAddress[2], macAddress[3], macAddress[4], macAddress[5]);
    UART_PRINT("\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\t ==============================================\n\r");
    UART_PRINT("\n\r");
    UART_PRINT("\n\r");
    status = sl_Stop(SL_STOP_TIMEOUT);


    return status;
}
//*****************************************************************************
//
//!  \brief this function is the main function which is running and
//!         receiving actions from the user.
//!
//!  \param  none
//!  \return none
//!
//
//*****************************************************************************
void readCmd(void)
{
    char        menuInput[SELECT_BUFFER_SIZE];
    int16_t     retVal = 0;
	json_action actionSelect;

	while(1)
    {
        UART_PRINT("JSON Menu: \n\n\r");
        UART_PRINT("=======================\n\n\r");
        UART_PRINT("0.Create template object \n\n\r");
        UART_PRINT("1.Create json object \n\n\r");
        UART_PRINT("2.Parse \n\n\r");
        UART_PRINT("3.Get value \n\n\r");
		UART_PRINT("4.Set value \n\n\r");
		UART_PRINT("5.Get array member count \n\n\r");
		UART_PRINT("6.Build the json \n\n\r");
        UART_PRINT("7.Destroy template object \n\n\r");
        UART_PRINT("8.Destroy json object \n\n\r");
		UART_PRINT("Choose number:  \n\r");

        retVal = GetCmd((char *)menuInput, SELECT_BUFFER_SIZE);
        if (retVal <= 0)
        {
            UART_PRINT("\n\r");
            continue;
        }
        UART_PRINT("\n\r");
        if ((menuInput[0] < ASCI_0) || (menuInput[0] > ASCI_9))
        {
            UART_PRINT("Invalid action chosen...  \n\r");
            UART_PRINT("\n\r");
            UART_PRINT("Press any key to continue.... \r\n");
            getch();
            continue;
        }
        /* convert menu input received from uart into integer */
        actionSelect = (json_action)atoi(menuInput);
        switch(actionSelect)
        {
            case JSON_CREATE_TEMPLATE:
                createTemplate();
                break;
            case JSON_CREATE_OBJECT:
                createObject();
                break;
            case JSON_PARSE:
                parse();
                break;
            case JSON_GET_VALUE:
                getValue();
                break;
			case JSON_SET_VALUE:
			    setValue();
                break;
			case JSON_GET_ARRAY_MEMBER_COUNT:
			    getArrayMemberCount();
                break;
			case JSON_BUILD:
			    build();
                break;
			case JSON_DESTROY_TEMPLATE:
			    destroyTemplate();
                break;
			case JSON_DESTROY_JSON_OBJECT:
			    destroyJsonObject();
                break;
            default: UART_PRINT("Invalid action chosen...  \n\r");
                break;
        }
        UART_PRINT("\n\r");
        UART_PRINT("Press any key to continue.... \r\n");
        getch();
    }

}


//*****************************************************************************
//
//! mainThread
//!
//!  \param  pvParameters
//!
//!  \return none
//!
//!  \brief Task handler
//
//*****************************************************************************


//static Display_Handle display;
#define TMP006_DIE_TEMP     0x0001
int32_t retVal;

char        *builtText;
uint16_t    builtTextSize;
uint16_t    id =0;
extern pthread_cond_t condition ; /* Création de la condition */
extern pthread_mutex_t mutex ; /* Création du mutex */
extern int d;
//void json_app()
void *threadjson_app(void *arg)
{
    int xDelay = 15000 / portTICK_PERIOD_MS;
char str[20];
char strid[20];
char str1[5];
char str2[5];
char* Type;
  /*  int32_t             status;
    pthread_attr_t      pAttrs_spawn;
    struct sched_param  priParam;*/
	//int16_t 			retVal = 0;
	int32_t             mode = -1;
	//pthread_t           spawn_thread = (pthread_t)NULL;

	  //  uint16_t    id_measure;
	    uint16_t     temperature;
	    time_t t;
	    uint16_t ret3;
	    uint16_t ret4;
	    uint8_t         txBuffer[1];
	    uint8_t         rxBuffer[2];
	    I2C_Handle      i2c;
	    I2C_Params      i2cParams;
	    I2C_Transaction i2cTransaction;
	//   int32_t             retVal;
	    int32_t             ret;
	    int32_t             ret1;
	    uint32_t ret2;
	  //  int32_t     ret3;
	         char *key =  "\"ID\"";
	         char *key1 = "\"Type\"";
	         char *key2 = "\"Flag\"";
	         char *key3 = "\"Checksum\"";
	         char *key4 = "\"Data\"";
	       //  char *key3 = "\"Time\"";
	         //uint32_t value;
	        // char *value;
	        // uint32_t valueSize = 24;
	        /* uint32_t value;
	             uint32_t valueSize = sizeof(value);

	         uint32_t value1;
	               uint32_t valueSize1 = sizeof(value1);
	               uint32_t value2;
	                  uint32_t valueSize2 = sizeof(value2);
	                   uint32_t value3;
	                  uint32_t valueSize3 = sizeof(value3);*/
	             char* value4;
	             uint32_t valueSize4 = 10;
	             char* value;
	             uint32_t valueSize = sizeof(value);
	             char* value1;
	             uint32_t valueSize1 = sizeof(value1);
	             char* value2;
	             uint32_t valueSize2 = sizeof(value2);
	             char* value3;
	             uint32_t valueSize3 = sizeof(value3);





   // UART_init();

    /* Configure the UART */
   // InitTerm();



    if (mode < 0)
    {
        sl_Stop(SL_STOP_TIMEOUT );
        UART_PRINT("[Common] CC3220 NWP reset request\r\n");

        /* Reset the MCU in order to test the bundle */
      //  sl_Start(0, 0, 0);
    }
while(1){
if (d==1){
    pthread_mutex_lock (&mutex);
 //   pthread_cond_signal (&condition);
   // pthread_mutex_unlock (&mutex);

  //  pthread_mutex_lock (&mutex);
         //  pthread_cond_signal (&condition);
   // Display_init();
    //   GPIO_init();
       I2C_init();


   //    UART_PRINT("Starting the i2ctmp006 example\n");
    I2C_Params_init(&i2cParams);
      i2cParams.bitRate = I2C_400kHz;
      i2c = I2C_open(Board_I2C_TMP, &i2cParams);
      if (i2c == NULL) {
          UART_PRINT("Error Initializing I2C\n");
          while (1);
      }
      else {
      //    UART_PRINT("I2C Initialized!\n");
      }

    txBuffer[0] = TMP006_DIE_TEMP;

       i2cTransaction.slaveAddress = Board_TMP_ADDR;
       i2cTransaction.writeBuf = txBuffer;
       i2cTransaction.writeCount = 1;
       i2cTransaction.readBuf = rxBuffer;
       i2cTransaction.readCount = 2;


     // Initialize and creation of the template

       char *templatestr = "{"
                                 "\"ID\":string,"
                                 "\"Type\":string,"
                      "\"Flag\":string,"
                      "\"Checksum\":string,"
                              "\"Data\":string}";
                   //   "\"time\":string}";



       retVal = Json_createTemplate(&templateHandle, templatestr, strlen(templatestr));
     //  UART_PRINT("##");
       // Take 20 samples and print them out onto the console

         Type = "0";
      // Type = "temperature";
           ret1 = Json_createObject(&h,templateHandle,512);

           if (I2C_transfer(i2c, &i2cTransaction)) {

               /* Extract degrees C from the received data; see TMP102 datasheet */
               temperature = (rxBuffer[0] << 6) | (rxBuffer[1] >> 2);

               /*
                * If the MSB is set '1', then we have a 2's complement
                * negative value which needs to be sign extended
                */
               if (rxBuffer[0] & 0x80) {
                   temperature |= 0xF000; ///S&M or CA2

               }
              /*
               * For simplicity, divide the temperature value by 32 to get rid of
               * the decimal precision; see TI's TMP006 datasheet
               */
             //  temperature /= 32;     This is the line to get temperature

               temperature = 8;


          /* int id = 0;
           int Flag = 3;
           int checksum = 7;*/
               /*  sprintf(strch, "%d",checksum );
               sprintf(strflag, "%d",Flag );

               sprintf(strtype, "%d",Type );*/
               sprintf(strid, "%d",id);
             sprintf(str, "%d", temperature);



                         /* int temp2 = temperature * 4;
                          temp2 = 0x2;
                          int temp3 = temperature * 8;
                          temp3 = 0x4;*/




                        //  uint16_t ch = temperature|temp2|temp3;


             // convert other value into string
              /*         sprintf(str1, "%d", temp2);
                         sprintf(str2, "%d", temp3);*/


             // Print real time
          /*   time_t t = time(NULL);
                printf("%s\n", ctime(&t));*/

// concatenate all measure into data
             /*   strcat(str,",");
                strcat(str,str1);
                strcat(str,",");
                strcat(str,str2);*/


             // set strings values
               /* value=strid;
                value1 = strtype;
                value2 = strflag;
                value3 = strch;*/


             // Set values
                value =  strid;
               value1 = Type;
                value2 = "1";
               // value3 = str;
                value3 = "8";
                //value3 = "4";
               // value3 = "8";
                value4 = str;


                // use the parse function if you want to
                 /*   char *jsonBuf = "{\"Timestamp\":\"0\","
                             "\"ID_measure\":0,"
                              "\"Data\":\"29\"}";
                       ret = Json_parse(h, jsonBuf, strlen(jsonBuf));*/


                //Set values into the json

                       ret2 = Json_setValue(h, key, value, valueSize);
                     ret2 = Json_setValue(h, key1, value1, valueSize1);
                     ret2 = Json_setValue(h, key2, value2, valueSize2);
                     ret2 = Json_setValue(h, key3, value3, valueSize3);
                     ret2 = Json_setValue(h, key4, value4, valueSize4);


                     // Build the json

                    builtTextSize = (objSize==0)?JSON_DEFAULT_SIZE:objSize;
                        /* creates buffer for building the json */
                        builtText = (char *)malloc(builtTextSize);
                        if (builtText)
                        {
                            retVal = Json_build(h,builtText,&builtTextSize);


                            if (retVal<0)
                            {
                                UART_PRINT("Error: %d  , Couldn't build the json.  \n\r", retVal);
                                free(builtText);
                               // return;
                            }


                           removeUnwantedChars(builtText);
                            /* prints json according to json convention */
                         //   validateForPrint(builtText);
                           // free(builtText);

                            // You can destroy template and object at the end

                            /* ret3 = Json_destroyObject(h);
                               ret4  = Json_destroyTemplate(templateHandle);*/

           }


           /* Sleep for 1 second */

         //  sleep(1);
       }

       /* Deinitialized I2C */

       I2C_close(i2c);
      // UART_PRINT("I2C closed!\n");
      pthread_mutex_unlock (&mutex); /* On déverrouille le mutex */
     //  sleep(30);
       vTaskDelay( xDelay );


}
    }}

