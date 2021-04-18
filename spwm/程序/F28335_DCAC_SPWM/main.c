#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "spwm.h"
#include "AdcAndEPwm.h"
#include "Pid.h"


/*******************************��������*********************************/
interrupt void EPwm2_isr();                            // PWM�жϺ���
/********************************�궨��*********************************/

#if (CPU_FRQ_150MHZ)                                   // Default - 150 MHz SYSCLKOUT
  #define ADC_MODCLK 0x3                               // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
#endif
#if (CPU_FRQ_100MHZ)
  #define ADC_MODCLK 0x2                               // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 100/(2*2)   = 25.0 MHz
#endif


/*******************************��������*********************************/
int Keyflag=0;

/*******************************�ṹ������*******************************/
ADC_VARIABLES adc;
PID_DUNC1 pid;
SPWM_VRB m_spwm;
/**********************************************************************/


#define KEY1 GpioDataRegs.GPADAT.bit.GPIO12



void main(void)
{
	PID_Init(&pid);

	m_spwm.cnt=0;
	m_spwm.N=400;
	m_spwm.M=0.8;
	m_spwm.isChange=0;
	m_spwm.frequency=50;

	ChangeFrequency(&m_spwm);

// Step 1.��ʼ��ϵͳ����:���໷�����Ź�������ʱ��ʹ��
	InitSysCtrl();

	EALLOW;
	SysCtrlRegs.HISPCP.all = ADC_MODCLK;	   // ����ADC�����ı�׼Ƶ�ʣ�����Ϊ25MHz
	EDIS;

// Step 2.��ʼ���ܿ�
	InitEPwmGpio();

// Step 3.��������ж�,����ʼ���ж�������:��ֹCPU�ж�
	DINT;                //��ֹ�ж�
// ��ʼ��PIE���ƼĴ�����Ĭ��״̬��Ĭ��״̬�����е�PIE�ж϶�����ֹ
// ���б�־λ�������
	InitPieCtrl();

// ��ֹ����CPU�жϣ����CPU�ж���ر�־λ
	IER = 0x0000;
	IFR = 0x0000;

// ��ʼ���ж��������ж��������ǲ���ָ��ָ���жϷ��������ڵ�ַ
	InitPieVectTable();  //�жϷ������


//	MemCopy(&RamfuncsLoadStart,&RamfuncsLoadEnd,&RamfuncsRunStart);
//	InitFlash(); //��дҪ��MemCopy InitFlash �����䣬����cmd�ļ�ΪF28335.cmd��������ߵ��Ա���ɾ�������䣬cmd�ļ�Ϊ28335_RAM_link.cmd

	EALLOW;
	PieVectTable.EPWM2_INT=&EPwm2_isr;
	EDIS;


// Step 4.��������EPWM�Լ����������ʼ����ʹ�ܸ���������ж�

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EDIS;

	InitEPwm2_3(&m_spwm);          //��ʼ��EPWM����

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1; //��ʼEPWM��ʱ��ʱ��
	EDIS;

// Step 5. User specific code, enable interrupts
	IER |= M_INT3;     	      // Enable CPU INT3 which is connected to EPWM1-3 INT

	PieCtrlRegs.PIEIER3.bit.INTx2 = 1;   //Enable EPWM INTn in the PIE: Group 3 interrupt 1

// Enable global Interrupts and higher priority real-time debug events:
	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM

// Step 6. IDLE loop. Just sit and loop forever (optional):

	InitAdcAll(&adc);


	for(;;)
	{
		if(m_spwm.isPid==1)
		{
			m_spwm.isPid=0;
			AccessResult(&adc);    // ���Ӻ�������һ���Լ�����Ľṹ�壬�����Ϳ��Դ��ݴ�������

			if(Keyflag==1)         // ��������־λ����ʼ�ջ�������CCS���ӷ������ֶ���ֵ�����߲��ð���
			{
				pid.ActualSpeed=adc.Voltage;
				PID_realize(&pid);

				m_spwm.M=pid.OutPut;
				m_spwm.isChange=1;

			}
		}
	}

}

interrupt void EPwm2_isr()
{
	Uint16 data=m_spwm.CMPA_data[m_spwm.cnt++];

	if(m_spwm.cnt>=400)
	{
		m_spwm.cnt=0;
		m_spwm.isPid=1;
	}

	EPwm2Regs.CMPA.half.CMPA=data;
	EPwm2Regs.CMPB=data+40;           // ������������

	EPwm3Regs.CMPA.half.CMPA=data;
	EPwm3Regs.CMPB=data+40;

	EPwm2Regs.ETCLR.bit.INT=1;
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP3;
}

