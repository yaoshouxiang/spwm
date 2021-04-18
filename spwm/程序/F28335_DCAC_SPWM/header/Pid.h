#ifndef PID_H
#define PID_H
#define PID_DEBUG 1

#include "DSP28x_Project.h"

#define TIME_PERIOD 15000

typedef struct{
	float Kp;
	float Ti;
	float Td;


	float Ek;   //��Ӧe(k)
	float Ek_1; //��Ӧ e(k-1)
	float EK_2; //��Ӧe(k-2)
	float OutPut;//���
	float LastOutPut;
	Uint16 IncrementRate;

}PID_FUNC;

typedef struct
{
	double SetSpeed;       //��ѹ�ջ�
	float ActualSpeed;
	float err;
	float err_next;
	float err_last;
	double OutPut;
	float Kp,Ki,Kd;


}PID_DUNC1;

void PID_Init(PID_DUNC1 *p)
{
	p->ActualSpeed=1.18;  //ʵ��ֵ Ҳ���ǵ�ǰ������ֵ
	p->SetSpeed=1.18;     //�趨ֵ
	p->err = 0.0;         //��� SetSpeed-ActualSpeed
	p->err_last = 0.0;    //��һ������� ��e(k-1)
	p->err_next = 0.0;    //�����������     e(k-2)
	p->OutPut=0.8;       //ʵ�����
	p->Kp = 0;
	p->Ki = 0.004;
	p->Kd = 0.1;

}

void PID_realize(PID_DUNC1 *p)
{
	double incrementSpeed=0;

	p->err=p->SetSpeed-p->ActualSpeed;    // ����ʽPID

	incrementSpeed=p->Ki*p->err+p->Kd*(p->err-2*p->err_next+p->err_last);

	if((p->OutPut)>=0.3&&(p->OutPut)<=0.9 || p->OutPut<0.3&&incrementSpeed>0 || p->OutPut>0.9&&incrementSpeed<0)   // ��ֹռ�ձȹ����С�������عܣ����Ʊ仯�ٶ�
	{
		if(incrementSpeed>=0.01)
		{
			incrementSpeed=0.01;
		}
		if(incrementSpeed<=-0.01)
		{
			incrementSpeed=-0.01;
		}

		p->OutPut+=incrementSpeed;
	}

	p->err_last=p->err_next;
	p->err_next=p->err;
}
#endif
