import time
name = input("请输入名称:")
时间 = time.strftime("%Y-%m-%d %H:%M",time.localtime(time.time()))
with open("LC截止频率计算记录.txt","a+") as file:
    file.write(时间 + "  " + name +  "\n")
    file.close()
while True:
    电感,电容,频率 = input("请输入参数：电感L（uH）、电容C（uF）、截止频率f（Hz）\n用中文逗号隔开，代求量输入为x：").split(',')
    if (电感 == "x" and 电容 !="x" and 频率 !="x") or (电感 != "x" and 电容 =="x" and 频率 !="x") or (电感 != "x" and 电容 !="x" and 频率 =="x"):
        if 电感 == "x":
            电容=eval(电容);频率=eval(频率);
            电感 = 1/(电容*1e-6*(2*3.14159*频率)**2)
            print("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz\n")
            with open("LC截止频率计算记录.txt","a+") as file:
                file.write("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz" + "\n")
                file.close()
        elif 电容 == "x":
            电感=eval(电感);频率=eval(频率);
            电容 = 1/(电感*1e-6*(2*3.14159*频率)**2)
            print("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz\n")
            with open("LC截止频率计算记录.txt","a+") as file:
                file.write("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz" + "\n")
                file.close()
        elif 频率 == "x":
            电感=eval(电感);电容=eval(电容);
            频率 = 1/(2*3.14159*(电感*1e-6*电容*1e-6)**(1/2))
            print("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz\n")
            with open("LC截止频率计算记录.txt","a+") as file:
                file.write("电容:" + str(电容) + "uF"   + " 电感：" + str(电感) + "uH" + " 频率：" + str(频率) + "Hz" + "\n")
                file.close()
    else:
        print("输入参数有误")
        continue
                    
    
    

