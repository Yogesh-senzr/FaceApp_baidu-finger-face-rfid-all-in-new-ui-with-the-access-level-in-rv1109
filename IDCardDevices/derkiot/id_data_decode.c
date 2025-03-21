/*
 * id_data_decode.c
 *
 *  Created on: 2020年11月3日
 *      Author: derkiot
 */
#include "string.h"
#include "stdint.h"
#include "id_data_decode.h"
#include "utf.h"
#include "dk_utils.h"
#include "stdlib.h"
#include "tcp_client.h"
#include "DKReader.h"

char DK_IDCardNumber[20] = { 0x0 };
char DK_Name[15] = { 0x0 };
char DK_Sex[15] = { 0x0 };
int g_cardRegister = 0;

int getInfoLenth(char* info)
{
	int i=0;
	while(i<35)
	{
		if((info[2*i]==0x20) && (info[2*i+1]==0x00))
		{
			return i*3;
		}
		i++;
	}
	return 0;
}

int GetNational( int nationCode ,unsigned char*national)
{

	int ret = 0;
	switch (nationCode)
	{
	case 1:
		{
			ret = strlen("汉");
			memcpy(national,"汉",ret);
			break;
		}
	case 2:
		{	ret = strlen("蒙古");
			memcpy(national,"蒙古",ret);

			break;
		}
	case 3:
		{
			ret = strlen("回");
			memcpy(national,"回",ret);
			break;
		}
	case 4:
		{
			ret = strlen("藏");
			memcpy(national,"藏",ret);
			break;
		}
	case 5:
		{
			ret = strlen("维吾尔");
			memcpy(national,"维吾尔",ret);
			break;
		}
	case 6:
		{
			ret = strlen("苗");
			memcpy(national,"苗",ret);
			break;
		}
	case 7:
		{

			ret = strlen("");
			memcpy(national,"",ret);
			break;
		}
	case 8:
		{
			ret = strlen("壮");
			memcpy(national,"壮",ret);
			break;
		}
	case 9:
		{
			ret = strlen("布依");
			memcpy(national,"布依",ret);
			break;
		}
	case 10:
		{
			ret = strlen("朝鲜");
			memcpy(national,"朝鲜",ret);
			break;
		}
	case 11:
		{
			ret = strlen("满");
			memcpy(national,"满",ret);
			break;
		}
	case 12:
		{
			ret = strlen("侗");
			memcpy(national,"侗",ret);
			break;
		}
	case 13:
		{
			ret = strlen("瑶");
			memcpy(national,"瑶",ret);
			break;
		}
	case 14:
		{
			ret = strlen("白");
			memcpy(national,"白",ret);
			break;
		}
	case 15:
		{
			ret = strlen("土家");
			memcpy(national,"土家",ret);
			break;
		}
	case 16:
		{
			ret = strlen("哈尼");
			memcpy(national,"哈尼",ret);
			break;
		}
	case 17:
		{
			ret = strlen("哈萨克");
			memcpy(national,"哈萨克",ret);
			break;
		}
	case 18:
		{
			ret = strlen("傣");
			memcpy(national,"傣",ret);
			break;
		}
	case 19:
		{
			ret = strlen("黎");
			memcpy(national,"黎",ret);
			break;
		}
	case 20:
		{
			ret = strlen("傈僳");
			memcpy(national,"傈僳",ret);
			break;
		}
	case 21:
		{
			ret = strlen("佤");
			memcpy(national,"佤",ret);
			break;
		}
	case 22:
		{
			ret = strlen("畲");
			memcpy(national,"畲",ret);
			break;
		}
	case 23:
		{
			ret = strlen("高山");
			memcpy(national,"高山",ret);
			break;
		}
	case 24:
		{
			ret = strlen("拉祜");
			memcpy(national,"拉祜",ret);
			break;
		}
	case 25:
		{
			ret = strlen("水");
			memcpy(national,"水",ret);
			break;
		}
	case 26:
		{
			ret = strlen("东乡");
			memcpy(national,"东乡",ret);
			break;
		}
	case 27:
		{
			ret = strlen("纳西");
			memcpy(national,"纳西",ret);
			break;
		}
	case 28:
		{
			ret = strlen("景颇");
			memcpy(national,"景颇",ret);
			break;
		}
	case 29:
		{
			ret = strlen("柯尔克孜");
			memcpy(national,"柯尔克孜",ret);
			break;
		}
	case 30:
		{
			ret = strlen("土");
			memcpy(national,"土",ret);
			break;
		}
	case 31:
		{
			ret = strlen("达斡尔");
			memcpy(national,"达斡尔",ret);
			break;
		}
	case 32:
		{
			ret = strlen("仫佬");
			memcpy(national,"仫佬",ret);
			break;
		}
	case 33:
		{
			ret = strlen("羌");
			memcpy(national,"羌",ret);
			break;
		}
	case 34:
		{
			ret = strlen("布朗");
			memcpy(national,"布朗",ret);
			break;
		}
	case 35:
		{
			ret = strlen("撒拉");
			memcpy(national,"撒拉",ret);
			break;
		}
	case 36:
		{
			ret = strlen("毛南");
			memcpy(national,"毛南",ret);
			break;
		}
	case 37:
		{
			ret = strlen("仡佬");
			memcpy(national,"仡佬",ret);
			break;
		}
	case 38:
		{
			ret = strlen("锡伯");
			memcpy(national,"锡伯",ret);
			break;
		}
	case 39:
		{
			ret = strlen("阿昌");
			memcpy(national,"阿昌",ret);
			break;
		}
	case 40:
		{
			ret = strlen("普米");
			memcpy(national,"普米",ret);
			break;
		}
	case 41:
		{
			ret = strlen("塔吉克");
			memcpy(national,"塔吉克",ret);
			break;
		}
	case 42:
		{
			ret = strlen("怒");
			memcpy(national,"怒",ret);
			break;
		}
	case 43:
		{
			ret = strlen("乌孜别克");
			memcpy(national,"乌孜别克",ret);
			break;
		}
	case 44:
		{
			ret = strlen("俄罗斯");
			memcpy(national,"俄罗斯",ret);
			break;
		}
	case 45:
		{
			ret = strlen("鄂温克");
			memcpy(national,"鄂温克",ret);
			break;
		}
	case 46:
		{
			ret = strlen("德昂");
			memcpy(national,"德昂",ret);
			break;
		}
	case 47:
		{
			ret = strlen("");
			memcpy(national,"",ret);
			break;
		}
	case 48:
		{
			ret = strlen("裕固");
			memcpy(national,"裕固",ret);
			break;
		}
	case 49:
		{
			ret = strlen("京");
			memcpy(national,"京",ret);
			break;
		}
	case 50:
		{
			ret = strlen("塔塔尔");
			memcpy(national,"塔塔尔",ret);
			break;
		}
	case 51:
		{
			ret = strlen("独龙");
			memcpy(national,"独龙",ret);
			break;
		}
	case 52:
		{
			ret = strlen("鄂伦春");
			memcpy(national,"鄂伦春",ret);
			break;
		}
	case 53:
		{
			ret = strlen("赫哲");
			memcpy(national,"赫哲",ret);
			break;
		}
	case 54:
		{
			ret = strlen("门巴");
			memcpy(national,"门巴",ret);
			break;
		}
	case 55:
		{
			ret = strlen("珞巴");
			memcpy(national,"珞巴",ret);
			break;
		}
	case 56:
		{
			ret = strlen("基诺");
			memcpy(national,"基诺",ret);
			break;
		}
	default:
		ret = -1;
		break;
	}
	return ret;
}


