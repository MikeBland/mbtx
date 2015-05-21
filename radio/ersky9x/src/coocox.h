/**
 *******************************************************************************
 * @file       coocox.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Gather for all header file of CooCox CoOS.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _COOCOX_H
#define _COOCOX_H

#define  OS_VERSION       (U16)0x0113   /*!< OS version.(format: Vx.xx), 	
                                            


																						e.g. value 0x0113 is version V1.13*/
/*---------------------------- Include ---------------------------------------*/
//#include "CoOS.h"
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
extern void    CoInitOS(void);          /*!< Initialize OS                    */
extern void    CoStartOS(void);         /*!< Start multitask                  */
extern void    CoEnterISR(void);        /*!< Enter a ISR                      */
extern void    CoExitISR(void);         /*!< Exit a ISR                       */
extern void    CoSchedLock(void);
extern void    CoSchedUnlock(void);
extern OS_VER  CoGetOSVersion(void);    /*!< Get OS version value             */


/* Implement in file "task.c"      */
#define CoCreateTask(task,argv,prio,stk,stkSz)              \
            CreateTask(task,argv,(prio)|((stkSz)<<8),stk)


#define CoCreateTaskEx(task,argv,prio,stk,stkSz,timeSlice,isWaitting)  \
           CreateTask(task,argv,(prio)|((stkSz)<<8)|((timeSlice)<<20)|(isWaitting<<31),stk)

extern void        CoExitTask(void);
extern OS_TID      CoGetCurTaskID(void);
extern StatusType  CoDelTask(OS_TID taskID);
extern StatusType  CoActivateTask(OS_TID taskID,void *argv);
extern StatusType  CoAwakeTask(OS_TID taskID);
extern StatusType  CoSuspendTask(OS_TID taskID);
extern StatusType  CoSetPriority(OS_TID taskID,U8 priority);
extern OS_TID      CreateTask(FUNCPtr task,void *argv,U32 parameter,OS_STK *stk);

/* Implement in file "time.c"      */
extern U64         CoGetOSTime(void);
extern StatusType  CoTickDelay(U32 ticks);
extern StatusType  CoResetTaskDelayTick(OS_TID taskID,U32 ticks);
extern StatusType  CoTimeDelay(U8 hour,U8 minute,U8 sec,U16 millsec);


/* Implement in file "timer.c"     */ 
extern StatusType  CoDelTmr(OS_TCID tmrID);
extern StatusType  CoStopTmr(OS_TCID tmrID);
extern StatusType  CoStartTmr(OS_TCID tmrID);
extern U32         CoGetCurTmrCnt(OS_TCID tmrID,StatusType* perr);
extern StatusType  CoSetTmrCnt(OS_TCID tmrID,U32 tmrCnt,U32 tmrReload);
extern OS_TCID     CoCreateTmr(U8 tmrType, U32 tmrCnt, U32 tmrReload, vFUNCPtr func);


/* Implement in file "kernelHeap.c"*/
extern void*       CoKmalloc(U32 size);
extern void        CoKfree(void* memBuf);


/* Implement in file "mm.c"        */
extern void*       CoGetMemoryBuffer(OS_MMID mmID);
extern StatusType  CoDelMemoryPartition(OS_MMID mmID);
extern StatusType  CoFreeMemoryBuffer(OS_MMID mmID,void* buf);
extern U32         CoGetFreeBlockNum(OS_MMID mmID,StatusType* perr);
extern OS_MMID     CoCreateMemPartition(U8* memBuf,U32 blockSize,U32 blockNum);

/* Implement in file "mutex.c"     */
extern OS_MutexID  CoCreateMutex(void);
extern StatusType  CoEnterMutexSection(OS_MutexID mutexID);
extern StatusType  CoLeaveMutexSection(OS_MutexID mutexID);


