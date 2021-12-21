#include "../lib/Config/DEV_Config.h"
#include "example.h"
#include "../lib/GUI/GUI_BMPfile.h"


#include <math.h>
#include <stdlib.h>     //exit()
#include <signal.h>     //signal()

#define Enhance false

#define USE_Factory_Test false

#define USE_Normal_Demo true

#define USE_Touch_Panel false

UWORD VCOM = 2510;

IT8951_Dev_Info Dev_Info;
UWORD Panel_Width;
UWORD Panel_Height;
UDOUBLE Init_Target_Memory_Addr;
int epd_mode = 0;	//0: no rotate, no mirror
					//1: no rotate, horizontal mirror, for 10.3inch
					//2: no totate, horizontal mirror, for 5.17inch
					//3: no rotate, no mirror, isColor, for 6inch color
					

void  Handler(int signo){
    Debug("\r\nHandler:exit\r\n");
    if(Refresh_Frame_Buf != NULL){
        free(Refresh_Frame_Buf);
        Debug("free Refresh_Frame_Buf\r\n");
        Refresh_Frame_Buf = NULL;
    }
    if(Panel_Frame_Buf != NULL){
        free(Panel_Frame_Buf);
        Debug("free Panel_Frame_Buf\r\n");
        Panel_Frame_Buf = NULL;
    }
    if(Panel_Area_Frame_Buf != NULL){
        free(Panel_Area_Frame_Buf);
        Debug("free Panel_Area_Frame_Buf\r\n");
        Panel_Area_Frame_Buf = NULL;
    }
    if(bmp_src_buf != NULL){
        free(bmp_src_buf);
        Debug("free bmp_src_buf\r\n");
        bmp_src_buf = NULL;
    }
    if(bmp_dst_buf != NULL){
        free(bmp_dst_buf);
        Debug("free bmp_dst_buf\r\n");
        bmp_dst_buf = NULL;
    }
    Debug("Going to sleep\r\n");
    EPD_IT8951_Sleep();
    DEV_Module_Exit();
    exit(0);
}

double estTime(clock_t start) {
	clock_t finish  = clock();
	return (double)((finish - start)/1000);
}


int main(int argc, char *argv[])
{
    char fileName[50];
    bool isDemo = true;
    int action = 0;
    
    signal(SIGINT, Handler);

    if (argc < 3){
        Debug("err:lack of info [vcom] [type]\r\n");
        Debug("Example: sudo ./epd -2.51 0\r\n");
        exit(1);
    }
    else{
        if (argc >= 4){
            sscanf(argv[3],"%d",&action);
            sscanf(argv[4],"%s",&fileName);
        }
    }
    //Init the BCM2835 Device
    if(DEV_Module_Init()!=0){
        return -1;
    }
    double temp;
    sscanf(argv[1],"%lf",&temp);
	sscanf(argv[2],"%d",&epd_mode);
    VCOM = (UWORD)(fabs(temp)*1000);
    Debug("VCOM value:%d\r\n", VCOM);
    Debug("Display mode:%d\r\n", epd_mode);
    Dev_Info = EPD_IT8951_Init(VCOM);

#if(Enhance)
    Debug("Attention! Enhanced driving ability, only used when the screen is blurred\r\n");
    Enhance_Driving_Capability();
#endif

    //get some important info from Dev_Info structure
    Panel_Width = Dev_Info.Panel_W;
    Panel_Height = Dev_Info.Panel_H;
    Init_Target_Memory_Addr = Dev_Info.Memory_Addr_L | (Dev_Info.Memory_Addr_H << 16);
    char* LUT_Version = (char*)Dev_Info.LUT_Version;
    if( strcmp(LUT_Version, "M641") == 0 ){
        //6inch e-Paper HAT(800,600), 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 4;
        Four_Byte_Align = true;
    }else if( strcmp(LUT_Version, "M841_TFAB512") == 0 ){
        //Another firmware version for 6inch HD e-Paper HAT(1448,1072), 6inch HD touch e-Paper HAT(1448,1072)
        A2_Mode = 6;
        Four_Byte_Align = true;
    }else if( strcmp(LUT_Version, "M841") == 0 ){
        //9.7inch e-Paper HAT(1200,825)
        A2_Mode = 6;
    }else if( strcmp(LUT_Version, "M841_TFA2812") == 0 ){
        //7.8inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }else if( strcmp(LUT_Version, "M841_TFA5210") == 0 ){
        //10.3inch e-Paper HAT(1872,1404)
        A2_Mode = 6;
    }else{
        //default set to 6 as A2 Mode
        A2_Mode = 6;
    }
    Debug("A2 Mode:%d\r\n", A2_Mode);
    /***init complete***/
	//EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
        char comment[128];
                    int j = 0;
        switch (action){
            case 0://clear
                EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, INIT_Mode);
                break;
            case 1://draw, file
                printf("-------------------------display Test # 1 - START-------------------------");
                printf("load file : %s", fileName);
                Display_BMP_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_4,fileName);
                DEV_Delay_ms(5000);
                printf("-------------------------display Test # 1 - END---------------------------");
                break;
            case 2://draw, demo            
                //for(int j = 1; j <= 20; j ++){
                    printf("-------------------------display Test # 2 - %d START-------------------------", j);                    
                    for(int i = 1; i <= 10 ; i ++){
                        char buff[50];
                        char bufff[3];
                        zerofill(i,bufff);
                        sprintf(buff, "/home/pi/bmp/IMG_%s.bmp",bufff); 
                        printf("display start # %d - %s", i, buff);
                        clock_t start = clock();
                        Display_BMP_Example(Panel_Width, Panel_Height, Init_Target_Memory_Addr, BitsPerPixel_1, buff);

                        printf("display end # %d - %s / est :%0f", i,buff, estTime(start));
                        DEV_Delay_ms(3000);
                        EPD_IT8951_Clear_Refresh(Dev_Info, Init_Target_Memory_Addr, GC16_Mode);
                        DEV_Delay_ms(1000);
                    }
                    printf("-------------------------display Test # 2 - %d END-------------------------", j);
                ///}
                break;
        }
    //EPD_IT8951_Standby();
    EPD_IT8951_Sleep();

    //In case RPI is transmitting image in no hold mode, which requires at most 10s

    DEV_Module_Exit();
    return 0;
    
}



