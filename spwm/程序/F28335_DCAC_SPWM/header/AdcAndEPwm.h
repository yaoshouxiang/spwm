#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#ifndef ADC_AD
#define ADC_AD

typedef struct
{
	float Voltage;       //电压闭环
	Uint16 sample[50];
	Uint16 AdcCnt;
	Uint16 i;
	Uint16 j;
	Uint16 sum;
	Uint16 t1;

}ADC_VARIABLES;

void InitAdcAll(ADC_VARIABLES *p)
{
    InitAdc();//初始化ADC
	AdcRegs.ADCTRL1.bit.ACQ_PS = 0x1;
	AdcRegs.ADCTRL3.bit.ADCCLKPS = 0x00;
	AdcRegs.ADCTRL1.bit.SEQ_CASC = 1;        // 级联模式
	AdcRegs.ADCTRL1.bit.CONT_RUN = 1;       // 设置连续运行

	AdcRegs.ADCTRL1.bit.SEQ_OVRD = 1;       // Enable Sequencer override feature
    AdcRegs.ADCCHSELSEQ1.bit.CONV00 = 0x1;  // 电压闭环
	AdcRegs.ADCMAXCONV.bit.MAX_CONV1 = 0x0;  // 设置最大通道为1  如果为0x1最大通道为2
	AdcRegs.ADCTRL2.all = 0x2000;//软件启动SEQ1
	p->AdcCnt=0;
	p->sum=0;
}

void AccessResult(ADC_VARIABLES *adc)
{
	for(adc->AdcCnt=0;adc->AdcCnt<50;adc->AdcCnt++)
	{
		while (AdcRegs.ADCST.bit.INT_SEQ1== 0);//等待AD转换完成
		adc->sample[adc->AdcCnt]=AdcRegs.ADCRESULT0>>4;          //把AD结果寄存器的值存到sample中
		AdcRegs.ADCST.bit.INT_SEQ1_CLR = 1;  //清除中断

	}
	for(adc->i=0;adc->i<49;adc->i++)  // 连个FOR循环 完成采样数组的从小到大排序
	for(adc->j=0;adc->j<49-adc->i;adc->j++)
	{
		if(adc->sample[adc->j]>adc->sample[adc->j+1])
		{
			adc->t1=adc->sample[adc->j];
			adc->sample[adc->j]=adc->sample[adc->j+1];
			adc->sample[adc->j+1]=adc->t1;
		}
	}
	adc->sum=0;
	for(adc->i=0;adc->i<10;adc->i++)   //取最中间的十个数
	{
		adc->sum+=adc->sample[adc->i+19];
	}
	adc->Voltage=adc->sum/10.0*3.0/4096.0;    //按照公式转换电压

}

#endif