/* Implement in file "sem.c"       */
extern StatusType  CoPostSem(OS_EventID id);
extern StatusType  CoAcceptSem(OS_EventID id);
extern StatusType  isr_PostSem(OS_EventID id);
extern StatusType  CoDelSem(OS_EventID id,U8 opt);
extern StatusType  CoPendSem(OS_EventID id,U32 timeout);
extern OS_EventID  CoCreateSem(U16 initCnt,U16 maxCnt,U8 sortType);


/* Implement in file "mbox.c"      */
extern OS_EventID  CoCreateMbox(U8 sortType);
extern StatusType  CoDelMbox(OS_EventID id,U8 opt);
extern StatusType  CoPostMail(OS_EventID id,void* pmail);
extern StatusType  isr_PostMail(OS_EventID id,void* pmail);
extern void*       CoAcceptMail(OS_EventID id,StatusType* perr);
extern void*       CoPendMail(OS_EventID id,U32 timeout,StatusType* perr);


/* Implement in file "queue.c"     */
extern StatusType  CoDelQueue(OS_EventID id,U8 opt);
extern StatusType  CoPostQueueMail(OS_EventID id,void* pmail);
extern StatusType  isr_PostQueueMail(OS_EventID id,void* pmail);
extern void*       CoAcceptQueueMail(OS_EventID id,StatusType* perr);
extern OS_EventID  CoCreateQueue(void **qStart, U16 size ,U8 sortType);
extern void*       CoPendQueueMail(OS_EventID id,U32 timeout,StatusType* perr);



/* Implement in file "flag.c"      */
extern StatusType  CoSetFlag (OS_FlagID id);
extern StatusType  CoClearFlag (OS_FlagID id);
extern StatusType  isr_SetFlag (OS_FlagID id);
extern StatusType  CoDelFlag (OS_FlagID id,U8 opt);
extern StatusType  CoAcceptSingleFlag (OS_FlagID id);
extern StatusType  CoWaitForSingleFlag (OS_FlagID id,U32 timeout);
extern OS_FlagID   CoCreateFlag (BOOL bAutoReset,BOOL bInitialState);
extern U32         CoAcceptMultipleFlags (U32 flags,U8 waitType,StatusType *perr);
extern U32         CoWaitForMultipleFlags (U32 flags,U8 waitType,U32 timeout,StatusType *perr);


/* Implement in file "utility.c"   */
extern StatusType  CoTimeToTick(U8 hour,U8 minute,U8 sec,U16 millsec,U32* ticks);
extern void        CoTickToTime(U32 ticks,U8* hour,U8* minute,U8* sec,U16* millsec);


/* Implement in file "hook.c"      */
extern void        CoIdleTask(void* pdata);
extern void        CoStkOverflowHook(OS_TID taskID);


#endif


//#include "OsArch.h"

/**
 *******************************************************************************
 * @file       OsArch.h
 * @version   V1.1.4    
 * @date      2011.04.20
 * @brief      Implement function declare related to Cortex-M3(ARM-v7)
 * @details    This header file including functions or defines related to 
 *             Cortex-M3(ARM-v7).	 		
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef  _CPU_H
#define  _CPU_H


#define NVIC_ST_CTRL    (*((volatile U32 *)0xE000E010))
#define NVIC_ST_RELOAD  (*((volatile U32 *)0xE000E014))
#define RELOAD_VAL      ((U32)(( (U32)CFG_CPU_FREQ) / (U32)CFG_SYSTICK_FREQ) -1)

/*!< Initial System tick.	*/
#define InitSysTick()   NVIC_ST_RELOAD =  RELOAD_VAL; \
                        NVIC_ST_CTRL   =  0x0007    

#define NVIC_SYS_PRI2   (*((volatile U32 *)0xE000ED1C))
#define NVIC_SYS_PRI3   (*((volatile U32 *)0xE000ED20))

/*!< Initialize PendSV,SVC and SysTick interrupt priority to lowest.          */
#define InitInt()       NVIC_SYS_PRI2 |=  0xFF000000;\
                        NVIC_SYS_PRI3 |=  0xFFFF0000


