 

 #ifdef  __cplusplus
  #if  __cplusplus
  extern "C"{
  #endif
#endif

/*****************************************************************************
   1 Э��ջ��ӡ��㷽ʽ�µ�.C�ļ��궨��
*****************************************************************************/
#define THIS_FILE_ID                PS_FILE_ID_CPULOAD_C

#include "product_config.h"
#if(FEATURE_ON == FEATURE_ACPU_STAT)

/******************************************************************************
   ͷ�ļ�����
******************************************************************************/

#include "vos_config.h"
#include "ACpuLoadInterface.h"
#include "ACpuLoad.h"
#include "NVIM_Interface.h"
#include "TTFTaskPriority.h"
#include "product_config.h"
#include "TtfNvInterface.h"

/*�������PC������Ҫ��������LINUXͷ�ļ�*/
#if(VOS_WIN32 != VOS_OS_VER)
/*lint -e322 -e7*/
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <asm/cputime.h>
#include <linux/msa.h>
/*lint +e322 +e7*/
#else
#include "linuxstub.h"
#endif




/******************************************************************************
   2 �ⲿ������������
******************************************************************************/
/* �ο�/fs/proc/stat.cʵ�� */
#ifndef arch_idle_time
#define arch_idle_time(cpu)         0
#endif


/*****************************************************************************
   3 ˽�ж���
*****************************************************************************/

/*****************************************************************************
   4 ȫ�ֱ�������
*****************************************************************************/

/* ����Timer��ʱ���е�CPU����ͳ�� */
CPULOAD_STAT_INFO_STRU  g_stRegularCpuLoad;

/* ʵʱ��Ӧ�û����е�CPU����ͳ�� */
CPULOAD_STAT_INFO_STRU  g_stUserDefCpuLoad;

/*��¼�Ƿ��ǵ�һ�γ�ʱ*/
VOS_INT32               g_ulFirstTimeout = VOS_TRUE;

/*�ص�����ָ������*/
CPULOAD_RPT_HOOK_FUNC   g_pFunRptHooks[CPULOAD_MAX_HOOK_NUM];

/*CPUͳ�����ڶ�ʱ��*/
HTIMER                  g_stRegularCpuLoadTmr;

/*CPU LOAD NV ������Ϣ*/
CPULOAD_CFG_STRU        g_stNvCfg;

/******************************************************************************
   5 ����ʵ��
******************************************************************************/


VOS_VOID CPULOAD_ReadCpuStat(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* V9R1��Ŀ��ʹ�ú꿪���ж��Ƿ��ȡCPUʹ��������� */
    /* ��ȡ�������������ʱ����ܵ�����ʱ�� */
    msa_getcpu_idle(&(pstCpu->stCurrRecord.ulTotalTime), &(pstCpu->stCurrRecord.ulIdleTime));

    return;
}


VOS_VOID CPULOAD_UpdateSavInfo(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* ���±���Ľ�� */
    VOS_MemCpy(&(pstCpu->stPrevRecord), &(pstCpu->stCurrRecord), sizeof(CPULOAD_STAT_RECORD_STRU));

    return;
}


VOS_UINT32 CPULOAD_CalLoad(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* V9R1��Ŀ��ʹ�ú꿪�ؽ��д�׮���� */
    VOS_UINT32                          ulIdle;
    VOS_UINT32                          ulTotal;
    VOS_UINT32                          ulLoad;


    ulIdle      = MOD_SUB((pstCpu->stCurrRecord.ulIdleTime),(pstCpu->stPrevRecord.ulIdleTime),SLIENCE_MAX);
    ulTotal     = MOD_SUB((pstCpu->stCurrRecord.ulTotalTime),(pstCpu->stPrevRecord.ulTotalTime),SLIENCE_MAX);

    /* �쳣����������ϱ���tickֵ�����⣬CPU load����0�������൱�ڰѴ˹���off�� */
    if (0 == ulTotal)
    {
        ulLoad = 0;

    }
    else
    {
        ulLoad = (100 * (ulTotal - ulIdle))/ulTotal;
    }

    /* ����˴μ����� */
    pstCpu->ulCpuLoad = ulLoad;

    return ulLoad;
}


VOS_UINT32  CPULOAD_GetCpuLoad(VOS_VOID)
{
    /* ʹ�ú꿪���ж��Ƿ���Ҫ��׮���� */
    return g_stRegularCpuLoad.ulCpuLoad;
}


VOS_VOID CPULOAD_InvokeRtpHooks(VOS_UINT32  ulLoad)
{
    VOS_UINT32                          ulHookLoop;


    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        if ( VOS_NULL_PTR != g_pFunRptHooks[ulHookLoop] )
        {
            g_pFunRptHooks[ulHookLoop](ulLoad);
        }
    }

    return;
}


VOS_VOID CPULOAD_RegularTimeoutProc(VOS_VOID)
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stRegularCpuLoad;
    VOS_UINT32                          ulLoad;

    /* ��ȡ��ǰ��CPUͳ������ */
    CPULOAD_ReadCpuStat(pstCpu);

    ulLoad = CPULOAD_CalLoad(pstCpu);

    /* ����˴����ݣ��������´μ��� */
    CPULOAD_UpdateSavInfo(pstCpu);

    /* ��һ���յ�TimeOut��Ϣ�������ϴμ�¼Ϊ�գ����㲻׼ */
    if ( VOS_TRUE == g_ulFirstTimeout )
    {
        g_ulFirstTimeout = VOS_FALSE;
        return;
    }

    CPULOAD_InvokeRtpHooks(ulLoad);

    return;
}


