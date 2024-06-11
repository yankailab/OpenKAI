/*
 * SMSBL.cpp
 * 飞特SMSBL系列串行舵机应用层程序
 * 日期: 2020.6.17
 * 作者: 
 */

#include "SMSBL.h"

SMSBL::SMSBL()
{
	End = 0;
}

SMSBL::SMSBL(u8 End):SCSerial(End)
{
}

SMSBL::SMSBL(u8 End, u8 Level):SCSerial(End, Level)
{
}

int SMSBL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return genWrite(ID, SMSBL_ACC, bBuf, 7);
}

int SMSBL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return regWrite(ID, SMSBL_ACC, bBuf, 7);
}

void SMSBL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[])
{

    u8 offbuf[IDN][7];
    for(u8 i = 0; i<IDN; i++){
		if(Position[i]<0){
			Position[i] = -Position[i];
			Position[i] |= (1<<15);
		}
        u8 bBuf[7];
		u16 V;
		if(Speed){
			V = Speed[i];
		}else{
			V = 0;
		}
		if(ACC){
			bBuf[0] = ACC[i];
		}else{
			bBuf[0] = 0;
		}
        Host2SCS(bBuf+1, bBuf+2, Position[i]);
        Host2SCS(bBuf+3, bBuf+4, 0);
        Host2SCS(bBuf+5, bBuf+6, V);
        memcpy(offbuf[i], bBuf, 7);
    }
    snycWrite(ID, IDN, SMSBL_ACC, (u8*)offbuf, 7);
}

int SMSBL::WheelMode(u8 ID)
{
	return writeByte(ID, SMSBL_MODE, 1);		
}

int SMSBL::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<15);
	}
	u8 bBuf[2];
	bBuf[0] = ACC;
	genWrite(ID, SMSBL_ACC, bBuf, 1);
	Host2SCS(bBuf+0, bBuf+1, Speed);
	
	return genWrite(ID, SMSBL_GOAL_SPEED_L, bBuf, 2);
}

int SMSBL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, Enable);
}

int SMSBL::unLockEprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 0);
}

int SMSBL::LockEprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 1);
}

int SMSBL::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, 128);
}

int SMSBL::FeedBack(int ID)
{
	int nLen = Read(ID, SMSBL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return nLen;
}

int SMSBL::ReadPos(int ID)
{
	int Pos = -1;
	if(ID==-1){
		Pos = Mem[SMSBL_PRESENT_POSITION_H-SMSBL_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= Mem[SMSBL_PRESENT_POSITION_L-SMSBL_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Pos = readWord(ID, SMSBL_PRESENT_POSITION_L);
		if(Pos==-1){
			Err = 1;
		}
	}
	if(!Err && (Pos&(1<<15))){
		Pos = -(Pos&~(1<<15));
	}
	
	return Pos;
}

int SMSBL::ReadSpeed(int ID)
{
	int Speed = -1;
	if(ID==-1){
		Speed = Mem[SMSBL_PRESENT_SPEED_H-SMSBL_PRESENT_POSITION_L];
		Speed <<= 8;
		Speed |= Mem[SMSBL_PRESENT_SPEED_L-SMSBL_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Speed = readWord(ID, SMSBL_PRESENT_SPEED_L);
		if(Speed==-1){
			Err = 1;
			return -1;
		}
	}
	if(!Err && (Speed&(1<<15))){
		Speed = -(Speed&~(1<<15));
	}	
	return Speed;
}

int SMSBL::ReadLoad(int ID)
{
	int Load = -1;
	if(ID==-1){
		Load = Mem[SMSBL_PRESENT_LOAD_H-SMSBL_PRESENT_POSITION_L];
		Load <<= 8;
		Load |= Mem[SMSBL_PRESENT_LOAD_L-SMSBL_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Load = readWord(ID, SMSBL_PRESENT_LOAD_L);
		if(Load==-1){
			Err = 1;
		}
	}
	if(!Err && (Load&(1<<10))){
		Load = -(Load&~(1<<10));
	}
	return Load;
}

int SMSBL::ReadVoltage(int ID)
{	
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[SMSBL_PRESENT_VOLTAGE-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Voltage = readByte(ID, SMSBL_PRESENT_VOLTAGE);
		if(Voltage==-1){
			Err = 1;
		}
	}
	return Voltage;
}

int SMSBL::ReadTemper(int ID)
{	
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[SMSBL_PRESENT_TEMPERATURE-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Temper = readByte(ID, SMSBL_PRESENT_TEMPERATURE);
		if(Temper==-1){
			Err = 1;
		}
	}
	return Temper;
}

int SMSBL::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[SMSBL_MOVING-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Move = readByte(ID, SMSBL_MOVING);
		if(Move==-1){
			Err = 1;
		}
	}
	return Move;
}

int SMSBL::ReadCurrent(int ID)
{
	int Current = -1;
	if(ID==-1){
		Current = Mem[SMSBL_PRESENT_CURRENT_H-SMSBL_PRESENT_POSITION_L];
		Current <<= 8;
		Current |= Mem[SMSBL_PRESENT_CURRENT_L-SMSBL_PRESENT_POSITION_L];
	}else{
		Err = 0;
		Current = readWord(ID, SMSBL_PRESENT_CURRENT_L);
		if(Current==-1){
			Err = 1;
			return -1;
		}
	}
	if(!Err && (Current&(1<<15))){
		Current = -(Current&~(1<<15));
	}	
	return Current;
}

