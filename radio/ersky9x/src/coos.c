 /**
 *******************************************************************************
 * @file       core.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Core implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

/*---------------------------- Include ---------------------------------------*/
#include <stdint.h>
#include <coocox.h>

#ifdef PCBSKY
 #ifndef PCBDUE
  #include "AT91SAM3S4.h"
 #else
	#include "sam3x8e.h"
 #endif
#endif

#if defined(PCBX9D) || defined(PCB9XT)
#include "X9D/stm32f2xx.h"
#endif
#if defined(PCBX12D) || defined(PCBX10)
#include "X12D/stm32f4xx.h"
#endif
#if defined(PCBLEM1)
#include <stm32f10x.h>
#endif



/*---------------------------- Variable Define -------------------------------*/
volatile U8     OSIntNesting  = 0;         /*!< Use to indicate interrupt nesting level*/
volatile U8     OSSchedLock   = 0;         /*!< Task Switch lock.                      */
volatile BOOL   TaskSchedReq  = FALSE;


/**
 *******************************************************************************
 * @brief      Enter a ISR.						   
 * @param[in]  None	 
 * @param[out] None   
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to notify OS when enter to an ISR.
 *
 * @note       When you call API in ISR,you must call CoEnterISR() before your
 *             interrupt handler code,and call CoExitISR() after your handler
 *             code and before exiting from ISR.	 
 *******************************************************************************
 */
void CoEnterISR(void)
{
    Inc8(&OSIntNesting);                /* OSIntNesting increment             */
}


/**
 *******************************************************************************
 * @brief      Exit a ISR.	 
 * @param[in]  None	 
 * @param[out] None   
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called when exit from a ISR.	  	
 *
 * @note 
 *******************************************************************************
 */
void CoExitISR(void)
{
    Dec8(&OSIntNesting);                /* OSIntNesting decrease              */
    if( OSIntNesting == 0)              /* Is OSIntNesting == 0?              */
    {
        if(TaskSchedReq == TRUE)
        {
			OSSchedLock++;
            Schedule();                 /* Call task schedule                 */
			OSSchedLock--;
        }
    }
}



/**
 *******************************************************************************
 * @brief      Unlock schedule 	  
 * @param[in]  None		 
 * @param[out] None   
 * @retval     None		 
 *
 * @par Description
 * @details   This function is called to unlock schedule(i.e.enable schedule again) 		 
 *
 * @note 
 *******************************************************************************
 */
void OsSchedUnlock(void)
{
    if(OSSchedLock == 1)                /* Is OSSchedLock == 0?               */
    {
#if CFG_TASK_WAITTING_EN > 0
        if(IsrReq == TRUE)
        {
            RespondSRQ();               /* Respond service request            */	
        }
#endif
        /* Judge task state change or higher PRI task coming in               */
        if(TaskSchedReq == TRUE)
        {
            Schedule();                 /* Call task schedule                 */
        }
		OSSchedLock = 0;
    }
	else
	{
		OSSchedLock--; 	
	}
}


/**
 *******************************************************************************
 * @brief      Lock schedule 	 
 * @param[in]  None		 
 * @param[out] None   
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called in application code to lock schedule.		 
 *
 * @note 
 *******************************************************************************
 */
void CoSchedLock(void)
{									    
    OsSchedLock();                      /* Lock schedule                      */
}


/**
 *******************************************************************************
 * @brief      Unlock schedule 	  
 * @param[in]  None		 
 * @param[out] None   
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called in APP to unlock schedule.		 
 *
 * @note 
 *******************************************************************************
 */
void CoSchedUnlock(void)
{
    OsSchedUnlock();                    /* Unlock schedule                    */
}


/**
 *******************************************************************************
 * @brief      Initialize OS	  
 * @param[in]  None 	 
 * @param[out] None 
 * @retval     None 
 *
 * @par Description
 * @details   This function is called to initialize OS.
 *
 * @note      You must call this function first,before any other OS API function
 *					
 * @code      There is a example for useage of this function,as follows: 
 *        e.g.															 
 *            ...                   // Your target initial code. 
 *				
 *            OsInit();             // Initial OS. 				
 *            CreateTask(...);      // Create tasks.				
 *            ...
 *            OsStart();            // Start multitask.
 * @endcode	
 *******************************************************************************		
 */
void CoInitOS(void)
{
    InitSysTick();                /* Initialize system tick.                  */
    InitInt();                    /* Initialize PendSV,SVC,SysTick interrupt  */	
    CreateTCBList();              /* Create TCB list.                         */   
#if CFG_EVENT_EN > 0				    
    CreateEventList();            /* Create event control list.               */
#endif  
#if CFG_KHEAP_EN > 0
    CoCreateKheap();              /* Create kernel heap within user define    */
#endif   
    OsSchedLock();                /* Lock Schedule                            */ 
                                  /* Create first task -- IDLE task.          */ 
    CoCreateTask(                      CoIdleTask,
                                             NULL,
                                  CFG_LOWEST_PRIO,
                 &idle_stk[CFG_IDLE_STACK_SIZE-1],
                              CFG_IDLE_STACK_SIZE
                 );
				                  /* Set PSP for CoIdleTask coming in */ 
	SetEnvironment(&idle_stk[CFG_IDLE_STACK_SIZE-1]);
}


/**
 *******************************************************************************
 * @brief      Start multitask	 
 * @param[in]  None 	 
 * @param[out] None 	 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to start multitask.After it is called,
 *             OS start schedule task by priority or/and time slice.	
 * @note       This function must be called to start OS when you use CoOS,and must
 *             call after CoOsInit().
 *******************************************************************************
 */
void CoStartOS(void)
{
    TCBRunning  = &TCBTbl[0];           /* Get running task                     */
    TCBNext     = TCBRunning;           /* Set next scheduled task as running task */
    TCBRunning->state = TASK_RUNNING;   /* Set running task status to RUNNING   */
    RemoveFromTCBRdyList(TCBRunning);   /* Remove running task from READY list  */
    OsSchedUnlock();					/* Enable Schedule,call task schedule   */
}


/**
 *******************************************************************************
 * @brief      Get OS version	   
 * @param[in]  None	 
 * @param[out] None  
 * @retval     The value is version of OS mutipled by 100.		 
 *
 * @par Description
 * @details    This function is used to return the version number of CooCox OS.
 *             the return value corresponds to CooCox's version number multiplied
 *             by 100. In other words, version 1.02 would be returned as 102.         
 *******************************************************************************
 */
OS_VER CoGetOSVersion(void)
{
    return OS_VERSION;                  /* Get CooCox CoOS version            */
}


// Event.c


/**
 *******************************************************************************
 * @file       event.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      event management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


/*---------------------------- Variable Define -------------------------------*/
#if CFG_EVENT_EN > 0

ECB    EventTbl[CFG_MAX_EVENT]= {{0}};/*!< Table which save event control block.*/
P_ECB  FreeEventList = NULL;        /*!< Pointer to free event control block. */


/**
 *******************************************************************************
 * @brief      Create a empty list of event control block 	   
 * @param[in]  None 	 
 * @param[out] None  	 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called by OSInit() API to create a ECB list,supply
 *             a  pointer to next event control block that not used.	 				
 *******************************************************************************
 */
void CreateEventList(void)
{	
    U8  i;
    P_ECB pecb1;
#if CFG_MAX_EVENT > 1
    P_ECB pecb2;
#endif
    i=0;
    pecb1 = &EventTbl[0];               /* Get first item                     */
#if CFG_MAX_EVENT == 1                  /* Build event list for only one item */									   
    pecb1->eventPtr  = NULL;
    pecb1->id        = i;               /* Assign ID.                         */
    pecb1->eventType = EVENT_TYPE_INVALID;  /* Sign that not to use.          */
#endif
    
#if CFG_MAX_EVENT > 1             /* Build event list for more than one item  */								   
    pecb2 = &EventTbl[1];
    for(;i< (CFG_MAX_EVENT-1);i++ )
    {
        pecb1->eventPtr  = (void*)pecb2;      /* Set link for list            */
        pecb1->id        = i;                 /* Assign ID.                   */
        pecb1->eventType = EVENT_TYPE_INVALID;/* Sign that not to use.        */
        pecb1++;                              /* Get next item                */
        pecb2++;	
    }
	pecb1->eventType = EVENT_TYPE_INVALID;    /* Sign that not to use.        */
    pecb1->eventPtr  = NULL;                  /* Set link for last item       */
    pecb1->id        = i;	
#endif
    
    FreeEventList    = &EventTbl[0];          /* Set free event item          */	
}



/**
 *******************************************************************************
 * @brief      Release a ECB	 
 * @param[in]  pecb     A pointer to event control block which be released.	 
 * @param[out] None 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to release a event control block when a 
 *             event be deleted.
 *******************************************************************************
 */
static void ReleaseECB(P_ECB pecb)
{
    pecb->eventType = EVENT_TYPE_INVALID;     /* Sign that not to use.        */ 
    OsSchedLock();                            /* Lock schedule                */
    pecb->eventPtr  = FreeEventList;          /* Release ECB that event hold  */
    FreeEventList   = pecb;                   /* Reset free event item        */
    OsSchedUnlock();                          /* Unlock schedule              */
}



/**
 *******************************************************************************
 * @brief      Create a event	  
 * @param[in]  eventType       The type of event which	being created.
 * @param[in]  eventSortType   Event sort type.
 * @param[in]  eventCounter    Event counter,ONLY for EVENT_TYPE_SEM.
 * @param[in]  eventPtr        Event struct pointer,ONLY for Queue.NULL for other 
 *                             event type.		
 * @param[out] None  
 * @retval     NULL     Invalid pointer,create event fail.					 
 * @retval     others   Pointer to event control block which had assigned right now.
 *
 * @par Description
 * @details    This function is called by CreateSem(),...
 *             to get a event control block and initial the event content. 
 *
 * @note       This is a internal function of CooCox CoOS,User can't call.
 *******************************************************************************
 */
P_ECB CreatEvent(U8 eventType,U8 eventSortType,void* eventPtr)
{
    P_ECB pecb;
    
    OsSchedLock();                      /* Lock schedule                      */
    if(FreeEventList == NULL)           /* Is there no free evnet item        */
    {
        OsSchedUnlock();                /* Yes,unlock schedule                */
        return NULL;                    /* Return error                       */
    }
    pecb          = FreeEventList;/* Assign the free event item to this event */
    FreeEventList = FreeEventList->eventPtr;  /* Reset free event item        */
    OsSchedUnlock();                    /* Unlock schedul                     */
    
    pecb->eventType     = eventType;    /* Initialize event item as user set  */
    pecb->eventSortType = eventSortType;
    pecb->eventPtr      = eventPtr;
    pecb->eventTCBList  = NULL;
    return pecb;                        /* Return event item pointer          */
}


/**
 *******************************************************************************
 * @brief      Delete a event	  
 * @param[in]  pecb     Pointer to event control block which will be deleted. 
 * @param[in]  opt      Delete option.
 * @arg        == OPT_DEL_ANYWAY     Delete event always   
 * @arg        == OPT_DEL_NO_PEND	 Delete event only when no task pending on.
 * @param[out] None  	 
 * @retval     E_INVALID_PARAMETER   Parameter passed is invalid,deleted fail.
 * @retval     E_TASK_WAITTING       These are one more tasks waitting event.  
 * @retval     E_OK                  Delete event control block successful.
 *		  	
 * @par Description
 * @details    This function is called to delete a event from the event wait list
 *             use specify option.
 *
 * @note       This is a internal function of Coocox CoOS,user can't call.		
 *******************************************************************************
 */
StatusType DeleteEvent(P_ECB pecb,U8 opt)
{
    P_OSTCB ptcb;
    if(opt == OPT_DEL_NO_PEND)          /* Do delete event when no task pend? */
    {
        if(pecb->eventTCBList != NULL)  /* Yes,is there task pend this event? */
        {
            return E_TASK_WAITING;      /* Yes,error return                   */
        }
        else
        {
            ReleaseECB(pecb);           /* No,release resource that event hold*/
        }
    }
    else if(opt == OPT_DEL_ANYWAY)      /* Do delete event anyway?            */
    {
        OsSchedLock();                      /* Lock schedule                  */
        while(pecb->eventTCBList != NULL)   /* Is there task pend this event? */
        {                                   /* Yes,remove it                  */
            ptcb = pecb->eventTCBList;/* Get first task in event waiting list */
            if(ptcb->delayTick != INVALID_VALUE) /* Is task in delay list?    */
            {
                RemoveDelayList(ptcb);    /* Yes,remove task from delay list  */
            }

            /* Set next item as event waiting list head */
            pecb->eventTCBList = ptcb->waitNext; 
            ptcb->waitNext     = NULL;  /* Clear link for event waiting list  */
            ptcb->eventID      = INVALID_ID;  /* Sign that not to use.        */

			InsertToTCBRdyList(ptcb);         /* Insert task into ready list  */
        }
        OsSchedUnlock();                  /* Unlock schedule                  */
        ReleaseECB(pecb);                 /* Release resource that event hold */
    }
    return E_OK;                          /* Return OK                        */
}


/**
 *******************************************************************************
 * @brief      Insert a task to event wait list 	  						  
 * @param[in]  pecb    Pointer to event control block corresponding to the event. 	
 * @param[in]  ptcb    Pointer to task that will be insert to event wait list.	 
 * @param[out] None   
 * @retval     None	 
 *
 * @par Description
 * @details   This function is called to insert a task by fllowing manner:
 *            opt == EVENT_SORT_TYPE_FIFO   By FIFO.
 *            opt == EVENT_SORT_TYPE_PRIO   By priority order,hghest priority 
 *                                          as head,lowest priority as end.
 *                                          (Highest-->...-->Lowest-->NULL)	
 *******************************************************************************
 */
void EventTaskToWait(P_ECB pecb,P_OSTCB ptcb)
{
    P_OSTCB ptcb1;
#if (CFG_EVENT_SORT == 2) || (CFG_EVENT_SORT == 3)
    P_OSTCB ptcb2;
#endif
    
    OsSchedLock();                  /* Lock schedule                          */
    ptcb1 = pecb->eventTCBList;     /* Get first task in event waiting list   */
    ptcb->eventID = pecb->id;       /* Set event ID for task                  */
    
#if CFG_EVENT_SORT == 3             /* Does event waiting list sort as FIFO?  */
                              
    if(pecb->eventSortType == EVENT_SORT_TYPE_FIFO)	
#endif
    
#if (CFG_EVENT_SORT == 1) || (CFG_EVENT_SORT == 3)
    {
        if(ptcb1 == NULL)                 /* Is no item in event waiting list?*/
        {
            pecb->eventTCBList = ptcb;    /* Yes,set task as first item       */
        }
        else
        {								
            while(ptcb1->waitNext != NULL)/* No,insert task in last           */
            {
                ptcb1 = ptcb1->waitNext;	
            }	
            ptcb1->waitNext = ptcb;       /* Set link for list                */
            ptcb->waitPrev  = ptcb1;	
        }
    }
#endif
    
#if CFG_EVENT_SORT ==3 /* Does event waiting list sort as preemptive priority?*/                           
    else if(pecb->eventSortType == EVENT_SORT_TYPE_PRIO)
#endif  
#if (CFG_EVENT_SORT == 2) || (CFG_EVENT_SORT == 3)
    {
        if(ptcb1 == NULL)               /* Is no item in event waiting list?  */
        {
            pecb->eventTCBList = ptcb;  /* Yes,set task as first item         */
        }
        /* Is PRI of task higher than list first item?                        */
        else if(ptcb1->prio > ptcb->prio) 
        {
            pecb->eventTCBList = ptcb;  /* Reset task as first item           */
            ptcb->waitNext     = ptcb1; /* Set link for list                  */
            ptcb1->waitPrev    = ptcb;	
        }
        else                            /* No,find correct place to insert    */
        {								
            ptcb2 = ptcb1->waitNext;
            while(ptcb2 != NULL)        /* Is last item?                      */
            {	                          
                if(ptcb2->prio > ptcb->prio)  /* No,is correct place?         */
                { 
                    break;                    /* Yes,break Circulation        */
                }
                ptcb1 = ptcb2;                /* Save current item            */
                ptcb2 = ptcb2->waitNext;      /* Get next item                */
            }
            ptcb1->waitNext = ptcb;           /* Set link for list            */
            ptcb->waitPrev  = ptcb1;
            ptcb->waitNext  = ptcb2;
            if(ptcb2 != NULL)
            {
                ptcb2->waitPrev = ptcb;	
            }
        }		
    }
#endif
    ptcb->state = TASK_WAITING;     /* Set task status to TASK_WAITING state  */
    TaskSchedReq = TRUE;
    OsSchedUnlock();                /* Unlock schedule,and call task schedule */
}


/**
 *******************************************************************************
 * @brief      Move a task from event WAITING list to the DELAY list	  
 * @param[in]  pecb    Pointer to event control block corresponding to the event. 		 	  
 * @param[out] None  
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to remove a task from event wait list,and	 
 *             then insert it into the READY list.
 *******************************************************************************
 */
void EventTaskToRdy(P_ECB pecb)
{
    P_OSTCB ptcb;
#if CFG_QUEUE_EN >0
    P_QCB   pqcb;
#endif
    ptcb = pecb->eventTCBList;
    if(ptcb == NULL)
        return;
    
    pecb->eventTCBList = ptcb->waitNext;/* Get first task in event waiting list*/
    if(pecb->eventTCBList != NULL)      /* Is no item in event waiting list?  */
    {
        pecb->eventTCBList->waitPrev = NULL; /* No,clear link for first item  */
    }
    
    ptcb->waitNext = NULL;                /* Clear event waiting link for task*/
    ptcb->eventID  = INVALID_ID;          /* Sign that not to use.            */
    
    if(ptcb->delayTick != INVALID_VALUE)  /* Is task in delay list?           */		         
    {
        RemoveDelayList(ptcb);            /* Yes,remove task from DELAY list  */
    }
    if(pecb->eventType == EVENT_TYPE_MBOX)/* Is it a mailbox event?           */
    {
        ptcb->pmail    = pecb->eventPtr;  /* Yes,send mail to task            */
        pecb->eventPtr = NULL;            /* Clear event sign                 */
        pecb->eventCounter--;
    }
#if CFG_QUEUE_EN >0
    else if(pecb->eventType == EVENT_TYPE_QUEUE)  /* Is it a queue event?     */
    {										   
        pqcb        = (P_QCB)pecb->eventPtr;      /* Yes,get queue pointer    */
        ptcb->pmail = *(pqcb->qStart + pqcb->head);   /* Send mail to task    */
        pqcb->head++;                             /* Clear event sign         */
        pqcb->qSize--;
        if(pqcb->head == pqcb->qMaxSize)
        {
            pqcb->head = 0;	
        }
    }
#endif

#if CFG_MAILBOX_EN >0
    else if(pecb->eventType == EVENT_TYPE_SEM)/* Is it a semaphore event?     */
    {
        pecb->eventCounter--;                 /* Yes,clear event sign         */
        ptcb->pmail = (void*)0xffffffff;      /* Indicate task woke by event  */
    }
#endif
	if(ptcb == TCBRunning)
	{
		ptcb->state = TASK_RUNNING;
	} 
	else
	{
		InsertToTCBRdyList(ptcb);            /* Insert task into ready list  */
	}
}



/**
 *******************************************************************************
 * @brief      Move a task from event wait list to the ready list	  
 * @param[in]  pecb    Pointer to event control block corresponding to the event. 		 	  
 * @param[out] None  
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to remove a task from event wait list,and	 
 *             then insert it to the ready list.
 *******************************************************************************
 */
void RemoveEventWaittingList(P_OSTCB ptcb)
{
    P_ECB pecb;
    pecb = &EventTbl[ptcb->eventID];    /* Get event control block            */
    
    /* Is there only one item in event waiting list?                          */
    if((ptcb->waitNext == NULL) && (ptcb->waitPrev == NULL))
    {
        pecb->eventTCBList = NULL;      /* Yes,set event waiting list as NULL */
    }
    else if(ptcb->waitPrev == NULL)/* Is the first item in event waiting list?*/
    {
        /* Yes,remove task from list,and reset event waiting list             */
        ptcb->waitNext->waitPrev = NULL;
		pecb->eventTCBList = ptcb->waitNext;	
        ptcb->waitNext = NULL;		
    }
    else if(ptcb->waitNext == NULL)/* Is the last item in event waiting list? */
    {
        ptcb->waitPrev->waitNext = NULL;  /* Yes,remove task form list        */
        ptcb->waitPrev = NULL;
    }
    else                                  /* No, remove task from list        */
    {										
        ptcb->waitPrev->waitNext = ptcb->waitNext;
        ptcb->waitNext->waitPrev = ptcb->waitPrev;
        ptcb->waitPrev = NULL;
        ptcb->waitNext = NULL;		
    }
    ptcb->eventID  = INVALID_ID;          /* Sign that not to use.            */
}

#endif	 //CFG_EVENT_EN

// Flag.c

/**
 *******************************************************************************
 * @file       flag.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Flag management implementation code of coocox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if CFG_FLAG_EN > 0
/*---------------------------- Variable Define -------------------------------*/
#define FLAG_MAX_NUM  32                /*!< Define max flag number.          */
FCB     FlagCrl = {0};                  /*!< Flags list struct                */


/*---------------------------- Function Declare ------------------------------*/
static  void FlagBlock(P_FLAG_NODE pnode,U32 flags,U8 waitType);
static  P_FLAG_NODE RemoveFromLink(P_FLAG_NODE pnode);

/**
 *******************************************************************************
 * @brief      Create a flag	 
 * @param[in]  bAutoReset      Reset mode,TRUE(Auto Reset)  FLASE(Manual Reset).
 * @param[in]  bInitialState   Initial state.	 
 * @param[out] None  
 * @retval     E_CREATE_FAIL   Create flag fail.
 * @retval     others          Create flag successful.			 
 *
 * @par Description
 * @details    This function use to create a event flag.	 
 * @note 
 *******************************************************************************
 */
OS_FlagID CoCreateFlag(BOOL bAutoReset,BOOL bInitialState)
{
    U8  i;
    OsSchedLock();
    
    for(i=0;i<FLAG_MAX_NUM;i++)
    {
        /* Assign a free flag control block                                   */
        if((FlagCrl.flagActive&(1<<i)) == 0 )
        {
            FlagCrl.flagActive |= (1<<i);         /* Initialize active flag   */
            FlagCrl.flagRdy    |= (bInitialState<<i);/* Initialize ready flag */
            FlagCrl.resetOpt   |= (bAutoReset<<i);/* Initialize reset option  */
            OsSchedUnlock();
            return i ;                  /* Return Flag ID                     */
        }	
    }
    OsSchedUnlock();
    
    return E_CREATE_FAIL;               /* There is no free flag control block*/	
}


