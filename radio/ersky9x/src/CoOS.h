/**
 *******************************************************************************
 * @file       ccrtos.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      API header file of CooCox CoOS.
 * @details    This file including all API functions's declare of CooCox CoOS.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

#ifndef _CCRTOS_H
#define _CCRTOS_H
#include "OsConfig.h"
/*---------------------------- Type Define  ----------------------------------*/
typedef signed   char      S8;              
typedef unsigned char      U8;	
typedef short              S16;
typedef unsigned short     U16;
typedef int                S32;
typedef unsigned int       U32;
typedef long long          S64;
typedef unsigned long long U64;
typedef unsigned char      BIT;
typedef unsigned char      BOOL;
typedef unsigned int       OS_STK;
typedef U8                 OS_TID;
typedef U8                 OS_TCID;
typedef U8                 OS_MutexID;
typedef U8                 OS_EventID;
typedef U8                 OS_FlagID;
typedef U8                 OS_MMID;
typedef U8                 StatusType;
typedef U16                OS_VER;
typedef void               (*FUNCPtr)(void*);
typedef void               (*vFUNCPtr)(void);


/*---------------------------- Constant Define -------------------------------*/
#ifndef NULL
#define NULL          ((void *)0)
#endif

#ifndef FALSE
#define FALSE         (0)
#endif

#ifndef TRUE
#define TRUE          (1)
#endif


/*---------------------------- Error Codes   ---------------------------------*/
#define E_CREATE_FAIL         (StatusType)-1
#define E_OK                  (StatusType)0
#define E_INVALID_ID          (StatusType)1
#define E_INVALID_PARAMETER   (StatusType)2
#define E_CALL                (StatusType)3
#define E_TASK_WAITING        (StatusType)4
#define E_TIMEOUT             (StatusType)5
#define E_SEM_FULL            (StatusType)6
#define E_MBOX_FULL           (StatusType)7
#define E_QUEUE_FULL          (StatusType)8
#define E_SEM_EMPTY           (StatusType)9
#define E_MBOX_EMPTY          (StatusType)10
#define E_QUEUE_EMPTY         (StatusType)11
#define E_FLAG_NOT_READY      (StatusType)12
#define E_ALREADY_IN_WAITING  (StatusType)13
#define E_TASK_NOT_WAITING    (StatusType)14
#define E_TASK_WAIT_OTHER     (StatusType)15
#define E_EXCEED_MAX_NUM      (StatusType)16
#define E_NOT_IN_DELAY_LIST   (StatusType)17
#define E_SEV_REQ_FULL        (StatusType)18	
#define E_NOT_FREE_ALL        (StatusType)19	
#define E_PROTECTED_TASK      (StatusType)20 
#define E_OS_IN_LOCK          (StatusType)21												


/*---------------------------- Wait Opreation type  --------------------------*/
#define OPT_WAIT_ALL          0         /*!< Wait for all flags.              */
#define OPT_WAIT_ANY          1         /*!< Wait for any one of flags.       */
#define OPT_WAIT_ONE          2         /*!< Waot for one flag.               */	


/*---------------------------- Delete Opreation type  ------------------------*/
#define OPT_DEL_NO_PEND       0         /*!< Delete when no task waitting for */
#define OPT_DEL_ANYWAY        1         /*!< Delete always.                   */


/*---------------------------- Timer Types  ----------------------------------*/
#if CFG_TMR_EN >0
#define TMR_TYPE_ONE_SHOT     0         /*!< Timer counter type: One-shot     */
#define TMR_TYPE_PERIODIC     1         /*!< Timer counter type: Periodic     */
#endif


/*---------------------------- Event Control ---------------------------------*/
#if CFG_EVENT_EN >0
#define EVENT_SORT_TYPE_FIFO  (U8)0x01  /*!< Insert a event by FIFO           */
#define EVENT_SORT_TYPE_PRIO  (U8)0x02  /*!< Insert a event by prio           */
#endif


/*---------------------------- Function declare-------------------------------*/

/* Implement in file "core.c"      */
extern "C" void    CoInitOS(void);          /*!< Initialize OS                    */
extern "C" void    CoStartOS(void);         /*!< Start multitask                  */
extern "C" void    CoEnterISR(void);        /*!< Enter a ISR                      */
extern "C" void    CoExitISR(void);         /*!< Exit a ISR                       */
extern "C" void    CoSchedLock(void);
extern "C" void    CoSchedUnlock(void);
extern "C" OS_VER  CoGetOSVersion(void);    /*!< Get OS version value             */


/* Implement in file "task.c"      */
#define CoCreateTask(task,argv,prio,stk,stkSz)              \
            CreateTask(task,argv,(prio)|((stkSz)<<8),stk)


#define CoCreateTaskEx(task,argv,prio,stk,stkSz,timeSlice,isWaitting)  \
           CreateTask(task,argv,(prio)|((stkSz)<<8)|((timeSlice)<<20)|(isWaitting<<31),stk)

extern "C" void        CoExitTask(void);
extern "C" OS_TID      CoGetCurTaskID(void);
extern "C" StatusType  CoDelTask(OS_TID taskID);
extern "C" StatusType  CoActivateTask(OS_TID taskID,void *argv);
extern "C" StatusType  CoAwakeTask(OS_TID taskID);
extern "C" StatusType  CoSuspendTask(OS_TID taskID);
extern "C" StatusType  CoSetPriority(OS_TID taskID,U8 priority);
extern "C" OS_TID      CreateTask(FUNCPtr task,void *argv,U32 parameter,OS_STK *stk);