VOS_VOID CPULOAD_RcvTimerExpireMsg(REL_TIMER_MSG *pTimerMsg)
{
    switch (pTimerMsg->ulName)
    {
        case CPULOAD_REGULAR_TMR_NAME:
            CPULOAD_RegularTimeoutProc();
            break;
        default:
            break;
    }

    return;
}


VOS_UINT32 CPULOAD_Init(VOS_VOID)
{
    VOS_UINT32                          ulHookLoop;
    VOS_UINT32                          ulRtn;
    CPULOAD_CFG_STRU                   *pstNvCfg = &g_stNvCfg;

    /* CPU IDĿǰֻ��һ������ֵΪ0 */
    VOS_MemSet((VOS_VOID *)&g_stRegularCpuLoad, 0, sizeof(CPULOAD_STAT_INFO_STRU));
    VOS_MemSet((VOS_VOID *)&g_stUserDefCpuLoad, 0, sizeof(CPULOAD_STAT_INFO_STRU));

    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        g_pFunRptHooks[ulHookLoop] = VOS_NULL_PTR;
    }

    ulRtn = NV_ReadEx(MODEM_ID_0, en_NV_Item_Linux_CPU_Moniter_Timer_Len, pstNvCfg, sizeof(CPULOAD_CFG_STRU));


    /* ����ȡen_NV_Item_Linux_CPU_Moniter_Timer_Lenʧ��,��NVֵ�Ƿ�����ʹ��Ĭ��ֵ */
    if( (NV_OK != ulRtn)
        || (CPULOAD_REGULAR_TMR_MIN_LEN > pstNvCfg->ulMonitorTimerLen)
        || (CPULOAD_REGULAR_TMR_MAX_LEN < pstNvCfg->ulMonitorTimerLen))
    {

        /*����쳣���򽫶�ʱ����ʼ��ΪĬ��ֵ400ms*/
        pstNvCfg->ulMonitorTimerLen = CPULOAD_REGULAR_TMR_DEF_LEN;
    }

    /* V9R1��Ŀ��ʹ�ú꿪���ж��Ƿ���Ҫ������ʱ�� */
    ulRtn = VOS_StartRelTimer(&g_stRegularCpuLoadTmr, ACPU_PID_CPULOAD,
                              pstNvCfg->ulMonitorTimerLen,
                              CPULOAD_REGULAR_TMR_NAME, 0,
                              VOS_RELTIMER_LOOP, VOS_TIMER_NO_PRECISION);
    if (VOS_OK != ulRtn)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID CPULOAD_PidMsgProc(MsgBlock *pRcvMsg)
{
    switch( pRcvMsg->ulSenderPid )
    {
        case VOS_PID_TIMER:
            CPULOAD_RcvTimerExpireMsg( (REL_TIMER_MSG *)pRcvMsg ); /* ���յ�TIMER��ʱ��Ϣʱ�Ĵ��� */
            break;

        default:
            break;
    }
}


VOS_UINT32 CPULOAD_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32  ulResult = VOS_ERR;


    switch (enPhase)
    {
        case   VOS_IP_LOAD_CONFIG:

            ulResult = VOS_RegisterPIDInfo(ACPU_PID_CPULOAD,
                                           (Init_Fun_Type)VOS_NULL_PTR,
                                           (Msg_Fun_Type)CPULOAD_PidMsgProc);
            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            ulResult = VOS_RegisterTaskPrio(ACPU_FID_CPULOAD, TTF_ACPULOAD_TASK_PRIO);

            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            ulResult = CPULOAD_Init();

            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            break;
        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;
        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32  CPULOAD_RegRptHook(CPULOAD_RPT_HOOK_FUNC pRptHook)
{
    VOS_UINT32                          ulHookLoop;
    VOS_INT32                           lLockKey;


    lLockKey = VOS_SplIMP();

    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        if ( VOS_NULL_PTR == g_pFunRptHooks[ulHookLoop] )
        {
            g_pFunRptHooks[ulHookLoop] = pRptHook;
            VOS_Splx(lLockKey);

            return VOS_OK;
        }
    }

    VOS_Splx(lLockKey);

    return VOS_ERR;
}


VOS_VOID CPULOAD_ResetUserDefLoad()
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stUserDefCpuLoad;


    /* ��ȡ��ǰ��CPUͳ������ */
    CPULOAD_ReadCpuStat(pstCpu);

    /* ����˴����ݣ��������´μ��� */
    CPULOAD_UpdateSavInfo(pstCpu);

    return;
}


VOS_UINT32 CPULOAD_GetUserDefLoad()
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stUserDefCpuLoad;
    VOS_UINT32                          ulLoad;


    /* ��ȡ��ǰ��CPUͳ������ */
    CPULOAD_ReadCpuStat(pstCpu);

    /* ���ʼ��ʱ���ݼ���õ� */
    ulLoad = CPULOAD_CalLoad(pstCpu);

    /* ����˴����ݣ��������´μ��� */
    CPULOAD_UpdateSavInfo(pstCpu);

    return ulLoad;
}


VOS_UINT32  CPULOAD_GetRegularTimerLen()
{
    return g_stNvCfg.ulMonitorTimerLen;
}

#else

/******************************************************************************
   1 ͷ�ļ�����
******************************************************************************/
#include "vos.h"

/******************************************************************************
   5 ����ʵ��
******************************************************************************/


VOS_UINT32 CPULOAD_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    return VOS_OK;
}

#endif

#ifdef  __cplusplus
  #if  __cplusplus
  }
  #endif
#endif