/**
 *******************************************************************************
 * @brief      Delete a flag
 * @param[in]  id      Flag ID. 	
 * @param[in]  opt     Delete option. 
 * @param[out] None          
 * @retval     E_CALL            Error call in ISR.
 * @retval     E_INVALID_ID      Invalid event ID.
 * @retval     E_TASK_WAITTING   Tasks waitting for the event,delete fail.
 * @retval     E_OK              Event deleted successful.   
 *
 * @par Description
 * @details    This function is called to delete a event flag.
 * @note 
 *******************************************************************************
 */
StatusType CoDelFlag(OS_FlagID id,U8 opt)
{
    P_FLAG_NODE pnode;
    P_FCB pfcb;
    pfcb  = &FlagCrl;
    if(OSIntNesting > 0)                /* If be called from ISR              */
    {
        return E_CALL;
    }
#if CFG_PAR_CHECKOUT_EN >0
    if((pfcb->flagActive&(1<<id)) == 0) /* Flag is valid or not               */
    {
        return E_INVALID_ID;	
    }
#endif
    OsSchedLock();
    pnode = pfcb->headNode;
    
    while(pnode != NULL)                /* Ready all tasks waiting for flags  */
    {
        if((pnode->waitFlags&(1<<id)) != 0) /* If no task is waiting on flags */
    	  {
            if(opt == OPT_DEL_NO_PEND)      /* Delete flag if no task waiting */
            {
              	OsSchedUnlock();
               	return E_TASK_WAITING;
            }
            else if (opt == OPT_DEL_ANYWAY) /* Always delete the flag         */
            {
                if(pnode->waitType == OPT_WAIT_ALL)
                {
                    /* If the flag is only required by NODE                   */
                    if( pnode->waitFlags == (1<<id) )	
                    {
                        /* Remove the NODE from waiting list                  */
                        pnode = RemoveFromLink(pnode); 	
                        continue;	
                    }	
                    else
                    {
                        pnode->waitFlags &= ~(1<<id);   /* Update waitflags   */
                    }		
                }
                else   							
                {
                    pnode = RemoveFromLink(pnode);
                    continue;	
                }	
            }
        }	
        pnode = pnode->nextNode;		
    }
    
    /* Remove the flag from the flags list */
    pfcb->flagActive &= ~(1<<id);			
    pfcb->flagRdy    &= ~(1<<id);
    pfcb->resetOpt   &= ~(1<<id);
    OsSchedUnlock();
    return E_OK;
}


/**
 *******************************************************************************
 * @brief      AcceptSingleFlag  
 * @param[in]  id     Flag ID.
 * @param[out] None
 * @retval     E_INVALID_ID      Invalid event ID.
 * @retval     E_FLAG_NOT_READY  Flag is not in ready state.
 * @retval     E_OK              The call was successful and your task owns the Flag.
 *
 * @par Description
 * @details    This fucntion is called to accept single flag
 * @note 
 *******************************************************************************
 */
StatusType CoAcceptSingleFlag(OS_FlagID id)
{
    P_FCB pfcb;
    pfcb  = &FlagCrl;
#if CFG_PAR_CHECKOUT_EN >0	
    if(id >= FLAG_MAX_NUM)              
    {
        return E_INVALID_ID;            /* Invalid 'id',return error          */
    }
    if((pfcb->flagActive&(1<<id)) == 0) 
    {
        return E_INVALID_ID;            /* Flag is deactive,return error      */
    }	
#endif
    if((pfcb->flagRdy&(1<<id)) != 0)    /* If the required flag is set        */
    {
        OsSchedLock()
        pfcb->flagRdy &= ~((FlagCrl.resetOpt)&(1<<id)); /* Clear the flag     */
        OsSchedUnlock();
        return E_OK;
    }
    else                                /* If the required flag is not set    */
    {
        return E_FLAG_NOT_READY;
    }
}


/**
 *******************************************************************************
 * @brief      AcceptMultipleFlags 
 * @param[in]  flags      Flags that waiting to active task.
 * @param[in]  waitType   Flags wait type.
 * @param[out] perr       A pointer to error code.
 * @retval     0
 * @retval     springFlag
 *
 * @par Description
 * @details    This fucntion is called to accept multiple flags. 
 * @note 
 *******************************************************************************
 */
U32 CoAcceptMultipleFlags(U32 flags,U8 waitType,StatusType *perr)
{
    U32  springFlag;
    P_FCB pfcb;
    pfcb  = &FlagCrl;
    
#if CFG_PAR_CHECKOUT_EN >0	
    if((flags&pfcb->flagActive) != flags )  /* Judge flag is active or not?   */    
    {
        *perr = E_INVALID_PARAMETER;        /* Invalid flags                  */
        return 0;
    }
#endif
    
    springFlag = flags & pfcb->flagRdy;
    
    OsSchedLock();
    /* If any required flags are set */
    if( (springFlag != 0) && (waitType == OPT_WAIT_ANY) )	
    {
        
        pfcb->flagRdy &= ~(springFlag & pfcb->resetOpt);  /* Clear the flags  */
        OsSchedUnlock();
        *perr = E_OK;
        return springFlag;
    }
    
    /* If all required flags are set */
    if((springFlag == flags) && (waitType == OPT_WAIT_ALL))
    {
        pfcb->flagRdy &= ~(springFlag&pfcb->resetOpt);    /* Clear the flags  */
        OsSchedUnlock();	
        *perr = E_OK;					
        return springFlag;		 	
    }
    OsSchedUnlock();
    *perr = E_FLAG_NOT_READY;		
    return 0;
}




/**
 *******************************************************************************
 * @brief      WaitForSingleFlag 
 * @param[in]  id        Flag ID.
 * @param[in]  timeout   The longest time for writting flag.
 * @param[out] None   
 * @retval     E_CALL         Error call in ISR.   
 * @retval     E_INVALID_ID   Invalid event ID.	
 * @retval     E_TIMEOUT      Flag wasn't received within 'timeout' time.
 * @retval     E_OK           The call was successful and your task owns the Flag,
 *                            or the event you are waiting for occurred.	 
 *
 * @par Description
 * @details    This function is called to wait for only one flag,
 *             (1) if parameter "timeout" == 0,waiting until flag be set;
 *             (2) when "timeout" != 0,if flag was set or wasn't set but timeout 
 *                 occured,the task will exit the waiting list,convert to READY 
 *                 or RUNNING state.  
 * @note 
 *******************************************************************************
 */
StatusType CoWaitForSingleFlag(OS_FlagID id,U32 timeout)
{
    FLAG_NODE flagNode;
    P_FCB     pfcb;
    P_OSTCB   curTCB;
    
    if(OSIntNesting > 0)                /* See if the caller is ISR           */
    {
        return E_CALL;
    }
    if(OSSchedLock != 0)                /* Schedule is lock?                  */
    {								 
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	
    
#if CFG_PAR_CHECKOUT_EN >0	
    if(id >= FLAG_MAX_NUM)              /* Judge id is valid or not?          */  
    {
        return E_INVALID_ID;            /* Invalid 'id'                       */      	
    }
    if((FlagCrl.flagActive&(1<<id)) == 0 )/* Judge flag is active or not?       */
    {
        return E_INVALID_ID;            /* Flag is deactive ,return error     */
    }	
#endif

   	OsSchedLock();
	pfcb = &FlagCrl;
    /* See if the required flag is set */
    if((pfcb->flagRdy&(1<<id)) != 0)    /* If the required flag is set        */
    {
        pfcb->flagRdy &= ~((pfcb->resetOpt&(1<<id))); /* Clear the flag       */
        OsSchedUnlock();
    }
    else                                /* If the required flag is not set    */
    {
        curTCB = TCBRunning;
        if(timeout == 0)                /* If time-out is not configured      */
        {
            /* Block task until the required flag is set                      */
            FlagBlock (&flagNode,(1<<id),OPT_WAIT_ONE);  
            curTCB->state  = TASK_WAITING;	
			TaskSchedReq   = TRUE;
            OsSchedUnlock();
            
            /* The required flag is set and the task is in running state      */
            curTCB->pnode  = NULL;					   		
            OsSchedLock();
            
            /* Clear the required flag or not                                 */	
            pfcb->flagRdy &= ~((1<<id)&(pfcb->resetOpt)); 
            OsSchedUnlock();
        }
        else                            /* If time-out is configured          */
        {
            /* Block task until the required flag is set or time-out occurs   */
            FlagBlock(&flagNode,(1<<id),OPT_WAIT_ONE);
            InsertDelayList(curTCB,timeout);
            
            OsSchedUnlock();
            if(curTCB->pnode == NULL)     /* If time-out occurred             */
            {
                return E_TIMEOUT;		
            }
            else                          /* If flag is set                   */
            {
                curTCB->pnode = NULL;
                OsSchedLock();
                
                /* Clear the required flag or not                             */
                pfcb->flagRdy &= ~((1<<id)&(pfcb->resetOpt));	 
                OsSchedUnlock();
            }	
        }
    }
    return E_OK;	
}


/**
 *******************************************************************************
 * @brief      WaitForMultipleFlags 
 * @param[in]  flags      Flags that waiting to active task.
 * @param[in]  waitType   Flags wait type.
 * @param[in]  timeout    The longest time for writting flag.
 * @param[out] perr       A pointer to error code.
 * @retval     0
 * @retval     springFlag	 
 *
 * @par Description
 * @details    This function is called to pend a task for waitting multiple flag. 
 * @note 
 *******************************************************************************
 */
U32 CoWaitForMultipleFlags(U32 flags,U8 waitType,U32 timeout,StatusType *perr)
{
    U32       springFlag;  	
    P_FCB     pfcb;
    FLAG_NODE flagNode;
    P_OSTCB   curTCB;
    
   
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        *perr = E_CALL;
        return 0;
    }
    if(OSSchedLock != 0)                /* Schedule is lock?                  */
    {	
        *perr = E_OS_IN_LOCK;							 
        return 0;                       /* Yes,error return                   */
    }
#if CFG_PAR_CHECKOUT_EN >0  
    if( (flags&FlagCrl.flagActive) != flags )
    {
        *perr = E_INVALID_PARAMETER;    /* Invalid 'flags'                    */
        return 0;
    }
#endif
    OsSchedLock();
	pfcb = &FlagCrl;
    springFlag = flags & pfcb->flagRdy;
    
    /* If any required flags are set  */
    if((springFlag != 0) && (waitType == OPT_WAIT_ANY))
    {
        pfcb->flagRdy &= ~(springFlag & pfcb->resetOpt);  /* Clear the flag   */
        OsSchedUnlock();
        *perr = E_OK;
        return springFlag;
    }
    
    /* If all required flags are set */
    if( (springFlag == flags) && (waitType == OPT_WAIT_ALL) )  
    {
        pfcb->flagRdy &= ~(springFlag & pfcb->resetOpt);  /* Clear the flags  */
        OsSchedUnlock();	
        *perr = E_OK;
        return springFlag;		 	
    }
    
    curTCB = TCBRunning;
    if(timeout == 0)                    /* If time-out is not configured      */
    {
        /* Block task until the required flag are set                         */
        FlagBlock(&flagNode,flags,waitType);
        curTCB->state  = TASK_WAITING;	
		TaskSchedReq   = TRUE;
		OsSchedUnlock();
        
        curTCB->pnode  = NULL;
        OsSchedLock();			 	
        springFlag     = flags & pfcb->flagRdy;		
        pfcb->flagRdy &= ~(springFlag & pfcb->resetOpt);/* Clear the flags    */	
        OsSchedUnlock();
        *perr = E_OK;
        return springFlag;
    }
    else                                /* If time-out is configured          */
    {
        /* Block task until the required flag are set or time-out occurred    */
        FlagBlock(&flagNode,flags,waitType);
        InsertDelayList(curTCB,timeout);
        
        OsSchedUnlock();
        if(curTCB->pnode == NULL)       /* If time-out occurred               */
        {
            *perr = E_TIMEOUT;
            return 0;	
        }
        else                            /* If the required flags are set      */
        {
            curTCB->pnode = NULL;
            OsSchedLock();
            springFlag    = flags & FlagCrl.flagRdy;
            
            /* Clear the required ready flags or not */
            pfcb->flagRdy &= ~(springFlag & pfcb->resetOpt);	
            OsSchedUnlock();
            *perr = E_OK;
            return springFlag;	
        }	
    }	
}


/**
 *******************************************************************************
 * @brief       Clear a Flag	 
 * @param[in]   id     Flag ID.
 * @param[out]  None
 * @retval      E_OK           Event deleted successful. 	 
 * @retval      E_INVALID_ID   Invalid event ID. 	 
 *
 * @par Description
 * @details     This function is called to clear a flag. 
 *
 * @note 
 *******************************************************************************
 */
StatusType CoClearFlag(OS_FlagID id)
{
    P_FCB pfcb;
    pfcb = &FlagCrl;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= FLAG_MAX_NUM)                  
    {
        return E_INVALID_ID;                /* Invalid id                     */	
    }
    if((pfcb->flagActive&(1<<id)) == 0)     
    {
        return E_INVALID_ID;                /* Invalid flag                   */
    }
#endif

    pfcb->flagRdy &= ~(1<<id);              /* Clear the flag                 */
    return E_OK;
}


/**
 *******************************************************************************
 * @brief      Set a flag	   
 * @param[in]  id     Flag ID.
 * @param[out] None
 * @retval     E_INVALID_ID   Invalid event ID.
 * @retval     E_OK           Event deleted successful. 	 
 *
 * @par Description
 * @details    This function is called to set a flag. 
 * @note 
 *******************************************************************************
 */
StatusType CoSetFlag(OS_FlagID id)
{
    P_FLAG_NODE pnode;
    P_FCB pfcb;
    pfcb  = &FlagCrl;
    
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= FLAG_MAX_NUM)              /* Flag is valid or not               */							
    {
        return E_INVALID_ID;            /* Invalid flag id                    */      	
    }
    if((pfcb->flagActive&(1<<id)) == 0)  
    {
        return E_INVALID_ID;            /* Flag is not exist                  */
    }
#endif
    
    if((pfcb->flagRdy&(1<<id)) != 0)    /* Flag had already been set          */
    {
    	return E_OK;
    }
    
    pfcb->flagRdy |= (1<<id);           /* Update the flags ready list        */
    
    OsSchedLock();
    pnode = pfcb->headNode;	  		
    while(pnode != NULL)
    {
        if(pnode->waitType == OPT_WAIT_ALL)   /* Extract all the bits we want */
      	{			
            if((pnode->waitFlags&pfcb->flagRdy) == pnode->waitFlags)
            {
               /* Remove the flag node from the wait list                    */
                pnode = RemoveFromLink(pnode);		
                if((pfcb->resetOpt&(1<<id)) != 0)/* If the flags is auto-reset*/	
                {
                    break;							
                }
                continue;	
            }	
      	}
        else                           /* Extract only the bits we want       */	
      	{
            if( (pnode->waitFlags & pfcb->flagRdy) != 0)
            {
                /* Remove the flag node from the wait list                    */
                pnode = RemoveFromLink(pnode);	 	
                if((pfcb->resetOpt&(1<<id)) != 0)
                {
                    break;              /* The flags is auto-reset            */	
                }
                continue;
            }	
      	}
      	pnode = pnode->nextNode;					
    }
    OsSchedUnlock();
    return E_OK;
}



/**
 *******************************************************************************
 * @brief      Set a flag	in ISR 
 * @param[in]  id     Flag ID.
 * @param[out] None 
 * @retval     E_INVALID_ID   Invalid event ID.
 * @retval     E_OK           Event deleted successful. 	 
 *
 * @par Description
 * @details    This function is called in ISR to set a flag. 
 * @note 
 *******************************************************************************
 */
#if CFG_MAX_SERVICE_REQUEST > 0
StatusType isr_SetFlag(OS_FlagID id)
{
    if(OSSchedLock > 0)         /* If scheduler is locked,(the caller is ISR) */
    {
        /* Insert the request into service request queue                      */
        if(InsertInSRQ(FLAG_REQ,id,NULL) == FALSE)	
        {
            return E_SEV_REQ_FULL;      /* The service requst queue is full   */
        }			
        else
        {
            return E_OK;   							
        }
    }
    else
    {
        return(CoSetFlag(id));          /* The caller is not ISR, set the flag*/
    }
}
#endif

/**
 *******************************************************************************
 * @brief      Block a task to wait a flag event  
 * @param[in]  pnode       A node that will link into flag waiting list.
 * @param[in]  flags       Flag(s) that the node waiting for.
 * @param[in]  waitType    Waiting type of the node.
 * @param[out] None	 
 * @retval     None
 *
 * @par Description
 * @details    This function is called to block a task to wait a flag event.	 
 * @note 
 *******************************************************************************
 */
static void FlagBlock(P_FLAG_NODE pnode,U32 flags,U8 waitType)
{
    P_FCB     pfcb;
    pfcb  = &FlagCrl;
    
    TCBRunning->pnode = pnode;	
    pnode->waitTask   = TCBRunning;
    pnode->waitFlags  = flags;      /* Save the flags that we need to wait for*/
    pnode->waitType   = waitType;   /* Save the type of wait                  */
        
    if(pfcb->tailNode == NULL)      /* If this is the first NODE to insert?   */
    {
        pnode->nextNode = NULL;
        pnode->prevNode = NULL;
        pfcb->headNode  = pnode;    /* Insert the NODE to the head            */	
    }
    else                            /* If it is not the first NODE to insert? */
    {
        pfcb->tailNode->nextNode = pnode;   /* Insert the NODE to the tail    */
        pnode->prevNode          = pfcb->tailNode;
        pnode->nextNode          = NULL;
    }
    pfcb->tailNode = pnode;
}


/**
 *******************************************************************************
 * @brief      Remove a flag node from list
 * @param[in]  pnode    A node that will remove from flag waiting list.
 * @param[out] None   
 * @retval     pnode    Next node of the node that have removed out.
 *
 * @par Description
 * @details   This function is called to remove a flag node from the wait list.			 
 * @note 
 *******************************************************************************
 */
static P_FLAG_NODE RemoveFromLink(P_FLAG_NODE pnode)
{
    P_OSTCB ptcb;
    
    RemoveLinkNode(pnode);            /* Remove the flag node from wait list. */			 
    ptcb = pnode->waitTask;
    
    /* The task in the delay list */
    if(ptcb->delayTick != INVALID_VALUE)/* If the task is in tick delay list  */			         
    {
        RemoveDelayList(ptcb);        /* Remove the task from tick delay list */	
    }
	
	ptcb->pnode = (void*)0xffffffff;

	if(ptcb == TCBRunning)
	{
		ptcb->state = TASK_RUNNING;
	} 
	else
	{
		InsertToTCBRdyList(ptcb);         /* Insert the task to ready list        */	
	}   
    return (pnode->nextNode);	
}

/**
 *******************************************************************************
 * @brief      Remove a flag node from list  
 * @param[in]  pnode 	A node that will remove from flag waiting list.
 * @param[out] None   
 * @retval     None		
 *
 * @par Description
 * @details    This function is called to remove a flag node from the wait list.			 
 * @note 
 *******************************************************************************
 */
void RemoveLinkNode(P_FLAG_NODE pnode)
{
    /* If only one NODE in the list*/
    if((pnode->nextNode == NULL) && (pnode->prevNode == NULL)) 
    {
        FlagCrl.headNode = NULL;
        FlagCrl.tailNode = NULL;
    }
    else if(pnode->nextNode == NULL)      /* If the NODE is tail              */
    {
        FlagCrl.tailNode          = pnode->prevNode;
        pnode->prevNode->nextNode = NULL;
    }
    else if(pnode->prevNode == NULL)      /* If the NODE is head              */
    {
        FlagCrl.headNode          = pnode->nextNode;
        pnode->nextNode->prevNode = NULL;	
    }
    else                                  /* The NODE is in the middle        */
    {
        pnode->nextNode->prevNode = pnode->prevNode;
        pnode->prevNode->nextNode = pnode->nextNode;
    }
    pnode->waitTask->pnode = NULL;
}

#endif

// Hook.c

/**
 *******************************************************************************
 * @file       hook.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      hook management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 



/**
 *******************************************************************************
 * @brief      IDLE task of OS	 
 * @param[in]  pdata	The parameter passed to IDLE task.		 
 * @param[out] None 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is system IDLE task code.	 
 *******************************************************************************
 */
extern volatile uint32_t IdleCount ;

void CoIdleTask(void* pdata)
{
	uint16_t i ;
	static uint16_t last = 0 ;
  /* Add your codes here */
  for(; ;) 
  {
#ifdef PCBSKY
		i = TC1->TC_CHANNEL[0].TC_CV ;
#endif
#if defined(PCBX9D) || defined(PCB9XT) || defined(PCBX12D) || defined(PCBX10) || defined(PCBLEM1)
		i = TIM7->CNT ;
#endif
      /* Add your codes here */
		if ( i != last )
		{
			last = i ;
			IdleCount += 1 ;
		}
	// Toggle bits BACKLIGHT and EXT1, Backlight now on PWM
//#ifdef PCBSKY
//#define wdt_reset()	(WDT->WDT_CR = 0xA5000001)
//#endif
//#ifdef PCBX9D
//#define wdt_reset()	(IWDG->KR = 0x0000AAAAL)
//#endif
		
//    wdt_reset();
//		i += 1 ;
  }
}


/**
 *******************************************************************************
 * @brief      Hook for stack overflow	 
 * @param[in]  taskID	Piont to the task which lead to stack overflow.		 
 * @param[out] None 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is a hook for stack overflow.	 
 *******************************************************************************
 */
void CoStkOverflowHook(OS_TID taskID)
{
    /* Process stack overflow  here */
    for(; ;) 
    {
      
    }
}

// Kernelheap.c

/**
 *******************************************************************************
 * @file       kernelHeap.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      kernel heap management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if CFG_KHEAP_EN >0
/*---------------------------- Variable Define -------------------------------*/
U32     KernelHeap[KHEAP_SIZE] = {0};   /*!< Kernel heap                      */
P_FMB   FMBlist = NULL;                 /*!< Free memory block list           */
KHeap   Kheap   = {0};                  /*!< Kernel heap control              */


/*---------------------------- Function Declare ------------------------------*/
static P_FMB  GetPreFMB(P_UMB usedMB);
/**
 *******************************************************************************
 * @brief      Create kernel heap	 
 * @param[in]  None
 * @param[out] None
 * @retval     None			 
 *
 * @par Description
 * @details    This function is called to create kernel heap.
 *******************************************************************************
 */