/* Implement in file "time.c"      */
extern "C" U64         CoGetOSTime(void);
extern "C" StatusType  CoTickDelay(U32 ticks);
extern "C" StatusType  CoResetTaskDelayTick(OS_TID taskID,U32 ticks);
extern "C" StatusType  CoTimeDelay(U8 hour,U8 minute,U8 sec,U16 millsec);


/* Implement in file "timer.c"     */ 
extern "C" StatusType  CoDelTmr(OS_TCID tmrID);
extern "C" StatusType  CoStopTmr(OS_TCID tmrID);
extern "C" StatusType  CoStartTmr(OS_TCID tmrID);
extern "C" U32         CoGetCurTmrCnt(OS_TCID tmrID,StatusType* perr);
extern "C" StatusType  CoSetTmrCnt(OS_TCID tmrID,U32 tmrCnt,U32 tmrReload);
extern "C" OS_TCID     CoCreateTmr(U8 tmrType, U32 tmrCnt, U32 tmrReload, vFUNCPtr func);


/* Implement in file "kernelHeap.c"*/
extern "C" void*       CoKmalloc(U32 size);
extern "C" void        CoKfree(void* memBuf);


/* Implement in file "mm.c"        */
extern "C" void*       CoGetMemoryBuffer(OS_MMID mmID);
extern "C" StatusType  CoDelMemoryPartition(OS_MMID mmID);
extern "C" StatusType  CoFreeMemoryBuffer(OS_MMID mmID,void* buf);
extern "C" U32         CoGetFreeBlockNum(OS_MMID mmID,StatusType* perr);
extern "C" OS_MMID     CoCreateMemPartition(U8* memBuf,U32 blockSize,U32 blockNum);

/* Implement in file "mutex.c"     */
extern "C" OS_MutexID  CoCreateMutex(void);
extern "C" StatusType  CoEnterMutexSection(OS_MutexID mutexID);
extern "C" StatusType  CoLeaveMutexSection(OS_MutexID mutexID);


/* Implement in file "sem.c"       */
extern "C" StatusType  CoPostSem(OS_EventID id);
extern "C" StatusType  CoAcceptSem(OS_EventID id);
extern "C" StatusType  isr_PostSem(OS_EventID id);
extern "C" StatusType  CoDelSem(OS_EventID id,U8 opt);
extern "C" StatusType  CoPendSem(OS_EventID id,U32 timeout);
extern "C" OS_EventID  CoCreateSem(U16 initCnt,U16 maxCnt,U8 sortType);


/* Implement in file "mbox.c"      */
extern "C" OS_EventID  CoCreateMbox(U8 sortType);
extern "C" StatusType  CoDelMbox(OS_EventID id,U8 opt);
extern "C" StatusType  CoPostMail(OS_EventID id,void* pmail);
extern "C" StatusType  isr_PostMail(OS_EventID id,void* pmail);
extern "C" void*       CoAcceptMail(OS_EventID id,StatusType* perr);
extern "C" void*       CoPendMail(OS_EventID id,U32 timeout,StatusType* perr);


/* Implement in file "queue.c"     */
extern "C" StatusType  CoDelQueue(OS_EventID id,U8 opt);
extern "C" StatusType  CoPostQueueMail(OS_EventID id,void* pmail);
extern "C" StatusType  isr_PostQueueMail(OS_EventID id,void* pmail);
extern "C" void*       CoAcceptQueueMail(OS_EventID id,StatusType* perr);
extern "C" OS_EventID  CoCreateQueue(void **qStart, U16 size ,U8 sortType);
extern "C" void*       CoPendQueueMail(OS_EventID id,U32 timeout,StatusType* perr);



/* Implement in file "flag.c"      */
extern "C" StatusType  CoSetFlag (OS_FlagID id);
extern "C" StatusType  CoClearFlag (OS_FlagID id);
extern "C" StatusType  isr_SetFlag (OS_FlagID id);
extern "C" StatusType  CoDelFlag (OS_FlagID id,U8 opt);
extern "C" StatusType  CoAcceptSingleFlag (OS_FlagID id);
extern "C" StatusType  CoWaitForSingleFlag (OS_FlagID id,U32 timeout);
extern "C" OS_FlagID   CoCreateFlag (BOOL bAutoReset,BOOL bInitialState);
extern "C" U32         CoAcceptMultipleFlags (U32 flags,U8 waitType,StatusType *perr);
extern "C" U32         CoWaitForMultipleFlags (U32 flags,U8 waitType,U32 timeout,StatusType *perr);


/* Implement in file "utility.c"   */
extern "C" StatusType  CoTimeToTick(U8 hour,U8 minute,U8 sec,U16 millsec,U32* ticks);
extern "C" void        CoTickToTime(U32 ticks,U8* hour,U8* minute,U8* sec,U16* millsec);


/* Implement in file "hook.c"      */
extern "C" void        CoIdleTask(void* pdata);
extern "C" void        CoStkOverflowHook(OS_TID taskID);


#endif
