/**
  * @file    app_event.c
  * @author  jiejie
  * @version V1.0
  * @date    2019
  * @brief   this is an event api example
  */ 
#include <task.h>
#include <log.h>
#include <memp.h>
#include "iperf.h"
//#include "client.h"
/** Variable declaration */
DOS_TaskCB_t task1 = DOS_NULL;
DOS_TaskCB_t task2 = DOS_NULL;

/** Macro definition */


/** Function declaration */
extern void BSP_Init(void);
extern void TCPIP_Init(void);

void test_task1(void *Parameter)
{
//    dos_uint32 pri;
    
    TCPIP_Init();

    iperf_server_init();
//    client_init();

    printf("��������ʾ�����巢�����ݵ�������\n\n");

    printf("��������ģ�����£�\n\t ����<--����-->·��<--����-->������\n\n");

    printf("ʵ����ʹ��TCPЭ�鴫�����ݣ�������ΪTCP Server����������ΪTCP Client\n\n");

    printf("�����̵�IP��ַ����User/arch/sys_arch.h�ļ����޸�\n\n");

    printf("�����̲ο�<<LwIPӦ��ʵս����ָ��>>��15�� ʹ�� NETCONN �ӿڱ��\n\n");
    
    while(1)
    {
//        pri = Dos_Interrupt_Disable();
////        printf("task1\n");
//        Dos_Interrupt_Enable(pri);
        Dos_TaskSleep(100000);
        
    }
}


void test_task2(void *Parameter)
{
    dos_uint32 pri;
    while(1)
    {
        pri = Dos_Interrupt_Disable();
        printf("mem:%d\n",Dos_MemInfoGet());
        Dos_Interrupt_Enable(pri);
        Dos_TaskSleep(1000);
        
    }
}

/** main function */

int main(void)
{
    BSP_Init();

    Dos_SystemInit();
    
   
    task1 = Dos_TaskCreate( "test_task1",
                            &test_task1,
                            DOS_NULL,
                            2048,
                            6,
                            20);

    task2 = Dos_TaskCreate( "test_task2",
                            &test_task2,
                            DOS_NULL,
                            2048,
                            7,
                            0);
                
    Dos_Start();
  
}




/********************************END OF FILE***************************************/