void CoCreateKheap(void)
{
    Kheap.startAddr  = (U32)(KernelHeap); /* Initialize kernel heap control   */
    Kheap.endAddr    = (U32)(KernelHeap) + KHEAP_SIZE*4;
    FMBlist          = (P_FMB)KernelHeap; /* Initialize free memory block list*/
    FMBlist->nextFMB = NULL;	
    FMBlist->nextUMB = NULL;
    FMBlist->preUMB  = NULL;
}


/**
 *******************************************************************************
 * @brief      Allocation size bytes of memory block from kernel heap.
 * @param[in]  size     Length of menory block.	
 * @param[out] None
 * @retval     NULL     Allocate fail.
 * @retval     others   Pointer to memory block.		 
 *
 * @par Description
 * @details    This function is called to allocation size bytes of memory block.
 *******************************************************************************
 */
void* CoKmalloc(U32 size)
{
    P_FMB freeMB,newFMB,preFMB;
    P_UMB usedMB,tmpUMB;
    U8*   memAddr;
    U32   freeSize;
    U32   kheapAddr;
    
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if( size == 0 )
    {
        return NULL;
    }
#endif

    /* Word alignment,and add used memory head size */
    size      = (((size+3)>>2)<<2) + 8;
    kheapAddr = Kheap.endAddr;        /* Get the end address of kernel heap   */
    OsSchedLock();                    /* Lock schedule                        */
    freeMB = FMBlist;                 /* Get first item of free memory list   */
    preFMB = NULL;
    while(freeMB != NULL )            /* Is out of free memory list?          */
    {                                 /* No                                   */
        if(freeMB->nextUMB == NULL)   /* Is last item of free memory list?    */
        {                             /* Yes,get size for this free item      */
            freeSize = kheapAddr - (U32)(freeMB);
        }
        else                          /* No,get size for this free item       */
        {							   
            freeSize = (U32)(freeMB->nextUMB) -1 - (U32)(freeMB);	
        }
        if(freeSize >= size)        /* If the size equal or greater than need */
        {                           /* Yes,assign in this free memory         */
            usedMB=(P_UMB)freeMB;/* Get the address for used memory block head*/
            
            /* Get the address for used memory block                          */
            memAddr = (U8*)((U32)(usedMB) + 8);	
            
            /* Is left size of free memory smaller than 12?                   */	 
            if((freeSize-size) < 12)	 
            {		
                /* Yes,malloc together(12 is the size of the header information 
                   of free memory block ).                                    */
                if(preFMB != NULL)/* Is first item of free memory block list? */
                {                             /* No,set the link for list     */
                    preFMB->nextFMB = freeMB->nextFMB;
                }
                else                          /* Yes,reset the first item     */
                {						
                    FMBlist = freeMB->nextFMB;		
                }
                
                if(freeMB->nextUMB != NULL)   /* Is last item?                */
                {                             /* No,set the link for list     */
                    tmpUMB = (P_UMB)((U32)(freeMB->nextUMB)-1); 
                    tmpUMB->preMB = (void*)((U32)usedMB|0x1);
                }
                
                usedMB->nextMB = freeMB->nextUMB;/* Set used memory block link*/
                usedMB->preMB  = freeMB->preUMB;
            }
            else                            /* No,the left size more than 12  */
            {		
                /* Get new free memory block address                          */
                newFMB = (P_FMB)((U32)(freeMB) + size);
                
                if(preFMB != NULL)/* Is first item of free memory block list? */ 
                {						
                    preFMB->nextFMB = newFMB; /* No,set the link for list     */		
                }	
                else
                {					    
                    FMBlist = newFMB;         /* Yes,reset the first item     */	
                }
                
                /* Set link for new free memory block                         */
                newFMB->preUMB  = (P_UMB)((U32)usedMB|0x1);
                newFMB->nextUMB = freeMB->nextUMB;
                newFMB->nextFMB = freeMB->nextFMB;
                
                if(freeMB->nextUMB != NULL) /* Is last item?                  */
                {                           /* No,set the link for list       */
                    tmpUMB = (P_UMB)((U32)(freeMB->nextUMB)-1); 
                    tmpUMB->preMB = newFMB;
                }
                
                usedMB->nextMB = newFMB;    /* Set used memory block link     */
                usedMB->preMB  = freeMB->preUMB;
            }
          
            if(freeMB->preUMB != NULL)      /* Is first item?                 */
            {                               /* No,set the link for list       */
                tmpUMB = (P_UMB)((U32)(freeMB->preUMB)-1); 
                tmpUMB->nextMB = (void*)((U32)usedMB|0x1);
            }
          
            OsSchedUnlock();              /* Unlock schedule                  */
            return memAddr;               /* Return used memory block address */
        }
        preFMB = freeMB;        /* Save current free memory block as previous */
        freeMB = freeMB->nextFMB;         /* Get the next item as current item*/
    }
    OsSchedUnlock();                      /* Unlock schedule                  */
    return NULL;                          /* Error return                     */
}


/**
 *******************************************************************************
 * @brief      Release memory block to kernel heap.  
 * @param[in]  memBuf    Pointer to memory block.
 * @param[out] None
 * @retval     None  		 
 *
 * @par Description
 * @details    This function is called to release memory block.
 *******************************************************************************
 */
void CoKfree(void* memBuf)
{
    P_FMB    curFMB,nextFMB,preFMB;
    P_UMB    usedMB,nextUMB,preUMB;

#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(memBuf == NULL)
    {
        return;
    }
#endif
    
    usedMB = (P_UMB)((U32)(memBuf)-8);
    
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if((U32)(memBuf) < Kheap.startAddr)
    {
        return;
    }
    if((U32)(memBuf) > Kheap.endAddr)
    {
        return;
    }
#endif
    
    
    OsSchedLock();                      /* Lock schedule                      */

#if CFG_PAR_CHECKOUT_EN >0              /* Check UMB in list                  */ 
    if((U32)(usedMB) < (U32)(FMBlist))
    {
        preUMB = (P_UMB)((U32)(FMBlist->preUMB)-1);
        while(preUMB != usedMB)	
        {
            if(preUMB == NULL)
            {
                OsSchedUnlock();
                return;
            }
            preUMB = (P_UMB)((U32)(preUMB->preMB)-1);	
        }
    }
    else
    {
        if(FMBlist == NULL)
        {
            nextUMB = (P_UMB)(Kheap.startAddr);	
        }
        else
        {
            if(FMBlist->nextUMB != NULL)
            {
                nextUMB = (P_UMB)((U32)(FMBlist->nextUMB)-1);	
            }
            else
            {
                nextUMB = NULL;	
            }
        }
    	
        while(nextUMB != usedMB)	
        {
            if(nextUMB == NULL)
            {
                OsSchedUnlock();
                return;
            }	
            if(((U32)(nextUMB->nextMB)&0x1) == 0)		
            {
                nextFMB = (P_FMB)(nextUMB->nextMB);
                nextUMB = (P_UMB)((U32)(nextFMB->nextUMB)-1);		
            }
            else
            {
                nextUMB = (P_UMB)((U32)(nextUMB->nextMB)-1);	
            }
        }		
    }
#endif
    
    
    /* Is between two free memory block? */	 
    if( (((U32)(usedMB->nextMB)&0x1) == 0) && (((U32)(usedMB->preMB)&0x1)==0) )	
    {                             /* Yes,is the only one item in kernel heap? */
        if((usedMB->nextMB == NULL) && (usedMB->preMB == NULL))
        {
            curFMB = (P_FMB)usedMB;       /* Yes,release this item            */
            curFMB->nextFMB = NULL;	
            curFMB->nextUMB = NULL;
            curFMB->preUMB  = NULL;	
            FMBlist = curFMB;	
        }
        else if(usedMB->preMB == NULL)    /* Is the first item in kernel heap */
        {		
            /* Yes,release this item,and set link for list                    */						
            curFMB  = (P_FMB)usedMB; 
            nextFMB = (P_FMB)usedMB->nextMB;
            
            curFMB->nextFMB = nextFMB->nextFMB;	
            curFMB->nextUMB = nextFMB->nextUMB;
            curFMB->preUMB  = NULL;
            FMBlist         = curFMB;
        }
        else if(usedMB->nextMB == NULL)   /* Is the last item in kernel heap  */
        {                      /* Yes,release this item,and set link for list */
            curFMB = (P_FMB)(usedMB->preMB);	
            curFMB->nextFMB = NULL;
            curFMB->nextUMB = NULL;
        }							    
        else                  /* All no,show this item between two normal FMB */
        {		
            /* release this item,and set link for list                        */						  
            nextFMB = (P_FMB)usedMB->nextMB;
            curFMB  = (P_FMB)(usedMB->preMB);	
            
            curFMB->nextFMB = nextFMB->nextFMB;
            curFMB->nextUMB = nextFMB->nextUMB;
        }
    }
    else if(((U32)(usedMB->preMB)&0x1) == 0)  /* Is between FMB and UMB?      */
    {								   
        if(usedMB->preMB == NULL)   /* Yes,is the first item in kernel heap?  */
        {
            /* Yes,release this item,and set link for list                    */
            curFMB          = (P_FMB)usedMB;      
            nextUMB         = (P_UMB)usedMB->nextMB;		
            curFMB->nextUMB = nextUMB;
            curFMB->preUMB  = NULL;
            curFMB->nextFMB = FMBlist;
            FMBlist         = curFMB;
        }
        else                    /* No,release this item,and set link for list */
        {							      
            curFMB          = (P_FMB)usedMB->preMB;
            nextUMB         = (P_UMB)usedMB->nextMB;
            curFMB->nextUMB = nextUMB;
        }
    
    }
    else if(((U32)(usedMB->nextMB)&0x1) == 0)   /* Is between UMB and FMB?    */
    {                                           /* Yes                        */
        preUMB = (P_UMB)(usedMB->preMB);        /* Get previous UMB           */
        curFMB = (P_FMB)(usedMB);               /* new FMB                    */
        preFMB = GetPreFMB(usedMB);             /* Get previous FMB           */
        if(preFMB == NULL)                      /* Is previous FMB==NULL?     */
        {	
            nextFMB = FMBlist;                  /* Yes,get next FMB           */ 
            FMBlist = curFMB;   /* Reset new FMB as the first item of FMB list*/
        }
        else
        {
            nextFMB = preFMB->nextFMB;          /* No,get next FMB            */
            preFMB->nextFMB  = curFMB;          /* Set link for FMB list      */
        }
        
        if(nextFMB == NULL)           /* Is new FMB as last item of FMB list? */
        {	
            curFMB->preUMB  = preUMB;           /* Yes,set link for list      */
            curFMB->nextUMB = NULL;
            curFMB->nextFMB = NULL;			
        }	
        else
        {
            curFMB->preUMB  = preUMB;           /* No,set link for list       */
            curFMB->nextUMB = nextFMB->nextUMB;
            curFMB->nextFMB = nextFMB->nextFMB;	
        }
    }
    else                                    /* All no,show UMB between two UMB*/
    {									  
        curFMB  = (P_FMB)(usedMB);          /* new FMB                        */
        preFMB  = GetPreFMB(usedMB);        /* Get previous FMB               */
        preUMB  = (P_UMB)(usedMB->preMB);   /* Get previous UMB               */
        nextUMB = (P_UMB)(usedMB->nextMB);  /* Get next UMB                   */
        
        if(preFMB == NULL )                 /* Is previous FMB==NULL?         */
        {
            nextFMB = FMBlist;              /* Yes,get next FMB               */
            FMBlist = curFMB;  /* Reset new FMB as the first item of FMB list */
      	}
      	else
      	{
            nextFMB = preFMB->nextFMB;      /* No,get next FMB                */
            preFMB->nextFMB = curFMB;       /* Set link for FMB list          */
      	}
      	
        curFMB->preUMB  = preUMB;           /* Set current FMB link for list  */
        curFMB->nextUMB = nextUMB;
        curFMB->nextFMB = nextFMB;
    }
    
    if(curFMB->preUMB != NULL)/* Is current FMB as first item in kernel heap? */
    {                         /* No,set link for list                         */
      	preUMB = (P_UMB)((U32)(curFMB->preUMB)-1); 
      	preUMB->nextMB = (void*)curFMB;
    }
    if(curFMB->nextUMB != NULL)/* Is current FMB as last item in kernel heap? */
    {                          /* No,set link for list                        */
      	nextUMB = (P_UMB)((U32)(curFMB->nextUMB)-1); 
      	nextUMB->preMB = (void*)curFMB;		
    }
    OsSchedUnlock();           /* Unlock schedule                             */
}


/**
 *******************************************************************************
 * @brief      Get previous free memory block pointer.  
 * @param[in]  usedMB    Current used memory block.
 * @param[out] None
 * @retval     Previous free memory block pointer.		 
 *
 * @par Description
 * @details    This function is called to get previous free memory block pointer.
 *******************************************************************************
 */
static P_FMB GetPreFMB(P_UMB usedMB)
{
    P_UMB preUMB;
    preUMB = usedMB;
    while(((U32)(preUMB->preMB)&0x1))   /* Is previous MB as FMB?             */
    {                                   /* No,get previous MB                 */
        preUMB = (P_UMB)((U32)(preUMB->preMB)-1);
    }	
    return (P_FMB)(preUMB->preMB);      /* Yes,return previous MB             */
}

#endif

// Mbox.c

/**
 *******************************************************************************
 * @file       mbox.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Mailbox management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if CFG_MAILBOX_EN > 0


/**
 *******************************************************************************
 * @brief      Create a mailbox	 
 * @param[in]  sortType     Mail box waiting list sort type.			 
 * @param[out] None  
 * @retval     E_CREATE_FAIL   Create mailbox fail.
 * @retval     others          Create mailbox successful.		 
 *
 * @par Description
 * @details    This function is called to create a mailbox. 
 * @note 
 *******************************************************************************
 */
OS_EventID CoCreateMbox(U8 sortType)
{
    P_ECB pecb;
    
    /* Create a mailbox type event control block                              */
    pecb = CreatEvent(EVENT_TYPE_MBOX,sortType,NULL);   
    if(pecb == NULL)                    /* If failed to create event block    */
    {
        return E_CREATE_FAIL;
    }
    pecb->eventCounter = 0;
    return (pecb->id);      /* Create a mailbox successfully, return event ID */		
}



/**
 *******************************************************************************
 * @brief      Delete a mailbox	   
 * @param[in]  id     Event ID.
 * @param[in]  opt    Delete option.	 
 * @param[out] None   	 
 * @retval     E_INVALID_ID         Invalid event ID.
 * @retval     E_INVALID_PARAMETER  Invalid parameter.
 * @retval     E_TASK_WAITTING      Tasks waitting for the event,delete fail.
 * @retval     E_OK                 Event deleted successful. 
 *
 * @par Description
 * @details    This function is called to delete a mailbox.	 
 * @note 
 *******************************************************************************
 */
StatusType CoDelMbox(OS_EventID id,U8 opt)
{
    P_ECB pecb;
    
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)               /* Judge id is valid or not?        */ 
    {
        return E_INVALID_ID;              /* Id is invalid ,return error      */
    }
#endif
    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_MBOX)/* Validate event control block type*/    
    {
        return E_INVALID_ID;              /* The event is not mailbox         */	
    }
#endif	
    return (DeleteEvent(pecb,opt)); /* Delete the mailbox event control block */
}



/**
 *******************************************************************************
 * @brief      Accept a mailbox	 
 * @param[in]  id    Event ID.  	 
 * @param[out] perr  A pointer to error code.  
 * @retval     NULL
 * @retval     A pointer to mailbox accepted.			 
 *
 * @par Description
 * @details    This function is called to accept a mailbox. 
 * @note 
 *******************************************************************************
 */
void* CoAcceptMail(OS_EventID id,StatusType* perr)
{
    P_ECB pecb;
    void* pmail;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	                
    {
        *perr = E_INVALID_ID;             /* Invalid 'id'                     */
        return NULL;
    }
#endif
    pecb = &EventTbl[id];
    
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_MBOX)/* Invalid event control block type */
    {
        *perr = E_INVALID_ID;	
        return NULL;
    }
#endif
	OsSchedLock();
    if(pecb->eventCounter == 1)             /* If there is already a message  */
    {
        *perr = E_OK;
        pmail = pecb->eventPtr;             /* Get the message                */
        pecb->eventPtr     = NULL;          /* Clear the mailbox              */
        pecb->eventCounter = 0;
		OsSchedUnlock();
        return pmail;                       /* Return the message received    */		
    }
    else                                    /* If the mailbox is empty        */
    {	
		OsSchedUnlock();
        *perr = E_MBOX_EMPTY;               /* Mailbox is empty,return NULL   */
        return NULL;	
    }
}



/**
 *******************************************************************************
 * @brief      Wait for a mailbox	 
 * @param[in]  id       Event ID.	 
 * @param[in]  timeout  The longest time for writting mail.	    
 * @param[out] perr     A pointer to error code.	  
 * @retval     NULL	
 * @retval     A pointer to mailbox accept.
 *
 * @par Description
 * @details    This function is called to wait a mailbox.	 
 * @note 
 *******************************************************************************
 */
void* CoPendMail(OS_EventID id,U32 timeout,StatusType* perr)
{
    P_ECB pecb;
    void* pmail;
    P_OSTCB  curTCB;
     
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        *perr = E_CALL;
        return NULL;
    }
    
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)              
    {
        *perr = E_INVALID_ID;           /* Invalid 'id',retrun error          */
        return NULL;
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_MBOX)
    {
        *perr = E_INVALID_ID;       /* Invalid event type,not EVENT_TYPE_MBOX */
        return NULL;
    }
#endif

    if(OSSchedLock != 0)                /* Judge schedule is locked or not?   */
    {	
        *perr = E_OS_IN_LOCK;           /* Schedule is locked                 */								 
        return NULL;                    /* return NULL                        */
    }	
    if( pecb->eventCounter == 1)        /* If there is already a message      */
    {
        *perr = E_OK;
        pmail = pecb->eventPtr;         /* Get the message                    */
        pecb->eventPtr     = NULL;      /* Clear the mailbox                  */
        pecb->eventCounter = 0;             
        return pmail;                   /* Return the message received        */
    }
    else                       /* If message is not available, task will pend */ 
    {
        curTCB = TCBRunning;
        if(timeout == 0)                /* If time-out is not configured      */
        {
            EventTaskToWait(pecb,curTCB); /* Block task until event occurs    */
            *perr = E_OK;
            
            /* Have recived a message or the mailbox have been deleted        */
            pmail = curTCB->pmail;          
            curTCB->pmail = NULL;
            return pmail;               /* Return received message or NULL    */
        }
        else                            /* If time-out is configured          */
        {
            OsSchedLock();
            
            /* Block task until event or timeout occurs                       */
            EventTaskToWait(pecb,curTCB);   
            InsertDelayList(curTCB,timeout);
            OsSchedUnlock();
            if( curTCB->pmail == NULL)  /* Time-out occurred                  */
            {
                *perr = E_TIMEOUT;
                return NULL;	
            }
            else    /* Have recived a message or the mailbox have been deleted*/
            {
                *perr = E_OK;
                pmail = curTCB->pmail;
                curTCB->pmail = NULL;
                return pmail;           /* Return received message or NULL    */	
            }			
        }	
    }
}

 
/**
 *******************************************************************************
 * @brief      Post a mailbox	  
 * @param[in]  id      Event ID.
 * @param[in]  pmail   Pointer to mail that want to send.		 
 * @param[out] None   
 * @retval     E_INVALID_ID	
 * @retval     E_OK		 
 *
 * @par Description
 * @details    This function is called to post a mail. 
 * @note 
 *******************************************************************************
 */
StatusType CoPostMail(OS_EventID id,void* pmail)
{
    P_ECB pecb;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	                
    {
        return E_INVALID_ID;            /* Invalid id,return error            */
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_MBOX)/* Validate event control block type*/
    {
        return E_INVALID_ID;              /* Event is not mailbox,return error*/
    }
#endif

    if(pecb->eventCounter == 0)   /* If mailbox doesn't already have a message*/	
    {
        OsSchedLock();
        pecb->eventPtr     = pmail;       /* Place message in mailbox         */
        pecb->eventCounter = 1;
        EventTaskToRdy(pecb);             /* Check waiting list               */
        OsSchedUnlock();
        return E_OK;	
    }
    else                          /* If there is already a message in mailbox */              
    {
        return E_MBOX_FULL;       /* Mailbox is full,and return "E_MBOX_FULL" */
    }
}

/**
 *******************************************************************************
 * @brief      Post a mailbox in ISR	    
 * @param[in]  id      Event ID.
 * @param[in]  pmail   Pointer to mail that want to send.		 
 * @param[out] None   
 * @retval     E_INVALID_ID	
 * @retval     E_OK		 
 *
 * @par Description
 * @details    This function is called to post a mail in ISR. 
 * @note 
 *******************************************************************************
 */
#if CFG_MAX_SERVICE_REQUEST > 0
StatusType isr_PostMail(OS_EventID id,void* pmail)
{
    if(OSSchedLock > 0)         /* If scheduler is locked,(the caller is ISR) */
    {
        /* Insert the request into service request queue                      */
        if(InsertInSRQ(MBOX_REQ,id,pmail) == FALSE)  
        {
            return E_SEV_REQ_FULL;        /* If service request queue is full */
        }			
        else                              /* Operate successfully             */
        {
            return E_OK;
        }
    }
    else
    {
        return(CoPostMail(id,pmail));     /* Sends the message to the mailbox */ 
    }
}
#endif

#endif

// Mm.c

/**
 *******************************************************************************
 * @file       mm.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      memory management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if  CFG_MM_EN > 0
/*---------------------------- Variable Define -------------------------------*/
MM    MemoryTbl[CFG_MAX_MM] = {{0}};/*!< Table which save memory control block. */
U32   MemoryIDVessel = 0;         /*!< Memory ID container.                   */

/**
 *******************************************************************************
 * @brief      Create a memory partition	 
 * @param[in]  memBuf       Specify memory partition head address.		 
 * @param[in]  blockSize    Specify memory block size.  
 * @param[in]  blockNum     Specify memory block number.
 * @param[out] None
 * @retval     E_CREATE_FAIL  Create memory partition fail.
 * @retval     others         Create memory partition successful.			 
 *
 * @par Description
 * @details    This function is called to create a memory partition.
 *******************************************************************************
 */