void getIDCardNumber(St_IDCardData info,char*IDCardNumber)
{
	Utf16_To_Utf8 ((const UTF16*)info.id, (UTF8*)IDCardNumber,18 , strictConversion);
}

void getName(St_IDCardData info,char*Name)
{
	int lenth = getInfoLenth((char*)info.name);
	Utf16_To_Utf8 ((const UTF16*)info.name, (UTF8*)Name,lenth , strictConversion);
}

void getGender(St_IDCardData info,char* Gender)
{
	char GenderCode[1];
	memset(GenderCode,0x0,1);
	Utf16_To_Utf8 ((const UTF16*)info.gender, (UTF8*)GenderCode,1 , strictConversion);
	int genderCode = atoi(GenderCode);
	if(genderCode==1)
	{
		memcpy(Gender,"男",strlen("男"));
	}
	else
	{
		memcpy(Gender,"女",strlen("男"));
	}
}

//民族
void getNational(St_IDCardData info,char*National)
{
	char NationalCode[2];
	memset(NationalCode,0x0,2);
	Utf16_To_Utf8 ((const UTF16*)info.national, (UTF8*)NationalCode,2 , strictConversion);
	int nationalCode = atoi(NationalCode);
	GetNational(nationalCode,(unsigned char*)National);
}
void getBirthday(St_IDCardData info,char*Birthday)
{
	Utf16_To_Utf8 ((const UTF16*)info.birthday, (UTF8*)Birthday,8, strictConversion);
}

