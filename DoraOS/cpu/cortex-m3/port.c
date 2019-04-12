#include <port.h>



static void Dos_TaskExit( void )
{
	while(1);
}

dos_uint32 *Dos_StackInit(dos_uint32 *top_of_stack , 
                          void *dos_task_entry,
                          dos_void *parameter)
{
	top_of_stack--; /* Offset added to account for the way the MCU uses the stack on entry/exit of interrupts. */
	*top_of_stack = 0x01000000;	/* xPSR */
	top_of_stack--;
	*top_of_stack = ((dos_uint32)dos_task_entry)&0xfffffffeUL;	/* PC */
	top_of_stack--;
	*top_of_stack = (dos_uint32)Dos_TaskExit;	/* LR */
	top_of_stack -= 5;	/* R12, R3, R2 and R1. */
	*top_of_stack = ( dos_uint32 ) parameter;	/* R0 */
	top_of_stack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

  return top_of_stack;
}



__asm void SVC_Handler( void )
{
  extern Dos_CurrentTCB;
  
  PRESERVE8

	ldr	r3, =Dos_CurrentTCB	/* ����Dos_CurrentTCB�ĵ�ַ��r3 */
	ldr r1, [r3]			/* ����Dos_CurrentTCB��r1 */
	ldr r0, [r1]			/* ����Dos_CurrentTCBָ���ֵ��r0��Ŀǰr0��ֵ���ڵ�һ�������ջ��ջ�� */
	ldmia r0!, {r4-r11}		/* ��r0Ϊ����ַ����ջ��������ݼ��ص�r4~r11�Ĵ�����ͬʱr0����� */
	msr psp, r0				/* ��r0��ֵ���������ջָ����µ�psp */
	isb
	mov r0, #0              /* ����r0��ֵΪ0 */
	msr	basepri, r0         /* ����basepri�Ĵ�����ֵΪ0�������е��ж϶�û�б����� */
	orr r14, #0xd           /* ����SVC�жϷ����˳�ǰ,ͨ����r14�Ĵ������4λ��λ����0x0D��
                               ʹ��Ӳ�����˳�ʱʹ�ý��̶�ջָ��PSP��ɳ�ջ���������غ�����߳�ģʽ������Thumb״̬ */
    
	bx r14                  /* �쳣���أ����ʱ��ջ�е�ʣ�����ݽ����Զ����ص�CPU�Ĵ�����
                               xPSR��PC��������ڵ�ַ����R14��R12��R3��R2��R1��R0��������βΣ�
                               ͬʱPSP��ֵҲ�����£���ָ������ջ��ջ�� */
}

__asm dos_uint32 Interrupt_Disable(void) 
{
  PRESERVE8
  mrs     r0, PRIMASK
  CPSID   I
  BX      LR
  nop
  nop
}

__asm void Interrupt_Enable(dos_uint32 pri) 
{
  PRESERVE8
  MSR     PRIMASK, r0
  BX      LR
  nop
  nop
}

__asm void PendSV_Handler( void )
{
	extern Dos_CurrentTCB;
	extern Dos_SwitchTask;

	PRESERVE8

    /* ������PendSVC Handlerʱ����һ���������еĻ�������
       xPSR��PC��������ڵ�ַ����R14��R12��R3��R2��R1��R0��������βΣ�
       ��ЩCPU�Ĵ�����ֵ���Զ����浽�����ջ�У�ʣ�µ�r4~r11��Ҫ�ֶ����� */
    /* ��ȡ����ջָ�뵽r0 */
	mrs r0, psp
	isb

	ldr	r3, =Dos_CurrentTCB		/* ����Dos_CurrentTCB�ĵ�ַ��r3 */
	ldr	r2, [r3]                /* ����Dos_CurrentTCB��r2 */

	stmdb r0!, {r4-r11}			/* ��CPU�Ĵ���r4~r11��ֵ�洢��r0ָ��ĵ�ַ */
	str r0, [r2]                /* ������ջ���µ�ջ��ָ��洢����ǰ����TCB�ĵ�һ����Ա����ջ��ָ�� */				
                               

	stmdb sp!, {r3, r14}        /* ��R3��R14��ʱѹ���ջ����Ϊ�������ú���Dos_SwitchTask,
                                  ���ú���ʱ,���ص�ַ�Զ����浽R14��,����һ�����÷���,R14��ֵ�ᱻ����,�����Ҫ��ջ����;
                                  R3����ĵ�ǰ���������TCBָ��(Dos_CurrentTCB)��ַ,�������ú���õ�,���ҲҪ��ջ���� */
  bl Interrupt_Disable
	dsb
	isb
	bl Dos_SwitchTask       /* ���ú���Dos_SwitchTask��Ѱ���µ���������,ͨ��ʹ����Dos_CurrentTCBָ���µ�������ʵ�������л� */ 
  
  bl Interrupt_Enable
	ldmia sp!, {r3, r14}        /* �ָ�r3��r14 */

	ldr r1, [r3]
	ldr r0, [r1] 				/* ��ǰ���������TCB��һ����������ջ��ջ��,����ջ��ֵ����R0*/
	ldmia r0!, {r4-r11}			/* ��ջ */
	msr psp, r0
	isb
	bx r14                          /* �쳣����ʱ,R14�б����쳣���ر�־,�������غ�����߳�ģʽ���Ǵ�����ģʽ��
                                   ʹ��PSP��ջָ�뻹��MSP��ջָ�룬������ bx r14ָ���Ӳ����֪��Ҫ���쳣���أ�
                                   Ȼ���ջ�����ʱ���ջָ��PSP�Ѿ�ָ�����������ջ����ȷλ�ã�
                                   ������������е�ַ����ջ��PC�Ĵ������µ�����Ҳ�ᱻִ�С�*/
	nop
  nop
  nop
  nop
}

__asm void Dos_StartFirstTask( void )
{
	PRESERVE8

	/* ��Cortex-M�У�0xE000ED08��SCB_VTOR����Ĵ����ĵ�ַ��
       �����ŵ������������ʼ��ַ����MSP�ĵ�ַ */
	ldr r0, =0xE000ED08
	ldr r0, [r0]
	ldr r0, [r0]

	/* ��������ջָ��msp��ֵ */
	msr msp, r0
    
	/* ʹ��ȫ���ж� */
	cpsie i
	cpsie f
	dsb
	isb
	
    /* ����SVCȥ������һ������ */
	svc 0  
	nop
	nop
}


dos_uint32 Dos_StartScheduler( void )
{
    /* ����PendSV �� SysTick ���ж����ȼ�Ϊ��� */
	SYSPRI2_REG |= PENDSV_PRI;
  SYSPRI2_REG |= SYSTICK_PRI;

  SYSTICK_LOAD_REG = ( DOS_SYSTEM_CLOCK_HZ / DOS_SYSTICK_CLOCK_HZ ) - 1UL;
  
	SYSTICK_CTRL_REG = ( SYSTICK_CLK_BIT |
                       SYSTICK_INT_BIT |
                       SYSTICK_ENABLE_BIT );

	/* ������һ�����񣬲��ٷ��� */
	Dos_StartFirstTask();

	/* ��Ӧ�����е����� */
	return 0;
}


void SysTick_Handler(void)
{
  dos_uint32 pri; 
  pri = Interrupt_Disable();
  
  
  INT_CTRL_REG = PENDSVSET_BIT;
  
  Interrupt_Enable(pri);
}