/*---------------------------- Variable declare ------------------------------*/
extern U64      OSTickCnt;          /*!< Counter for current system ticks.    */									

/*!< Initial context of task being created	*/
extern OS_STK  *InitTaskContext(FUNCPtr task,void *param,OS_STK *pstk);
extern void    SwitchContext(void);         /*!< Switch context                   */
extern void    SetEnvironment(OS_STK *pstk);/*!< Set environment for run          */
extern U8      Inc8 (volatile U8 *data);
extern U8      Dec8 (volatile U8 *data);
extern void    IRQ_ENABLE_RESTORE(void);
extern void    IRQ_DISABLE_SAVE(void);
#endif


//#include "OsCore.h"

/**
 *******************************************************************************
 * @file       core.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file	related to kernel	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _CORE_H
#define _CORE_H

//#include <CoOS.h>


//#define  OsSchedLock()  OSSchedLock++;      //!< Lock schedule
//-#define OsSchedLock() OSSchedLock++;
//#define OsSchedLock() { OSSchedLock++; asm volatile ("" : : : "memory"); } // !< Lock scheduleextern   void OsSchedUnlock(void);
#define OsSchedLock() { asm(""); OSSchedLock++; asm volatile ("" : : : "memory"); } // !< Lock scheduleextern   void OsSchedUnlock(void);

#endif

//#include "OsTask.h"

/**
 *******************************************************************************
 * @file       task.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file	related to task.
 * @details    This file including some defines and function declare related to task.  	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

#ifndef _TASK_H
#define _TASK_H

#define  SYS_TASK_NUM   (1)             /*!< System task number.              */

/*---------------------------- Task Status -----------------------------------*/
#define  TASK_READY     0               /*!< Ready status of task.            */	 				
#define  TASK_RUNNING   1               /*!< Running status of task.          */
#define  TASK_WAITING   2               /*!< Waitting status of task.         */
#define  TASK_DORMANT   3               /*!< Dormant status of task.          */ 


#define  INVALID_ID     (U8)0xff
#define  INVALID_VALUE  (U32)0xffffffff
#define  MAGIC_WORD     (U32)0x5a5aa5a5


/**
 * @struct  TCB  task.h  	
 * @brief   Task control blcok.
 * @details This struct use to manage task.	 	
 */
typedef  struct TCB
{
    OS_STK      *stkPtr;                /*!< The current point of task.       */
    U8          prio;                   /*!< Task priority.                   */
    U8          state;                  /*!< TaSk status.                     */
    OS_TID      taskID;                 /*!< Task ID.                         */

#if CFG_MUTEX_EN > 0
    OS_MutexID  mutexID;                /*!< Mutex ID.                        */
#endif
   
#if CFG_EVENT_EN > 0
    OS_EventID  eventID;                /*!< Event ID.                        */
#endif
    
#if CFG_ROBIN_EN >0
    U16         timeSlice;              /*!< Task time slice                  */
#endif
    
#if CFG_STK_CHECKOUT_EN >0
    OS_STK      *stack;                 /*!< The top point of task.           */
#endif
    
#if CFG_EVENT_EN > 0
    void*       pmail;                  /*!< Mail to task.                    */
    struct TCB  *waitNext;  /*!< Point to next TCB in the Event waitting list.*/
    struct TCB  *waitPrev;  /*!< Point to prev TCB in the Event waitting list.*/
#endif

#if CFG_TASK_SCHEDULE_EN == 0
	FUNCPtr     taskFuc;
	OS_STK      *taskStk;
#endif  

    
#if CFG_FLAG_EN > 0
    void*       pnode;                  /*!< Pointer to node of event flag.   */
#endif   

#if CFG_TASK_WAITTING_EN >0
    U32         delayTick;              /*!< The number of ticks which delay. */
#endif    
    struct TCB  *TCBnext;               /*!< The pointer to next TCB.         */
    struct TCB  *TCBprev;               /*!< The pointer to prev TCB.         */

}OSTCB,*P_OSTCB;