OS_MMID CoCreateMemPartition(U8* memBuf,U32 blockSize,U32 blockNum)
{
    U8        i,j;
    U8        *memory;
    P_MemBlk  memBlk;
    memory = memBuf;
	
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(memBuf == NULL)
    {
        return 	E_CREATE_FAIL;
    }
    if(blockSize == 0)
    {
        return 	E_CREATE_FAIL;	
    }
    if((blockSize&0x3) != 0)
    {
        return 	E_CREATE_FAIL;	
    }
    if(blockNum<=1)
    {
        return 	E_CREATE_FAIL;
    }
#endif

    OsSchedLock();                      /* Lock schedule                      */
    for(i = 0; i < CFG_MAX_MM; i++)
    {
        if((MemoryIDVessel & (1 << i)) == 0)  /* Is free memory ID?           */
        {
            MemoryIDVessel |= (1<<i);   /* Yes,assign ID to this memory block */
            OsSchedUnlock();            /* Unlock schedule                    */
            MemoryTbl[i].memAddr   = memory;/* Initialize memory control block*/
            MemoryTbl[i].freeBlock = memory;  	
            MemoryTbl[i].blockSize = blockSize;
            MemoryTbl[i].blockNum  = blockNum;
            memBlk  = (P_MemBlk)memory;     /* Bulid list in this memory block*/ 
            for(j=0;j<blockNum-1;j++)
            {
                memory = memory+blockSize;
                memBlk->nextBlock = (P_MemBlk)memory;
                memBlk = memBlk->nextBlock;
            }
            memBlk->nextBlock = NULL;
            return i;                   /* Return memory block ID             */
        }
    }
    OsSchedUnlock();                    /* Unlock schedule                    */
    return E_CREATE_FAIL;               /* Error return                       */
}


/**
 *******************************************************************************
 * @brief      Delete a memory partition	  
 * @param[in]  mmID     Specify	memory partition that want to delete.	
 * @param[out] None
 * @retval     E_INVALID_ID   The memory partition id passed was invalid,delete fail.
 * @retval     E_OK           Delete successful.			 
 *
 * @par Description
 * @details    This function is called to Delete a memory partition.
 *******************************************************************************
 */
StatusType CoDelMemoryPartition(OS_MMID mmID)
{
    P_MM  memCtl;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(mmID >= CFG_MAX_MM)
    {
        return E_INVALID_ID;
    }
    if( ((1<<mmID)&MemoryIDVessel) == 0)
    {
        return E_INVALID_ID;
    }
#endif	
    OsSchedLock();                      /* Lock schedule                      */
    memCtl = &MemoryTbl[mmID];          /* Release memory control block       */
    MemoryIDVessel &= ~(1<<mmID);
    OsSchedUnlock();                    /* Unlock schedule                    */
    
    memCtl->memAddr   = NULL;
    memCtl->freeBlock = NULL;	
    memCtl->blockSize = 0;
    memCtl->blockNum  = 0;	
    return E_OK;                        /* Return OK                          */
}


/**
 *******************************************************************************
 * @brief      Get free block number in a memory partition	  
 * @param[in]  mmID    Specify memory partition.	
 *
 * @param[out] E_INVALID_ID  Invalid ID was passed and get counter failure.	  
 * @param[out] E_OK          Get current counter successful.
 * @retval     fbNum         The number of free block.	
 *
 * @par Description
 * @details    This function is called to get free block number in a memory 
 *             partition.
 *******************************************************************************
 */
U32 CoGetFreeBlockNum(OS_MMID mmID,StatusType* perr)
{
    U32       fbNum;	
    P_MM      memCtl;
    P_MemBlk  memBlk;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(mmID >= CFG_MAX_MM)
    {
        *perr = E_INVALID_ID;
        return 0;
    }
    if( ((1<<mmID)&MemoryIDVessel) == 0)
    {
        *perr = E_INVALID_ID;           /* Invalid memory id,return 0         */
        return 0;
    }
#endif	
    memCtl = &MemoryTbl[mmID];
    OsSchedLock();                      /* Lock schedule                      */
    memBlk = (P_MemBlk)(memCtl->freeBlock);/* Get the free item in memory list*/
    fbNum  = 0;
    while(memBlk != NULL)               /* Get counter of free item           */
    {
        fbNum++;
        memBlk = memBlk->nextBlock;     /* Get next free iterm                */
    }
    OsSchedUnlock();                    /* Unlock schedul                     */
    *perr = E_OK;							   
    return fbNum;                       /* Return the counter of free item    */
}


/**
 *******************************************************************************
 * @brief      Get a memory buffer from memory partition	    
 * @param[in]  mmID     Specify	memory partition that want to assign buffer.	
 * @param[out] None
 * @retval     NULL     Assign buffer fail.
 * @retval     others   Assign buffer successful,and return the buffer pointer.	
 *		 
 * @par Description
 * @details    This function is called to Delete a memory partition.
 *******************************************************************************
 */
void* CoGetMemoryBuffer(OS_MMID mmID)
{
    P_MM      memCtl;
    P_MemBlk  memBlk;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(mmID >= CFG_MAX_MM)
    {
        return NULL;
    }
    if( ((1<<mmID)&MemoryIDVessel)  == 0)
    {
        return NULL;
    }
#endif
    memCtl = &MemoryTbl[mmID];	
    OsSchedLock();                      /* Lock schedule                      */	        
    if(memCtl->freeBlock == NULL )    /* Is there no free item in memory list */
    {
        OsSchedUnlock();                /* Unlock schedule                    */
        return NULL;                    /* Yes,error return                   */
    }
    memBlk = (P_MemBlk)memCtl->freeBlock;       /* Get free memory block      */
    memCtl->freeBlock = (U8*)memBlk->nextBlock; /* Reset the first free item  */
    OsSchedUnlock();                    /* Unlock schedule                    */
    return memBlk;                      /* Return free memory block address   */
}



/**
 *******************************************************************************
 * @brief      Free a memory buffer to memory partition	 
 * @param[in]  mmID    Specify	memory partition.
 * @param[in]  buf     Specify	memory buffer that want to free.	
 * @param[out] None
 * @retval     E_INVALID_ID          The memory partition id passed was invalid.
 * @retval     E_INVALID_PARAMETER   The parameter passed was invalid.	
 * @retval     E_OK                  Free successful.	 
 *
 * @par Description
 * @details    This function is called to Delete a memory partition.
 *******************************************************************************
 */
StatusType CoFreeMemoryBuffer(OS_MMID mmID,void* buf)
{
    P_MM      memCtl;
    P_MemBlk  memBlk;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(mmID >= CFG_MAX_MM)
    {
        return E_INVALID_ID;
    }
    if( ((1<<mmID)&MemoryIDVessel) == 0)
    {
        return E_INVALID_ID;
    }
    if(buf == NULL)
    {
        return E_INVALID_PARAMETER;
    }
#endif	

    memCtl = &MemoryTbl[mmID];
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if((U32)buf < (U32)(memCtl->memAddr))
    {
        return E_INVALID_PARAMETER;
    }
    if((U32)buf > (U32)(memCtl->memAddr + memCtl->blockSize*memCtl->blockNum))
    {
        return E_INVALID_PARAMETER;
    }
    if(((U32)buf - (U32)(memCtl->memAddr))%(memCtl->blockSize) != 0)
    {	
        return E_INVALID_PARAMETER;	
    }
#endif
    memBlk = (P_MemBlk)buf;             /* Reset the first free item          */
    OsSchedLock();
    memBlk->nextBlock = (P_MemBlk)memCtl->freeBlock;
    memCtl->freeBlock = buf;
    OsSchedUnlock();
    return E_OK;                        /* Return OK                          */
}

#endif

// Mutex.c

/**
 *******************************************************************************
 * @file       mutex.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Mutex management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 



/*---------------------------- Variable Define -------------------------------*/
#if CFG_MUTEX_EN > 0

OS_MutexID MutexFreeID = 0;               /*!< Point to next vliad mutex ID.  */
MUTEX      MutexTbl[CFG_MAX_MUTEX] = {{0}}; /*!< Mutex struct array             */
	


/**
 *******************************************************************************
 * @brief      Create a mutex	 
 * @param[in]  None	 	 
 * @param[out] None  
 * @retval     E_CREATE_FAIL  Create mutex fail.
 * @retval     others         Create mutex successful.		 
 *
 * @par Description					  
 * @details    This function is called to create a mutex. 
 * @note  		
 *******************************************************************************
 */
OS_MutexID CoCreateMutex(void)
{
    OS_MutexID id;
    P_MUTEX pMutex;
    OsSchedLock();
    
    /* Assign a free mutex control block */
    if(MutexFreeID < CFG_MAX_MUTEX )
    {
        id  = MutexFreeID++;
        OsSchedUnlock();
        pMutex = &MutexTbl[id];
        pMutex->hipriTaskID  = INVALID_ID;
        pMutex->originalPrio = 0xff;
        pMutex->mutexFlag    = MUTEX_FREE;  /* Mutex is free,not was occupied */
        pMutex->taskID       = INVALID_ID;
        pMutex->waittingList = NULL;
        return id;                      /* Return mutex ID                    */			
    }	
    
    OsSchedUnlock();	 
    return E_CREATE_FAIL;               /* No free mutex control block        */	
}



/**	
 *******************************************************************************		 	
 * @brief      Enter a critical area  
 * @param[in]  mutexID    Specify mutex. 	 
 * @param[out] None   
 * @retval     E_INVALID_ID  Invalid mutex id. 	
 * @retval     E_CALL        Error call in ISR.
 * @retval     E_OK          Enter critical area successful.
 *
 * @par Description
 * @details    This function is called when entering a critical area.	 
 * @note 
 *******************************************************************************
 */
StatusType CoEnterMutexSection(OS_MutexID mutexID)
{
    P_OSTCB ptcb,pCurTcb;
    P_MUTEX pMutex;

#if CFG_EVENT_EN >0
    P_ECB pecb;
#endif

    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        return E_CALL;
    }
    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {								 
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	

#if CFG_PAR_CHECKOUT_EN >0
    if(mutexID >= MutexFreeID)          /* Invalid 'mutexID'                  */
    {
        return E_INVALID_ID;	
    }
#endif

    OsSchedLock();
    pCurTcb = TCBRunning;
    pMutex  = &MutexTbl[mutexID];
    
    pCurTcb->mutexID = mutexID;
    if(pMutex->mutexFlag == MUTEX_FREE)       /* If mutex is available        */	 
    {
        pMutex->originalPrio = pCurTcb->prio; /* Save priority of owning task */   
        pMutex->taskID       = pCurTcb->taskID;   /* Acquire the resource     */
        pMutex->hipriTaskID  = pCurTcb->taskID;
        pMutex->mutexFlag    = MUTEX_OCCUPY;      /* Occupy the mutex resource*/
    }
    /* If the mutex resource had been occupied                                */
    else if(pMutex->mutexFlag == MUTEX_OCCUPY)	 	
    {	
		ptcb = &TCBTbl[pMutex->taskID];
        if(ptcb->prio > pCurTcb->prio)  /* Need to promote priority of owner? */
        {
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
			DeleteTaskPri(ptcb->prio);
			ActiveTaskPri(pCurTcb->prio);
#endif	
            ptcb->prio = pCurTcb->prio;	    /* Promote prio of owner          */
            
            /* Upgarde the highest priority about the mutex                   */
            pMutex->hipriTaskID	= pCurTcb->taskID;	
            if(ptcb->state == TASK_READY)   /* If the task is ready to run    */
            {
                RemoveFromTCBRdyList(ptcb); /* Remove the task from READY list*/
                InsertToTCBRdyList(ptcb);   /* Insert the task into READY list*/
            }
#if CFG_EVENT_EN >0
            /* If the task is waiting on a event                              */
            else if(ptcb->eventID != INVALID_ID) 
            {
                pecb = &EventTbl[ptcb->eventID];
                
                /* If the event waiting type is preemptive Priority           */
                if(pecb->eventSortType == EVENT_SORT_TYPE_PRIO)	
                {
                    /* Remove the task from event waiting list                */
                    RemoveEventWaittingList(ptcb);
                    
                    /* Insert the task into event waiting list                */ 	
                    EventTaskToWait(pecb,ptcb);		
                }	
            }
#endif	
        }
        
        pCurTcb->state   = TASK_WAITING;    /* Block current task             */
		TaskSchedReq     = TRUE;
        pCurTcb->TCBnext = NULL;
        pCurTcb->TCBprev = NULL;
        
        ptcb = pMutex->waittingList;
        if(ptcb == NULL)               /* If the event waiting list is empty  */
        {
            pMutex->waittingList = pCurTcb; /* Insert the task to head        */
        }
        else                        /* If the event waiting list is not empty */
        {            	
            while(ptcb->TCBnext != NULL)    /* Insert the task to tail        */
            {
                ptcb = ptcb->TCBnext;		
            }
            ptcb->TCBnext    = pCurTcb;
            pCurTcb->TCBprev = ptcb;
            pCurTcb->TCBnext = NULL;	
        }
    }
    OsSchedUnlock();
    return E_OK;			
}


/**
 *******************************************************************************
 * @brief      Leave from a critical area	 
 * @param[in]  mutexID 	Specify mutex id.	 
 * @param[out] None 
 * @retval     E_INVALID_ID  Invalid mutex id.
 * @retval     E_CALL        Error call in ISR.
 * @retval     E_OK          Exit a critical area successful.
 *
 * @par Description		 
 * @details    This function must be called when exiting from a critical area.	
 * @note 
 *******************************************************************************
 */
StatusType CoLeaveMutexSection(OS_MutexID mutexID)
{
    P_OSTCB ptcb;
    P_MUTEX pMutex;
    U8      prio;
    U8      taskID;
    
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        return E_CALL;
    }

#if CFG_PAR_CHECKOUT_EN >0
    if(mutexID >= MutexFreeID)
    {
        return E_INVALID_ID;            /* Invalid mutex id, return error     */
    }
#endif	
    OsSchedLock();
    pMutex = &MutexTbl[mutexID];        /* Obtain point of mutex control block*/   
    ptcb = &TCBTbl[pMutex->taskID];
	ptcb->mutexID = INVALID_ID;
	if(pMutex->waittingList == NULL)    /* If the mutex waiting list is empty */
    {
        pMutex->mutexFlag = MUTEX_FREE; /* The mutex resource is available    */
        pMutex->taskID    = INVALID_ID;
        OsSchedUnlock();
    }	
    else              /* If there is at least one task waitting for the mutex */
    { 
        taskID = pMutex->taskID;        /* Get task ID of mutex owner         */
        
                                /* we havn't promoted current task's priority */
        if(pMutex->hipriTaskID == taskID)   
        {
            ptcb = pMutex->waittingList;/* Point to mutex first waiting task  */		
            prio = ptcb->prio; 
            while(ptcb != NULL)         /* Find the highest priority task     */
            {
                if(ptcb->prio < prio)  		
                {
                    prio = ptcb->prio;
                    pMutex->hipriTaskID = ptcb->taskID;
                }
                ptcb = ptcb->TCBnext;					
            }
        }
        else                     /* we have promoted current task's priority  */
        {
			prio = TCBTbl[taskID].prio;
        }
        
        /* Reset the task priority */
		pMutex->taskID = INVALID_ID;	
		CoSetPriority(taskID,pMutex->originalPrio);
        
        /* Find first task in waiting list ready to run  */	
        ptcb                 = pMutex->waittingList; 		
        pMutex->waittingList = ptcb->TCBnext;	
        pMutex->originalPrio = ptcb->prio;
        pMutex->taskID       = ptcb->taskID;

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if(prio != ptcb->prio)
		{
			DeleteTaskPri(ptcb->prio);
			ActiveTaskPri(prio);			
		}
#endif	

        ptcb->prio           = prio;    /* Raise the task's priority          */       
        				   
        /* Insert the task which acquire the mutex into ready list.           */
        ptcb->TCBnext = NULL;
        ptcb->TCBprev = NULL;

		InsertToTCBRdyList(ptcb);     /* Insert the task into the READY list  */
        OsSchedUnlock();
    }
    return E_OK;			
}

/**
 *******************************************************************************
 * @brief      Remove a task from mutex waiting list	   
 * @param[in]  ptcb   TCB which will remove out.	 
 * @param[out] None 	 
 * @retval     None
 *
 * @par Description		 
 * @details   This function be called when delete a task.	
 * @note 
 *******************************************************************************
 */
void RemoveMutexList(P_OSTCB ptcb)
{
    U8 prio;
	OS_TID taskID;
    P_MUTEX pMutex;
    pMutex = &MutexTbl[ptcb->mutexID];
    
    /* If only one task waiting on mutex                                      */	
    if((ptcb->TCBnext ==NULL) && (ptcb->TCBprev == NULL)) 
    {
        pMutex->waittingList = NULL;     /* Waiting list is empty             */
    }
    else if(ptcb->TCBnext == NULL)  /* If the task is the last of waiting list*/
    {
        /* Remove task from mutex waiting list                                */
        ptcb->TCBprev->TCBnext = NULL;
        ptcb->TCBprev = NULL;		
    }	
    else if(ptcb->TCBprev ==  NULL)/* If the task is the first of waiting list*/	
    {
        /* Remove task from waiting list                                      */
        ptcb->TCBnext->TCBprev = NULL;
        ptcb->TCBnext = NULL;	
    }
    else                      /* If the task is in the middle of waiting list */
    {
        /* Remove task from wait list */
        ptcb->TCBnext->TCBprev = ptcb->TCBprev;
        ptcb->TCBprev->TCBnext = ptcb->TCBnext;
        ptcb->TCBprev          = NULL;
        ptcb->TCBnext          = NULL;	
    }
    
    ptcb->mutexID = INVALID_ID;
    
    /* If the task have highest priority in mutex waiting list                */	
    if(pMutex->hipriTaskID == ptcb->taskID)						
    {
        ptcb = pMutex->waittingList;
        prio = pMutex->originalPrio; 
        pMutex->hipriTaskID = pMutex->taskID;
        while(ptcb != NULL)           /* Find task ID of highest priority task*/					
        {
            if(ptcb->prio < prio)
            {
                prio = ptcb->prio;
                pMutex->hipriTaskID = ptcb->taskID;
            }
            ptcb = ptcb->TCBnext;			
        }
		taskID = pMutex->taskID;
		pMutex->taskID = INVALID_ID;
		CoSetPriority(taskID,prio);         /* Reset the mutex ower priority  */
		pMutex->taskID = taskID;
    }
}

#endif

// Queue.c

/**
 *******************************************************************************
 * @file       queue.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Queue management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

#if CFG_QUEUE_EN > 0										 
/*---------------------------- Variable Define -------------------------------*/
QCB   QueueTbl[CFG_MAX_QUEUE] = {{0}};    /*!< Queue control block table        */
U32   QueueIDVessel = 0;                /*!< Queue list mask                  */


 
/**
 *******************************************************************************
 * @brief      Create a queue	 
 * @param[in]  qStart    Pointer to mail pointer buffer.
 * @param[in]  size      The length of queue.
 * @param[in]  sortType  Mail queue waiting list sort type.
 * @param[out] None  
 * @retval     E_CREATE_FAIL  Create queue fail.
 * @retval     others         Create queue successful.
 *
 * @par Description
 * @details    This function is called to create a queue. 
 * @note 
 *******************************************************************************
 */			 		   
OS_EventID CoCreateQueue(void **qStart, U16 size ,U8 sortType)
{
    U8    i;  
    P_ECB pecb;

#if CFG_PAR_CHECKOUT_EN >0	
    if((qStart == NULL) || (size == 0)) 	
    {
        return E_CREATE_FAIL;
    }
#endif

    OsSchedLock();
    for(i = 0; i < CFG_MAX_QUEUE; i++)
    {
        /* Assign a free QUEUE control block                                  */
        if((QueueIDVessel & (1 << i)) == 0)	
        {
            QueueIDVessel |= (1<<i);		
            OsSchedUnlock();
            
            QueueTbl[i].qStart   = qStart;  /* Initialize the queue           */
            QueueTbl[i].id       = i;
            QueueTbl[i].head     = 0;
            QueueTbl[i].tail     = 0;
            QueueTbl[i].qMaxSize = size; 
            QueueTbl[i].qSize    = 0;
            
            /* Get a event control block and initial the event content        */
            pecb = CreatEvent(EVENT_TYPE_QUEUE,sortType,&QueueTbl[i]);
            
            if(pecb == NULL )       /* If there is no free EVENT control block*/
            {
                return E_CREATE_FAIL;
            }
            return (pecb->id);		
        }
    }
    
    OsSchedUnlock();
    return E_CREATE_FAIL;             /* There is no free QUEUE control block */	
}


/**
 *******************************************************************************
 * @brief      Delete a queue	  
 * @param[in]  id     Event ID. 	
 * @param[in]  opt    Delete option. 	 
 * @param[out] None   
 * @retval     E_INVALID_ID         Invalid event ID.
 * @retval     E_INVALID_PARAMETER  Invalid parameter.
 * @retval     E_TASK_WAITTING      Tasks waitting for the event,delete fail.
 * @retval     E_OK                 Event deleted successful. 
 *
 * @par Description
 * @details    This function is called to delete a queue. 
 * @note 
 *******************************************************************************
 */
StatusType CoDelQueue(OS_EventID id,U8 opt)
{
    P_ECB   pecb;
    P_QCB   pqcb;
    StatusType err;
#if CFG_PAR_CHECKOUT_EN >0      
    if(id >= CFG_MAX_EVENT)	                     
    {
        return E_INVALID_ID;            /* Invalid id,return error            */
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if( pecb->eventType != EVENT_TYPE_QUEUE)
    {
        return E_INVALID_ID;            /* The event is not queue,return error*/	
    }
#endif
    pqcb = (P_QCB)pecb->eventPtr;       /* Point at queue control block       */
    err  = DeleteEvent(pecb,opt);       /* Delete the event control block     */
    if(err == E_OK)                   /* If the event block have been deleted */
    {
        QueueIDVessel &= ~((U32)(1<<(pqcb->id)));   /* Update free queue list             */
        pqcb->qStart   = NULL;
		    pqcb->id       = 0;
        pqcb->head     = 0;
        pqcb->tail     = 0;
        pqcb->qMaxSize = 0;
        pqcb->qSize    = 0;
    }
    return err;	
}


 
/**
 *******************************************************************************
 * @brief      Accept a mail from queue   
 * @param[in]  id     Event ID.	 	 
 * @param[out] perr   A pointer to error code.  
 * @retval     NULL	
 * @retval     A pointer to mail accepted.
 *
 * @par Description
 * @details    This function is called to accept a mail from queue.
 * @note 
 *******************************************************************************
 */
void* CoAcceptQueueMail(OS_EventID id,StatusType* perr)
{
  P_ECB pecb;
  P_QCB pqcb;
  void* pmail;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)             
    {
        *perr = E_INVALID_ID;           /* Invalid id,return error            */
        return NULL;
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_QUEUE)/* Invalid event control block type*/          		
    {
        *perr = E_INVALID_ID;
        return NULL;	
    }
#endif	
    pqcb = (P_QCB)pecb->eventPtr;       /* Point at queue control block       */
	OsSchedLock();
    if(pqcb->qSize != 0)            /* If there are any messages in the queue */
    {
        /* Extract oldest message from the queue */
        pmail = *(pqcb->qStart + pqcb->head);  
        pqcb->head++;                   /* Update the queue head              */ 
        pqcb->qSize--;          /* Update the number of messages in the queue */  
        if(pqcb->head == pqcb->qMaxSize)
        {
            pqcb->head = 0;	
        }
		OsSchedUnlock();
        *perr = E_OK;
        return pmail;                   /* Return message received            */
    }
    else                                /* If there is no message in the queue*/
    {
		OsSchedUnlock();
        *perr = E_QUEUE_EMPTY;                 
        return NULL;                    /* Return NULL                        */ 
    }	
}



