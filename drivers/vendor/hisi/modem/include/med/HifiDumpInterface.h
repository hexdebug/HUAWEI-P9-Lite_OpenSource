

/*****************************************************************************
  1 ����ͷ�ļ�����
*****************************************************************************/
#include "vos.h"
#include "OmCodecInterface.h"

#ifndef __HIFIDUMPINTERFACE_H__
#define __HIFIDUMPINTERFACE_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif


/*****************************************************************************
  2 �궨��
*****************************************************************************/

/* �����������, �����HiFi��λʱ��Ҫdump���沢���ļ����ڴ����Ϣ */
#define OM_DEFINE_HIFI_DUMP_BLK_TABLE   \
OM_DUMP_BLK_INFO_STRU g_astOmHiFiToDumpBlks[] = \
{   \
    {0x2bff8000UL,                      0x8000UL},  /* DTCM, 32KB */    \
    {0x2c000000UL,                      0x2000UL},  /* ITCM,  8KB */    \
    {IPC_MAILBOX_HEAD_ADDR_HIFI2CARM,   sizeof(IPC_MAILBOX_HEAD_STRU)}, \
    {IPC_MAILBOX_ADDR_HIFI2CARM,        IPC_MAILBOX_SIZE_HIFI2CARM},    \
    {IPC_MAILBOX_HEAD_ADDR_CARM2HIFI,   sizeof(IPC_MAILBOX_HEAD_STRU)}, \
    {IPC_MAILBOX_ADDR_CARM2HIFI,        IPC_MAILBOX_SIZE_CARM2HIFI},    \
    {IPC_MAILBOX_HEAD_ADDR_HIFI2ZSP,    sizeof(IPC_MAILBOX_HEAD_STRU)}, \
    {IPC_MAILBOX_ADDR_HIFI2ZSP,         IPC_MAILBOX_SIZE_HIFI2ZSP},     \
    {IPC_MAILBOX_HEAD_ADDR_ZSP2HIFI,    sizeof(IPC_MAILBOX_HEAD_STRU)}, \
    {IPC_MAILBOX_ADDR_ZSP2HIFI,         IPC_MAILBOX_SIZE_ZSP2HIFI},     \
    {PC_VOICE_OM_CODEC_ADDR,              sizeof(OMMED_RING_BUFFER_CONTROL_STRU)},\
    {PC_VOICE_CODEC_OM_ADDR,              sizeof(OMMED_RING_BUFFER_CONTROL_STRU)},\
    {0x2bff8000UL,                      0x8000UL}   /* DTCM, 32KB */    \
}

/* ��ȡHiFi dump�ڴ����Ϣ�����ַ */
#define OM_GetHiFiDumpBlkTbl()          (&g_astOmHiFiToDumpBlks[0])

/* ��ȡHiFi dump�ڴ���� */
#define OM_GetHiFiDumpBlkNum()          (sizeof(g_astOmHiFiToDumpBlks)/sizeof(OM_DUMP_BLK_INFO_STRU))


/*****************************************************************************
  3 ö�ٶ���
*****************************************************************************/


/*****************************************************************************
  4 ��Ϣͷ����
*****************************************************************************/


/*****************************************************************************
  5 ��Ϣ����
*****************************************************************************/


/*****************************************************************************
  6 STRUCT����
*****************************************************************************/

/*****************************************************************************
 �� �� ��  : OM_DUMP_BLK_INFO_STRU
 ��������  : ����HiFi DUMP�ڴ��
*****************************************************************************/
typedef struct
{
    VOS_UINT32                          uwBlkAddr;          /* DUMP���׵�ַ */
    VOS_UINT32                          uwBlkLen;           /* DUMP�鳤��, ��λbyte */
}OM_DUMP_BLK_INFO_STRU;



/*****************************************************************************
  7 UNION����
*****************************************************************************/


/*****************************************************************************
  8 OTHERS����
*****************************************************************************/
/*
 DUMP�ļ���ʽ:

 ʱ��(32bit), �ڴ��1
 ʱ��(32bit), �ڴ��2
 ʱ��(32bit), �ڴ��3
 ...
 ʱ��(32bit), �ڴ��n
*/

/*****************************************************************************
  9 ȫ�ֱ�������
*****************************************************************************/


/*****************************************************************************
  10 ��������
*****************************************************************************/





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* end of HifiDumpInterface.h */