/*---------------------------- Variable declare ------------------------------*/
// save tcb ptr that created
extern P_OSTCB  FreeTCB;      /*!< A pointer to free TCB.                     */
extern OSTCB    TCBTbl[CFG_MAX_USER_TASKS+SYS_TASK_NUM];
extern P_OSTCB  TCBRdy;       /*!< A pointer to TCB that is ready status      */
extern P_OSTCB  TCBNext;      /*!< A pointer to TCB next be scheduled.        */	
extern P_OSTCB  TCBRunning;   /*!< A pointer to TCB that is running.          */

extern U64      OSCheckTime;
extern volatile U8   OSIntNesting; /*!< Use to indicate interrupt nesting level.*/   				
extern volatile U8   OSSchedLock;  /*!< Schedule is lock(LOCK) or unlock(UN_LOCK).*/  	
extern volatile BOOL TaskSchedReq;
extern OS_STK   idle_stk[CFG_IDLE_STACK_SIZE];


void  Schedule(void);         /*!< Schedule function                          */
void  IdleTask(void* pdata);  /*!< IDLE task code                             */
void  InsertToTCBRdyList  (P_OSTCB tcbInser);	
void  RemoveFromTCBRdyList(P_OSTCB ptcb);
void  CreateTCBList(void);
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
void  ActiveTaskPri(U8 pri);
void  DeleteTaskPri(U8 pri);
#endif

#endif


//#include "OsServiceReq.h"


/**
 *******************************************************************************
 * @file       serviceReq.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file	related to service request	
 * @details    This file including some defines and function declare related to
 *             service request.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _SERVICEREQ_H
#define _SERVICEREQ_H

#if CFG_MAX_SERVICE_REQUEST > 0
#define   SEM_REQ       (U8)0x1
#define   MBOX_REQ      (U8)0x2
#define   FLAG_REQ      (U8)0x3
#define   QUEUE_REQ     (U8)0x4


typedef struct ServiceReqCell
{
    U8      type;
    U8      id;
    void*   arg;
}SQC,*P_SQC;

typedef struct ServiceReqQueue
{
    U8    cnt;
    U8    head;    
    SQC   cell[CFG_MAX_SERVICE_REQUEST];
}SRQ,*P_SRQ;


extern SRQ  ServiceReq;
extern BOOL InsertInSRQ(U8 type,U8 id,void* arg);
#endif

extern void RespondSRQ(void);
extern BOOL TimeReq;
extern BOOL TimerReq;
extern BOOL IsrReq;
#endif


//#include "OsError.h"

/**
 *******************************************************************************
 * @file       error.h	
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      rror dispose header file	
 * @details    This file use to dispose error which from error configure for OS.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _ERROR_H
#define _ERROR_H

#if (CFG_SYSTICK_FREQ > 1000) ||(CFG_SYSTICK_FREQ < 1) 
    #error " OsConfig.h System Tick time must between 1ms and 1s!"
#endif

#if CFG_MAX_USER_TASKS > 253
    #error " OsConfig.h, CFG_MAX_USER_TASKS must be <= 253! "
#endif

#if CFG_LOWEST_PRIO > 254
    #error " OsConfig.h, CFG_LOWEST_PRIO must be <= 254! "
#endif

#if CFG_IDLE_STACK_SIZE <25
	#error " OsConfig.h, CFG_IDLE_STACK_SIZE must be >= 25! "
#endif


#if CFG_ROBIN_EN > 0
    #if CFG_TIME_SLICE > 4095
    #error " OsConfig.h, CFG_TIME_SLICE must be <= 4095! "
    #endif
#endif

#if CFG_TMR_EN > 0
    #if CFG_MAX_TMR > 32
    #error " OsConfig.h, CFG_MAX_TMR must be <= 32! "
    #endif
#endif


#if CFG_MM_EN > 0
    #if CFG_MAX_MM > 32
    #error " config.h, CFG_MAX_MM must be <= 32! "
    #endif
#endif


#if CFG_KHEAP_EN > 0
    #if KHEAP_SIZE < 0x20
    #error " config.h, CFG_MAX_MM must be >= 0x20! "
    #endif
#endif

#if CFG_MUTEX_EN > 0
    #if CFG_MAX_MUTEX > 254
    #error " config.h, CFG_MAX_MUTEX must be <= 254! "
    #endif
#endif


#if CFG_EVENT_EN > 0
    #if (CFG_MAX_EVENT > 254 || CFG_MAX_EVENT <= 0)
    #error " config.h, CFG_MAX_EVENT must be <= 254 && > 0! "
    #endif


    #if CFG_QUEUE_EN > 0 
        #if CFG_MAX_QUEUE > CFG_MAX_EVENT
        #error " config.h, CFG_MAX_QUEUE must be <= CFG_MAX_EVENT! "	
        #endif
    #endif	
#endif      /* CFG_EVENT_EN  */

