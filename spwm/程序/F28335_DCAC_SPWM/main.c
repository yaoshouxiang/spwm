#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"
#include "spwm.h"
#include "AdcAndEPwm.h"
#include "Pid.h"


/*******************************函数声明*********************************/
interrupt void EPwm2_isr();                            // PWM中断函数
/********************************宏定义*********************************/

#if (CPU_FRQ_150MHZ)                                   // Default - 150 MHz SYSCLKOUT
  #define ADC_MODCLK 0x3                               // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 150/(2*3)   = 25.0 MHz
#endif
#if (CPU_FRQ_100MHZ)
  #define ADC_MODCLK 0x2                               // HSPCLK = SYSCLKOUT/2*ADC_MODCLK2 = 100/(2*2)   = 25.0 MHz
#endif


/*******************************变量声明*********************************/
int Keyflag=0;

/*******************************结构体声明*******************************/
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

// Step 1.初始化系统控制:锁相环、看门狗、外设时钟使能
	InitSysCtrl();

	EALLOW;
	SysCtrlRegs.HISPCP.all = ADC_MODCLK;	   // 设置ADC工作的标准频率，这里为25MHz
	EDIS;

// Step 2.初始化管口
	InitEPwmGpio();

// Step 3.清除所有中断,并初始化中断向量表:禁止CPU中断
	DINT;                //禁止中断
// 初始化PIE控制寄存器至默认状态，默认状态是所有的PIE中断都被禁止
// 所有标志位都被清除
	InitPieCtrl();

// 禁止所有CPU中断，清除CPU中断相关标志位
	IER = 0x0000;
	IFR = 0x0000;

// 初始化中断向量表，中断向量就是采用指针指向中断服务程序入口地址
	InitPieVectTable();  //中断服务程序


//	MemCopy(&RamfuncsLoadStart,&RamfuncsLoadEnd,&RamfuncsRunStart);
//	InitFlash(); //烧写要加MemCopy InitFlash 这两句，并且cmd文件为F28335.cmd。如果在线调试必须删掉这两句，cmd文件为28335_RAM_link.cmd

	EALLOW;
	PieVectTable.EPWM2_INT=&EPwm2_isr;
	EDIS;


// Step 4.设置外设EPWM以及定义变量初始化、使能各种外设和中断

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 0;
	EDIS;

	InitEPwm2_3(&m_spwm);          //初始化EPWM配置

	EALLOW;
	SysCtrlRegs.PCLKCR0.bit.TBCLKSYNC = 1; //开始EPWM的时基时钟
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
			AccessResult(&adc);    // 向子函数传递一个自己定义的结构体，这样就可以传递大量参数

			if(Keyflag==1)         // 当检查出标志位，开始闭环，利用CCS连接仿真器手动改值，或者采用按键
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
	EPwm2Regs.CMPB=data+40;           // 另类死区方法

	EPwm3Regs.CMPA.half.CMPA=data;
	EPwm3Regs.CMPB=data+40;

	EPwm2Regs.ETCLR.bit.INT=1;
	PieCtrlRegs.PIEACK.all=PIEACK_GROUP3;
}