void getMaker(St_IDCardData info,char*Maker)
{
	int lenth = getInfoLenth((char*)info.maker);
	Utf16_To_Utf8 ((const UTF16*)info.maker, (UTF8*)Maker,lenth , strictConversion);
}

void getStartDate(St_IDCardData info,char*StartDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.start_date, (UTF8*)StartDate,8 , strictConversion);
}

void getEndDate(St_IDCardData info,char*EndDate)
{
	Utf16_To_Utf8 ((const UTF16*)info.end_date, (UTF8*)EndDate,8 , strictConversion);
}
void getAddress(St_IDCardData info,char*Address)
{
	int lenth = getInfoLenth((char*)info.address);
	Utf16_To_Utf8 ((const UTF16*)info.address,(UTF8*)Address,lenth , strictConversion);
}

void id_data_decode(uint8_t *id_info, uint16_t id_info_len, uint8_t *id_bitmap,
		uint16_t id_bitmap_len) {

	St_IDCardData stIDCardData;
	
	dk_log_hex("id_info", id_info, id_info_len);
	memcpy(&stIDCardData, id_info, id_info_len);

	//get idCardNumber
    memset(DK_IDCardNumber, 0x0, 20);
    getIDCardNumber(stIDCardData, DK_IDCardNumber);
    printf("\nIDCardNumber=%s\n", DK_IDCardNumber);

	//get name
    memset(DK_Name, 0x0, 15);
    getName(stIDCardData, DK_Name);
    printf("Name=%s\n", DK_Name);

    //获取性别
    memset(DK_Sex, 0x0, 15);
    getGender(stIDCardData, DK_Sex);
    printf("Sex=%s\n", DK_Sex);

	//get national
//	char National[15] = { 0x0 };
//	memset(National, 0x0, 15);
//	getNational(stIDCardData, National);
//	printf("National=%s\n", National);

	//get address
	char Address[100] = { 0x0 };
	memset(Address, 0x0, 100);
	getAddress(stIDCardData, Address);
	printf("Address=%s\n", Address);

	char Birthday[100] = { 0x0 };
	memset(Birthday, 0x0, 100);
	getBirthday(stIDCardData, Birthday);
	printf("Birthday=%s\n", Birthday);

	char Maker[100] = { 0x0 };
	memset(Maker, 0x0, 100);
	getMaker(stIDCardData, Maker);
	printf("Maker=%s\n", Maker);

	char StartDate[100] = { 0x0 };
	memset(StartDate, 0x0, 100);
	getStartDate(stIDCardData, StartDate);
	printf("StartDate=%s\n", StartDate);

	char EndDate[100] = { 0x0 };
	memset(EndDate, 0x0, 100);
	getEndDate(stIDCardData, EndDate);
	printf("EndDate=%s\n", EndDate);
	
#ifdef  LOCAL_DECODE_IMG
    int result = saveWlt2Bmp((char *)id_bitmap, "/mnt/user/photo.bmp"); //解码照片
	if (result == 1) {
        printf("saveWlt2Bmp success\r\n");
	} else {
        printf("saveWlt2Bmp(id_bitmap) = %d\r\n", result);
	}
#else
	remote_get_img();
#endif

}