#endif      /* _ERROR_H      */

//#include "OsTime.h"

/**
 *******************************************************************************
 * @file       task.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file related to time management
 * @details    Thie file including some data declare related to time managment. 	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 
  
#ifndef _TIME_H
#define _TIME_H

/*---------------------------- Variable declare ------------------------------*/
extern P_OSTCB  DlyList;            /*!< A pointer to ther delay list.        */

/*---------------------------- Function declare ------------------------------*/
extern void  TimeDispose(void);     /*!< Time dispose function.               */
extern void  isr_TimeDispose(void);
extern void  RemoveDelayList(P_OSTCB ptcb);
extern void  InsertDelayList(P_OSTCB ptcb,U32 ticks);
#endif


#if CFG_TMR_EN > 0
//	#include "OsTimer.h"

/**
 *******************************************************************************
 * @file       timer.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Timer manage header file	
 * @details    This file including some declares and defines related to timer 
 *             management.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

#ifndef _TIMER_H
#define _TIMER_H

#define   TMR_STATE_RUNNING     0       /*!< Timer State: Running             */
#define   TMR_STATE_STOPPED     1       /*!< Timer State: Stopped             */

/**
 * @struct   tmrCtrl  timer.h  	
 * @brief    Timer control block 
 * @details  This struct is use to manage user timer. 
 *	
 */
typedef struct tmrCtrl                  /* Timer Control Block Define.        */
{
    OS_TCID          tmrID;             /*!< Timer ID.                        */
    U8               tmrType;           /*!< Timer Type.                      */
    U8               tmrState;          /*!< Timer State.                     */
    U32              tmrCnt;            /*!< Timer Counter.                   */
    U32              tmrReload;         /*!< Timer Reload Counter Value.      */	
    vFUNCPtr         tmrCallBack; /*!< Call-back Function When Timer overrun. */	
    struct tmrCtrl*  tmrNext;       /*!< Point to Next Timer Control Block.   */
    struct tmrCtrl*  tmrPrev;       /*!< Point to Previous Timer Control Block*/

}TmrCtrl,*P_TmrCtrl;

/*---------------------------- Variable declare ------------------------------*/
extern P_TmrCtrl  TmrList;              /*!< A pointer to the timer list.     */ 
extern U32        TmrIDVessel;
/*---------------------------- Function declare ------------------------------*/
extern void  TmrDispose(void);          /*!< Timer counter function.          */
extern void  isr_TmrDispose(void);
#endif

#endif

#if CFG_KHEAP_EN > 0
//	#include "OsKernelHeap.h"