/**
 *******************************************************************************
 * @brief      Pend for a mail	 
 * @param[in]  id       Event ID.	 
 * @param[in]  timeout  The longest time for writting mail.	
 * @param[out] perr     A pointer to error code.   
 * @retval     NULL	
 * @retval     A pointer to mail accept.	 
 *
 * @par Description
 * @details    This function is called to wait for a mail.		   	
 * @note 
 *******************************************************************************
 */
void* CoPendQueueMail(OS_EventID id,U32 timeout,StatusType* perr)
{
    P_ECB   pecb;
    P_QCB   pqcb;
    P_OSTCB curTCB;
    void*   pmail;
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        *perr = E_CALL;
        return NULL;
    }
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	         
    {
        *perr = E_INVALID_ID;           /* Invalid event id,return error      */
        return NULL;
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_QUEUE) /* The event type is not queue    */
    {
        *perr = E_INVALID_ID;
        return NULL;	
    }
#endif	
    if(OSSchedLock != 0)                /* Judge schedule is locked or not?   */
    {	
        *perr = E_OS_IN_LOCK;           /* Schedule is locked,return error    */								 
        return NULL;          
    }	
    pqcb = (P_QCB)pecb->eventPtr;       /* Point at queue control block       */
	 
    if(pqcb->qSize != 0)            /* If there are any messages in the queue */
    {
        /* Extract oldest message from the queue                              */
        pmail = *(pqcb->qStart + pqcb->head);   
        pqcb->head++;                   /* Update the queue head              */ 
        pqcb->qSize--;          /* Update the number of messages in the queue */  
        if(pqcb->head == pqcb->qMaxSize)/* Check queue head                   */
        {
            pqcb->head = 0;	
        }
        *perr = E_OK;
        return pmail;                   /* Return message received            */
    }
    else                                /* If there is no message in the queue*/
    {
        curTCB = TCBRunning;
        if(timeout == 0)                /* If time-out is not configured      */
        {
            /* Block current task until the event occur                       */
            EventTaskToWait(pecb,curTCB); 
            
            /* Have recived message or the queue have been deleted            */
            pmail = curTCB->pmail;              
            curTCB->pmail = NULL;
            *perr = E_OK;
            return pmail;               /* Return message received or NULL    */
        }
        else                            /* If time-out is configured          */
        {
            OsSchedLock(); 
            
            /* Block current task until event or timeout occurs               */           
            EventTaskToWait(pecb,curTCB);       
            InsertDelayList(curTCB,timeout);
            OsSchedUnlock();
            if(curTCB->pmail == NULL)   /* If time-out occurred               */
            {
                *perr = E_TIMEOUT;
                return NULL;
            }
            else                        /* If event occured                   */
            {
                pmail = curTCB->pmail;
                curTCB->pmail = NULL;
                *perr = E_OK;
                return pmail;           /* Return message received or NULL    */
            }				
        }	
    }
}


 
/**
 *******************************************************************************
 * @brief      Post a mail to queue	   
 * @param[in]  id      Event ID.
 * @param[in]  pmail   Pointer to mail that want to send.	 	 
 * @param[out] None   
 * @retval     E_OK
 * @retval     E_INVALID_ID
 * @retval     E_QUEUE_FULL		 
 *
 * @par Description
 * @details    This function is called to post a mail to queue.
 * @note 
 *******************************************************************************
 */
StatusType CoPostQueueMail(OS_EventID id,void* pmail)
{	
    P_ECB pecb;
    P_QCB pqcb;
#if CFG_PAR_CHECKOUT_EN >0                     
    if(id >= CFG_MAX_EVENT)	
    {
        return E_INVALID_ID;          
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_QUEUE)   
    {
        return E_INVALID_ID;            /* The event type isn't queue,return  */	
    }	
#endif
    pqcb = (P_QCB)pecb->eventPtr;	
    if(pqcb->qSize == pqcb->qMaxSize)   /* If queue is full                   */
    {
        return E_QUEUE_FULL;
    }
    else                                /* If queue is not full               */
    {
        OsSchedLock();
        *(pqcb->qStart + pqcb->tail) = pmail;   /* Insert message into queue  */
        pqcb->tail++;                           /* Update queue tail          */
        pqcb->qSize++;          /* Update the number of messages in the queue */
        if(pqcb->tail == pqcb->qMaxSize)        /* Check queue tail           */   
        {
            pqcb->tail = 0;	
        }
        EventTaskToRdy(pecb);           /* Check the event waiting list       */
        OsSchedUnlock();
        return E_OK;
    }
}


/**
 *******************************************************************************
 * @brief      Post a mail to queue in ISR	 
 * @param[in]  id      Event ID.
 * @param[in]  pmail   Pointer to mail that want to send.	 	 
 * @param[out] None   
 * @retval     E_OK
 * @retval     E_INVALID_ID
 * @retval     E_QUEUE_FULL		 
 *
 * @par Description
 * @details    This function is called in ISR to post a mail to queue.
 * @note 				   
 *******************************************************************************
 */
#if CFG_MAX_SERVICE_REQUEST > 0
StatusType isr_PostQueueMail(OS_EventID id,void* pmail)
{
    if(OSSchedLock > 0)         /* If scheduler is locked,(the caller is ISR) */
    {
        /* Insert the request into service request queue                      */
        if(InsertInSRQ(QUEUE_REQ,id,pmail) == FALSE)   
        {
            return E_SEV_REQ_FULL;      /* If service request queue is full   */          
        }			
        else  /* If the request have been inserted into service request queue */
        {
            return E_OK;
        }
    }
    else                                /* The scheduler is unlocked          */
    {
        return(CoPostQueueMail(id,pmail));    /* Send the message to the queue*/ 
    }
}
#endif
							   	 
#endif

// Sem.c

/**
 *******************************************************************************
 * @file       sem.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Semaphore management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if CFG_SEM_EN >0

/**
 *******************************************************************************
 * @brief      Create a semaphore	  
 * @param[in]  initCnt   Semaphore valid counter.
 * @param[in]  maxCnt    Semaphore max initialize counter.
 * @param[in]  sortType  Semaphore sort type.		 
 * @param[out] None
 * @retval     E_CREATE_FAIL   Create semaphore fail.
 * @retval     others          Create semaphore successful.
 *
 * @par Description
 * @details    This function is called to create a semaphore. 
 *******************************************************************************
 */
OS_EventID CoCreateSem(U16 initCnt,U16 maxCnt,U8 sortType)
{
    P_ECB pecb;
#if CFG_PAR_CHECKOUT_EN >0
    if(initCnt > maxCnt)    
    {
        return E_CREATE_FAIL;           /* Invalid 'initCnt' or 'maxCnt'      */	
    }
    
    if ((sortType != EVENT_SORT_TYPE_FIFO) && (sortType != EVENT_SORT_TYPE_PRIO))
    {
        return E_CREATE_FAIL;           /* Illegal sort type,return error     */
    }
#endif	
    
    /* Create a semaphore type event control block                            */
    pecb = CreatEvent(EVENT_TYPE_SEM,sortType,NULL);
    if(pecb == NULL)                    /* If failed to create event block    */
    {
        return E_CREATE_FAIL;
    }
    pecb->eventCounter        = initCnt;/* Initialize event block             */
    pecb->initialEventCounter = maxCnt;
    return (pecb->id);                  /* Return event id                    */
}

 
/**
 *******************************************************************************
 * @brief      Delete a semaphore	   
 * @param[in]  id    Event ID which to be deleted.
 * @param[in]  opt   Delete option.	 
 * @arg        == OPT_DEL_ANYWAY    Delete semaphore always   
 * @arg        == OPT_DEL_NO_PEND	Delete semaphore only when no task pending on.
 * @param[out] None   
 * @retval     E_INVALID_ID         Invalid event ID.
 * @retval     E_INVALID_PARAMETER  Invalid parameter.
 * @retval     E_TASK_WAITTING      Tasks waitting for the event,delete fail.
 * @retval     E_OK                 Event deleted successful. 	 
 *
 * @par Description
 * @details    This function is called to delete a semaphore. 
 *
 * @note 
 *******************************************************************************
 */
StatusType CoDelSem(OS_EventID id,U8 opt)
{
    P_ECB pecb;

#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	                 
    {
        return E_INVALID_ID;
    }
#endif

    pecb = &EventTbl[id];

#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_SEM)  
    {
        return E_INVALID_ID;             /* The event type is not semaphore   */	
    }	
#endif

    return (DeleteEvent(pecb,opt));/* Delete the semaphore event control block*/
}


/**
 *******************************************************************************
 * @brief      Accept a semaphore without waitting 	  
 * @param[in]  id      Event ID   	 
 * @param[out] None  
 * @retval     E_INVALID_ID    Invalid event ID.
 * @retval     E_SEM_EMPTY     No semaphore exist.
 * @retval     E_OK            Get semaphore successful. 	
 *
 * @par Description
 * @details    This function is called accept a semaphore without waitting. 
 *******************************************************************************
 */
StatusType CoAcceptSem(OS_EventID id)
{
    P_ECB pecb;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	                 
    {
        return E_INVALID_ID;
    }
#endif

	pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if( pecb->eventType != EVENT_TYPE_SEM)   
    {
        return E_INVALID_ID;	
    }
#endif
	OsSchedLock();
    if(pecb->eventCounter > 0) /* If semaphore is positive,resource available */
    {	
		OsSchedUnlock();
        pecb->eventCounter--;         /* Decrement semaphore only if positive */
        return E_OK;	
    }
    else                                /* Resource is not available          */
    {	
		OsSchedUnlock();
        return E_SEM_EMPTY;
    }	
}

 
/**
 *******************************************************************************
 * @brief       wait for a semaphore	   
 * @param[in]   id       Event ID.	
 * @param[in]   timeout  The longest time for writting semaphore.
 * @para        0        
 * @para        0x1~0xff 	 
 * @param[out]  None  
 * @retval      E_CALL         Error call in ISR.   
 * @retval      E_INVALID_ID   Invalid event ID.	
 * @retval      E_TIMEOUT      Semaphore was not received within the specified 
 *                             'timeout' time.
 * @retval      E_OK           The call was successful and your task owns the 
 *                             resource,or the event you are waiting for occurred.	
 * 
 * @par Description
 * @details    This function is called to waits for a semaphore. 
 * @note       IF this function is called in ISR,nothing to do and return immediately.
 *******************************************************************************
 */
StatusType CoPendSem(OS_EventID id,U32 timeout)
{
    P_ECB 	 pecb;
    P_OSTCB  curTCB;
    if(OSIntNesting > 0)                /* If the caller is ISR               */
    {
        return E_CALL;
    }
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	            
    {
        return E_INVALID_ID;
    }
#endif

	  pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_SEM)     
    {
       return E_INVALID_ID;	
    }
#endif
    if(OSSchedLock != 0)                /* Schdule is locked?                 */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	
    if(pecb->eventCounter > 0) /* If semaphore is positive,resource available */       
    {	
        pecb->eventCounter--;         /* Decrement semaphore only if positive */
        return E_OK;	
    }
    else                                /* Resource is not available          */
    {
        curTCB = TCBRunning;
        if(timeout == 0)                /* If time-out is not configured      */
        {
            EventTaskToWait(pecb,curTCB); /* Block task until event occurs    */
            curTCB->pmail = NULL;           
            return E_OK;
        }
        else                            /* If time-out is configured          */
        {
            OsSchedLock();
            
            /* Block task until event or timeout occurs                       */
            EventTaskToWait(pecb,curTCB);
            InsertDelayList(curTCB,timeout);
            
            OsSchedUnlock();
            if (curTCB->pmail == NULL)  /* If pmail is NULL, time-out occurred*/
            {
              return E_TIMEOUT;	
            }                               
            else                  /* Event occurred or event have been deleted*/    
            {
                curTCB->pmail = NULL;
                return E_OK;	
            }				
        }		
    }
}


/**
 *******************************************************************************
 * @brief       Post a semaphore	 
 * @param[in]   id   id of event control block associated with the desired semaphore.	 	 
 * @param[out]  None   
 * @retval      E_INVALID_ID   Parameter id passed was invalid event ID.
 * @retval      E_SEM_FULL     Semaphore full. 
 * @retval      E_OK           Semaphore had post successful.
 *
 * @par Description
 * @details    This function is called to post a semaphore to corresponding event. 
 *
 * @note 
 *******************************************************************************
 */
StatusType CoPostSem(OS_EventID id)
{
    P_ECB pecb;
#if CFG_PAR_CHECKOUT_EN >0
    if(id >= CFG_MAX_EVENT)	                  
    {
        return E_INVALID_ID;
    }
#endif

    pecb = &EventTbl[id];
#if CFG_PAR_CHECKOUT_EN >0
    if(pecb->eventType != EVENT_TYPE_SEM) /* Invalid event control block type */
    {
        return E_INVALID_ID;	
    }
#endif

    /* Make sure semaphore will not overflow */
    if(pecb->eventCounter == pecb->initialEventCounter) 
    {
        return E_SEM_FULL;    /* The counter of Semaphore reach the max number*/
    }
    OsSchedLock();
    pecb->eventCounter++;     /* Increment semaphore count to register event  */
    EventTaskToRdy(pecb);     /* Check semaphore event waiting list           */
    OsSchedUnlock();
    return E_OK;
		
}


/**
 *******************************************************************************
 * @brief       Post a semaphore in ISR	 
 * @param[in]   id    identifier of event control block associated with the 
 *                    desired semaphore.	 	 
 * @param[out]  None  
 * @retval      E_INVALID_ID        Parameter id passed was invalid event ID.
 * @retval      E_NO_TASK_WAITTING  There are one more tasks waitting for the event. 
 * @retval      E_OK                Semaphore had signaled successful.
 *
 * @par Description
 * @details    This function is called in ISR to post a semaphore to corresponding
 *             event. 
 * @note 
 *******************************************************************************
 */
#if CFG_MAX_SERVICE_REQUEST > 0
StatusType isr_PostSem(OS_EventID id)
{
    if(OSSchedLock > 0)         /* If scheduler is locked,(the caller is ISR) */      
    {
        /* Initiate a post service handling request */
        if(InsertInSRQ(SEM_REQ,id,NULL) == FALSE) 
        {
            return E_SEV_REQ_FULL;        /* If service request queue is full */
        }			
        else                              /* Operate successfully             */
        {
            return E_OK;                        
        }
    }
    else
    {
        return(CoPostSem(id));            /* Post semaphore                   */
    }
}
#endif

#endif

// Servicereq.c

/**
 *******************************************************************************
 * @file       serviceReq.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      servive request management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


#if (CFG_TASK_WAITTING_EN > 0) || (CFG_TMR_EN >0)

#if CFG_MAX_SERVICE_REQUEST > 0
/*---------------------------- Variable Define -------------------------------*/
SRQ   ServiceReq = {0,0};             /*!< ISR server request queue         */		     
#endif       
BOOL  IsrReq   = FALSE;
#if (CFG_TASK_WAITTING_EN > 0)
BOOL  TimeReq  = FALSE;                 /*!< Time delay dispose request       */
#endif

#if CFG_TMR_EN  > 0
BOOL  TimerReq = FALSE;                 /*!< Timer dispose request            */
#endif

/**
 *******************************************************************************
 * @brief      Insert into service requst queue	 
 * @param[in]  type     Service request type.	
 * @param[in]  id       Service request event id,event id/flag id.
 * @param[in]  arg      Service request argument. 
 * @param[out] None 
 * 	 
 * @retval     FALSE    Successfully insert into service request queue. 
 * @retval     TRUE     Failure to insert into service request queue.  
 *
 * @par Description		 
 * @details    This function be called to insert a requst into service request	
 *             queue.
 * @note 
 *******************************************************************************
 */
#if (CFG_MAX_SERVICE_REQUEST > 0)
BOOL InsertInSRQ(U8 type,U8 id,void* arg)
{
    P_SQC   pcell;
	U8 cnt;
	U8 heed;
    IRQ_DISABLE_SAVE();
    if (ServiceReq.cnt >= CFG_MAX_SERVICE_REQUEST)
    {
        IRQ_ENABLE_RESTORE ();

        return FALSE;                   /* Error return                       */
    }
	cnt = Inc8(&ServiceReq.cnt);
	heed = ServiceReq.head;
    IsrReq = TRUE;
    pcell = &ServiceReq.cell[((cnt+heed)%CFG_MAX_SERVICE_REQUEST)];/*the tail */
    pcell->type = type;                 /* Save service request type,         */
    pcell->id   = id;                   /* event id                           */
    pcell->arg  = arg;                  /* and parameter                      */
    IRQ_ENABLE_RESTORE ();

    return TRUE;                        /* Return OK                          */
}
#endif



/**
 *******************************************************************************
 * @brief      Respond the request in the service request queue.	 
 * @param[in]  None
 * @param[out] None 
 * @retval     None  
 *
 * @par Description		 
 * @details    This function be called to respond the request in the service  
 *             request queue.
 * @note 
 *******************************************************************************
 */
void RespondSRQ(void)
{

#if CFG_MAX_SERVICE_REQUEST > 0
    SQC cell;

#endif

#if (CFG_TASK_WAITTING_EN > 0)
    if(TimeReq == TRUE)                 /* Time delay request?                */
    {
        TimeDispose();                  /* Yes,call handler                   */
        TimeReq = FALSE;                /* Reset time delay request false     */
    }
#endif
#if CFG_TMR_EN  > 0
    if(TimerReq == TRUE)                /* Timer request?                     */
    {
        TmrDispose();                   /* Yes,call handler                   */
        TimerReq = FALSE;               /* Reset timer request false          */
    }
#endif

#if CFG_MAX_SERVICE_REQUEST > 0

    while (ServiceReq.cnt != 0)
    {
        IRQ_DISABLE_SAVE ();            /* need to protect the following      */
        cell = ServiceReq.cell[ServiceReq.head];  /* extract one cell         */
        ServiceReq.head = (ServiceReq.head + 1) % /* move head (pop)          */
                     CFG_MAX_SERVICE_REQUEST;
        ServiceReq.cnt--;
        IRQ_ENABLE_RESTORE ();          /* now use the cell copy              */

        switch(cell.type)               /* Judge service request type         */
        {
#if CFG_SEM_EN > 0
        case SEM_REQ:                   /* Semaphore post request,call handler*/
            CoPostSem(cell.id);
            break;
#endif
#if CFG_MAILBOX_EN > 0
        case MBOX_REQ:                  /* Mailbox post request,call handler  */
            CoPostMail(cell.id, cell.arg);
            break;
#endif
#if CFG_FLAG_EN > 0
        case FLAG_REQ:                  /* Flag set request,call handler      */
            CoSetFlag(cell.id);
            break;
#endif
#if CFG_QUEUE_EN > 0
        case QUEUE_REQ:                 /* Queue post request,call handler    */
            CoPostQueueMail(cell.id, cell.arg);
            break;
#endif
        default:                        /* Others,break                       */
            break;
        }
    }
#endif
    IRQ_DISABLE_SAVE ();                /* need to protect the following      */

    if (ServiceReq.cnt == 0)            /* another item in the queue already? */
    {
        IsrReq = FALSE;                 /* queue still empty here             */
    }
    IRQ_ENABLE_RESTORE ();              /* now it is done and return          */
}

#endif

// Task.c

/**
 *******************************************************************************
 * @file       task.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      task management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


/*---------------------------- Variable Define -------------------------------*/

/*!< Table use to save TCB pointer.              */
OSTCB    TCBTbl[CFG_MAX_USER_TASKS+SYS_TASK_NUM] = {{0}};

/*!< The stack of IDLE task.                     */
OS_STK   idle_stk[CFG_IDLE_STACK_SIZE] = {0};

P_OSTCB  FreeTCB     = NULL;  /*!< pointer to free TCB                        */	
P_OSTCB  TCBRdy      = NULL;  /*!< Pointer to the READY list.                 */
P_OSTCB  TCBNext     = NULL;  /*!< Poniter to task that next scheduled by OS  */
P_OSTCB  TCBRunning  = NULL;  /*!< Pointer to TCB that current running task.  */
U64      OSCheckTime = 0;     /*!< The counter of system tick.                */

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
OS_TID   PriNum;
U8       ActivePri[CFG_MAX_USER_TASKS+SYS_TASK_NUM];
U8       TaskNumPerPri[CFG_MAX_USER_TASKS+SYS_TASK_NUM];
OS_TID   RdyTaskPri[CFG_MAX_USER_TASKS+SYS_TASK_NUM] = {0};	
U32      RdyTaskPriInfo[(CFG_MAX_USER_TASKS+SYS_TASK_NUM+31)/32];
#endif


/**
 *******************************************************************************
 * @brief      Create a TCB list.	  
 * @param[in]  None 	 
 * @param[out] None    
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called by CoOSInit() to initial the empty list	 
 *             of OS_TCBS,supply a pointer to free TCB.
 *******************************************************************************
 */
void CreateTCBList(void)
{	
    U8  i;
    P_OSTCB ptcb1,ptcb2;
    
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	PriNum = 0;
#endif

	ptcb1 = &TCBTbl[0];	                /* Build the free TCB list            */
    ptcb2 = &TCBTbl[1];  
    for(i=0;i< (CFG_MAX_USER_TASKS+SYS_TASK_NUM-1);i++ )
    {
		ptcb1->taskID    = i;
		ptcb1->state     = TASK_DORMANT;
        ptcb1->TCBnext   = ptcb2;
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		RdyTaskPri[i]    = INVALID_ID;
		ActivePri[i]	 = INVALID_ID;
#endif
        ptcb1++;
        ptcb2++;	
    }
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		ActivePri[i]	 = INVALID_ID;
#endif

	ptcb1->taskID    = i;	
    ptcb1->TCBnext   = NULL;
    FreeTCB = &TCBTbl[0];         /* Initialize FreeTCB as head item of list  */			
}