/**
 *******************************************************************************
 * @file       kernelHeap.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file related to memory management	
 * @details    This file including some defines and function declare related to 
 *             kernel heap management. 
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef  _KERNELHEAP_H
#define  _KERNELHEAP_H


typedef struct KennelHeap
{
  U32   startAddr;
  U32   endAddr;
}KHeap,*P_KHeap;


typedef struct UsedMemBlk
{
  void* nextMB;
  void* preMB;	
}UMB,*P_UMB;


typedef struct FreeMemBlk
{
  struct FreeMemBlk* nextFMB;
  struct UsedMemBlk* nextUMB;
  struct UsedMemBlk* preUMB;
}FMB,*P_FMB;

/*---------------------------- Function Declare ------------------------------*/
extern void   CoCreateKheap(void);

#endif  /* _KERNELHEAP_H */


#endif

#if CFG_MM_EN >0
//	#include "OsMM.h"

/**
 *******************************************************************************
 * @file       mm.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Header file	related to memory management 
 * @details    This file including some defines and function declare related to 
 *             memory management. 	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef  _MM_H
#define  _MM_H


typedef struct Memory
{
    U8*   memAddr;
    U8*   freeBlock;
    U32   blockSize;
    U32   blockNum;			
}MM,*P_MM;


typedef struct MemoryBlock
{
    struct MemoryBlock* nextBlock;
}MemBlk,*P_MemBlk;


extern U32  MemoryIDVessel;

#endif   /* _MM_H */



#endif

#if CFG_EVENT_EN > 0
//	#include "OsEvent.h"

/**
 *******************************************************************************
 * @file       event.h	
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Event management header file
 * @details    This file including some defines and declares related to event 
 *             (semaphore,mailbox,queque) management.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _EVENT_H
#define _EVENT_H

#define EVENT_TYPE_SEM        (U8)0x01      /*!< Event type:Semaphore.        */
#define EVENT_TYPE_MBOX       (U8)0x02      /*!< Event type:Mailbox.          */
#define EVENT_TYPE_QUEUE      (U8)0x03      /*!< Event type:Queue.            */
#define EVENT_TYPE_INVALID    (U8)0x04      /*!< Invalid event type.          */


/**
 * @struct  EventCtrBlk  event.h	  	
 * @brief   Event control block
 * @details This struct is use to manage event,
 *          e.g. semaphore,mailbox,queue.	
 */
typedef struct EventCtrBlk
{
    void*   eventPtr;                   /*!< Point to mailbox or queue struct */
    U8      id;                         /*!< ECB id                           */
    U8      eventType:4;                /*!< Type of event                    */
    U8      eventSortType:4;            /*!< 0:FIFO 1: Preemptive by prio     */
    U16     eventCounter;               /*!< Counter of semaphore.            */
    U16     initialEventCounter;        /*!< Initial counter of semaphore.    */
    P_OSTCB eventTCBList;               /*!< Task waitting list.              */
}ECB,*P_ECB;

/*---------------------------- Variable declare ------------------------------*/
extern ECB  EventTbl[CFG_MAX_EVENT];    /*!< Table use to save TCB.           */

/*---------------------------- Function declare ------------------------------*/
/*!< Create a event   */
extern P_ECB      CreatEvent(U8 eventType,U8 eventSortType,void* eventPtr);

/*!< Remove a task from wait list */	
extern void       EventTaskToWait(P_ECB pecb,P_OSTCB ptcb);
extern StatusType DeleteEvent(P_ECB pecb,U8 opt);   /*!< Delete a event.      */
extern void       EventTaskToRdy(P_ECB pecb); /*!< Insert a task to ready list*/
extern void       CreateEventList(void);    /*!< Create a event list.         */			
extern void       RemoveEventWaittingList(P_OSTCB ptcb);
#endif  

#endif

#if CFG_MUTEX_EN > 0
//	#include "OsMutex.h"