#if CFG_ORDER_LIST_SCHEDULE_EN ==0

/**
 *******************************************************************************
 * @brief      Get sequence number for Assign priority	  
 * @param[in]  pri            Assign priority	 
 * @param[out] SequenceNum    priority number 
 * @retval     TRUE           Assign priority in priority queue.    
 *             FALSE          Assign priority not in priority queue. 		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to get sequence number for Assign priority.     
 *******************************************************************************
 */
static BOOL  GetPriSeqNum(U8 pri,OS_TID* SequenceNum)
{
	OS_TID  seqNum;
	OS_TID  num,tmpNum;
	num = 0;
	seqNum = PriNum;
	while(num != seqNum)
	{
		tmpNum = num;
		num = (num+seqNum)/2;
		if(pri == ActivePri[num])
		{
			*SequenceNum = num;
			return TRUE;
		}
		else if (pri < ActivePri[num])
		{
			seqNum = num;
			num = tmpNum;
		}
		else
		{
			num++;
		}
	}
	*SequenceNum = num;
	return FALSE;		
}


/**
 *******************************************************************************
 * @brief      Get the nearest ready priority sequence number for Assign number	  
 * @param[in]  seqNum         Assign sequence number	 
 * @param[out] None
 * @retval     INVALID_ID     Cannot find higher ready priority.   
 *             Others         Nearest ready priority sequence number 		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to get the nearest ready priority sequence number.    
 *******************************************************************************
 */
static U8 GetRdyPriSeqNum(U8 seqNum)
{
	U32 tmp;
	U8  i,j,num;
	S8  cnt;
	i = seqNum/32;
	j = seqNum%32;

	do
	{
	  	tmp = RdyTaskPriInfo[i];
		if(tmp != 0)
		{
			num = j/8;
			do
			{
				if((tmp&(0xff<<(num*8))) !=0 )
				{
					if((tmp&(0xf0<<(num*8))) !=0)
					{
						for(cnt=j; cnt >=(num*8+4); cnt--)	
						{
							if( (tmp&(1<<cnt)) !=0)
							{
								return (32*i+cnt);
							}	
						}			
					}

					if((j&0x4)==4)
						j = (j|0x3) -4;
					
					for(cnt=j; cnt >=num*8; cnt--)	
					{
						if( (tmp&(1<<cnt)) !=0)
						{
							return (32*i+cnt);
						}	
					}
				}
				j = num*8 -1;
			}while((num--)!=0);
		}
		j=31;
	}while((i--)!=0);
	return INVALID_ID;											
}


/**
 *******************************************************************************
 * @brief      Remap the ready status of priority queue from Assign sequence number 
 * @param[in]  seqNum         Assign sequence number	 
 * @param[out] None
 * @retval     None    		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to Remap the ready status for priority queue.    
 *******************************************************************************
 */
static void PrioRemap(OS_TID  seqNum)
{
	U8 i,j;
	U32 tmp;
	tmp = j = 0;
	j = seqNum/32;
	for(i=0;i<seqNum%32;i++)
	{
		tmp |= 1<<i;
	}
	tmp &= RdyTaskPriInfo[j];
	
	for(i=seqNum; i<PriNum; i++)
	{
		if((i%32==0)&&(i!=seqNum))
		{
			RdyTaskPriInfo[j++] = tmp;
			tmp = 0;
		}
		if(RdyTaskPri[i] != INVALID_ID)
		{
			tmp = tmp | (1<<(i%32));
		}
	}
	RdyTaskPriInfo[j++] = tmp;
}


/**
 *******************************************************************************
 * @brief      Get the ready status for assign sequence number 
 * @param[in]  seqNum      Assign sequence number	 
 * @param[out] None
 * @retval     TRUE        This priority has ready task   
 *             FALSE       This priority doesn't have ready task  		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to get the ready status for assign sequence number.    
 *******************************************************************************
 */
static BOOL GetPrioSeqNumStatus(U8 seqNum)
{
	if( (RdyTaskPriInfo[seqNum/32] & (1<<(seqNum%32))) == 0)
	{
		return FALSE;
	}
	return TRUE;
}


/**
 *******************************************************************************
 * @brief      Set the ready status for assign sequence number 
 * @param[in]  seqNum      Assign sequence number
 * @param[in]  isRdy       Ready statues for assign sequence number 	 
 * @param[out] None
 * @retval     None 		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to set the ready status for assign sequence number.    
 *******************************************************************************
 */
static void SetPrioSeqNumStatus(U8 seqNum, BOOL isRdy)
{
	U32 tmp;
	tmp = RdyTaskPriInfo[seqNum/32];
	tmp	&= ~(1<<(seqNum%32));
	tmp |= isRdy<<(seqNum%32);
	RdyTaskPriInfo[seqNum/32] = tmp;
}


/**
 *******************************************************************************
 * @brief      Active priority in queue 
 * @param[in]  pri       Task priority
 * @param[in]  None     
 * @param[out] None
 * @retval     None 		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to active priority in queue, if this priority had been in activation,
 *             increate the task num for this priority.    
 *******************************************************************************
 */
void ActiveTaskPri(U8 pri)
{
	OS_TID  seqNum,num;
	if(GetPriSeqNum(pri,&seqNum) == FALSE)
	{
		for(num=PriNum;num>seqNum;num--)
		{
			ActivePri[num]     = ActivePri[num-1];
			TaskNumPerPri[num] = TaskNumPerPri[num-1];
			RdyTaskPri[num]    = RdyTaskPri[num-1];
		}
		ActivePri[seqNum]     = pri;
		TaskNumPerPri[seqNum] = 1;
		RdyTaskPri[seqNum]    = INVALID_ID;
		PriNum++;
		PrioRemap(seqNum);
	}
	else
	{
		 TaskNumPerPri[seqNum]++;
	}
}



/**
 *******************************************************************************
 * @brief      Delete priority in queue 
 * @param[in]  pri       Task priority
 * @param[in]  None     
 * @param[out] None
 * @retval     None 		 
 *					
 * @par Description
 * @details    This function is called in Binary-Scheduling Algorithm 
 *             to decrease the task num for this priority, if the num goto 0,
 *             remove the priority for queue.
 *******************************************************************************
 */
void DeleteTaskPri(U8 pri)
{
	OS_TID  seqNum,num;

	GetPriSeqNum(pri,&seqNum);
	TaskNumPerPri[seqNum]--;
	if(TaskNumPerPri[seqNum]==0)
	{
		for(num=seqNum; num<(PriNum-1); num++)
		{
			ActivePri[num]     = ActivePri[num+1];
			TaskNumPerPri[num] = TaskNumPerPri[num+1];
			RdyTaskPri[num]    = RdyTaskPri[num+1];
		}
		PriNum--;
		PrioRemap(seqNum);
	}
}

#endif


/**
 *******************************************************************************
 * @brief      Insert a task to the ready list	   
 * @param[in]  tcbInsert    A pointer to task will be inserted.
 * @param[out] None  
 * @retval     None	 
 *
 * @par Description
 * @details   This function is called to insert a task to the READY list. 
 *******************************************************************************
 */
void InsertToTCBRdyList(P_OSTCB tcbInsert)
{
    P_OSTCB ptcbNext,ptcb;
    U8  prio;
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	U8  seqNum;
	U8  RdyTaskSeqNum;
#endif
    
    prio = tcbInsert->prio;             /* Get PRI of inserted task           */
    tcbInsert->state     = TASK_READY;  /* Set task as TASK_READY             */

#if CFG_ROBIN_EN >0
	ptcb = TCBRunning;
    /* Set schedule time for the same PRI task as TCBRunning.                 */
    if(prio == ptcb->prio)  /* Is PRI of inserted task equal to running task? */
    {
        if(ptcb != tcbInsert) /* Yes,is inserted task equal to running task?  */
        {
            if(ptcb != NULL)            /* No,TCBRunning == NULL?             */
            {                           /* N0,OSCheckTime < OSTickCnt?        */
                if(OSCheckTime < OSTickCnt)	 
                {                       /* Yes,set OSCheckTime for task robin */
                    OSCheckTime = OSTickCnt + ptcb->timeSlice;	
                }			
            }			
        }
    }
#endif


#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	GetPriSeqNum(prio,&seqNum);
	if(GetPrioSeqNumStatus(seqNum) == TRUE)
	{
		ptcb = &TCBTbl[RdyTaskPri[seqNum]];
		RdyTaskPri[seqNum] = tcbInsert->taskID;
	}
	else
	{
		RdyTaskPri[seqNum] = tcbInsert->taskID;
		RdyTaskSeqNum = GetRdyPriSeqNum(seqNum);
		SetPrioSeqNumStatus(seqNum, 1);
		if(RdyTaskSeqNum == INVALID_ID)
		{
		    ptcb = TCBRdy;
		    TaskSchedReq = TRUE;
			if(ptcb == NULL)
			{
				TCBRdy   = tcbInsert;	
			}
			else
			{
				tcbInsert->TCBnext = ptcb;  /* Yes,set tcbInsert as head item of list */
				ptcb->TCBprev = tcbInsert;
				TCBRdy         = tcbInsert;
			}
			return;
		}
		else
		{
			ptcb = &TCBTbl[RdyTaskPri[RdyTaskSeqNum]];	
		}
	}

	ptcbNext = ptcb->TCBnext;
	tcbInsert->TCBnext = ptcbNext;    /* Set link for list                  */
	ptcb->TCBnext      = tcbInsert;
	tcbInsert->TCBprev = ptcb;
	if(ptcbNext != NULL)
	{
	    ptcbNext->TCBprev  = tcbInsert;
	}


#else
    ptcb = TCBRdy;
    if (ptcb == NULL)                   /* Is ready list NULL?                */
    {
        TaskSchedReq = TRUE;
        TCBRdy = tcbInsert;         /* Yse,set tcbInsert as head item of list */
    }
    else if (prio < ptcb->prio)/* Is PRI of inserted task higher than TCBRdy? */
    {
        TaskSchedReq = TRUE;
        tcbInsert->TCBnext = ptcb;  /* Yes,set tcbInsert as head item of list */
        ptcb->TCBprev  = tcbInsert;
        TCBRdy         = tcbInsert;
    }
    else                                /* No,find correct place              */
    {								    
        ptcbNext = ptcb->TCBnext;       /* Get next item                      */
        while(ptcbNext != NULL)         /* Is last item in ready list?        */
        {                               /* No,find correct place              */
            if(prio < ptcbNext->prio)   /* Is correct place?                  */
                break;                  /* Yes,break circulation              */
            ptcb     = ptcbNext;        /* Save current item                  */
            ptcbNext = ptcbNext->TCBnext; /* Get next item                    */
        }
        tcbInsert->TCBnext = ptcbNext;  /* Set link for list                  */
        ptcb->TCBnext      = tcbInsert;
        tcbInsert->TCBprev = ptcb;
        if(ptcbNext != NULL)
        {
            ptcbNext->TCBprev  = tcbInsert;
        }		
    }
#endif
}



/**
 *******************************************************************************
 * @brief      Remove a task from the READY list	   
 * @param[in]  ptcb     A pointer to task which be removed.	 
 * @param[out] None 				 
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called to remove a task from the READY list.
 *******************************************************************************
 */
void RemoveFromTCBRdyList(P_OSTCB ptcb)
{

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	U8 prio;
	U8 seqNum;
	BOOL isChange;
	isChange = FALSE;
	prio = ptcb->prio;
	GetPriSeqNum(prio,&seqNum);
#endif

    /* Is there only one item in READY list?                                  */
    if((ptcb->TCBnext == NULL) && (ptcb->TCBprev == NULL) )
    {
        TCBRdy = NULL;                  /* Yes,set READY list as NULL         */
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		isChange = TRUE;
#endif
    }
    else if(ptcb->TCBprev == NULL)      /* Is the first item in READY list?   */
    {   
	    /* Yes,remove task from the list,and reset the head of READY list     */
        TCBRdy = ptcb->TCBnext;		    
        ptcb->TCBnext   = NULL;
        TCBRdy->TCBprev = NULL;
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if(TCBRdy->prio != prio)
			isChange = TRUE;
		
#endif
    }
    else if( ptcb->TCBnext == NULL)     /* Is the last item in READY list?    */
    {                                   /* Yes,remove task from list          */
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if(ptcb->TCBprev->prio != prio)
			isChange = TRUE;
		else 
			RdyTaskPri[seqNum] = ptcb->TCBprev->taskID;
#endif
        ptcb->TCBprev->TCBnext = NULL;  
        ptcb->TCBprev          = NULL;
    }
    else                                /* No, remove task from list          */
    {	
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if((ptcb->TCBprev->prio != prio) && (ptcb->TCBnext->prio != prio))
			isChange = TRUE;
		else if((ptcb->TCBprev->prio == prio) && (ptcb->TCBnext->prio != prio))
			RdyTaskPri[seqNum] = ptcb->TCBprev->taskID;
#endif								
        ptcb->TCBprev->TCBnext = ptcb->TCBnext;
        ptcb->TCBnext->TCBprev = ptcb->TCBprev;
        ptcb->TCBnext = NULL;
        ptcb->TCBprev = NULL;
    }
#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		if(isChange == TRUE)
		{
			RdyTaskPri[seqNum] = INVALID_ID;
			SetPrioSeqNumStatus(seqNum, 0);
		}
#endif
}


#if CFG_MUTEX_EN > 0
#define CFG_PRIORITY_SET_EN       (1)
#endif
#if CFG_PRIORITY_SET_EN >0
/**
 *******************************************************************************
 * @brief      Change task priority	   
 * @param[in]  taskID     Specify task id.
 * @param[in]  priority   New priority.	 
 * @param[out] None		   
 * @retval     E_OK              Change priority successful.
 * @retval     E_INVALID_ID      Invalid id,change priority fail.
 * @retval     E_PROTECTED_TASK  Can't change idle task priority.		 
 *
 * @par Description
 * @details    This function is called to change priority for a specify task. 	
 *******************************************************************************
 */
StatusType CoSetPriority(OS_TID taskID,U8 priority)
{			
    P_OSTCB ptcb;
#if CFG_MUTEX_EN >0
    U8 prio;
    P_MUTEX	pMutex;
#endif
#if CFG_EVENT_EN >0
    P_ECB pecb;
#endif

    if(taskID == 0)                     /* Is idle task?                      */
    {											 
        return E_PROTECTED_TASK;        /* Yes,error return                   */
    }   
	
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif
	ptcb = &TCBTbl[taskID];             /* Get TCB of task ID                 */
#if CFG_PAR_CHECKOUT_EN >0    
    if(ptcb->state == TASK_DORMANT)
    {
        return E_INVALID_ID;
    }
    if(priority > CFG_LOWEST_PRIO)
    {
        return E_INVALID_ID;
    }
#endif

    if(ptcb->prio != priority)          /* Is PRI equal to original PRI?      */
    {                                   /* No                                 */
#if CFG_MUTEX_EN >0
        if(ptcb->mutexID != INVALID_ID)
        {
            pMutex = &MutexTbl[ptcb->mutexID];
            if(pMutex->taskID == ptcb->taskID)  /* Task hold mutex?               */
            {
                 pMutex->originalPrio= priority;/* Yes,change original PRI in mutex*/
                 if(ptcb->prio < priority)     /* Is task priority higher than set?*/
                 {
                     return E_OK;                /* Yes,do nothing,return OK       */
                 }
            }		
         }

#endif	

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
		DeleteTaskPri(ptcb->prio);
		ActiveTaskPri(priority);	
#endif	

        ptcb->prio = priority;              /* Change task PRI                */
        if(ptcb->state == TASK_READY)       /* Is task in READY list?         */
        {
            OsSchedLock();                  /* Yes,reorder task in READY list */
            RemoveFromTCBRdyList(ptcb);
            InsertToTCBRdyList(ptcb);	
            OsSchedUnlock();
        }
        else if(ptcb->state == TASK_RUNNING)/* Is task running?               */
        {
            if(ptcb->prio > TCBRdy->prio)   /* Yes,Is PRI higher than TCBRdy? */
            {
				OsSchedLock();              /* Yes,reorder task in READY list */
				TaskSchedReq = TRUE;
                OsSchedUnlock();
            }
        }
        else
        {                                   /* No,task in WAITING list        */
#if CFG_MUTEX_EN >0
            if(ptcb->mutexID != INVALID_ID) /* Is task in mutex WAITING list? */
            {
                /* Yes,reset the highest PRI in the list */
				OsSchedLock(); 
				pMutex = &MutexTbl[ptcb->mutexID];
                ptcb = pMutex->waittingList;  
                prio = pMutex->originalPrio; 
                pMutex->hipriTaskID = pMutex->taskID;
                while(ptcb != NULL)
                {
                    if(ptcb->prio < prio)
                    {
                        prio = ptcb->prio;
                        pMutex->hipriTaskID = ptcb->taskID;
                    }
                    ptcb = ptcb->TCBnext;			
                }
				OsSchedUnlock();
                if(pMutex->originalPrio != prio)
                {
                    CoSetPriority(pMutex->taskID,prio);	
                }	
            }
#endif

#if CFG_EVENT_EN >0
			ptcb = &TCBTbl[taskID];
            if(ptcb->eventID != INVALID_ID) /* Is task in event WAITING list? */
            {								    
                pecb = &EventTbl[ptcb->eventID];
                
                /* Yes,is event sort type as preemptive PRI?                  */
                if(pecb->eventSortType == EVENT_SORT_TYPE_PRIO)
                {	  
                    /* Yes,reorder task in the list                           */
                    RemoveEventWaittingList(ptcb);
                    EventTaskToWait(pecb,ptcb);
                }	
            }
#endif
        }
    }
    return E_OK;
}
#endif

/**
 *******************************************************************************
 * @brief      Schedule function	  
 * @param[in]  None 	 
 * @param[out] None  	 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called by every where need to switch context,
 *             It is schedule function of OS kernel.
 *******************************************************************************
 */
void Schedule(void)
{
    U8  RunPrio,RdyPrio;
    P_OSTCB pRdyTcb,pCurTcb;
   
	
    pCurTcb = TCBRunning;    
    pRdyTcb = TCBRdy;

	if((pRdyTcb==NULL) || (pCurTcb != TCBNext) || (OSSchedLock >1) || (OSIntNesting >0))
	{
		return;
	}
    
	TaskSchedReq = FALSE;
    RunPrio = pCurTcb->prio;
    RdyPrio = pRdyTcb->prio;

	/* Is Running task status was changed? */
    if(pCurTcb->state != TASK_RUNNING)	
    {
        TCBNext        = pRdyTcb;   /* Yes,set TCBNext and reorder READY list */
        pRdyTcb->state = TASK_RUNNING;
        RemoveFromTCBRdyList(pRdyTcb);
    }

    else if(RdyPrio < RunPrio )     /* Is higher PRI task coming in?          */
    {
        TCBNext        = pRdyTcb;   /* Yes,set TCBNext and reorder READY list */
        InsertToTCBRdyList(pCurTcb);
		RemoveFromTCBRdyList(pRdyTcb);
        pRdyTcb->state = TASK_RUNNING;
    }
    
#if CFG_ROBIN_EN >0                 /* Is time for robinning                  */                            
    else if((RunPrio == RdyPrio) && (OSCheckTime == OSTickCnt))
    {
        TCBNext        = pRdyTcb;   /* Yes,set TCBNext and reorder READY list */
        InsertToTCBRdyList(pCurTcb);
		RemoveFromTCBRdyList(pRdyTcb);
        pRdyTcb->state = TASK_RUNNING;
    }
#endif
    else
    {								    
        return;	
    }
    
#if CFG_ROBIN_EN >0
    if(TCBNext->prio == TCBRdy->prio)  /* Reset OSCheckTime for task robinnig */
        OSCheckTime = OSTickCnt + TCBNext->timeSlice;
#endif
    
  
#if CFG_STK_CHECKOUT_EN > 0                       /* Is stack overflow?       */
    if((pCurTcb->stkPtr < pCurTcb->stack)||(*(U32*)(pCurTcb->stack) != MAGIC_WORD))       
    {									
        CoStkOverflowHook(pCurTcb->taskID);       /* Yes,call handler         */		
    }   
#endif
 	
    SwitchContext();                              /* Call task context switch */
}


/**
 *******************************************************************************
 * @brief      Assign a TCB to task being created	 					
 * @param[in]  None     
 * @param[out] None     
 * 	 
 * @retval     XXXX							 
 *
 * @par Description
 * @details    This function is called to assign a task control block for task 
 *              being created.
 *******************************************************************************
 */
static P_OSTCB AssignTCB(void)
{
    P_OSTCB	ptcb;
    
    OsSchedLock();                      /* Lock schedule                      */
    if(FreeTCB == NULL)                 /* Is there no free TCB               */
    {
        OsSchedUnlock();                /* Yes,unlock schedule                */
        return NULL;                    /* Error return                       */
    }	
	ptcb    = FreeTCB;          /* Yes,assgin free TCB for this task  */    
	/* Set next item as the head of free TCB list                     */
    FreeTCB = FreeTCB->TCBnext; 
	OsSchedUnlock();
	return ptcb;
}


/**
 *******************************************************************************
 * @brief      Create a task	   
 * @param[in]  task       Task code entry.
 * @param[in]  argv       The parameter passed to task.
 * @param[in]  parameter  Task priority + stack size + time slice + isWaitting.
 * @param[in]  stk        Pointer to stack top of task.
 * @param[out] None   
 * @retval     E_CREATE_FAIL    Fail to create a task .
 * @retval     others           Valid task id.				 
 *
 * @par Description
 * @details    This function is called by application to create a task,return a id 
 *             to mark this task.
 *******************************************************************************
 */
OS_TID CreateTask(FUNCPtr task,void *argv,U32 parameter,OS_STK *stk)
{
    OS_STK* stkTopPtr;
    P_OSTCB ptcb;
    U8      prio;
#if CFG_ROBIN_EN >0	
    U16     timeSlice;
#endif
   
#if CFG_STK_CHECKOUT_EN >0              /* Check validity of parameter        */
    U16 sktSz;
    sktSz = (parameter&0xfff00)>>8;    
#endif
    prio = parameter&0xff;

#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(task == NULL)
    {
        return E_CREATE_FAIL;
    }
    if(stk == NULL)
    {
        return E_CREATE_FAIL;
    }
    if(prio > CFG_LOWEST_PRIO)
    {
        return E_CREATE_FAIL;		
    }
#if CFG_STK_CHECKOUT_EN >0
    if(sktSz < 20)
    {
        return E_CREATE_FAIL;		
    }
#endif	  // CFG_STK_CHECKOUT_EN
#endif	  // CFG_PAR_CHECKOUT_EN

#if CFG_TASK_SCHEDULE_EN == 0
	if(TCBRunning != NULL)
		 return E_CREATE_FAIL;	
#endif   

    stkTopPtr = InitTaskContext(task,argv,stk);   /* Initialize task context. */
    
    ptcb = AssignTCB();                 /* Get free TCB to use                */
    
    if(ptcb == NULL)                    /* Is free TCB equal to NULL?         */
    {
        return E_CREATE_FAIL;           /* Yes,error return                   */
    }
    
    ptcb->stkPtr = stkTopPtr;           /* Initialize TCB as user set         */
    ptcb->prio   = prio;
#if CFG_STK_CHECKOUT_EN >0
    ptcb->stack = stk+1 - sktSz; /* Set bottom stack for stack overflow check */
    *(U32*)(ptcb->stack) = MAGIC_WORD;
#endif	

#if CFG_TASK_WAITTING_EN >0
    ptcb->delayTick	= INVALID_VALUE;	
#endif		 

#if CFG_TASK_SCHEDULE_EN == 0
	ptcb->taskFuc = task;
	ptcb->taskStk = stk;
#endif     
    ptcb->TCBnext = NULL;               /* Initialize TCB link in READY list  */
    ptcb->TCBprev = NULL;

#if CFG_ROBIN_EN >0						/* Set task time slice for task robin */
    timeSlice = (parameter&0x7fff0000)>>20; 
    if(timeSlice == 0)
    {
        timeSlice = CFG_TIME_SLICE;
    }
    ptcb->timeSlice = timeSlice;
#endif

#if CFG_FLAG_EN > 0
    ptcb->pnode = NULL;                 /* Initialize task as no flag waiting */
#endif

#if CFG_EVENT_EN > 0
    ptcb->eventID  = INVALID_ID;      	/* Initialize task as no event waiting*/
    ptcb->pmail    = NULL;
    ptcb->waitNext = NULL;
    ptcb->waitPrev = NULL;
#endif

#if CFG_MUTEX_EN > 0
    /* Initialize task as no mutex holding or waiting                         */
    ptcb->mutexID = INVALID_ID; 
#endif 

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	ActiveTaskPri(prio);	
#endif	

	if((parameter>>31) == 0)			/* Is task in waitting state?         */
	{									/* No,set it into ready list          */
		OsSchedLock();                  /* Lock schedule                      */
		InsertToTCBRdyList(ptcb);       /* Insert into the READY list         */
	    OsSchedUnlock();                /* Unlock schedule                    */
	}
	else
	{									/* Yes,Set task status as TASK_WAITING*/
		ptcb->state   = TASK_WAITING;	
	}
    return ptcb->taskID;                /* Return task ID                     */
}


/**
 *******************************************************************************
 * @brief      Delete Task	 
 * @param[in]  taskID      Task ID 
 * @param[out] None  
 * @retval     E_INVALID_ID      Invalid task ID.	 
 * @retval     E_PROTECTED_TASK  Protected task in OS.	 
 * @retval     E_OK              Delete successful.	
 *
 * @par Description
 * @details    This function is called to delete assign task.	 
 *******************************************************************************
 */
StatusType CoDelTask(OS_TID taskID)
{
    P_OSTCB ptcb;

#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif
	ptcb = &TCBTbl[taskID];
#if CFG_PAR_CHECKOUT_EN >0 
    if(ptcb->state == TASK_DORMANT)
    {
        return E_INVALID_ID;
    }
#endif
    if(taskID == 0)                     /* Is idle task?                      */
    {											 
        return E_PROTECTED_TASK;        /* Yes,error return                   */
    }    
    
    if(ptcb->state == TASK_RUNNING)     /* Is task running?                   */
    {
        if(OSSchedLock != 0)            /* Yes,is OS lock?                    */
        {
            return E_OS_IN_LOCK;        /* Yes,error return                   */
        }	
    }
		
#if CFG_MUTEX_EN >0                     /* Do task hold mutex?                */
    if(ptcb->mutexID != INVALID_ID)
	{
        if(MutexTbl[ptcb->mutexID].taskID == ptcb->taskID)
        {                               /* Yes,leave the mutex                */
            CoLeaveMutexSection(ptcb->mutexID);
        }
    }
	
#endif	

    OsSchedLock();                      /* Lock schedule                      */
    
    if(ptcb->state == TASK_READY)       /* Is task in READY list?             */
    {
        RemoveFromTCBRdyList(ptcb);     /* Yes,remove task from the READY list*/
    }

#if CFG_TASK_WAITTING_EN > 0 
    else if(ptcb->state == TASK_WAITING)/* Is task in the WAITING list?       */
    {
        /* Yes,Is task in delay list? */
        if(ptcb->delayTick != INVALID_VALUE)			         
        {
            RemoveDelayList(ptcb);      /* Yes,remove task from READY list    */
        }

#if CFG_EVENT_EN > 0
        if(ptcb->eventID != INVALID_ID) /* Is task in event waiting list?     */
        {		
            /* Yes,remove task from event waiting list                        */
            RemoveEventWaittingList(ptcb);	
        }
#endif

#if CFG_FLAG_EN > 0
        if(ptcb->pnode != NULL)         /* Is task in flag waiting list?      */
        {
            /* Yes,remove task from flag waiting list                         */
            RemoveLinkNode(ptcb->pnode);	
        }
#endif

#if CFG_MUTEX_EN >0
        if(ptcb->mutexID != INVALID_ID) /* Is task in mutex waiting list?     */
        {
            RemoveMutexList(ptcb);  /* Yes,remove task from mutex waiting list*/
        }
#endif
	  }
#endif
    ptcb->state   = TASK_DORMANT;       /* Release TCB                        */
	TaskSchedReq  = TRUE;	

#if CFG_ORDER_LIST_SCHEDULE_EN ==0
	DeleteTaskPri(ptcb->prio);	
#endif	

#if CFG_TASK_SCHEDULE_EN >0
    ptcb->TCBnext = FreeTCB;
    FreeTCB       = ptcb;
#endif
    OsSchedUnlock();                    /* Unlock schedule                    */
    return E_OK;                        /* return OK                          */
}


/**
 *******************************************************************************
 * @brief      Exit Task	   
 * @param[in]  None 
 * @param[out] None  
 * @retval     None			 
 *
 * @par Description
 * @details    This function is called to exit current task.	 
 *******************************************************************************
 */
void CoExitTask(void)
{
    CoDelTask(TCBRunning->taskID);      /* Call task delete function          */
}


#if CFG_TASK_SCHEDULE_EN ==0
/**
 *******************************************************************************
 * @brief      Activate Task	   
 * @param[in]  taskID      Task ID 
 * @param[in]  argv        Task argv 
 * @param[out] None  
 * @retval     E_INVALID_ID      Invalid task ID.	  
 * @retval     E_OK              Activate task successful.			 
 *
 * @par Description
 * @details    This function is called to activate current task.	 
 *******************************************************************************
 */
StatusType CoActivateTask(OS_TID taskID,void *argv)
{
	P_OSTCB ptcb;
	OS_STK* stkTopPtr;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif
	ptcb = &TCBTbl[taskID];
#if CFG_PAR_CHECKOUT_EN >0
	if(ptcb->stkPtr == NULL)
		return E_INVALID_ID;
#endif
	if(ptcb->state != TASK_DORMANT)	
		return E_OK;


									    /* Initialize task context. */
	stkTopPtr = InitTaskContext(ptcb->taskFuc,argv,ptcb->taskStk);   
        
    ptcb->stkPtr = stkTopPtr;           /* Initialize TCB as user set         */
	OsSchedLock();                      /* Lock schedule                      */
	InsertToTCBRdyList(ptcb);           /* Insert into the READY list         */
    OsSchedUnlock();                    /* Unlock schedule                    */
	return E_OK;
}
#endif


/**
 *******************************************************************************
 * @brief      Get current task id	  
 * @param[in]  None
 * @param[out] None
 * @retval     ID of the current task.			 
 *
 * @par Description
 * @details    This function is called to get current task id.	 
 *******************************************************************************
 */
OS_TID CoGetCurTaskID(void)
{
    return (TCBRunning->taskID);        /* Return running task ID             */
}

#if CFG_TASK_SUSPEND_EN >0
/**
 *******************************************************************************
 * @brief      Suspend Task	  
 * @param[in]  taskID    ID of task that want to suspend.
 * @param[out] None  
 * @retval     E_OK                  Task suspend successful. 
 * @retval     E_INVALID_ID          Invalid event ID. 
 * @retval     E_PROTECTED_TASK      Can't suspend idle task. 
 * @retval     E_ALREADY_IN_WAITING  Task now in waiting state.
 	 
 *
 * @par Description
 * @details    This function is called to exit current task.	 
 *******************************************************************************
 */
StatusType CoSuspendTask(OS_TID taskID)
{
    P_OSTCB ptcb;

	if(taskID == 0)                     /* Is idle task?                      */
    {											 
        return E_PROTECTED_TASK;        /* Yes,error return                   */
    }   
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif
	ptcb = &TCBTbl[taskID];
#if CFG_PAR_CHECKOUT_EN >0  
    if(ptcb->state == TASK_DORMANT)
    {
        return E_INVALID_ID;
    }
#endif
    if(OSSchedLock != 0)
    {
        return E_OS_IN_LOCK;
    }
    if(ptcb->state == TASK_WAITING)     /* Is task in WAITING list?           */
    {
        return E_ALREADY_IN_WAITING;    /* Yes,error return                   */
    }
    
    OsSchedLock();	
    if(ptcb != TCBRunning)              /* Is runing task?                    */
    {
        RemoveFromTCBRdyList(ptcb);     /* No,Remove task from READY list     */
    }
	else
	{
		TaskSchedReq = TRUE;
	}

    ptcb->state = TASK_WAITING;	        /* Set task status as TASK_WAITING    */
    OsSchedUnlock();                    /* Call task schedule                 */
    return E_OK;                        /* Return OK                          */
}


/**
 *******************************************************************************
 * @brief      Awake Task	 
 * @param[in]  taskID      ID of task that will been awaked.
 * @param[out] None  
 * @retval     E_OK                 Task awake successful. 
 * @retval     E_INVALID_ID         Invalid task ID.
 * @retval     E_TASK_NOT_WAITING   Task now not in waiting state.
 * @retval     E_TASK_WAIT_OTHER    Task now waiting other awake event.
 * @retval     E_PROTECTED_TASK     Idle task mustn't be awaked. 	 
 *
 * @par Description
 * @details    This function is called to awake current task.	 
 *******************************************************************************
 */
StatusType CoAwakeTask(OS_TID taskID)
{
    P_OSTCB ptcb;
	
 	if(taskID == 0)                     /* Is idle task?                      */
    {											 
        return E_PROTECTED_TASK;        /* Yes,error return                   */
    } 
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif
	ptcb = &TCBTbl[taskID];
#if CFG_PAR_CHECKOUT_EN >0  
    if(ptcb->state == TASK_DORMANT)
    {
        return E_INVALID_ID;
    }
#endif
    
    if(ptcb->state != TASK_WAITING)     /* Is task in WAITING list            */
    {
        return E_TASK_NOT_WAITING;      /* No,error return                    */
    }	

#if CFG_TASK_WAITTING_EN > 0
    if(ptcb->delayTick != INVALID_VALUE)/* Is task in READY list              */
    {
        return E_TASK_WAIT_OTHER;       /* Yes,error return                   */
    }

#if CFG_FLAG_EN > 0
    if(ptcb->pnode != NULL)             /* Is task in flag waiting list       */
    {
        return E_TASK_WAIT_OTHER;       /* Yes,error return                   */
    }
#endif

#if CFG_EVENT_EN>0
    if(ptcb->eventID != INVALID_ID)     /* Is task in event waiting list      */
    {
        return E_TASK_WAIT_OTHER;       /* Yes,error return                   */
    }
#endif	

#if CFG_MUTEX_EN > 0
    if(ptcb->mutexID != INVALID_ID)     /* Is task in mutex waiting list      */
    {
        return E_TASK_WAIT_OTHER;       /* Yes,error return                   */
    }
#endif

#endif	  //CFG_TASK_WAITTING_EN

    /* All no,so WAITING state was set by CoSuspendTask()                     */
    OsSchedLock();                      /* Lock schedule                      */
	InsertToTCBRdyList(ptcb);           /* Insert the task into the READY list*/
    OsSchedUnlock();                    /* Unlock schedule                    */
    return E_OK;                        /* return OK                          */
}
#endif

// Time.c

/**
 *******************************************************************************
 * @file       time.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      time management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */  

#if CFG_TASK_WAITTING_EN > 0

/*---------------------------- Variable Define -------------------------------*/
P_OSTCB DlyList   = NULL;               /*!< Header pointer to the DELAY list.*/


/**
 *******************************************************************************
 * @brief      Insert into DELAY list			 
 *   
 * @param[in]  ptcb    Task that want to insert into DELAY list. 
 * @param[in]  ticks   Delay system ticks.	 
 * @param[out] None   
 * @retval     None.	 	 
 *
 * @par Description
 * @details    This function is called to insert task into DELAY list.
 *******************************************************************************
 */
void InsertDelayList(P_OSTCB ptcb,U32 ticks)
{
    S32 deltaTicks;
    P_OSTCB dlyNext;
    
    if(ticks == 0)                      /* Is delay tick == 0?                */
        return;                         /* Yes,do nothing,return              */
    if(DlyList == NULL)                 /* Is no item in DELAY list?          */
    {
        ptcb->delayTick = ticks;        /* Yes,set this as first item         */
        DlyList         = ptcb;
    }
    else
    {	
        /* No,find correct place ,and insert the task */
        dlyNext    = DlyList; 
        deltaTicks = ticks;             /* Get task delay ticks               */
        
        /* Find correct place */
        while(dlyNext != NULL)
        {		
            /* Get delta ticks with previous item */ 
            deltaTicks -= dlyNext->delayTick;  
            if(deltaTicks < 0)          /* Is delta ticks<0?                  */
            {	  
                /* Yes,get correct place */
                if(dlyNext->TCBprev != NULL)   /* Is head item of DELAY list? */
                {							   
                    dlyNext->TCBprev->TCBnext = ptcb;   /* No,insert into     */ 
                    ptcb->TCBprev             = dlyNext->TCBprev;
                    ptcb->TCBnext             = dlyNext;
                    dlyNext->TCBprev          = ptcb;
                }
                else                    /* Yes,set task as first item         */
                {							   
                    ptcb->TCBnext    = DlyList;
                    DlyList->TCBprev = ptcb;
                    DlyList          = ptcb;
                }
                ptcb->delayTick           = ptcb->TCBnext->delayTick+deltaTicks;
                ptcb->TCBnext->delayTick -= ptcb->delayTick; 
                break;
            }
            /* Is last item in DELAY list? */
            else if((deltaTicks >= 0) && (dlyNext->TCBnext == NULL) )
            {								   
                ptcb->TCBprev    = dlyNext; /* Yes,insert into                */
                dlyNext->TCBnext = ptcb;	
                ptcb->delayTick  = deltaTicks;
                break;
            }
            dlyNext = dlyNext->TCBnext; /* Get the next item in DELAY list    */
        }
    }

    ptcb->state  = TASK_WAITING;        /* Set task status as TASK_WAITING    */
    TaskSchedReq = TRUE;
}


/**
 *******************************************************************************
 * @brief      Remove from the DELAY list			  
 * @param[in]  ptcb   Task that want to remove from the DELAY list. 
 * @param[out] None   
 * @retval     None	 	 
 *
 * @par Description
 * @details    This function is called to remove task from the DELAY list.
 *******************************************************************************
 */
void RemoveDelayList(P_OSTCB ptcb)
{
    
    /* Is there only one item in the DELAY list?   */
    if((ptcb->TCBprev == NULL) && ( ptcb->TCBnext == NULL))
    {
        DlyList = NULL;	                /* Yes,set DELAY list as NULL         */
    }
    else if(ptcb->TCBprev == NULL)      /* Is the first item in DELAY list?   */
    {   
	    /* Yes,remove task from the DELAY list,and reset the list             */
        DlyList	                  = ptcb->TCBnext;
        ptcb->TCBnext->delayTick += ptcb->delayTick;
        ptcb->TCBnext->TCBprev    = NULL;	
        ptcb->TCBnext             = NULL;
        
    }
    else if(ptcb->TCBnext == NULL)      /* Is the last item in DELAY list?    */
    {									
        ptcb->TCBprev->TCBnext = NULL;  /* Yes,remove task form DELAY list    */
        ptcb->TCBprev          = NULL;	
    }
    else                                /* No, remove task from DELAY list    */
    {									
        ptcb->TCBprev->TCBnext    = ptcb->TCBnext;	
        ptcb->TCBnext->TCBprev    = ptcb->TCBprev;	
        ptcb->TCBnext->delayTick += ptcb->delayTick;
        ptcb->TCBnext     	      = NULL;
        ptcb->TCBprev             = NULL;
    }
    ptcb->delayTick = INVALID_VALUE;  /* Set task delay tick value as invalid */		
}

/**
 *******************************************************************************
 * @brief      Get current ticks			 
 * @param[in]  None	 
 * @param[out] None   
 * @retval     Return current system tick counter.	 	 
 *
 * @par Description
 * @details    This function is called to obtain current system tick counter.
 *******************************************************************************
 */
U64 CoGetOSTime(void)
{
    return OSTickCnt;                   /* Get system time(tick)              */
}


/**
 *******************************************************************************
 * @brief      Delay current task for specify ticks number	  
 * @param[in]  ticks    Specify system tick number which will delay.			 
 * @param[out] None  
 * @retval     E_CALL   Error call in ISR.
 * @retval     E_OK     The current task was insert to DELAY list successful,it
 *                      will delay specify time.		 
 * @par Description
 * @details    This function delay specify ticks for current task.
 *
 * @note       This function be called in ISR,do nothing and return immediately.
 *******************************************************************************	
 */
StatusType CoTickDelay(U32 ticks)
{
    if(OSIntNesting >0)	                /* Is call in ISR?                    */
    {
        return E_CALL;                  /* Yes,error return                   */
    }
    
    if(ticks == INVALID_VALUE)          /* Is tick==INVALID_VALUE?            */
    {
        return E_INVALID_PARAMETER;     /* Yes,error return                   */
    }	
    if(ticks == 0)                      /* Is tick==0?                        */
    {
        return E_OK;                    /* Yes,do nothing ,return OK          */
    }
    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	
    OsSchedLock();                      /* Lock schedule                      */
    InsertDelayList(TCBRunning,ticks);	/* Insert task in DELAY list          */
    OsSchedUnlock();                /* Unlock schedule,and call task schedule */
    return E_OK;                        /* Return OK                          */
}


/**
 *******************************************************************************
 * @brief      Reset task delay ticks	 
 * @param[in]  ptcb    Task that want to insert into DELAY list.
 * @param[in]  ticks   Specify system tick number which will delay .			 
 * @param[out] None  
 * @retval     E_CALL               Error call in ISR.
 * @retval     E_INVALID_ID         Invalid task id.
 * @retval     E_NOT_IN_DELAY_LIST  Task not in delay list.
 * @retval     E_OK                 The current task was inserted to DELAY list 
 *                                  successful,it will delay for specify time.		 
 * @par Description
 * @details    This function delay specify ticks for current task.
 ******************************************************************************* 	
 */
StatusType CoResetTaskDelayTick(OS_TID taskID,U32 ticks)
{
    P_OSTCB ptcb;
	

#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(taskID >= CFG_MAX_USER_TASKS + SYS_TASK_NUM)
    {
        return E_INVALID_ID;
    }
#endif

	ptcb = &TCBTbl[taskID];
#if CFG_PAR_CHECKOUT_EN >0 
    if(ptcb->stkPtr == NULL)
    {
        return E_INVALID_ID;
    }
#endif

    if(ptcb->delayTick == INVALID_VALUE)  /* Is tick==INVALID_VALUE?          */
    {
        return E_NOT_IN_DELAY_LIST;       /* Yes,error return                 */
    }
    OsSchedLock();                        /* Lock schedule                    */
    RemoveDelayList(ptcb);                /* Remove task from the DELAY list  */
    
    if(ticks == 0)                        /* Is delay tick==0?                */
    {
        InsertToTCBRdyList(ptcb);         /* Insert task into the DELAY list  */
    }
    else
    {									
        InsertDelayList(ptcb,ticks);      /* No,insert task into DELAY list   */
    }
    OsSchedUnlock();                /* Unlock schedule,and call task schedule */
    return E_OK;                          /* Return OK                        */
}


/**
 *******************************************************************************
 * @brief      Delay current task for detail time	   
 * @param[in]  hour      Specify the number of hours.
 * @param[in]  minute    Specify the number of minutes.
 * @param[in]  sec       Specify the number of seconds.
 * @param[in]  millsec   Specify the number of millseconds.	 
 * @param[out] None  
 * @retval     E_CALL               Error call in ISR.
 * @retval     E_INVALID_PARAMETER  Parameter passed was invalid,delay fail.
 * @retval     E_OK                 The current task was inserted to DELAY list
 *                                  successful,it will delay for specify time.							 
 * @par Description
 * @details    This function delay specify time for current task.	 
 *
 * @note       If this function called in ISR,do nothing and return immediately.
 *******************************************************************************
 */
#if CFG_TIME_DELAY_EN >0
StatusType  CoTimeDelay(U8 hour,U8 minute,U8 sec,U16 millsec)
{
    U32	ticks;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(OSIntNesting > 0)
    {
        return E_CALL;
    }
    if((minute > 59)||(sec > 59)||(millsec > 999))
        return E_INVALID_PARAMETER;
#endif
    if(OSSchedLock != 0)                /* Is OS lock?                        */
    {
        return E_OS_IN_LOCK;            /* Yes,error return                   */
    }	
    
    /* Get tick counter from time */
    ticks = ((hour*3600) + (minute*60) + (sec)) * (CFG_SYSTICK_FREQ)\
            + (millsec*CFG_SYSTICK_FREQ + 500)/1000;
    
    CoTickDelay(ticks);                 /* Call tick delay                    */
    return E_OK;                        /* Return OK                          */
}
#endif




/**
 *******************************************************************************
 * @brief      Dispose time delay	 
 * @param[in]  None	 
 * @param[out] None 
 * @retval     None 
 *
 * @par Description
 * @details    This function is called to dispose time delay of all task.  
 *******************************************************************************
 */