/**
 *******************************************************************************
 * @file       mutex.h	
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Mutex management header file
 * @details    This file including some defines and declare related to mutex
 *             management.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _MUTEX_H
#define _MUTEX_H

#include <coocox.h>

#if CFG_MUTEX_EN > 0

/*---------------------------- Resource status -------------------------------*/
#define   MUTEX_FREE        0           /*!< Mutex is free                    */
#define   MUTEX_OCCUPY      1           /*!< Mutex is occupy                  */
#define   WAITING_MUTEX     0x80

/**
 * @struct   Mutex  mutex.h 	
 * @brief    Mutex control block
 * @details  This struct is use to mutex management.	
 */
typedef struct Mutex
{
    U8       originalPrio;              /*!< Mutex priority.                  */
    U8       mutexFlag;                 /*!< Mutex flag.                      */
    OS_TID   taskID;                    /*!< Task ID.                         */	
    OS_TID   hipriTaskID;               /*!< Mutex ID.                        */
    P_OSTCB  waittingList;              /*!< waitting the Mutex.              */
}MUTEX,*P_MUTEX;


/*---------------------------- Variable declare ------------------------------*/
/*!< Table use to save mutex control block.                                   */
extern MUTEX      MutexTbl[CFG_MAX_MUTEX];
extern OS_MutexID MutexFreeID;      /*!< A pointer to next vliad resource ID. */


/*---------------------------- Function declare ------------------------------*/
extern void   RemoveMutexList(P_OSTCB ptcb);

#endif    /* CFG_MUTEX_EN  */

#endif    /* _MUTEX_H      */


#endif

#if CFG_QUEUE_EN > 0
//	#include "OsQueue.h"

/**
 *******************************************************************************
 * @file       queue.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Queue management header file	
 * @details    This file including some defines and declares about queue management.
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _QUEUE_H
#define _QUEUE_H


/**
 * @struct   Queue  queue.h	
 * @brief    Queue struct
 * @details  This struct use to manage queue.
 *	
 */
typedef struct Queue
{
    void    **qStart;                   /*!<                                  */
    U8      id;                         /*!<                                  */
    U16     head;                       /*!< The header of queue              */
    U16     tail;                       /*!< The end of queue                 */
    U16     qMaxSize;                   /*!< The max size of queue            */
    U16     qSize;                      /*!< Current size of queue            */
}QCB,*P_QCB;


#endif


#endif

#if CFG_FLAG_EN	 > 0
//	#include "OsFlag.h"

/**
 *******************************************************************************
 * @file       flag.h
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Evnet flag management header file
 * @details    This file including some defines and declares about flag management.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#ifndef _FLAG_H
#define _FLAG_H

/**
 * @struct  FlagNode  flag.h 	
 * @brief   Flag node struct
 * @details 	 	
 */
typedef struct FlagNode
{
    struct FlagNode*  nextNode;         /*!< A pointer to next flag node      */
    struct FlagNode*  prevNode;         /*!< A pointer to prev flag node      */
    U32               waitFlags;        /*!< Flag value                       */
    P_OSTCB           waitTask;         /*!< A pointer to task waitting flag  */
    U8                waitType;         /*!< Wait type                        */
}FLAG_NODE,*P_FLAG_NODE;


/**
 * @struct  Flag    flag.h  	
 * @brief   Flag control block
 * @details This struct use to mange event flag.	
 */
typedef struct Flag
{
    U32           flagRdy;              /*!< Ready flag                       */
    U32           resetOpt;             /*!< Reset option                     */
    U32           flagActive;           /*!< Active flag                      */
    P_FLAG_NODE   headNode;             /*!< Head node                        */
    P_FLAG_NODE   tailNode;             /*!< Tail node                        */
}FCB,*P_FCB;


/*---------------------------- Variable declare ------------------------------*/
extern FCB FlagCrl;					

/*---------------------------- Function declare ------------------------------*/
extern void        RemoveLinkNode(P_FLAG_NODE pnode);
#endif


#endif

#endif    /* _COOCOX_H    */  