void TimeDispose(void)
{  
    P_OSTCB	dlyList;
    
    dlyList = DlyList;                  /* Get first item of DELAY list       */
    while((dlyList != NULL) && (dlyList->delayTick == 0) )
    {	
    
#if CFG_EVENT_EN > 0
        if(dlyList->eventID != INVALID_ID) /* Is task in event waiting list?  */
        {								   
            RemoveEventWaittingList(dlyList); /* Yes,remove task from list    */	
        }
#endif

#if CFG_FLAG_EN  > 0
        if(dlyList->pnode != NULL)          /* Is task in flag waiting list?  */
        {
            RemoveLinkNode(dlyList->pnode); /* Yes,remove task from list      */	
        }
#endif
        dlyList->delayTick = INVALID_VALUE; /* Set delay tick value as invalid*/
        DlyList = dlyList->TCBnext; /* Get next item as the head of DELAY list*/
        dlyList->TCBnext   = NULL;		   

		InsertToTCBRdyList(dlyList);        /* Insert task into READY list    */
        
        dlyList = DlyList;                /* Get the first item of DELAY list */
        if(dlyList != NULL)                 /* Is DELAY list as NULL?         */
        {
            dlyList->TCBprev = NULL;        /* No,initialize the first item   */
        }
    }
}


/**
 *******************************************************************************
 * @brief      Dispose time delay in ISR	  
 * @param[in]  None	 
 * @param[out] None 
 * @retval     None 
 *
 * @par Description
 * @details    This function is called in systick interrupt to dispose time delay   
 *             of all task.
 *******************************************************************************
 */
void isr_TimeDispose(void)
{
    if(OSSchedLock > 1)                 /* Is schedule lock?                  */
    {
        IsrReq = TRUE;
        TimeReq = TRUE;                 /* Yes,set time request true          */
    }
    else
    {
        TimeDispose();                  /* No,call handler                    */
    }
}


#endif


// Arch.c

/**
 *******************************************************************************
 * @file      arch.c
 * @version   V1.13    
 * @date      2010.04.26
 * @brief     This file provides InitTaskContext() and SysTick_Handler().
 *******************************************************************************
 * @copy
 *	WRITE COPY INFORMATION USE CAPITAL LETTER
 *
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */  

U64     OSTickCnt = 0;                  /*!< Current system tick counter      */ 																			 

/**
 ******************************************************************************
 * @brief      Initial task context	  
 * @param[in]  task    Entry point of task.
 * @param[in]  param   The parameter pass to task.	
 * @param[in]  pstk    The pointer to stack top.	 
 * @param[out] None  
 * @retval     Returns location of new stack top.		 
 *
 * @par Description
 * @details    This function is called to initialize the stack frame of the 
 *             task being created.
 ******************************************************************************
 */
OS_STK *InitTaskContext(FUNCPtr task,void *param,OS_STK *pstk)
{
    OS_STK *context;
	context  = pstk;
    *(context--) = (U32)0x01000000L;      /* xPSR	        */
	*(context--) = (U32)task;             /* Entry point of task.                         */
	*(context)   = (U32)0xFFFFFFFEL;
    context      = context - 5;
	*(context)   = (U32)param;            /* R0: argument */
	context      = context - 8;
  	
    return (context);                   /* Returns location of new stack top. */
}


 
/**
 *******************************************************************************
 * @brief      System tick interrupt handler.			 
 * @param[in]  None	 
 * @param[out] None  	 
 * @retval     None
 *		 
 * @par Description
 * @details    This is system tick interrupt headler.		 
 * @note       CoOS may schedule when exiting this ISR. 
 *******************************************************************************
 */ 
void SysTick_Handler(void)
{
    OSSchedLock++;                  /* Lock scheduler.                        */
    OSTickCnt++;                    /* Increment systerm time.                */
#if CFG_TASK_WAITTING_EN >0    
    if(DlyList != NULL)             /* Have task in delay list?               */
    {
        if(DlyList->delayTick > 1)  /* Delay time > 1?                        */
        {
			DlyList->delayTick--;   /* Decrease delay time of the list head.  */         
        }
		else
		{
			DlyList->delayTick = 0;
			isr_TimeDispose();       /* Call hander for delay time list        */
		}
    }
#endif
    
#if CFG_TMR_EN > 0	
    if(TmrList != NULL)             /* Have timer in working?                 */
    {
        if(TmrList->tmrCnt > 1)     /* Timer time > 1?                        */
        {
			TmrList->tmrCnt--;      /* Decrease timer time of the list head.  */        
        }
		else
		{
			TmrList->tmrCnt = 0;
			isr_TmrDispose();         /* Call hander for timer list             */
		}
    }	
#endif
	TaskSchedReq = TRUE;
    OsSchedUnlock();
}

// Timer.c

/**
 *******************************************************************************
 * @file       timer.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      timer management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 


/*---------------------------- Variable Define -------------------------------*/
#if CFG_TMR_EN > 0

TmrCtrl    TmrTbl[CFG_MAX_TMR]= {{0}};/*!< Table which save timer control block.*/
P_TmrCtrl  TmrList     = NULL;      /*!< The header of the TmrCtrl list.      */
U32        TmrIDVessel = 0;         /*!< Timer ID container.                  */


/**
 *******************************************************************************
 * @brief      Insert a timer into the timer list	   
 * @param[in]  tmrID    Specify timer ID which insertted.		 
 * @param[out] None  
 * @retval     E_INVALID_ID  Timer ID passed was invalid,insert fail.
 * @retval     E_OK          Insert successful.			 
 *
 * @par Description
 * @details    This function is called to insert a timer into the timer list.  
 *******************************************************************************
 */
static void InsertTmrList(OS_TCID tmrID)
{
    P_TmrCtrl pTmr;
    S32 deltaTicks;
    U32 tmrCnt;
    tmrCnt = TmrTbl[tmrID].tmrCnt;      /* Get timer time                     */
    
    if(tmrCnt == 0)                     /* Is timer time==0?                  */
    {
        return;                         /* Do nothing,return                  */
    }
    
    OsSchedLock();                      /* Lock schedule                      */
    if(TmrList == NULL)                 /* Is no item in timer list?          */
    {
        TmrList = &TmrTbl[tmrID];       /* Yes,set this as first item         */
    }
    else                  /* No,find correct place ,and insert inserted timer */
    {								    
      	pTmr       = TmrList; 
      	deltaTicks = tmrCnt;            /* Get timer tick                     */
      	
      	/* find correct place */
      	while(pTmr != NULL)
      	{				    
            deltaTicks -= pTmr->tmrCnt; /* Get ticks with previous item       */
            if(deltaTicks < 0)          /* Is delta ticks<0?                  */  
            {	
                /* Yes,get correct place */
                if(pTmr->tmrPrev!= NULL)/* Is head item of timer list?        */
                {	
                    /* No,insert into */
                    pTmr->tmrPrev->tmrNext = &TmrTbl[tmrID]; 
                    TmrTbl[tmrID].tmrPrev  = pTmr->tmrPrev;
                    TmrTbl[tmrID].tmrNext  = pTmr;
                    pTmr->tmrPrev          = &TmrTbl[tmrID];
                }
                else                    /* Yes,set task as first item         */ 	
                {
                    TmrTbl[tmrID].tmrNext = TmrList;
                    TmrList->tmrPrev      = &TmrTbl[tmrID];
                    TmrList               = &TmrTbl[tmrID];
                }
                TmrTbl[tmrID].tmrCnt            = TmrTbl[tmrID].tmrNext->tmrCnt+deltaTicks;
                TmrTbl[tmrID].tmrNext->tmrCnt  -= TmrTbl[tmrID].tmrCnt; 
                break;	
            }
            /* Is last item in list? */									
            else if((deltaTicks >= 0) && (pTmr->tmrNext == NULL))
            {	
                /* Yes,insert into */
                TmrTbl[tmrID].tmrPrev = pTmr;
                pTmr->tmrNext         = &TmrTbl[tmrID];	
                TmrTbl[tmrID].tmrCnt  = deltaTicks;
                break;	
            }
            pTmr = pTmr->tmrNext;       /* Get the next item in timer list    */	
      	}
    }
    OsSchedUnlock();                    /* Unlock schedule                    */
}


/**
 *******************************************************************************
 * @brief      Remove a timer from the timer list	  
 * @param[in]  tmrID    Specify ID for a timer which removed form timer list.	 
 * @param[out] None 
 * @retval     None
 *
 * @par Description
 * @details    This function is called to remove a timer from the timer list. 
 *******************************************************************************
 */
static void RemoveTmrList(OS_TCID tmrID)
{
    P_TmrCtrl pTmr;
    
    pTmr = &TmrTbl[tmrID];
    
    OsSchedLock();                      /* Lock schedule                      */
    
    /* Is there only one item in timer list?                                  */
    if((pTmr->tmrPrev == NULL) && (pTmr->tmrNext == NULL))
    {		
        TmrList = NULL;                 /* Yes,set timer list as NULL         */ 	
    }
    else if(pTmr->tmrPrev == NULL)      /* Is the first item in timer list?   */
    {   /* Yes,remove timer from list,and reset timer list                    */
        TmrList  = pTmr->tmrNext;
        TmrList->tmrPrev = NULL;
        pTmr->tmrNext->tmrCnt += pTmr->tmrCnt;
        pTmr->tmrNext    = NULL;  
    }
    else if(pTmr->tmrNext == NULL)      /* Is the last item in timer list?    */
    {
        /* Yes,remove timer form list */
        pTmr->tmrPrev->tmrNext = NULL;	
        pTmr->tmrPrev = NULL;
    }
    else                                /* No, remove timer from list         */
    {
        pTmr->tmrPrev->tmrNext  =  pTmr->tmrNext;
        pTmr->tmrNext->tmrPrev  =  pTmr->tmrPrev;
        pTmr->tmrNext->tmrCnt  += pTmr->tmrCnt;
        pTmr->tmrNext = NULL;
        pTmr->tmrPrev = NULL;
    }
    OsSchedUnlock();                    /* Unlock schedule                    */
}


/**
 *******************************************************************************
 * @brief      Create a timer	   
 * @param[in]  tmrType     Specify timer's type.		 
 * @param[in]  tmrCnt      Specify timer initial counter value.  
 * @param[in]  tmrReload   Specify timer reload value.
 * @param[in]  func        Specify timer callback function entry.
 * @param[out] None
 * @retval     E_CREATE_FAIL   Create timer fail.
 * @retval     others          Create timer successful.			 
 *
 * @par Description
 * @details    This function is called to create a timer.
 *******************************************************************************
 */
OS_TCID CoCreateTmr(U8 tmrType, U32 tmrCnt, U32 tmrReload, vFUNCPtr func)
{
    U8 i;
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if((tmrType != TMR_TYPE_ONE_SHOT) && (tmrType != TMR_TYPE_PERIODIC))
    {
        return E_CREATE_FAIL;	
    }
    if(func == NULL)
    {
        return E_CREATE_FAIL;
    }
#endif
    OsSchedLock();                        /* Lock schedule                    */
    for(i = 0; i < CFG_MAX_TMR; i++)
    {
        if((TmrIDVessel & (1 << i)) == 0) /* Is free timer ID?                */
        {
            TmrIDVessel |= (1<<i);        /* Yes,assign ID to this timer      */
            OsSchedUnlock();              /* Unlock schedule                  */
            TmrTbl[i].tmrID     = i;      /* Initialize timer as user set     */
            TmrTbl[i].tmrType   = tmrType;	
            TmrTbl[i].tmrState  = TMR_STATE_STOPPED;
            TmrTbl[i].tmrCnt    = tmrCnt;
            TmrTbl[i].tmrReload	= tmrReload;
            TmrTbl[i].tmrCallBack = func;
            TmrTbl[i].tmrPrev   = NULL;
            TmrTbl[i].tmrNext   = NULL;
            return i;                     /* Return timer ID                  */
        }
    }
    OsSchedUnlock();                      /* Unlock schedule                  */
    return E_CREATE_FAIL;                 /* Error return                     */
}


/**
 *******************************************************************************
 * @brief      Start counter	 
 * @param[in]  tmrID    Specify a timer which startted.		 
 * @param[out] None 
 * @retval     E_INVALID_ID  The timer id passed was invalid,can't start timer	
 * @retval     E_OK          Insert a timer to timer list and start it successful. 
 *
 * @par Description
 * @details    This function is called to make a timer start countering. 
 *******************************************************************************
 */
StatusType CoStartTmr(OS_TCID tmrID)
{
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(tmrID >= CFG_MAX_TMR)
    {
        return E_INVALID_ID;
    }
    if( (TmrIDVessel & (1<<tmrID)) == 0)
    {
        return E_INVALID_ID;
    }
#endif
    
    if(TmrTbl[tmrID].tmrState == TMR_STATE_RUNNING)   /* Is timer running?    */
    {
        return E_OK;                              /* Yes,do nothing,return OK */
    }
    
    /* No,set timer status as TMR_STATE_RUNNING */
    TmrTbl[tmrID].tmrState = TMR_STATE_RUNNING; 
    InsertTmrList(tmrID);               /* Insert this timer into timer list  */
    return E_OK;                        /* Return OK                          */
}



/**
 *******************************************************************************
 * @brief      Stop countering for a spcify timer	  
 * @param[in]  tmrID    Specify a timer which stopped.	 
 * @param[out] None  	 
 * @retval     E_INVALID_ID  The timer id passed was invalid, stop failure.
 * @retval     E_OK          Stop a timer countering successful.
 *
 * @par Description
 * @details    This function is called to stop a timer from counting. 
 *******************************************************************************
 */
StatusType CoStopTmr(OS_TCID tmrID)
{	
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(tmrID >= CFG_MAX_TMR)
    {
        return E_INVALID_ID;
    }
    if((TmrIDVessel & (1<<tmrID)) == 0)
    {
        return E_INVALID_ID;
    }
#endif
    
    
    if(TmrTbl[tmrID].tmrState == TMR_STATE_STOPPED)/* Does timer stop running?*/
    {
        return E_OK;                    /* Yes,do nothing,return OK           */
    }
    RemoveTmrList(tmrID);             /* No,remove this timer from timer list */
    
    /* Set timer status as TMR_STATE_STOPPED  */
    TmrTbl[tmrID].tmrState = TMR_STATE_STOPPED;	
    return E_OK;                        /* Return OK                          */
}


/**
 *******************************************************************************
 * @brief      Delete a timer	 
 * @param[in]  tmrID     Specify a timer which deleted.		 
 * @param[out] None   
 * @retval     E_INVALID_ID  The timer id passed was invalid,deleted failure.	
 * @retval     E_OK          Delete a timer successful.
 *
 * @par Description
 * @details    This function is called to delete a timer which created before.	
 *******************************************************************************
 */
StatusType CoDelTmr(OS_TCID tmrID)
{
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(tmrID >= CFG_MAX_TMR)
    {
        return E_INVALID_ID;
    }
    if( (TmrIDVessel & (1<<tmrID)) == 0)
    {
        return E_INVALID_ID;
    }
#endif
	
    if(TmrTbl[tmrID].tmrState == TMR_STATE_RUNNING) /* Is timer running?      */
    {
        RemoveTmrList(tmrID);         /* Yes,remove this timer from timer list*/
    }
    TmrIDVessel &=~(1<<tmrID);        /* Release resource that this timer hold*/
    return E_OK;                      /* Return OK                            */
}

 
/**
 *******************************************************************************
 * @brief      Get current counter of specify timer	 
 * @param[in]  tmrID          Specify timer by ID.		 
 * @param[out] E_INVALID_ID   Invalid ID was passed and get counter failure.	  
 * @param[out] E_OK           Get current counter successful.	 
 * @retval     Current counter of a timer which specify by id.			 
 *
 * @par Description
 * @details    This function is called to obtain current counter of specify timer.
 *******************************************************************************
 */
U32 CoGetCurTmrCnt(OS_TCID tmrID,StatusType* perr)
{
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(tmrID >= CFG_MAX_TMR)
    {
        *perr = E_INVALID_ID;
        return 0;
    }
    if((TmrIDVessel & (1<<tmrID)) == 0)
    {
        *perr = E_INVALID_ID;
        return 0;
    }
#endif
    *perr = E_OK;
    return TmrTbl[tmrID].tmrCnt;        /* Return timer counter               */
}


/**
 *******************************************************************************
 * @brief      Setting for a specify timer	  		   	
 * @param[in]  tmrID       Specify timer by ID.
 * @param[in]  tmrCnt      Specify timer counter which need to be set.
 * @param[in]  tmrReload   Specify timer reload value which need to be set.		 
 * @param[out] None  
 * @retval     E_INVALID_ID  The ID passed was invalid,set fail.
 * @retval     E_OK          Set timer counter successful.				 
 *
 * @par Description
 * @details    This function is called to set timer counter and reload value.
 *******************************************************************************
 */
StatusType CoSetTmrCnt(OS_TCID tmrID,U32 tmrCnt,U32 tmrReload)
{
#if CFG_PAR_CHECKOUT_EN >0              /* Check validity of parameter        */
    if(tmrID >= CFG_MAX_TMR)
    {
        return E_INVALID_ID;
    }
    if( (TmrIDVessel & (1<<tmrID)) == 0)
    {
        return E_INVALID_ID;
    }
#endif
    TmrTbl[tmrID].tmrCnt    = tmrCnt; /* Reset timer counter and reload value */
    TmrTbl[tmrID].tmrReload = tmrReload;
    								
    if(TmrTbl[tmrID].tmrState == TMR_STATE_RUNNING)   /* Is timer running?    */
    {
        RemoveTmrList(tmrID);           /* Yes,reorder timer in timer list    */
        InsertTmrList(tmrID);	
    }
    return E_OK;                        /* Return OK                          */
}


/**
 *******************************************************************************
 * @brief      Timer counter dispose	   
 * @param[in]  None 	 
 * @param[out] None	 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to dispose timer counter.
 *******************************************************************************
 */
void TmrDispose(void)
{
    P_TmrCtrl	pTmr;
    
    pTmr = TmrList;                     /* Get first item of timer list       */
    while((pTmr != NULL) && (pTmr->tmrCnt == 0) )
    {	
        if(pTmr->tmrType == TMR_TYPE_ONE_SHOT)    /* Is a One-shot timer?     */
        {
            /* Yes,remove this timer from timer list                          */
            RemoveTmrList(pTmr->tmrID);
            
            /* Set timer status as TMR_STATE_STOPPED                          */
            pTmr->tmrState = TMR_STATE_STOPPED;
            (pTmr->tmrCallBack)();          /* Call timer callback function   */
        }
        else if(pTmr->tmrType == TMR_TYPE_PERIODIC)   /* Is a periodic timer? */
        {
            /* Yes,remove this timer from timer list                          */
            RemoveTmrList(pTmr->tmrID); 
            pTmr->tmrCnt = pTmr->tmrReload;   /* Reset timer tick             */
            InsertTmrList(pTmr->tmrID);       /* Insert timer into timer list */
            (pTmr->tmrCallBack)();            /* Call timer callback function */
        }
        pTmr = TmrList;	                      /* Get first item of timer list */
    }
}


/**
 *******************************************************************************
 * @brief      Timer counter dispose in ISR	   
 * @param[in]  None 	 
 * @param[out] None	 
 * @retval     None	 
 *
 * @par Description
 * @details    This function is called to dispose timer counter.
 *******************************************************************************
 */
void isr_TmrDispose(void)
{
    if(OSSchedLock > 1)                 /* Is schedule lock?                  */
    {
        IsrReq = TRUE;
        TimerReq  = TRUE;               /* Yes,set timer request true         */
    }
    else
    {
        TmrDispose();                   /* No,call handler                    */
    }
}	 

#endif

// Utility.c

/**
 *******************************************************************************
 * @file       utility.c
 * @version    V1.13    
 * @date       2010.04.26
 * @brief      Utility management implementation code of CooCox CoOS kernel.	
 *******************************************************************************
 * @copy
 *
 * INTERNAL FILE,DON'T PUBLIC.
 * 
 * <h2><center>&copy; COPYRIGHT 2009 CooCox </center></h2>
 *******************************************************************************
 */ 

#if CFG_UTILITY_EN > 0


/**
 *******************************************************************************
 * @brief      Convert tick number to time 	  
 * @param[in]  ticks    Specifies the systerm tick numbers that will be converted.	 
 * @param[out] hour     Hours which converted. 
 * @param[out] minute   minutes which converted.
 * @param[out] sec      seconds which converted.
 * @param[out] millsec  millseconds which converted.
 * @retval     None		 
 *
 * @par Description
 * @details    This function is called to convert specify ticks to time format.	  	 	
 *******************************************************************************				
 */
#if CFG_TICK_TO_TIME_EN > 0
void CoTickToTime(U32 ticks,U8* hour,U8* minute,U8* sec,U16* millsec)
{
    U32 totalTime;
    
    /* Convert ticks to time*/
    totalTime = ticks * (1000/CFG_SYSTICK_FREQ);
    *millsec  = totalTime%1000;
    totalTime = totalTime/1000;
    *sec      = totalTime%60;
    totalTime = totalTime/60;
    *minute   = totalTime%60;
    totalTime = totalTime/60;
    *hour     = totalTime;		
}
#endif    /* CFG_TICK_TO_TIME_EN    */


/**
 *******************************************************************************
 * @brief      Convert time to tick	  
 * @param[in]  hour     Specifies the number of hours.
 * @param[in]  minute   Specifies the number of minutes.
 * @param[in]  sec      Specifies the number of seconds.
 * @param[in]  millsec  Specifies the number of millseconds.	 
 * @param[out] ticks    Tick numbers that converted.  
 * @retval     E_INVALID_PARAMETER   Invalid parameter be passed and convert fail.
 * @retval     E_OK                  Convert successful.
 *
 * @par Description
 * @details    This function is called to convert specify time to tick number. 		 
 *******************************************************************************
 */
#if CFG_TIME_TO_TICK_EN > 0
StatusType  CoTimeToTick(U8 hour,U8 minute,U8 sec,U16 millsec,U32* ticks)
{
#if CFG_PAR_CHECKOUT_EN >0
    /* Validate arguments to be within range */
    if((minute > 59)||(sec > 59)||(millsec > 999))
        return E_INVALID_PARAMETER;
#endif

    /* Convert time to ticks */
    *ticks = ((hour*3600) + (minute*60) + (sec)) * (CFG_SYSTICK_FREQ)\
              + (millsec*CFG_SYSTICK_FREQ + 500)/1000;
    return E_OK;
}
#endif    /* CFG_TIME_TO_TICK_EN  */

#endif    /* CFG_UTILITY_EN       */



