/* Himax Android Driver Sample Code Ver 2.6
*
* Copyright (C) 2012 Himax Corporation.
*
* This software is licensed under the terms of the GNU General Public
* License version 2, as published by the Free Software Foundation, and
* may be copied, distributed, and modified under those terms.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
*/

//=============================================================================================================
// Segment list :
// Include Header file 
// Himax Define Options
// Himax Define Variable
// Himax Include Header file / Data Structure
// Himax Variable/Pre Declation Function
// Himax Normal Function
// Himax SYS Debug Function
// Himax Touch Work Function
// Himax Linux Driver Probe Function
// Other Function
//============================================================================================================= 


//=============================================================================================================
//
//	Segment : Include Header file 
//
//=============================================================================================================
	#include <linux/module.h>
	#include <linux/input/mt.h>
	#include <linux/interrupt.h>
	#include <linux/earlysuspend.h>
	#include <linux/platform_device.h>
	#include <linux/i2c.h>
	#include <linux/delay.h>
	#include <linux/gpio.h>
	#include <linux/device.h>
	#include <linux/jiffies.h>
	#include <linux/miscdevice.h>
	#include <linux/debugfs.h>
	#include <linux/irq.h>
	#include <linux/syscalls.h>
	#include <linux/time.h>
	#include <linux/usb/penwell_otg.h>
	
	// for linux 2.6.36.3
	#include <linux/cdev.h>
	#include <linux/slab.h>
	#include <linux/uaccess.h>
	#include <asm/ioctl.h>
	#include <linux/switch.h>
	#include <linux/proc_fs.h>
	#include <linux/seq_file.h>
	#include <linux/wakelock.h>
	#include <linux/regulator/consumer.h>
	
	#include <linux/fs.h>
	#include <asm/segment.h>
	#include <asm/uaccess.h>
	#include <linux/buffer_head.h>
	#include <linux/kthread.h>
	
//=============================================================================================================
//
//	Segment : Himax Define Options 
//
//=============================================================================================================
	//TODO START : Select the function you need!
	//------------------------------------------// Support Function Enable :
	#define HX_TP_PROC_DIAG									// Support Proc : Diag function				,default is open
	#define HX_TP_RAW_DATA									// Support Proc : Raw data function
	#define HX_TP_PROC_REGISTER								// Support Proc : Register function			,default is open
	#define HX_TP_PROC_DEBUG_LEVEL							// Support Proc : Debug Level function		,default is open
	#define HX_TP_PROC_FLASH_DUMP							// Support Proc : Flash dump function		,default is open
	#define HX_TP_PROC_SELF_TEST							// Support Proc : Self Test Function		,default is open
	//#define HX_EN_BUTTON									// Support Virtual key						,default is close
	//#define HX_EN_GESTURE									// Support Gesture , need porting			,default is close
	#define HX_RST_PIN_FUNC									// Support HW Reset							,default is open
	//#define HX_LOADIN_CONFIG								// Support Common FW,load in config
	//#define HX_PORTING_DEB_MSG							// Support Driver Porting Message			,default is close
	//#define HX_IREF_MODIFY								// Support IREF Modify Function				,default is close
	#define HX_FW_UPDATE_BY_I_FILE						// Support Update FW by i file				,default is close
	//TODO END

	//------------------------------------------// Support Different IC. Select one at one time.
	#define HX_85XX_A_SERIES_PWON		1
	#define HX_85XX_B_SERIES_PWON		2
	#define HX_85XX_C_SERIES_PWON		3
	#define HX_85XX_D_SERIES_PWON		4
	#define HX_85XX_E_SERIES_PWON		5
	
	//------------------------------------------// Supoort ESD Issue

	//Shouchung add for ESD solution
	// #define HX_85XX_E_nWATCH_DOG
	//Shouchung end

	//Shouchung modify for ESD solution
	#ifdef HX_RST_PIN_FUNC
		//#define HX_RST_BY_POWER						// Support Reset by power pin			,default is close
		#define HX_ESD_WORKAROUND						// Support ESD Workaround               ,default is close
		#define ENABLE_CHIP_RESET_MACHINE				// Support Chip Reset Workqueue         ,default is open
		//#define HX_ESD_WORKAROUND_HANDSHAKING
	#endif
	//Shouchung end

	#ifdef ENABLE_CHIP_RESET_MACHINE 
		#define HX_TP_PROC_RESET						// Support Proc : HW Reset function		,default is open
		#define ENABLE_CHIP_STATUS_MONITOR			// Support Polling ic status            ,default is close
		#define ENABLE_ADV_CHIP_STATUS_MONITOR			// Support Advance Polling ic status            ,default is close
	#endif

	//------------------------------------------// Support FW Bin checksum method,mapping with Hitouch *.bin
	#define HX_TP_BIN_CHECKSUM_SW		1
	#define HX_TP_BIN_CHECKSUM_HW		2
	#define HX_TP_BIN_CHECKSUM_CRC	3
	
//=============================================================================================================
//
//	Segment : Himax Define Variable
//
//=============================================================================================================
	//TODO START : Modify follows deinfe variable
	//#define HX_ENABLE_EDGE_TRIGGER						// define:Level triggle , un-defined:Level triggle
	#define HX_KEY_MAX_COUNT             4			// Max virtual keys
	#define DEFAULT_RETRY_CNT            3			// For I2C Retry count
	//TODO END

	//TODO START : Modify follows power gpio / interrupt gpio / reset gpio
	//------------------------------------------// power supply , i2c , interrupt gpio
	//Shouchung modify, initial porting touch in TF103CG
	#define HIMAX_PWR_GPIO				59 //not use
	//#define HIMAX_INT_GPIO				60
	//#define HIMAX_RST_GPIO				62
	#define HIMAX_INT_GPIO				62
	#define HIMAX_RST_GPIO				88
	//Shouchung end
	//TODO END

	//TODO START : Modify the I2C address
	//------------------------------------------// I2C
	#define HIMAX_I2C_ADDR				0x48
	//Shouchung modify, initial porting touch in TF103CG
	//#define HIMAX_TS_NAME			"himax-ts"
	#define HIMAX_TS_NAME			"hx8529-e88"
	//Shouchung end
	//TODO END

	//------------------------------------------// Input Device
	#define INPUT_DEV_NAME	"himax-touchscreen"	

	//------------------------------------------// Flash dump file
	#define FLASH_DUMP_FILE "/sdcard/Flash_Dump.bin"

	//------------------------------------------// Diag Coordinate dump file
	#define DIAG_COORDINATE_FILE "/sdcard/Coordinate_Dump.csv"

	//------------------------------------------// Virtual key
	#define HX_VKEY_0   KEY_MENU
	#define HX_VKEY_1   KEY_BACK
	#define HX_VKEY_2   KEY_HOMEPAGE
	#define HX_VKEY_3   104
	#define HX_KEY_ARRAY    {HX_VKEY_0, HX_VKEY_1, HX_VKEY_2, HX_VKEY_3}
	
	//------------------------------------------// Himax TP COMMANDS -> Do not modify the below definition
	#define HX_CMD_NOP                   0x00   /* no operation */
	#define HX_CMD_SETMICROOFF           0x35   /* set micro on */
	#define HX_CMD_SETROMRDY             0x36   /* set flash ready */
	#define HX_CMD_TSSLPIN               0x80   /* set sleep in */
	#define HX_CMD_TSSLPOUT              0x81   /* set sleep out */
	#define HX_CMD_TSSOFF                0x82   /* sense off */
	#define HX_CMD_TSSON                 0x83   /* sense on */
	#define HX_CMD_ROE                   0x85   /* read one event */
	#define HX_CMD_RAE                   0x86   /* read all events */
	#define HX_CMD_RLE                   0x87   /* read latest event */
	#define HX_CMD_CLRES                 0x88   /* clear event stack */
	#define HX_CMD_TSSWRESET             0x9E   /* TS software reset */
	#define HX_CMD_SETDEEPSTB            0xD7   /* set deep sleep mode */
	#define HX_CMD_SET_CACHE_FUN         0xDD   /* set cache function */
	#define HX_CMD_SETIDLE               0xF2   /* set idle mode */
	#define HX_CMD_SETIDLEDELAY          0xF3   /* set idle delay */
	#define HX_CMD_SELFTEST_BUFFER       0x8D   /* Self-test return buffer */
	#define HX_CMD_MANUALMODE            0x42
	#define HX_CMD_FLASH_ENABLE          0x43
	#define HX_CMD_FLASH_SET_ADDRESS     0x44
	#define HX_CMD_FLASH_WRITE_REGISTER  0x45
	#define HX_CMD_FLASH_SET_COMMAND     0x47
	#define HX_CMD_FLASH_WRITE_BUFFER    0x48
	#define HX_CMD_FLASH_PAGE_ERASE      0x4D
	#define HX_CMD_FLASH_SECTOR_ERASE    0x4E
	#define HX_CMD_CB                    0xCB
	#define HX_CMD_EA                    0xEA
	#define HX_CMD_4A                    0x4A
	#define HX_CMD_4F                    0x4F
	#define HX_CMD_B9                    0xB9
	#define HX_CMD_76                    0x76

	#define TOUCH_DC "1"
	#define TOUCH_IIR "2"
	#define TOUCH_BANK "3"
	#define TP_HIMAX 0
	#define TP_OFILM 3
	#define TP_HIMAX_FW 0
	#define TP_OFILM_FW 3

//=============================================================================================================
//
//	Segment : Himax Include Header file / Data Structure
//
//=============================================================================================================
	
	struct himax_i2c_setup_data 
	{
		unsigned		i2c_bus;  
		unsigned short	i2c_address;
		int				irq;
		char			name[I2C_NAME_SIZE];
	};	

	struct himax_ts_data 
	{
		struct i2c_client 						*client;
		struct input_dev 						*input_dev;
		struct workqueue_struct 				*himax_wq;
		struct work_struct 						work;
		int (*power)(int on);
		struct early_suspend early_suspend;
		int intr_gpio;
		// Firmware Information
		int fw_ver;
		int fw_id;
		int x_resolution;
		int y_resolution;
		// For Firmare Update 
		struct miscdevice firmware;
		struct attribute_group attrs;
		struct switch_dev touch_sdev;
		int abs_x_max;
		int abs_y_max;
		int rst_gpio;
		int init_success;
		struct regulator *vdd;
		
		// Wakelock Protect start
		struct wake_lock wake_lock;
		// Wakelock Protect end
		
		// Mutexlock Protect Start
		struct mutex mutex_lock;
		// Mutexlock Protect End
		
		//----[HX_TP_PROC_FLASH_DUMP]--------------------------------------------------------------------------start
			#ifdef HX_TP_PROC_FLASH_DUMP
			struct workqueue_struct 			*flash_wq;
			struct work_struct 					flash_work;
			#endif
		//----[HX_TP_PROC_FLASH_DUMP]----------------------------------------------------------------------------end
		
		//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
			#ifdef ENABLE_CHIP_RESET_MACHINE
			int retry_time;
			struct delayed_work himax_chip_reset_work;
			#endif
		//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end
		
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
			#ifdef ENABLE_CHIP_STATUS_MONITOR
			struct delayed_work himax_chip_monitor;
			int running_status;
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end	
	};

//=============================================================================================================
//
//	Segment : Himax Variable/Pre Declation Function
//
//=============================================================================================================
	static struct		himax_ts_data *private_ts				= NULL;					// himax_ts_data variable
	static uint8_t		IC_STATUS_CHECK							= 0xAA;					// for Hand shaking to check IC status
	static int			tpd_keys_local[HX_KEY_MAX_COUNT]		= HX_KEY_ARRAY;			// for Virtual key array
	struct i2c_client	*touch_i2c								= NULL;					// for I2C transfer
	//Shouchung add, initial porting touch in TF103CG
	static struct 		kobject *android_touch_kobj				= NULL;					// for Sys FS debug node
	//Shouchung end
	
	static unsigned char	IC_CHECKSUM							= 0;	
	static unsigned char 	IC_TYPE 							= 0;

	static int				HX_TOUCH_INFO_POINT_CNT				= 0;

	static int				HX_RX_NUM		= 0;
	static int				HX_TX_NUM		= 0;
	static int				HX_BT_NUM		= 0;
	static int				HX_X_RES		= 0;
	static int				HX_Y_RES		= 0;
	static int				HX_MAX_PT		= 0;
	static bool 			HX_XY_REVERSE		= false;
	static bool				HX_INT_IS_EDGE	= false;
	static unsigned int		FW_VER_MAJ_FLASH_ADDR;
	static unsigned int	 	FW_VER_MAJ_FLASH_LENG;
	static unsigned int 	FW_VER_MIN_FLASH_ADDR;
	static unsigned int 	FW_VER_MIN_FLASH_LENG;	 
	static unsigned int 	CFG_VER_MAJ_FLASH_ADDR;
	static unsigned int 	CFG_VER_MAJ_FLASH_LENG;
	static unsigned int 	CFG_VER_MIN_FLASH_ADDR; 
	static unsigned int 	CFG_VER_MIN_FLASH_LENG;
	
	static u16				FW_VER_MAJ_buff[1];				// for Firmware Version
	static u16				FW_VER_MIN_buff[1];
	static u16				CFG_VER_MAJ_buff[12];
	static u16				CFG_VER_MIN_buff[12];


	static bool			is_suspend		= false;
	
	static int	 		hx_point_num	= 0;				// for himax_ts_work_func use
	static int			p_point_num		= 0xFFFF;									
	static int			tpd_key				= 0;
	static int			tpd_key_old		= 0xFF;

	static uint8_t rFE96_setting[8] = { 0xB4, 0x64, 0x3F, 0x3F, 0x3C, 0x00, 0x3C, 0x00};	// for initializing rFE96 setting parameters
	static int self_test_delay_time = 4;	// for initializing self-test delay time
	static bool diag_dump_to_factory = false;	// for store function

	static ssize_t himax_self_test_setting(struct device *dev,struct device_attribute *attr, const char *buf, size_t count);
	static ssize_t himax_chip_raw_data_store_func(char *buf, char *PreFilePath);
	static ssize_t himax_raw_data_read_func(struct seq_file *s, char *PreFilePath);
	static unsigned int Flag_print_point = 0;
	static unsigned int Touch_flag = 0;

	struct workqueue_struct *ac_work_queue = NULL;
	struct delayed_work ac_work;

	static int i2c_himax_read(struct i2c_client *client, uint8_t command, uint8_t *data, uint8_t length, uint8_t toRetry); 
	static int i2c_himax_write(struct i2c_client *client, uint8_t command, uint8_t *data, uint8_t length, uint8_t toRetry);
	static int i2c_himax_master_write(struct i2c_client *client, uint8_t *data, uint8_t length, uint8_t toRetry);
	static int i2c_himax_write_command(struct i2c_client *client, uint8_t command, uint8_t toRetry);
	
	static int himax_lock_flash(void);
	static int himax_unlock_flash(void);
	
	static int himax_hang_shaking(void); 													// Hand shaking function
	static int himax_ts_poweron(struct himax_ts_data *ts_modify);							// Power on

	//----[HX_LOADIN_CONFIG]--------------------------------------------------------------------------------start
		#ifdef HX_LOADIN_CONFIG
		unsigned char c1[] 	 =	{ 0x37, 0xFF, 0x08, 0xFF, 0x08};
		unsigned char c2[] 	 =	{ 0x3F, 0x00};
		unsigned char c3[] 	 =	{ 0x62, 0x01, 0x00, 0x01, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c4[] 	 =	{ 0x63, 0x10, 0x00, 0x10, 0x30, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c5[] 	 =	{ 0x64, 0x01, 0x00, 0x01, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c6[] 	 =	{ 0x65, 0x10, 0x00, 0x10, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c7[] 	 =	{ 0x66, 0x01, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c8[] 	 =	{ 0x67, 0x10, 0x00, 0x10, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c9[] 	 =	{ 0x68, 0x01, 0x00, 0x01, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c10[]	 =	{ 0x69, 0x10, 0x00, 0x10, 0x30, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c11[]	 =	{ 0x6A, 0x01, 0x00, 0x01, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c12[]	 =	{ 0x6B, 0x10, 0x00, 0x10, 0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c13[]	 =	{ 0x6C, 0x01, 0x00, 0x01, 0x30, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c14[]	 =	{ 0x6D, 0x10, 0x00, 0x10, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c15[]	 =	{ 0x6E, 0x01, 0x00, 0x01, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c16[]	 =	{ 0x6F, 0x10, 0x00, 0x10, 0x20, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c17[]	 =	{ 0x70, 0x01, 0x00, 0x01, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c18[]	 =	{ 0x7B, 0x03};
		unsigned char c19[]	 =	{ 0x7C, 0x00, 0xD8, 0x8C};
		unsigned char c20[]	 =	{ 0x7F, 0x00, 0x04, 0x0A, 0x0A, 0x04, 0x00, 0x00, 0x00};
		unsigned char c21[]	 =	{ 0xA4, 0x94, 0x62, 0x94, 0x86};
		unsigned char c22[]	 =	{ 0xB4, 0x04, 0x01, 0x01, 0x01, 0x01, 0x03, 0x0F, 0x04, 0x07, 0x04, 0x07, 0x04, 0x07, 0x00};
		unsigned char c23[]	 =	{ 0xB9, 0x01, 0x36};
		unsigned char c24[]	 =	{ 0xBA, 0x00};
		unsigned char c25[]	 =	{ 0xBB, 0x00};
		unsigned char c26[]	 =	{ 0xBC, 0x00, 0x00, 0x00, 0x00};
		unsigned char c27[]	 =	{ 0xBD, 0x04, 0x0C};
		unsigned char c28[]	 =	{ 0xC2, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		unsigned char c29[]	 =	{ 0xC5, 0x0A, 0x1D, 0x00, 0x10, 0x1A, 0x1E, 0x0B, 0x1D, 0x08, 0x16};
		unsigned char c30[]	 =	{ 0xC6, 0x1A, 0x10, 0x1F};
		unsigned char c31[]	 =	{ 0xC9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x15, 0x15, 0x17, 0x17, 0x19, 0x19, 0x1F, 0x1F, 0x1B, 0x1B, 0x1D, 0x1D, 0x21, 0x21, 0x23, 0x23, 
															0x25, 0x25, 0x27, 0x27, 0x29, 0x29, 0x2B, 0x2B, 0x2D, 0x2D, 0x2F, 0x2F, 0x16, 0x16, 0x18, 0x18, 0x1A, 0x1A, 0x20, 0x20, 0x1C, 0x1C, 
															0x1E, 0x1E, 0x22, 0x22, 0x24, 0x24, 0x26, 0x26, 0x28, 0x28, 0x2A, 0x2A, 0x2C, 0x2C, 0x2E, 0x2E, 0x30, 0x30, 0x00, 0x00, 0x00};
		unsigned char c32[] 	= { 0xCB, 0x01, 0xF5, 0xFF, 0xFF, 0x01, 0x00, 0x05, 0x00, 0x9F, 0x00, 0x00, 0x00};
		unsigned char c33[] 	= { 0xD0, 0x06, 0x01};
		unsigned char c34[] 	= { 0xD3, 0x06, 0x01};
		unsigned char c35[] 	= { 0xD5, 0xA5, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
		
		unsigned char c36[] 	= { 0x40,0x01, 0x5A
													, 0x77, 0x02, 0xF0, 0x13, 0x00, 0x00
													, 0x56, 0x10, 0x14, 0x18, 0x06, 0x10, 0x0C, 0x0F, 0x0F, 0x0F, 0x52, 0x34, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};//start:0x00 ,size 31
	
		unsigned char c37[] 	= { 0x40, 0xA5, 0x00, 0x80, 0x82, 0x85, 0x00
														, 0x35, 0x25, 0x0F, 0x0F, 0x83, 0x3C, 0x00, 0x00
														, 0x11, 0x11, 0x00, 0x00
														, 0x01, 0x01, 0x00, 0x0A, 0x00, 0x00
														, 0x10, 0x02, 0x10, 0x64, 0x00, 0x00}; // start 0x1E :size 31

		unsigned char c38[] 	= {	0x40, 0x40, 0x38, 0x38, 0x02, 0x14, 0x00, 0x00, 0x00
														, 0x04, 0x03, 0x12, 0x06, 0x06, 0x00, 0x00, 0x00}; // start:0x3C ,size 17

		unsigned char c39[] 	= {	0x40, 0x18, 0x18, 0x05, 0x00, 0x00, 0xD8, 0x8C, 0x00, 0x00, 0x42, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00
														, 0x10, 0x02, 0x80, 0x00, 0x00, 0x00, 0x00, 0x0C}; //start 0x4C,size 25

		unsigned char c40[] 	= {	0x40, 0x10, 0x12, 0x20, 0x32, 0x01, 0x04, 0x07, 0x09
														, 0xB4, 0x6E, 0x32, 0x00
														, 0x0F, 0x1C, 0xA0, 0x16
														, 0x00, 0x00, 0x04, 0x38, 0x07, 0x80}; //start 0x64,size 23

		unsigned char c41[]	 	= {	0x40, 0x03, 0x2F, 0x08, 0x5B, 0x56, 0x2D, 0x05, 0x00, 0x69, 0x02, 0x15, 0x4B, 0x6C, 0x05
														, 0x03, 0xCE, 0x09, 0xFD, 0x58, 0xCC, 0x00, 0x00, 0x7F, 0x02, 0x85, 0x4C, 0xC7, 0x00};//start 0x7A,size 29

		unsigned char c42[] 	= {	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //start 0x96,size 9

		unsigned char c43_1[]	= {	0x40, 0x00, 0xFF, 0x15, 0x28, 0x01, 0xFF, 0x16, 0x29, 0x02, 0xFF, 0x1B, 0x2A, 0x03, 0xFF, 0x1C, 0xFF, 0x04, 0xFF, 0x1D, 0xFF, 0x05, 0x0F, 0x1E, 0xFF, 0x06, 0x10, 0x1F, 0xFF, 0x07, 0x11, 0x20}; //start 0x9E,size 32
		unsigned char c43_2[] = {	0x40, 0xFF, 0x08, 0x12, 0x21, 0xFF, 0x09, 0x13, 0x22, 0xFF, 0x0A, 0x14, 0x23, 0xFF, 0x0B, 0x17, 0x24, 0xFF, 0x0C, 0x18, 0x25, 0xFF, 0x0D, 0x19, 0x26, 0xFF, 0x0E, 0x1A, 0x27, 0xFF}; //start 0xBD,size 29
		
		unsigned char c44_1[] = {	0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //start 0xDA,size 32
		unsigned char c44_2[] = {	0x40, 0x00, 0x00, 0x00, 0x00, 0x00}; //0xF9 size 6
		unsigned char c45[] 	= {	0x40, 0x1D, 0x00}; //start 0xFE,size 3
		#endif
	//----[HX_LOADIN_CONFIG]----------------------------------------------------------------------------------end

	//----[HX_FW_UPDATE_BY_I_FILE]--------------------------------------------------------------------------start
		#ifdef HX_FW_UPDATE_BY_I_FILE
		static struct task_struct		*i_update_firmware_tsk;
		static bool i_Needupdate = true;
		static unsigned char i_isTP_Updated = 0;
		static unsigned char* i_CTPM_FW;
		static unsigned char i_CTPM_FW_Himax[]=
		{
		    #include "hx8529_fw_tf103cg_himax.h"
		};
		static unsigned char i_CTPM_FW_Ofilm[]=
		{
		    #include "hx8529_fw_tf103cg_ofilm.h"
		};
		#endif
	//----[HX_FW_UPDATE_BY_I_FILE]----------------------------------------------------------------------------end
	
	//----[CONFIG_HAS_EARLYSUSPEND]-------------------------------------------------------------------------start
		#ifdef CONFIG_HAS_EARLYSUSPEND
		static void himax_ts_early_suspend(struct early_suspend *h);
		static void himax_ts_late_resume(struct early_suspend *h);
		#endif
	//----[CONFIG_HAS_EARLYSUSPEND]---------------------------------------------------------------------------end
	
	//----[HX_IREF_MODIFY]----------------------------------------------------------------------------------start
		#ifdef HX_IREF_MODIFY 
		unsigned char SFR_3u_1[16][2] = {{0x18,0x06},{0x18,0x16},{0x18,0x26},{0x18,0x36},{0x18,0x46},
										{0x18,0x56},{0x18,0x66},{0x18,0x76},{0x18,0x86},{0x18,0x96},
										{0x18,0xA6},{0x18,0xB6},{0x18,0xC6},{0x18,0xD6},{0x18,0xE6},
										{0x18,0xF6}};
		
		unsigned char SFR_6u_1[16][2] = {{0x98,0x04},{0x98,0x14},{0x98,0x24},{0x98,0x34},{0x98,0x44},
										{0x98,0x54},{0x98,0x64},{0x98,0x74},{0x98,0x84},{0x98,0x94},
										{0x98,0xA4},{0x98,0xB4},{0x98,0xC4},{0x98,0xD4},{0x98,0xE4},
										{0x98,0xF4}};                        	
		#endif
	//----[HX_IREF_MODIFY]------------------------------------------------------------------------------------end
	
	/*	
	//----[HX_EN_GESTURE]-----------------------------------------------------------------------------------start
		#ifdef HX_EN_GESTURE 
		static int 				Dist_Cal_EX = 0xFFFF;
		static int 				Dist_Cal_Now = 0xFFFF;
		static int 				ZoomInCnt = 0;
		static int 				ZoomOutCnt = 0; 
		#endif 
	//----[HX_EN_GESTURE]-------------------------------------------------------------------------------------end
	*/

	//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------start
		#ifdef HX_ESD_WORKAROUND	
		static u8 		ESD_RESET_ACTIVATE 	= 2;
		static u8 		ESD_COUNTER 				= 0;
		static u8 		TOUCH_UP_COUNTER 			= 0;
		static int 		ESD_COUNTER_SETTING = 3;
		
		void ESD_HW_REST(void);
		#endif 
	//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------------end
	
	//----[HX_TP_PROC_SELF_TEST]-----------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_SELF_TEST 
		#define	HIMAX_PROC_SELFTEST_FILE	"himax_selftest"
		static struct proc_dir_entry *himax_proc_selftest_file;
		static int himax_chip_self_test(void);
		#endif 
	//----[HX_TP_PROC_SELF_TEST]-------------------------------------------------------------------------------end
		#ifdef HX_TP_PROC_RESET
		#define	HIMAX_PROC_RESET_FILE	"himax_reset"
		static struct proc_dir_entry *himax_proc_reset_file;
		#endif
	
	//----[HX_TP_PROC_DEBUG_LEVEL]---------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_DEBUG_LEVEL
		#define	HIMAX_PROC_DEBUG_FILE	"himax_debug"
		static struct proc_dir_entry *himax_proc_debug_file;
		static uint8_t 	debug_log_level= 0;
		static bool	fw_update_complete = false;
		static bool irq_enable = false;
		static int handshaking_result = 0;
		static unsigned char debug_level_cmd = 0;
		static unsigned char upgrade_fw[32*1024];
		
		static uint8_t getDebugLevel(void);
		#endif
	//----[HX_TP_PROC_DEBUG_LEVEL]-----------------------------------------------------------------------------end
	
	//----[HX_TP_PROC_REGISTER]------------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_REGISTER
		#define	HIMAX_PROC_REGISTER_FILE	"himax_register"
		static struct proc_dir_entry *himax_proc_register_file;
		static uint8_t register_command 			= 0;
		static uint8_t multi_register_command = 0;
		static uint8_t multi_register[8] 			= {0x00};
		static uint8_t multi_cfg_bank[8] 			= {0x00};
		static uint8_t multi_value[1024] 			= {0x00};
		static bool 	config_bank_reg 				= false;
		#endif
	//----[HX_TP_PROC_REGISTER]--------------------------------------------------------------------------------end

		#define	HIMAX_RAW_DATA_FILE	"himax_raw"
		static struct proc_dir_entry *himax_raw_data_file;
	
	//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------start
		#define	HIMAX_PROC_DIAG_FILE	"himax_diag"
		static struct proc_dir_entry *himax_proc_diag_file;
		#ifdef HX_TP_PROC_DIAG || HX_TP_RAW_DATA
		static uint8_t x_channel 		= 0; 
		static uint8_t y_channel 		= 0; 
		static uint8_t *diag_mutual = NULL;
		static uint8_t diag_command = 0;
		static uint8_t diag_coor[128];// = {0xFF};
		
		static uint8_t diag_self[100] = {0};
		static int g_usb_state;
		
		static uint8_t *getMutualBuffer(void);
		static uint8_t *getSelfBuffer(void);
		static uint8_t 	getDiagCommand(void);
		static uint8_t 	getXChannel(void);
		static uint8_t 	getYChannel(void);
		
		static void 		setMutualBuffer(void);
		static void 		setXChannel(uint8_t x);
		static void 		setYChannel(uint8_t y);
	
		static uint8_t	coordinate_dump_enable = 0;
		struct file			*coordinate_fn;
		#endif
	//----[HX_TP_PROC_DIAG]------------------------------------------------------------------------------------end

	
	//----[HX_TP_PROC_FLASH_DUMP]----------------------------------------------------------------------------start	
		#ifdef HX_TP_PROC_FLASH_DUMP
		#define	HIMAX_PROC_FLASH_FILE	"himax_flash"
		static struct proc_dir_entry *himax_proc_flash_file;
		static uint8_t *flash_buffer 				= NULL;
		static uint8_t flash_command 				= 0;
		static uint8_t flash_read_step 			= 0;
		static uint8_t flash_progress 			= 0;
		static uint8_t flash_dump_complete	= 0;
		static uint8_t flash_dump_fail 			= 0;
		static uint8_t sys_operation				= 0;
		static uint8_t flash_dump_sector	 	= 0;
		static uint8_t flash_dump_page 			= 0;
		static bool    flash_dump_going			= false;			
	
		static uint8_t getFlashCommand(void);
		static uint8_t getFlashDumpComplete(void);
		static uint8_t getFlashDumpFail(void);
		static uint8_t getFlashDumpProgress(void);
		static uint8_t getFlashReadStep(void);
		static uint8_t getSysOperation(void);
		static uint8_t getFlashDumpSector(void);
		static uint8_t getFlashDumpPage(void);
		static bool	   getFlashDumpGoing(void);
		
		static void setFlashBuffer(void);
		static void setFlashCommand(uint8_t command);
		static void setFlashReadStep(uint8_t step);
		static void setFlashDumpComplete(uint8_t complete);
		static void setFlashDumpFail(uint8_t fail);
		static void setFlashDumpProgress(uint8_t progress);
		static void setSysOperation(uint8_t operation);
		static void setFlashDumpSector(uint8_t sector);
		static void setFlashDumpPage(uint8_t page);
		static void setFlashDumpGoing(bool going);
		#endif
	//----[HX_TP_PROC_FLASH_DUMP]------------------------------------------------------------------------------end

		static unsigned int TP_ID = TP_HIMAX;//0xFF; KK porting
		static unsigned int FW_ID = TP_HIMAX_FW;
		extern int Read_TP_ID(void);

	//=============================================================================================================
	//
	//  Segment : Other Function
	//
	//=============================================================================================================
	int setTouchACMode(int usb_state) {
		g_usb_state = usb_state;
		queue_delayed_work(ac_work_queue, &ac_work, 0);

		return 0;
	}

	int himax_cable_status(struct work_struct *work)
	{
		uint8_t buf0[2] = {0};
		int ret = 0;
		int status = g_usb_state;
		//for debug message start
		uint16_t loop_i;
		uint8_t inData[128];
		memset(inData, 0x00, sizeof(inData));
		//for debug message end
		printk("[Himax] %s: cable_status=%d, init_success=%d.\n", __func__, status,private_ts->init_success);

		if(private_ts->init_success == 1)
		{
			if(status == 0x02) //non usb
			{
				printk("[Himax] %s: cable_status=%d, init_success=%d.\n", __func__, status,private_ts->init_success);
				buf0[0] = 0x00;
				ret = i2c_himax_write(touch_i2c, 0xF0 ,&buf0[0], 1, DEFAULT_RETRY_CNT);
				printk("[Himax] %s: ret=%d. \n", __func__, ret);

				//for debug message start
				i2c_himax_read(touch_i2c, 0xF0, inData, 128, DEFAULT_RETRY_CNT);

				printk("[Himax] command: 0xF0 \n");
				for (loop_i = 0; loop_i < 128; loop_i++)
				{
					printk("0x%2.2X ", inData[loop_i]);
					if ((loop_i % 16) == 15)
					{
						printk("\n");
					}
				}
				//for debug message end
			}
			else if((status == 0x00) || (status == 0x01)) //usb plug in
			{
				printk("[Himax] %s: cable_status=%d, init_success=%d.\n", __func__, status,private_ts->init_success);
				buf0[0] = 0x01;
				ret = i2c_himax_write(touch_i2c, 0xF0 ,&buf0[0], 1, DEFAULT_RETRY_CNT);
				printk("[Himax] %s: ret=%d. \n", __func__, ret);

				//for debug message start
				i2c_himax_read(touch_i2c, 0xF0, inData, 128, DEFAULT_RETRY_CNT);

				printk("[Himax] command: 0xF0 \n");
				for (loop_i = 0; loop_i < 128; loop_i++)
				{
					printk("0x%2.2X ", inData[loop_i]);
					if ((loop_i % 16) == 15)
					{
						printk("\n");
					}
				}
				//for debug message end
			}
		}
		return 0;
	}
	EXPORT_SYMBOL(himax_cable_status);

	static int cable_status_notify(struct notifier_block *self, unsigned long action, void *dev)
	{
		switch (action) {
		case CHRG_UNKNOWN:
			printk(KERN_INFO "[Himax] %s CHRG_UNKNOWN !!!\n", __func__);
			setTouchACMode(2);
			break;
		case CHRG_SDP:
			printk(KERN_INFO "[Himax] %s CHRG_SDP !!!\n", __func__);
			setTouchACMode(0);
			break;
		/*case CHRG_CDP:*/
		case CHRG_DCP:
			printk(KERN_INFO "[Himax] %s CHRG_DCP !!!\n", __func__);
			setTouchACMode(0);
			break;
		/*case CHRG_ACA:
		case CHRG_SE1:
		case CHRG_MHL:*/
		}
		return NOTIFY_OK;
	}

	static struct notifier_block cable_status_notifier = {
		.notifier_call = cable_status_notify,
	};

	extern int cable_status_register_client(struct notifier_block *nb);
	extern int cable_status_unregister_client(struct notifier_block *nb);

//=============================================================================================================
//
//	Segment : Himax Normal Function
//
//=============================================================================================================
	//----[ normal function]--------------------------------------------------------------------------------start
		void calculate_point_number(void)
		{
			HX_TOUCH_INFO_POINT_CNT = HX_MAX_PT * 4 ;

			if( (HX_MAX_PT % 4) == 0)
			{
				HX_TOUCH_INFO_POINT_CNT += (HX_MAX_PT / 4) * 4 ;
			}
			else
			{
				HX_TOUCH_INFO_POINT_CNT += ((HX_MAX_PT / 4) +1) * 4 ;
			}
		}	

		void himax_ic_package_check(struct himax_ts_data *ts_modify)
		{
			uint8_t cmd[3];
			uint8_t data[3];

			if( i2c_himax_read(ts_modify->client, 0xD1, cmd, 3, DEFAULT_RETRY_CNT) < 0)
			{
				return ;
			}

			if( i2c_himax_read(ts_modify->client, 0x31, data, 3, DEFAULT_RETRY_CNT) < 0)
			{
				return ;
			}

			if((data[0] == 0x85 && data[1] == 0x30))
			{
				IC_TYPE 		= HX_85XX_E_SERIES_PWON;
				IC_CHECKSUM = HX_TP_BIN_CHECKSUM_CRC;
				//Himax: Set FW and CFG Flash Address                                          
				FW_VER_MAJ_FLASH_ADDR	= 133;	//0x0085                              
				FW_VER_MAJ_FLASH_LENG	= 1;;                                                
				FW_VER_MIN_FLASH_ADDR	= 134;  //0x0086                                     
				FW_VER_MIN_FLASH_LENG	= 1;                                                
				CFG_VER_MAJ_FLASH_ADDR = 160;	//0x00A0         
				CFG_VER_MAJ_FLASH_LENG = 12; 	                                 
				CFG_VER_MIN_FLASH_ADDR = 172;	//0x00AC
				CFG_VER_MIN_FLASH_LENG = 12;   
				
				printk("Himax IC package 8530 E\n");
			}
			else if((data[0] == 0x85 && data[1] == 0x28) || (cmd[0] == 0x04 && cmd[1] == 0x85 && (cmd[2] == 0x26 || cmd[2] == 0x27 || cmd[2] == 0x28)))
			{
				IC_TYPE 		= HX_85XX_D_SERIES_PWON;
				IC_CHECKSUM = HX_TP_BIN_CHECKSUM_CRC;
				//Himax: Set FW and CFG Flash Address                                          
				FW_VER_MAJ_FLASH_ADDR	= 133;	//0x0085                              
				FW_VER_MAJ_FLASH_LENG	= 1;;                                                
				FW_VER_MIN_FLASH_ADDR	= 134;  //0x0086                                     
				FW_VER_MIN_FLASH_LENG	= 1;                                                
				CFG_VER_MAJ_FLASH_ADDR = 160;	//0x00A0         
				CFG_VER_MAJ_FLASH_LENG = 12; 	                                 
				CFG_VER_MIN_FLASH_ADDR = 172;	//0x00AC
				CFG_VER_MIN_FLASH_LENG = 12;   
				
				printk("Himax IC package 8528 D\n");
			}
			else if((data[0] == 0x85 && data[1] == 0x23) || (cmd[0] == 0x03 && cmd[1] == 0x85 && (cmd[2] == 0x26 || cmd[2] == 0x27 || cmd[2] == 0x28 || cmd[2] == 0x29)))
			{
				IC_TYPE 		= HX_85XX_C_SERIES_PWON;  
				IC_CHECKSUM = HX_TP_BIN_CHECKSUM_SW;
				//Himax: Set FW and CFG Flash Address                                                  
				FW_VER_MAJ_FLASH_ADDR = 133;			//0x0085
				FW_VER_MAJ_FLASH_LENG = 1;
				FW_VER_MIN_FLASH_ADDR = 134;			//0x0086
				FW_VER_MIN_FLASH_LENG = 1;
				CFG_VER_MAJ_FLASH_ADDR = 135;			//0x0087
				CFG_VER_MAJ_FLASH_LENG = 12;
				CFG_VER_MIN_FLASH_ADDR = 147;			//0x0093
				CFG_VER_MIN_FLASH_LENG = 12;
						
				printk("Himax IC package 8523 C\n");
			}
			else if ((data[0] == 0x85 && data[1] == 0x26) || (cmd[0] == 0x02 && cmd[1] == 0x85 && (cmd[2] == 0x19 || cmd[2] == 0x25 || cmd[2] == 0x26)))
			{
				IC_TYPE 		= HX_85XX_B_SERIES_PWON;
				IC_CHECKSUM = HX_TP_BIN_CHECKSUM_SW;
				//Himax: Set FW and CFG Flash Address                                                  
				FW_VER_MAJ_FLASH_ADDR = 133;			//0x0085
				FW_VER_MAJ_FLASH_LENG = 1;
				FW_VER_MIN_FLASH_ADDR = 728;			//0x02D8
				FW_VER_MIN_FLASH_LENG = 1;
				CFG_VER_MAJ_FLASH_ADDR = 692;			//0x02B4
				CFG_VER_MAJ_FLASH_LENG = 3;
				CFG_VER_MIN_FLASH_ADDR = 704;			//0x02C0
				CFG_VER_MIN_FLASH_LENG = 3;
				
				printk("Himax IC package 8526 B\n");
			}	
			else if ((data[0] == 0x85 && data[1] == 0x20) || (cmd[0] == 0x01 && cmd[1] == 0x85 && cmd[2] == 0x19))
			{
				IC_TYPE 		= HX_85XX_A_SERIES_PWON;
				IC_CHECKSUM = HX_TP_BIN_CHECKSUM_SW;
				printk("Himax IC package 8520 A\n");
			}
			else
			{ 
				printk("Himax IC package incorrect!!\n");
			}
		}
	
		static int himax_ts_suspend(struct i2c_client *client, pm_message_t mesg)
		{
			struct 	himax_ts_data *ts_modify 	= i2c_get_clientdata(client);
			uint8_t buf[11] 										= {0};
			int 		ret 											= 0;

			is_suspend = false;

			#ifdef HX_TP_PROC_FLASH_DUMP
			if(getFlashDumpGoing())
			{
				printk(KERN_INFO "[himax] %s: Flash dump is going, reject suspend\n",__func__);
				return 0;
			}
			#endif


			printk(KERN_INFO "[himax] %s: TS suspend\n", __func__);
			
			//Wakelock Protect Start
			wake_lock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			//Mutexlock Protect Start
			mutex_lock(&ts_modify->mutex_lock);
			//Mutexlock Protect End
			
			buf[0] = HX_CMD_TSSOFF;
			ret = i2c_himax_master_write(ts_modify->client, buf, 1, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			msleep(120);
			
			buf[0] = HX_CMD_TSSLPIN;
			ret = i2c_himax_master_write(ts_modify->client, buf, 1, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			msleep(120);
			
			//Shouchung remove, modify suspend/resume function
			/*
			buf[0] = HX_CMD_SETDEEPSTB;
			buf[1] = 0x01;
			ret = i2c_himax_master_write(ts_modify->client, buf, 2, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			msleep(120);
			*/
			//Shouchung end
			
			//Mutexlock Protect Start
			mutex_unlock(&ts_modify->mutex_lock);
			//Mutexlock Protect End
			
			//Wakelock Protect Start
			wake_unlock(&ts_modify->wake_lock);
			//Wakelock Protect End	
			
			//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				ts_modify->running_status = 1;
				cancel_delayed_work_sync(&ts_modify->himax_chip_monitor);
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end
			
			disable_irq(client->irq);
			
			ret = cancel_work_sync(&ts_modify->work);
			if (ret)
			{
				enable_irq(client->irq);
			}

			is_suspend = true;
			TOUCH_UP_COUNTER = 0;

			return 0;
		}
		
		static int himax_ts_resume(struct i2c_client *client)
		{
			struct himax_ts_data *ts_modify = i2c_get_clientdata(client);
			uint8_t buf[11] = {0};
			int ret = 0;
			printk(KERN_INFO "[himax] %s: TS resume\n", __func__);
			
			if(!is_suspend)
			{
				printk(KERN_INFO "[himax] %s TP never enter suspend , reject the resume action\n",__func__);
				return 0;
			}			

			//Wakelock Protect Start
			wake_lock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			//Shouchung remove, modify suspend/resume function
			/*

			buf[0] = HX_CMD_SETDEEPSTB;
			buf[1] = 0x00;
			ret = i2c_himax_master_write(ts_modify->client, buf, 2, DEFAULT_RETRY_CNT);//sense on
			if(ret < 0) 
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			} 
			udelay(100);
			*/
			//Shouchung end

			
			//Wakelock Protect Start
			wake_unlock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			buf[0] = 0xF6;
			buf[1] = 0x00;
			buf[2] = 0x00;
			buf[3] = 0x00;
			buf[4] = 0x00;
			buf[5] = 0x00;
			buf[6] = 0x00;
			buf[7] = 0x00;
			buf[8] = 0x00;
			buf[9] = 0x00;
			buf[10] = 0x00;
			ret = i2c_himax_master_write(ts_modify->client, buf, 11, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			udelay(100);

			buf[0] = HX_CMD_TSSON;	//0x83
			ret = i2c_himax_master_write(ts_modify->client, buf, 1, DEFAULT_RETRY_CNT);//sense on
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			msleep(100);
			
			buf[0] = HX_CMD_TSSLPOUT;	//0x81
			ret = i2c_himax_master_write(ts_modify->client, buf, 1, DEFAULT_RETRY_CNT);//sleep out
			if(ret < 0)
			{
				printk(KERN_ERR "[himax] %s: I2C access failed addr = 0x%x\n", __func__, ts_modify->client->addr);
			}
			msleep(100);
			
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start  
				//Shouchung modify for ESD solution
				//#ifdef HX_ESD_WORKAROUND
				#ifdef HX_ESD_WORKAROUND_HANDSHAKING
				//Shouchung end
				ret = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail
				if(ret == 2)
				{
					queue_delayed_work(ts_modify->himax_wq, &ts_modify->himax_chip_reset_work, 0);
					printk(KERN_INFO "[Himax] %s: I2C Fail \n", __func__);
				}
				if(ret == 1)
				{
					printk(KERN_INFO "[Himax] %s: MCU Stop \n", __func__);
					//Do HW_RESET??
					ESD_HW_REST();
				}
				else
				{
					printk(KERN_INFO "[Himax] %s: MCU Running \n", __func__);
				}
				#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
			
			enable_irq(client->irq);
			
			//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifndef ENABLE_ADV_CHIP_STATUS_MONITOR
				queue_delayed_work(ts_modify->himax_wq, &ts_modify->himax_chip_monitor, 10*HZ); //for ESD solution
				#endif
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end
			
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
				#ifdef HX_ESD_WORKAROUND
				ESD_COUNTER = 0;
				#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end

			return 0;
		}
		
		//----[CONFIG_HAS_EARLYSUSPEND]-----------------------------------------------------------------------start
			#ifdef CONFIG_HAS_EARLYSUSPEND
			static void himax_ts_early_suspend(struct early_suspend *h)
			{
				struct himax_ts_data *ts;
				ts = container_of(h, struct himax_ts_data, early_suspend);
				himax_ts_suspend(ts->client, PMSG_SUSPEND);
			}
		
			static void himax_ts_late_resume(struct early_suspend *h)
			{
				struct himax_ts_data *ts;
				ts = container_of(h, struct himax_ts_data, early_suspend);
				himax_ts_resume(ts->client);
			}
			#endif
		//----[CONFIG_HAS_EARLYSUSPEND]-------------------------------------------------------------------------end	
	//----[ normal function]----------------------------------------------------------------------------------end
	
	//----[ i2c read/write function]------------------------------------------------------------------------start
		static int i2c_himax_read(struct i2c_client *client, uint8_t command, uint8_t *data, uint8_t length, uint8_t toRetry)
		{
			int retry;
			struct i2c_msg msg[] = 
			{
				{
					.addr = client->addr,
					.flags = 0,
					.len = 1,
					.buf = &command,
				},
				{
					.addr = client->addr,
					.flags = I2C_M_RD,
					.len = length,
					.buf = data,
				}
			};
			
			for (retry = 0; retry < toRetry; retry++) 
			{
				if (i2c_transfer(client->adapter, msg, 2) == 2)
				{
					break;
				}
				msleep(10);
			}
			if (retry == toRetry) 
			{
				printk(KERN_INFO "[TP] %s: i2c_read_block retry over %d\n", __func__, toRetry);
				return -EIO;
			}
			return 0;
		}
		
		static int i2c_himax_write(struct i2c_client *client, uint8_t command, uint8_t *data, uint8_t length, uint8_t toRetry)
		{
			int retry, loop_i;
			uint8_t *buf = kzalloc(sizeof(uint8_t)*(length+1), GFP_KERNEL);
			
			struct i2c_msg msg[] = 
			{
				{
					.addr = client->addr,
					.flags = 0,
					.len = length + 1,
					.buf = buf,
				}
			};
			
			buf[0] = command;
			for (loop_i = 0; loop_i < length; loop_i++)
			{
				buf[loop_i + 1] = data[loop_i];
			}
			for (retry = 0; retry < toRetry; retry++) 
			{
				if (i2c_transfer(client->adapter, msg, 1) == 1)
				{
					break;
				}
				msleep(10);
			}
			
			if (retry == toRetry) 
			{
				printk(KERN_ERR "[TP] %s: i2c_write_block retry over %d\n", __func__, toRetry);
				kfree(buf);
				return -EIO;
			}
			kfree(buf);
			return 0;
		}
		
		static int i2c_himax_write_command(struct i2c_client *client, uint8_t command, uint8_t toRetry)
		{
			return i2c_himax_write(client, command, NULL, 0, toRetry);
		}
		
		int i2c_himax_master_write(struct i2c_client *client, uint8_t *data, uint8_t length, uint8_t toRetry)
		{
			int retry, loop_i;
			uint8_t *buf = kzalloc(sizeof(uint8_t)*length, GFP_KERNEL);
			
			struct i2c_msg msg[] = 
			{
				{
					.addr = client->addr,
					.flags = 0,
					.len = length,
					.buf = buf,
				}
			};
			
			for (loop_i = 0; loop_i < length; loop_i++)
			{
				buf[loop_i] = data[loop_i];
			}
			for (retry = 0; retry < toRetry; retry++) 
			{
				if (i2c_transfer(client->adapter, msg, 1) == 1)
				{
					break;
				}
				msleep(10);
			}
			
			if (retry == toRetry) 
			{
				printk(KERN_ERR "[TP] %s: i2c_write_block retry over %d\n", __func__, toRetry);
				kfree(buf);
				return -EIO;
			}
			kfree(buf);
			return 0;
		}
	//----[ i2c read/write function]--------------------------------------------------------------------------end
	
	#ifdef HX_LOADIN_CONFIG
		static int himax_config_flow()
		{   
			char data[4];
			uint8_t buf0[4];
			int i2c_CheckSum = 0;
			unsigned long i = 0;
			
			if( i2c_himax_master_write(touch_i2c, &c1[0],sizeof(c1),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c2[0],sizeof(c2),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c3[0],sizeof(c3),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c4[0],sizeof(c4),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c5[0],sizeof(c5),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c6[0],sizeof(c6),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c7[0],sizeof(c7),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c8[0],sizeof(c8),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c9[0],sizeof(c9),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c10[0],sizeof(c10),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c11[0],sizeof(c11),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c12[0],sizeof(c12),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c13[0],sizeof(c13),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c14[0],sizeof(c14),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c15[0],sizeof(c15),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c16[0],sizeof(c16),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c17[0],sizeof(c17),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c18[0],sizeof(c18),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c19[0],sizeof(c19),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c20[0],sizeof(c20),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c21[0],sizeof(c21),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c22[0],sizeof(c22),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c23[0],sizeof(c23),DEFAULT_RETRY_CNT) < 0)
			{	
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c24[0],sizeof(c24),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c25[0],sizeof(c25),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c26[0],sizeof(c26),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c27[0],sizeof(c27),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c28[0],sizeof(c28),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c29[0],sizeof(c29),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c30[0],sizeof(c30),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c31[0],sizeof(c31),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c32[0],sizeof(c32),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c33[0],sizeof(c33),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c34[0],sizeof(c34),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			if( i2c_himax_master_write(touch_i2c, &c35[0],sizeof(c35),DEFAULT_RETRY_CNT) < 0)
			{
				goto HimaxErr;
			}
			
			//i2c check sum start.
			data[0] = 0xAB;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			data[0] = 0xAB;
			data[1] = 0x01;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
		
			//------------------------------------------------------------------config bank PART c36 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x00;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c36[0],sizeof(c36),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c36) ; i++ )
			{
				i2c_CheckSum += c36[i];
			}
			printk("Himax i2c_checksum_36_size = %d \n",sizeof(c36));
			printk("Himax i2c_checksum_36 = %d \n",i2c_CheckSum);
			
			i2c_CheckSum += 0x2AF;
			
			printk("Himax i2c_checksum_36 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c36 END

			//------------------------------------------------------------------config bank PART c37 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x1E;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c37[0],sizeof(c37),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c37) ; i++ )
			{
				i2c_CheckSum += c37[i];
			}
			printk("Himax i2c_checksum_37_size = %d \n",sizeof(c37));
			printk("Himax i2c_checksum_37 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x2CD;
			printk("Himax i2c_checksum_37 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c37 END
			
			//------------------------------------------------------------------config bank PART c38 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x3C;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c38[0],sizeof(c38),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c38) ; i++ )
			{
				i2c_CheckSum += c38[i];
			}
			printk("Himax i2c_checksum_38_size = %d \n",sizeof(c38));
			printk("Himax i2c_checksum_38 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x2EB;
			printk("Himax i2c_checksum_38 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c38 END
			
			//------------------------------------------------------------------config bank PART c39 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x4C;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c39[0],sizeof(c39),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c39) ; i++ )
			{
				i2c_CheckSum += c39[i];
			}
			printk("Himax i2c_checksum_39_size = %d \n",sizeof(c39));
			printk("Himax i2c_checksum_39 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x2FB;	
			printk("Himax i2c_checksum_39 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c39 END
			
			//------------------------------------------------------------------config bank PART c40 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x64;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c40[0],sizeof(c40),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
		
			for( i=0 ; i<sizeof(c40) ; i++ )
			{
				i2c_CheckSum += c40[i];
			}
			printk("Himax i2c_checksum_40_size = %d \n",sizeof(c40));
			printk("Himax i2c_checksum_40 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x313;
			printk("Himax i2c_checksum_40 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c40 END
			
			//------------------------------------------------------------------config bank PART c41 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x7A;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c41[0],sizeof(c41),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c41) ; i++ )
			{
				i2c_CheckSum += c41[i];
			}
			printk("Himax i2c_checksum_41_size = %d \n",sizeof(c41));
			printk("Himax i2c_checksum_41 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x329;
			printk("Himax i2c_checksum_41 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c41 END	
			
			//------------------------------------------------------------------config bank PART c42 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x96;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c42[0],sizeof(c42),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c42) ; i++ )
			{
				i2c_CheckSum += c42[i];
			}
			printk("Himax i2c_checksum_42_size = %d \n",sizeof(c42));
			printk("Himax i2c_checksum_42 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x345;
			printk("Himax i2c_checksum_42 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c42 END
			
			//------------------------------------------------------------------config bank PART c43_1 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0x9E;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c43_1[0],sizeof(c43_1),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c43_1) ; i++ )
			{
				i2c_CheckSum += c43_1[i];
			}
			printk("Himax i2c_checksum_43_1_size = %d \n",sizeof(c43_1));
			printk("Himax i2c_checksum_43_1 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x34D;
			printk("Himax i2c_checksum_43_1 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c43_1 END
			
			//------------------------------------------------------------------config bank PART c43_2 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0xBD;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c43_2[0],sizeof(c43_2),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c43_2) ; i++ )
			{
				i2c_CheckSum += c43_2[i];
			}
			printk("Himax i2c_checksum_43_2_size = %d \n",sizeof(c43_2));
			printk("Himax i2c_checksum_43_2 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x36C;
			printk("Himax i2c_checksum_43_2 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c43_2 END
			
			//------------------------------------------------------------------config bank PART c44_1 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0xDA;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c44_1[0],sizeof(c44_1),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
		
			for( i=0 ; i<sizeof(c44_1) ; i++ )
			{
				i2c_CheckSum += c44_1[i];
			}
			printk("Himax i2c_checksum_44_1_size = %d \n",sizeof(c44_1));
			printk("Himax i2c_checksum_44_1 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x389;
			printk("Himax i2c_checksum_44_1 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c44_1 END
			
			//------------------------------------------------------------------config bank PART c44_2 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0xF9;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c44_2[0],sizeof(c44_2),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c44_2) ; i++ )
			{
				i2c_CheckSum += c44_2[i];
			}
			printk("Himax i2c_checksum_44_2_size = %d \n",sizeof(c44_2));
			printk("Himax i2c_checksum_44_2 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x3A8;
			printk("Himax i2c_checksum_44_2 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c44_2 END
			
			//------------------------------------------------------------------config bank PART c45 START
			data[0] = 0x8C;//0xE1;
			data[1] = 0x15;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
					
			data[0] = 0x8B;//0xD8;
			data[1] = 0x00;
			data[2] = 0xFE;	//Start addr
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}			
					
			if((i2c_himax_master_write(touch_i2c, &c45[0],sizeof(c45),3))<0)
			{	
				goto HimaxErr;
			}
			
			data[0] = 0x8C;//0xE1;
			data[1] = 0x00;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			for( i=0 ; i<sizeof(c45) ; i++ )
			{
				i2c_CheckSum += c45[i];
			}
			printk("Himax i2c_checksum_45_size = %d \n",sizeof(c45));
			printk("Himax i2c_checksum_45 = %d \n",i2c_CheckSum);
			i2c_CheckSum += 0x3AD;
			printk("Himax i2c_checksum_45 = %d \n",i2c_CheckSum);
			//------------------------------------------------------------------config bank PART c45 END
			
			data[0] = 0xAB;
			data[1] = 0x10;
			if((i2c_himax_master_write(touch_i2c, &data[0],2,3))<0)
			{
				goto HimaxErr;
			}
			
			i2c_CheckSum += 0xAB;
			i2c_CheckSum += 0x10;
			
			printk("Himax i2c_checksum_Final = %d \n",i2c_CheckSum);
			
			data[0] = 0xAC;
			data[1] = i2c_CheckSum & 0xFF;
			data[2] = (i2c_CheckSum >> 8) & 0xFF;
			if((i2c_himax_master_write(touch_i2c, &data[0],3,3))<0)
			{
				goto HimaxErr;
			}
			
			printk("Himax i2c_checksum_AC = %d , %d \n",data[1],data[2]);
			
			int ret = i2c_himax_read(touch_i2c, 0xAB, buf0, 2, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[Himax]:i2c_himax_read 0xDA failed line: %d \n",__LINE__);
				goto HimaxErr;
			}
			
			if(buf0[0] == 0x18)
			{
				return -1;
			}
			
			if(buf0[0] == 0x10)
			{
				return 1;
			}
					
			return 1;
			HimaxErr:
			return -1;		
		}
	#endif
	
	//----[ register flow function]-------------------------------------------------------------------------start
		static int himax_ts_poweron(struct himax_ts_data *ts_modify)
		{
			uint8_t buf0[11];
			int ret = 0;
			int config_fail_retry = 0;
			
			//Wakelock Protect Start
			wake_lock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			//Mutexlock Protect Start
			mutex_lock(&ts_modify->mutex_lock);
			//Mutexlock Protect End
			
			//----[ HX_85XX_C_SERIES_PWON]----------------------------------------------------------------------start
			if (IC_TYPE == HX_85XX_C_SERIES_PWON)
			{
				buf0[0] = HX_CMD_MANUALMODE;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				
				buf0[0] = HX_CMD_SETMICROOFF;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable	
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);   
				
				buf0[0] = HX_CMD_SETROMRDY;
				buf0[1] = 0x0F;
				buf0[2] = 0x53;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//enable flash
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_SET_CACHE_FUN;
				buf0[1] = 0x05;
				buf0[2] = 0x03;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_B9;
				buf0[1] = 0x01;
				buf0[2] = 0x2D;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
				
				#ifdef HX_LOADIN_CONFIG
					if(himax_config_flow() == -1)
					{
						printk("Himax send config fail\n");
						//goto send_i2c_msg_fail;
					}
					msleep(100); //100ms
				#endif
	  
				buf0[0] = HX_CMD_TSSLPOUT;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				msleep(120); //120ms	  
			}
			else if (IC_TYPE == HX_85XX_A_SERIES_PWON)
			{
				buf0[0] = HX_CMD_MANUALMODE;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				
				buf0[0] = HX_CMD_SETMICROOFF;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);   
				
				buf0[0] = HX_CMD_SETROMRDY;
				buf0[1] = 0x0F;
				buf0[2] = 0x53;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//enable flash
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_SET_CACHE_FUN;
				buf0[1] = 0x06;
				buf0[2] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_76;
				buf0[1] = 0x01;
				buf0[2] = 0x2D;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				#ifdef HX_LOADIN_CONFIG
					if(himax_config_flow() == -1)
					{
						printk("Himax send config fail\n");
					}
					msleep(100); //100ms
				#endif
				
				buf0[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
				
				buf0[0] = HX_CMD_TSSLPOUT;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
			}
			else if (IC_TYPE == HX_85XX_B_SERIES_PWON) 
			{
				buf0[0] = HX_CMD_MANUALMODE;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				
				buf0[0] = HX_CMD_SETMICROOFF;
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);   
				
				buf0[0] = HX_CMD_SETROMRDY;
				buf0[1] = 0x0F;
				buf0[2] = 0x53;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//enable flash
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_SET_CACHE_FUN;
				buf0[1] = 0x06;
				buf0[2] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_76;
				buf0[1] = 0x01;
				buf0[2] = 0x2D;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//prefetch
				if(ret < 0) 
				{	
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				#ifdef HX_LOADIN_CONFIG
					if(himax_config_flow() == -1)
					{
						printk("Himax send config fail\n");
					}
					msleep(100); //100ms
				#endif
				
				buf0[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
				
				buf0[0] = HX_CMD_TSSLPOUT;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
			}
			else if (IC_TYPE == HX_85XX_D_SERIES_PWON)
			{
				buf0[0] = HX_CMD_MANUALMODE;	//0x42
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);//Reload Disable
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				
				buf0[0] = HX_CMD_SETROMRDY;	//0x36
				buf0[1] = 0x0F;
				buf0[2] = 0x53;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//enable flash
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_SET_CACHE_FUN;	//0xDD
				buf0[1] = 0x04;
				buf0[2] = 0x03;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_B9;	//setCVDD
				buf0[1] = 0x01;
				buf0[2] = 0x36;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				buf0[0] = HX_CMD_CB;
				buf0[1] = 0x01;
				buf0[2] = 0xF5;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				udelay(100);
				
				#ifdef HX_LOADIN_CONFIG				
					//load config
					printk("Himax start load config.\n");
					config_fail_retry = 0;
					while(true)
					{
						if(himax_config_flow() == -1)
						{
							config_fail_retry++;
							if(config_fail_retry == 3)
							{
								printk("himax_config_flow retry fail.\n");
								goto send_i2c_msg_fail;
							}
							printk("Himax config retry = %d \n",config_fail_retry);
						}
						else
						{
							break;
						}
					}
					printk("Himax end load config.\n");
					msleep(100); //100ms
				#endif
				
				buf0[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms
				
				buf0[0] = HX_CMD_TSSLPOUT;	//0x81
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 	
				msleep(120); //120ms
			}
			else if (IC_TYPE == HX_85XX_E_SERIES_PWON)
			{
				buf0[0] = 0x35;	//reload disable
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);

				buf0[0] = 0x42;	
				buf0[1] = 0x02;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);	

				//Shouchung modify for ESD solution
				#ifdef HX_85XX_E_nWATCH_DOG
				buf0[0] = 0xCA; //Disable WDT
				buf0[1] = 0xA5;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);

				buf0[0] = 0xCE; //Disable WDT
				buf0[1] = 0xCA;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 2, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				#endif
				//Shouchung end

				buf0[0] = 0x36; //ROMRDY		
				buf0[1] = 0x0F;
				buf0[2] = 0x53;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);

				buf0[0] = 0xDD; //prefetch	
				buf0[1] = 0x07; //wait 6.5T
				buf0[2] = 0x03; //enable prefetch and cache 
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);			

				//buf0[0] = 0xB9; //SETCVDD
				//buf0[1] = 0x01;
				//buf0[2] = 0x36;
				//ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);
				//if(ret < 0) 
				//{
				//	printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
				//	goto send_i2c_msg_fail;
				//}
				//udelay(100);

				buf0[0] = 0xCB;
				buf0[1] = 0x01;
				buf0[2] = 0xF5;
				buf0[3] = 0xFF;
				buf0[4] = 0xFF;
				buf0[5] = 0x01;
				buf0[6] = 0x00;
				buf0[7] = 0x05;
				buf0[8] = 0x00;
				buf0[9] = 0x0F; 
				buf0[10] = 0x00;
				//buf0[11] = 0x00;
				//buf0[12] = 0x00; 
				ret = i2c_himax_master_write(ts_modify->client, buf0, 11, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				}
				udelay(100);
				
				#ifdef HX_LOADIN_CONFIG
					if(himax_config_flow() == -1)
					{
						printk("Himax send config fail\n");
					}
					msleep(100); //100ms
				#endif

				buf0[0] = 0x83;
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 
				msleep(120); //120ms

				buf0[0] = 0x81;	//0x81
				ret = i2c_himax_master_write(ts_modify->client, buf0, 1, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
					goto send_i2c_msg_fail;
				} 	
				msleep(120); //120ms
			}
			
			//Mutexlock Protect Start
			mutex_unlock(&ts_modify->mutex_lock);
			//Mutexlock Protect End
			
			//Wakelock Protect Start
			wake_unlock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			return ret;
			
			send_i2c_msg_fail:
			
			printk(KERN_ERR "[Himax]:send_i2c_msg_failline: %d \n",__LINE__);
			//todo mutex_unlock(&himax_chip->mutex_lock);
			
			//Mutexlock Protect Start
			mutex_unlock(&ts_modify->mutex_lock);
			//Mutexlock Protect End
			
			//Wakelock Protect Start
			wake_unlock(&ts_modify->wake_lock);
			//Wakelock Protect End
			
			//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				if(private_ts->init_success)
				{
					queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
				}
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]-------------------------------------------------------------------end
			return -1;
		}
		
		int himax_ManualMode(int enter)
		{
			uint8_t cmd[2];
			cmd[0] = enter;
			if( i2c_himax_write(touch_i2c, 0x42 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			return 0;
		}
		
		int himax_FlashMode(int enter)
		{
			uint8_t cmd[2];
			cmd[0] = enter;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			return 0;
		}
		
		static int himax_lock_flash(void)
		{
			uint8_t cmd[5];
			
			/* lock sequence start */
			cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x03;cmd[1] = 0x00;cmd[2] = 0x00;
			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x7D;cmd[3] = 0x03;
			if( i2c_himax_write(touch_i2c, 0x45 ,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			if( i2c_himax_write_command(touch_i2c, 0x4A, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			mdelay(50);
			return 0;
			/* lock sequence stop */
		}
		
		static int himax_unlock_flash(void)
		{
			uint8_t cmd[5];
			
			/* unlock sequence start */
			cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x03;cmd[1] = 0x00;cmd[2] = 0x00;
			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x3D;cmd[3] = 0x03;
			if( i2c_himax_write(touch_i2c, 0x45 ,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			if( i2c_himax_write_command(touch_i2c, 0x4A, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			mdelay(50);
			
			return 0;
			/* unlock sequence stop */
		}
		
		static uint8_t himax_calculateChecksum(char *ImageBuffer, int fullLength)//, int address, int RST)
		{
			//----[ HX_TP_BIN_CHECKSUM_SW]----------------------------------------------------------------------start
			if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_SW)
			{
				u16 checksum = 0;
				uint8_t cmd[5], last_byte;
				int FileLength, i, readLen, k, lastLength;
				
				FileLength = fullLength - 2;
				memset(cmd, 0x00, sizeof(cmd));
				
				//himax_HW_reset(RST);
				
				//if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)
				//return 0;
				
				//mdelay(120);
				//printk("himax_marked, Sleep out: %d\n", __LINE__);
				//himax_unlock_flash();
				
				himax_FlashMode(1);
				
				FileLength = (FileLength + 3) / 4;
				for (i = 0; i < FileLength; i++) 
				{
					last_byte = 0;
					readLen = 0;
					
					cmd[0] = i & 0x1F;
					if (cmd[0] == 0x1F || i == FileLength - 1)
					last_byte = 1;
					cmd[1] = (i >> 5) & 0x1F;cmd[2] = (i >> 10) & 0x1F;
					if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
				
					if( i2c_himax_write_command(touch_i2c, 0x46, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
				
					if (i < (FileLength - 1))
					{
						checksum += cmd[0] + cmd[1] + cmd[2] + cmd[3];
						if (i == 0)
						{
							printk(KERN_ERR "[TP] %s: himax_marked cmd 0 to 3 (first 4 bytes): %d, %d, %d, %d\n", __func__, cmd[0], cmd[1], cmd[2], cmd[3]);
						}
					}
					else 
					{
						printk(KERN_ERR "[TP] %s: himax_marked cmd 0 to 3 (last 4 bytes): %d, %d, %d, %d\n", __func__, cmd[0], cmd[1], cmd[2], cmd[3]);
						printk(KERN_ERR "[TP] %s: himax_marked, checksum (not last): %d\n", __func__, checksum);
						
						lastLength = (((fullLength - 2) % 4) > 0)?((fullLength - 2) % 4):4;
						
						for (k = 0; k < lastLength; k++) 
						{
							checksum += cmd[k];
						}
						printk(KERN_ERR "[TP] %s: himax_marked, checksum (final): %d\n", __func__, checksum);
				
						//Check Success
						if (ImageBuffer[fullLength - 1] == (u8)(0xFF & (checksum >> 8)) && ImageBuffer[fullLength - 2] == (u8)(0xFF & checksum)) 
						{
							himax_FlashMode(0);
							return 1;
						} 
						else //Check Fail
						{
							himax_FlashMode(0);
							return 0;
						}
					}
				}
			}
			else if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_HW)
			{
				u32 sw_checksum = 0;
				u32 hw_checksum = 0;
				uint8_t cmd[5], last_byte;
				int FileLength, i, readLen, k, lastLength;
				
				FileLength = fullLength;
				memset(cmd, 0x00, sizeof(cmd));
				
				//himax_HW_reset(RST);
				
				//if((i2c_smbus_write_i2c_block_data(i2c_client, 0x81, 0, &cmd[0]))< 0)
				//return 0;
				
				//mdelay(120);
				//printk("himax_marked, Sleep out: %d\n", __LINE__);
				//himax_unlock_flash();
				
				himax_FlashMode(1);
				
				FileLength = (FileLength + 3) / 4;
				for (i = 0; i < FileLength; i++) 
				{
					last_byte = 0;
					readLen = 0;
					
					cmd[0] = i & 0x1F;
					if (cmd[0] == 0x1F || i == FileLength - 1)
					{
						last_byte = 1;
					}
					cmd[1] = (i >> 5) & 0x1F;cmd[2] = (i >> 10) & 0x1F;
					if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					if( i2c_himax_write_command(touch_i2c, 0x46, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
					
					if (i < (FileLength - 1))
					{
						sw_checksum += cmd[0] + cmd[1] + cmd[2] + cmd[3];
						if (i == 0)
						{
							printk(KERN_ERR "[TP] %s: himax_marked cmd 0 to 3 (first 4 bytes): %d, %d, %d, %d\n", __func__, cmd[0], cmd[1], cmd[2], cmd[3]);
						}
					}
					else 
					{
						printk(KERN_ERR "[TP] %s: himax_marked cmd 0 to 3 (last 4 bytes): %d, %d, %d, %d\n", __func__, cmd[0], cmd[1], cmd[2], cmd[3]);
						printk(KERN_ERR "[TP] %s: himax_marked, sw_checksum (not last): %d\n", __func__, sw_checksum);
						
						lastLength = ((fullLength % 4) > 0)?(fullLength % 4):4;
						
						for (k = 0; k < lastLength; k++) 
						{
							sw_checksum += cmd[k];
						}
						printk(KERN_ERR "[TP] %s: himax_marked, sw_checksum (final): %d\n", __func__, sw_checksum);
					
						//Enable HW Checksum function.
						cmd[0] = 0x01;
						if( i2c_himax_write(touch_i2c, 0xE5 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)	
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						//Must sleep 5 ms.
						msleep(30);
						
						//Get HW Checksum. 
						if( i2c_himax_read(touch_i2c, 0xAD, cmd, 4, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return -1;
						}
				
						hw_checksum = cmd[0] + cmd[1]*0x100 + cmd[2]*0x10000 + cmd[3]*1000000;
						printk("[Touch_FH] %s: himax_marked, sw_checksum (final): %d\n", __func__, sw_checksum);
						printk("[Touch_FH] %s: himax_marked, hw_checkusm (final): %d\n", __func__, hw_checksum);
	
						//Compare the checksum.
						if( hw_checksum == sw_checksum )
						{
							himax_FlashMode(0);
							return 1;
						}
						else
						{
							himax_FlashMode(0);
							return 0;
						}
					}
				}
			}
			else if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_CRC)
			{
				uint8_t cmd[5];
				if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					//Set Flash Clock Rate
					if( i2c_himax_read(touch_i2c, 0xED, cmd, 5, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
					cmd[3] = 0x04;

					if( i2c_himax_write(touch_i2c, 0xED ,&cmd[0], 5, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}

					//Enable Flash
					//himax_FlashMode(1);
					cmd[0] = 0x01;
					cmd[1] = 0x00;
					cmd[2] = 0x02;
					if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}

					//Select CRC Mode
					cmd[0] = 0x05;
					if( i2c_himax_write(touch_i2c, 0xD2 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}

					//Enable CRC Function
					cmd[0] = 0x01;
					if( i2c_himax_write(touch_i2c, 0x53 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}

					//Must delay 30 ms
					msleep(30);

					//Read HW CRC
					if( i2c_himax_read(touch_i2c, 0xAD, cmd, 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
					
					printk("Himax CRC : 0xAD[0] = %d,0xAD[1] = %d,0xAD[2] = %d,0xAD[3] = %d \n",cmd[0],cmd[1],cmd[2],cmd[3]);

					if( cmd[0] == 0 && cmd[1] == 0 && cmd[2] == 0 && cmd[3] == 0 )
					{
						himax_FlashMode(0);
						return 1;
					}
					else
					{
						himax_FlashMode(0);
						return 0;
					}
				}
				else
				{
					
					//Set Flash Clock Rate
					if( i2c_himax_read(touch_i2c, 0x7F, cmd, 5, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
					cmd[3] = 0x02;
					
					if( i2c_himax_write(touch_i2c, 0x7F ,&cmd[0], 5, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					//Enable Flash
					himax_FlashMode(1);
					
					//Select CRC Mode
					cmd[0] = 0x05;
					cmd[1] = 0x00;
					cmd[2] = 0x00;
					if( i2c_himax_write(touch_i2c, 0xD2 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					} 
					
					//Enable CRC Function
					cmd[0] = 0x01;
					if( i2c_himax_write(touch_i2c, 0xE5 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					//Must delay 30 ms
					msleep(30);
					
					//Read HW CRC
					if( i2c_himax_read(touch_i2c, 0xAD, cmd, 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return -1;
					}
					
					if( cmd[0] == 0 && cmd[1] == 0 && cmd[2] == 0 && cmd[3] == 0 )
					{
						himax_FlashMode(0);
						return 1;
					}
					else 
					{
						himax_FlashMode(0);
						return 0;
					}
				}
			}
			return 0;
		}
		
		//----[HX_RST_PIN_FUNC]-------------------------------------------------------------------------------start
			#ifdef HX_RST_PIN_FUNC    
			void himax_HW_reset(void)
			{
				#ifdef HX_ESD_WORKAROUND
				ESD_RESET_ACTIVATE = 2;
				#endif
				
				#ifdef HX_RST_BY_POWER
					//power off by different platform's API
					msleep(100);
					
					//power on by different platform's API
					msleep(100);
					
				#else
					gpio_set_value(private_ts->rst_gpio, 0);
					msleep(100);
					gpio_set_value(private_ts->rst_gpio, 1);
					msleep(100);
				#endif
			}
			#endif	
		//----[HX_RST_PIN_FUNC]---------------------------------------------------------------------------------end
	//----[ register flow function]---------------------------------------------------------------------------end
	
	//----[ ESD function]-----------------------------------------------------------------------------------start
		int himax_hang_shaking(void)    //0:Running, 1:Stop, 2:I2C Fail
		{
			int ret, result;
			uint8_t hw_reset_check[1];
			uint8_t hw_reset_check_2[1];
			uint8_t buf0[2];
			
			//Mutexlock Protect Start
			mutex_lock(&private_ts->mutex_lock);
			//Mutexlock Protect End	
			
			//Write 0xF2
			buf0[0] = 0xF2;
			if(IC_STATUS_CHECK == 0xAA)
			{
				buf0[1] = 0xAA;
				IC_STATUS_CHECK = 0x55;
			}
			else
			{
				buf0[1] = 0x55;
				IC_STATUS_CHECK = 0xAA;
			}
			
			ret = i2c_himax_master_write(private_ts->client, buf0, 2, DEFAULT_RETRY_CNT);
			if(ret < 0) 
			{
				printk(KERN_ERR "[Himax]:write 0xF2 failed line: %d \n",__LINE__);
				goto work_func_send_i2c_msg_fail;
			}
			msleep(15); //Must more than 1 frame
			
			buf0[0] = 0xF2;
			buf0[1] = 0x00;
			ret = i2c_himax_master_write(private_ts->client, buf0, 2, DEFAULT_RETRY_CNT);
			if(ret < 0) 
			{
				printk(KERN_ERR "[Himax]:write 0xF2 failed line: %d \n",__LINE__);
				goto work_func_send_i2c_msg_fail;
			}
			msleep(2);
			
			ret = i2c_himax_read(private_ts->client, 0x90, hw_reset_check, 1, DEFAULT_RETRY_CNT);
			if(ret < 0)
			{
				printk(KERN_ERR "[Himax]:i2c_himax_read 0xD1 failed line: %d \n",__LINE__);
				goto work_func_send_i2c_msg_fail;
			}
			printk("[Himax]: ESD 0x90 - 0x%x.\n", hw_reset_check[0]);
			
			if((IC_STATUS_CHECK != hw_reset_check[0]))
			{
				msleep(2);
				ret = i2c_himax_read(private_ts->client, 0x90, hw_reset_check_2, 1, DEFAULT_RETRY_CNT);
				if(ret < 0)
				{
					printk(KERN_ERR "[Himax]:i2c_himax_read 0xD1 failed line: %d \n",__LINE__);
					goto work_func_send_i2c_msg_fail;
				}
				//printk("[Himax]: ESD check 2 0xD1 - 0x%x.\n", hw_reset_check_2[0]);
			
				if(hw_reset_check[0] == hw_reset_check_2[0])
				{
					result = 1; //MCU Stop
				}
				else
				{
					result = 0; //MCU Running
				}
			}
			else
			{
				result = 0; //MCU Running
			}
			
			//Mutexlock Protect Start
			mutex_unlock(&private_ts->mutex_lock);
			//Mutexlock Protect End
			return result;
			
			work_func_send_i2c_msg_fail:
			//Mutexlock Protect Start
			mutex_unlock(&private_ts->mutex_lock);
			//Mutexlock Protect End
			return 2;
		}
		
		//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
			#ifdef ENABLE_CHIP_RESET_MACHINE
			static void himax_chip_reset_function(struct work_struct *dat)
			{
				printk("[Himax]:himax_chip_reset_function ++ \n");
				
				//if(private_ts->retry_time <= 10)
				{
					//Wakelock Protect start
					wake_lock(&private_ts->wake_lock);
					//Wakelock Protect end
					
					//Mutexlock Protect Start
					mutex_lock(&private_ts->mutex_lock);
					//Mutexlock Protect End
					
					#ifdef HX_ESD_WORKAROUND
					ESD_RESET_ACTIVATE = 2;
					#endif
					
					#ifdef HX_RST_BY_POWER
						//power off by different platform's API
						msleep(100);
						
						//power on by different platform's API
						msleep(100);
						
					#else
						gpio_set_value(private_ts->rst_gpio, 0);
						msleep(30);
						gpio_set_value(private_ts->rst_gpio, 1);
						msleep(30);
					#endif
					
					//Mutexlock Protect Start
					mutex_unlock(&private_ts->mutex_lock);
					//Mutexlock Protect End		
					
					himax_ts_poweron(private_ts);
					
					//Wakelock Protect start
					wake_unlock(&private_ts->wake_lock);
					//Wakelock Protect end
					
					// if(gpio_get_value(private_ts->intr_gpio) == 0)
					// {
					// 	printk("[Himax]%s: IRQ = 0, Enable IRQ\n", __func__);
					// 	enable_irq(private_ts->client->irq);
					// }
				}
				private_ts->retry_time ++;
				printk("[Himax]:himax_chip_reset_function retry_time =%d --\n",private_ts->retry_time);
			}
			#endif
		//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end
		
		//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
			#ifdef HX_ESD_WORKAROUND
			void ESD_HW_REST(void)
			{
				ESD_RESET_ACTIVATE = 2;
				ESD_COUNTER = 0;
				
				printk("Himax TP: ESD - Reset\n");
				
				//Wakelock Protect start
				wake_lock(&private_ts->wake_lock);
				//Wakelock Protect end
				
				//Mutexlock Protect Start
				mutex_lock(&private_ts->mutex_lock);
				//Mutexlock Protect End
				
				#ifdef HX_RST_BY_POWER
					//power off by different platform's API
					msleep(100);
					
					//power on by different platform's API
					msleep(100);
					
				#else
					gpio_set_value(private_ts->rst_gpio, 0);
					msleep(30);
					gpio_set_value(private_ts->rst_gpio, 1);
					msleep(30);
				#endif
				
				//Mutexlock Protect Start
				mutex_unlock(&private_ts->mutex_lock);
				//Mutexlock Protec End
				
				himax_ts_poweron(private_ts);
				
				//Wakelock Protect start
				wake_unlock(&private_ts->wake_lock);
				//Wakelock Protect end
				
				// if(gpio_get_value(private_ts->intr_gpio) == 0)
				// {
				// 	printk("[Himax]%s: IRQ = 0, Enable IRQ\n", __func__);
				// 	enable_irq(private_ts->client->irq);
				// }
			}
			#endif
		//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end

		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
			#ifdef ENABLE_CHIP_STATUS_MONITOR
			static int himax_chip_monitor_function(struct work_struct *dat) //for ESD solution
			{
				int ret;
				
				printk("[Himax]: ESD Polling : himax_chip_monitor_function.\n");
				if(private_ts->running_status == 0)//&& himax_chip->suspend_state == 0)
				{
					//printk(KERN_INFO "[Himax] %s \n", __func__);
					if(gpio_get_value(private_ts->intr_gpio) == 0)
					{
						printk("[Himax]%s: IRQ = 0, Enable IRQ\n", __func__);
						enable_irq(private_ts->client->irq);
					}
					
					ret = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail
					if(ret == 2)
					{
						queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
						printk(KERN_INFO "[Himax] %s: I2C Fail \n", __func__);
					}
					if(ret == 1)
					{
						printk(KERN_INFO "[Himax] %s: MCU Stop \n", __func__);
						private_ts->retry_time = 0;
						ESD_HW_REST();
					}
					else
						printk(KERN_INFO "[Himax] %s: MCU Running \n", __func__);
					
					queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_monitor, 10*HZ);
				}
				return 0;
			}
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end
	//----[ ESD function]-----------------------------------------------------------------------------------start
	
	//----[HX_PORTING_DEB_MSG]------------------------------------------------------------------------------start
		#ifdef HX_PORTING_DEB_MSG  
		/*
		static int himax_i2c_test_function(struct himax_ts_data *ts_modify)
		{
			uint8_t buf0[5];
			int ret = 0;
			
			buf0[0] = 0xE9;	
			buf0[1] = 0x01;
			buf0[2] = 0x01;
			
			while(1)
			{     
				ret = i2c_himax_master_write(ts_modify->client, buf0, 3, DEFAULT_RETRY_CNT);//sleep out
				if(ret < 0) 
				{
					printk(KERN_ERR "*****HIMAX PORTING: i2c_master_send failed addr = 0x%x\n",ts_modify->client->addr);
				}
				else
				{
					printk(KERN_ERR "*****HIMAX PORTING: OK addr = 0x%x\n",ts_modify->client->addr);
				} 
				mdelay(200);  
			}
			return ret;
		}*/
		#endif 
	//----[HX_PORTING_DEB_MSG]--------------------------------------------------------------------------------end
	
	//----[HX_IREF_MODIFY]----------------------------------------------------------------------------------start
		#ifdef HX_IREF_MODIFY
		int himax_modifyIref(void)
		{
			//int readLen;
			unsigned char i;
			uint8_t cmd[5];
			uint8_t Iref[2] = {0x00,0x00};
			
			cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x08;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x00;
			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			if( i2c_himax_write_command(touch_i2c, 0x46, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			mdelay(5);
			for(i=0;i<16;i++)
			{
				if(cmd[1]==SFR_3u_1[i][0]&&cmd[2]==SFR_3u_1[i][1])
				{
					Iref[0]= SFR_6u_1[i][0];
					Iref[1]= SFR_6u_1[i][1];
				} 
			}
				
			cmd[0] = 0x01;cmd[1] = 0x00;cmd[2] = 0x06;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = 0x00;cmd[1] = 0x00;cmd[2] = 0x00;
			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			cmd[0] = Iref[0];cmd[1] = Iref[1];cmd[2] = 0x27;cmd[3] = 0x27;
			if( i2c_himax_write(touch_i2c, 0x45 ,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			
			if( i2c_himax_write_command(touch_i2c, 0x4A, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}  
			return 1;    
		}
		#endif
	//----[HX_IREF_MODIFY]------------------------------------------------------------------------------------end
	
		static int himax_read_flash(unsigned char *buf, unsigned int addr_start, unsigned int length) //OK
		{
			u16 i = 0;
			u16 j = 0;	
			u16 k = 0;
			uint8_t cmd[4];
			u16 local_start_addr   = addr_start / 4;                                                
			u16 local_length       = length;                        
			u16 local_end_addr	   = (addr_start + length ) / 4 + 1;     
			u16 local_addr		   = addr_start % 4;   		
			
			printk("Himax %s addr_start = %d , local_start_addr = %d , local_length = %d , local_end_addr = %d , local_addr = %d \n",__func__,addr_start,local_start_addr,local_length,local_end_addr,local_addr);
			if( i2c_himax_write_command(touch_i2c, 0x81, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 81 fail.\n",__func__);
				return 0;
			}
			msleep(120);
			if( i2c_himax_write_command(touch_i2c, 0x82, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 82 fail.\n",__func__);
				return 0;
			}
			msleep(100);
			cmd[0] = 0x01;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
				return 0;
			}
			msleep(100);
			i = local_start_addr;
			do
			{
				cmd[0] = i & 0x1F;					
				cmd[1] = (i >> 5) & 0x1F;		
				cmd[2] = (i >> 10) & 0x1F;	
				if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				printk("Himax cmd[0]=%d,cmd[1]=%d,cmd[2]=%d,cmd[3]=%d\n",cmd[0],cmd[1],cmd[2],cmd[3]);
				if(i == local_start_addr) //first page
				{
					j = 0;
					for(k = local_addr; k < 4 && j < local_length; k++)
					{
						buf[j++] = cmd[k];
					}
				}
				else //other page
				{
					for(k = 0; k < 4 && j < local_length; k++)
					{
						buf[j++] = cmd[k];
					}
				}
				i++;
			}
			while(i < local_end_addr);
			cmd[0] = 0;
			if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 1, DEFAULT_RETRY_CNT) < 0)
			{
				return 0;
			}
			return 1;
		}	
	//----[firmware version read]---------------------------------------------------------------------------start
		u8 himax_read_FW_ver(bool hw_reset)
		{
			u16 fw_ver_maj_start_addr;
			u16 fw_ver_maj_end_addr;
			u16 fw_ver_maj_addr;
			u16 fw_ver_maj_length;
			
			u16 fw_ver_min_start_addr;
			u16 fw_ver_min_end_addr;
			u16 fw_ver_min_addr;
			u16 fw_ver_min_length;
			
			u16 cfg_ver_maj_start_addr;
			u16 cfg_ver_maj_end_addr;
			u16 cfg_ver_maj_addr;
			u16 cfg_ver_maj_length;
			
			u16 cfg_ver_min_start_addr;
			u16 cfg_ver_min_end_addr;
			u16 cfg_ver_min_addr;
			u16 cfg_ver_min_length;
			
			uint8_t cmd[3];
			u16 i = 0;
			u16 j = 0;	
			u16 k = 0;
			
			fw_ver_maj_start_addr 	= FW_VER_MAJ_FLASH_ADDR / 4;									// start addr = 133 / 4 = 33 
			fw_ver_maj_length				= FW_VER_MAJ_FLASH_LENG;								// length = 1
			fw_ver_maj_end_addr 		= (FW_VER_MAJ_FLASH_ADDR + fw_ver_maj_length ) / 4 + 1;		// end addr = 134 / 4 = 33
			fw_ver_maj_addr 				= FW_VER_MAJ_FLASH_ADDR % 4;							// 133 mod 4 = 1
			
			fw_ver_min_start_addr   = FW_VER_MIN_FLASH_ADDR / 4;								// start addr = 134 / 4 = 33
			fw_ver_min_length       = FW_VER_MIN_FLASH_LENG;									// length = 1
			fw_ver_min_end_addr     = (FW_VER_MIN_FLASH_ADDR + fw_ver_min_length ) / 4 + 1;		// end addr = 135 / 4 = 33
			fw_ver_min_addr         = FW_VER_MIN_FLASH_ADDR % 4;								// 134 mod 4 = 2
			
			cfg_ver_maj_start_addr  = CFG_VER_MAJ_FLASH_ADDR / 4;								// start addr = 160 / 4 = 40
			cfg_ver_maj_length      = CFG_VER_MAJ_FLASH_LENG;									// length = 12
			cfg_ver_maj_end_addr    = (CFG_VER_MAJ_FLASH_ADDR + cfg_ver_maj_length ) / 4 + 1;	// end addr = (160 + 12) / 4 = 43
			cfg_ver_maj_addr        = CFG_VER_MAJ_FLASH_ADDR % 4;								// 160 mod 4 = 0
			
			cfg_ver_min_start_addr  = CFG_VER_MIN_FLASH_ADDR / 4;								// start addr = 172 / 4 = 43
			cfg_ver_min_length      = CFG_VER_MIN_FLASH_LENG;									// length = 12
			cfg_ver_min_end_addr    = (CFG_VER_MIN_FLASH_ADDR + cfg_ver_min_length ) / 4 + 1;	// end addr = (172 + 12) / 4 = 46
			cfg_ver_min_addr        = CFG_VER_MIN_FLASH_ADDR % 4;								// 172 mod 4 = 0
			
			disable_irq(private_ts->client->irq);
			
			#ifdef HX_RST_PIN_FUNC
			if(hw_reset)
			{
				himax_HW_reset();
			}
			#endif
			
			//Sleep out
			if( i2c_himax_write(touch_i2c, 0x81 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)	
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				return 0;
			}
			mdelay(120);

			//Enter flash mode
			himax_FlashMode(1);

			//Read Flash Start
			//FW Version MAJ
			i = fw_ver_maj_start_addr;
			do
			{
				cmd[0] = i & 0x1F;					//column 	= 33 mod 32 	= 1
				cmd[1] = (i >> 5) & 0x1F;		//page 		= 33 / 32 		= 1
				cmd[2] = (i >> 10) & 0x1F;	//sector 	= 33 / 1024 	= 0

				if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}

				if(i == fw_ver_maj_start_addr) //first page
				{
					j = 0;
					for( k = fw_ver_maj_addr; k < 4 && j < fw_ver_maj_length; k++)
					{
						FW_VER_MAJ_buff[j++] = cmd[k];
					}
				}
				else //other page
				{
					for( k = 0; k < 4 && j < fw_ver_maj_length; k++)
					{
						FW_VER_MAJ_buff[j++] = cmd[k];
					}
				}
				i++;
			}
			while(i < fw_ver_maj_end_addr);

			//FW Version MIN
			i = fw_ver_min_start_addr;
			do
			{
				cmd[0] = i & 0x1F;					//column 	= 33 mod 32 	= 1
				cmd[1] = (i >> 5) & 0x1F;		//page		= 33 / 32			= 1
				cmd[2] = (i >> 10) & 0x1F;	//sector	= 33 / 1024		= 0
				
				if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
			
				if(i == fw_ver_min_start_addr) //first page
				{
					j = 0;
					for(k = fw_ver_min_addr; k < 4 && j < fw_ver_min_length; k++)
					{
						FW_VER_MIN_buff[j++] = cmd[k];
					}
				}
				else //other page
				{
					for(k = 0; k < 4 && j < fw_ver_min_length; k++)
					{
						FW_VER_MIN_buff[j++] = cmd[k];
					}
				}
				i++;
			}while(i < fw_ver_min_end_addr);


			//CFG Version MAJ
			i = cfg_ver_maj_start_addr;
			do
			{
				cmd[0] = i & 0x1F;					//column 	= 40 mod 32 	= 8
				cmd[1] = (i >> 5) & 0x1F;		//page 		= 40 / 32 		= 1
				cmd[2] = (i >> 10) & 0x1F;	//sector 	= 40 / 1024 	= 0
				
				if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				
				if(i == cfg_ver_maj_start_addr) //first page
				{
					j = 0;
					for( k = cfg_ver_maj_addr; k < 4 && j < cfg_ver_maj_length; k++)
					{
						CFG_VER_MAJ_buff[j++] = cmd[k];
					}
				}
				else //other page
				{
					for(k = 0; k < 4 && j < cfg_ver_maj_length; k++)
					{
						CFG_VER_MAJ_buff[j++] = cmd[k];
					}
				}
				i++;
			}
			while(i < cfg_ver_maj_end_addr);

			//CFG Version MIN
			i = cfg_ver_min_start_addr;
			do
			{
				cmd[0] = i & 0x1F;					//column 	= 43 mod 32 	= 11
				cmd[1] = (i >> 5) & 0x1F;		//page 		= 43 / 32 		= 1
				cmd[2] = (i >> 10) & 0x1F;	//sector 	= 43 / 1024		= 0
				
				if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
			
				if(i == cfg_ver_min_start_addr) //first page
				{
					j = 0;
					for(k = cfg_ver_min_addr; k < 4 && j < cfg_ver_min_length; k++)
					{
						CFG_VER_MIN_buff[j++] = cmd[k];
					}
				}
				else //other page
				{
					for(k = 0; k < 4 && j < cfg_ver_min_length; k++)
					{
						CFG_VER_MIN_buff[j++] = cmd[k];
					}
				}
				i++;
			}
			while(i < cfg_ver_min_end_addr);

			//Exit flash mode
			himax_FlashMode(0);

			/*********************************** 
			Check FW Version , TBD
			FW Major version 		: FW_VER_MAJ_buff
			FW Minor version 		: FW_VER_MIN_buff
			CFG Major version 	: CFG_VER_MAJ_buff
			CFG Minor version 	: CFG_VER_MIN_buff
			
			return 0 : 
			return 1 :
			return 2 :
			
			***********************************/

			printk("FW_VER_MAJ_buff : 0x%2.2X \n",FW_VER_MAJ_buff[0]);
			printk("FW_VER_MIN_buff : 0x%2.2X \n",FW_VER_MIN_buff[0]);

			printk("CFG_VER_MAJ_buff : ");			
			for(i=0; i<12; i++)
				printk(" 0x%2.2X ,",CFG_VER_MAJ_buff[i]);
			printk("\n");

			printk("CFG_VER_MIN_buff : ");			
			for(i=0; i<12; i++)
				printk(" 0x%2.2X ,",CFG_VER_MIN_buff[i]);
			printk("\n");
			#ifdef ENABLE_CHIP_RESET_MACHINE
			if(private_ts->init_success)
			{
				queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
			}
			#endif
			enable_irq(private_ts->client->irq);
			return 0;
	}

	static bool i_Check_FW_Version_func()
	{
		u16 fw_ver_maj_start_addr;
		u16 fw_ver_maj_end_addr;
		u16 fw_ver_maj_addr;
		u16 fw_ver_maj_length;

		u16 fw_ver_min_start_addr;
		u16 fw_ver_min_end_addr;
		u16 fw_ver_min_addr;
		u16 fw_ver_min_length;

		u16 cfg_ver_maj_start_addr;
		u16 cfg_ver_maj_end_addr;
		u16 cfg_ver_maj_addr;
		u16 cfg_ver_maj_length;

		u16 cfg_ver_min_start_addr;
		u16 cfg_ver_min_end_addr;
		u16 cfg_ver_min_addr;
		u16 cfg_ver_min_length;

		uint8_t cmd[3];
		u16 i = 0;
		u16 j = 0;
		u16 k = 0;

		fw_ver_maj_start_addr 	= FW_VER_MAJ_FLASH_ADDR / 4;															// start addr = 133 / 4 = 33
		fw_ver_maj_length				= FW_VER_MAJ_FLASH_LENG;																	// length = 1
		fw_ver_maj_end_addr 		= (FW_VER_MAJ_FLASH_ADDR + fw_ver_maj_length ) / 4 + 1;		// end addr = 134 / 4 = 33
		fw_ver_maj_addr 				= FW_VER_MAJ_FLASH_ADDR % 4;

		fw_ver_min_start_addr   = FW_VER_MIN_FLASH_ADDR / 4;															// start addr = 134 / 4 = 33
		fw_ver_min_length       = FW_VER_MIN_FLASH_LENG;																	// length = 1
		fw_ver_min_end_addr     = (FW_VER_MIN_FLASH_ADDR + fw_ver_min_length ) / 4 + 1;		// end addr = 135 / 4 = 33
		fw_ver_min_addr         = FW_VER_MIN_FLASH_ADDR % 4;															// 134 mod 4 = 2

		cfg_ver_maj_start_addr  = CFG_VER_MAJ_FLASH_ADDR / 4;															// start addr = 160 / 4 = 40
		cfg_ver_maj_length      = CFG_VER_MAJ_FLASH_LENG;																	// length = 12
		cfg_ver_maj_end_addr    = (CFG_VER_MAJ_FLASH_ADDR + cfg_ver_maj_length ) / 4 + 1;	// end addr = (160 + 12) / 4 = 43
		cfg_ver_maj_addr        = CFG_VER_MAJ_FLASH_ADDR % 4;															// 160 mod 4 = 0

		cfg_ver_min_start_addr  = CFG_VER_MIN_FLASH_ADDR / 4;															// start addr = 172 / 4 = 43
		cfg_ver_min_length      = CFG_VER_MIN_FLASH_LENG;																	// length = 12
		cfg_ver_min_end_addr    = (CFG_VER_MIN_FLASH_ADDR + cfg_ver_min_length ) / 4 + 1;	// end addr = (172 + 12) / 4 = 46
		cfg_ver_min_addr        = CFG_VER_MIN_FLASH_ADDR % 4;

		//Sleep out
		if( i2c_himax_write(touch_i2c, 0x81 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
		{
			printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
			goto  firmware_read_fail;
		}
		mdelay(120);

		if( i2c_himax_write(touch_i2c, 0x82 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
		{
			printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 82 fail.\n",__func__);
			goto  firmware_read_fail;
		}
		msleep(100);

		//Enter flash mode
		himax_FlashMode(1);

		//Read Flash Start
		//FW Version MAJ
		i = fw_ver_maj_start_addr;
		do
		{
			cmd[0] = i & 0x1F;					//column 	= 33 mod 32 	= 1
			cmd[1] = (i >> 5) & 0x1F;		//page 		= 33 / 32 		= 1
			cmd[2] = (i >> 10) & 0x1F;	//sector 	= 33 / 1024 	= 0

			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if(i == fw_ver_maj_start_addr) //first page
			{
				j = 0;
				for( k = fw_ver_maj_addr; k < 4 && j < fw_ver_maj_length; k++)
				{
					FW_VER_MAJ_buff[j++] = cmd[k];
				}
			}
			else //other page
			{
				for( k = 0; k < 4 && j < fw_ver_maj_length; k++)
				{
					FW_VER_MAJ_buff[j++] = cmd[k];
				}
			}
			i++;
		}while(i < fw_ver_maj_end_addr);

		//FW Version MIN
		i = fw_ver_min_start_addr;
		do
		{
			cmd[0] = i & 0x1F;					//column	= 33 mod 32		= 1
			cmd[1] = (i >> 5) & 0x1F;		//page		= 33 / 32			= 1
			cmd[2] = (i >> 10) & 0x1F;	//sector	= 33 / 1024		= 0

			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if(i == fw_ver_min_start_addr) //first page
			{
				j = 0;
				for(k = fw_ver_min_addr; k < 4 && j < fw_ver_min_length; k++)
				{
					FW_VER_MIN_buff[j++] = cmd[k];
				}
			}
			else //other page
			{
				for(k = 0; k < 4 && j < fw_ver_min_length; k++)
				{
					FW_VER_MIN_buff[j++] = cmd[k];
				}
			}
			i++;
		}while(i < fw_ver_min_end_addr);


		//CFG Version MAJ
		i = cfg_ver_maj_start_addr;
		do
		{
			cmd[0] = i & 0x1F;					//column 	= 40 mod 32 	= 8
			cmd[1] = (i >> 5) & 0x1F;		//page 		= 40 / 32 		= 1
			cmd[2] = (i >> 10) & 0x1F;	//sector 	= 40 / 1024 	= 0

			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if(i == cfg_ver_maj_start_addr) //first page
			{
				j = 0;
				for( k = cfg_ver_maj_addr; k < 4 && j < cfg_ver_maj_length; k++)
				{
					CFG_VER_MAJ_buff[j++] = cmd[k];
				}
			}
			else //other page
			{
				for(k = 0; k < 4 && j < cfg_ver_maj_length; k++)
				{
					CFG_VER_MAJ_buff[j++] = cmd[k];
				}
			}
			i++;
		}while(i < cfg_ver_maj_end_addr);

		//CFG Version MIN
		i = cfg_ver_min_start_addr;
		do
		{
			cmd[0] = i & 0x1F;					//column 	= 43 mod 32 	= 11
			cmd[1] = (i >> 5) & 0x1F;		//page 		= 43 / 32 		= 1
			cmd[2] = (i >> 10) & 0x1F;	//sector 	= 43 / 1024		= 0

			if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_write(touch_i2c, 0x46 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if( i2c_himax_read(touch_i2c, 0x59, cmd, 4, DEFAULT_RETRY_CNT) < 0)
			{
				printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
				goto  firmware_read_fail;
			}

			if(i == cfg_ver_min_start_addr) //first page
			{
				j = 0;
				for(k = cfg_ver_min_addr; k < 4 && j < cfg_ver_min_length; k++)
				{
					CFG_VER_MIN_buff[j++] = cmd[k];
				}
			}
			else //other page
			{
				for(k = 0; k < 4 && j < cfg_ver_min_length; k++)
				{
					CFG_VER_MIN_buff[j++] = cmd[k];
				}
			}
			i++;
		}
		while(i < cfg_ver_min_end_addr);

		printk("FW_VER_MAJ_buff : 0x%2.2X \n",FW_VER_MAJ_buff[0]);
		printk("FW_VER_MIN_buff : 0x%2.2X \n",FW_VER_MIN_buff[0]);
		printk("CFG_VER_MAJ_buff : ");
		for(k = 0; k < cfg_ver_maj_length; k++)
		{
			printk("0x%2.2X,",CFG_VER_MAJ_buff[k]);
		}
		printk("\nCFG_VER_MIN_buff : ");

		for(k = 0; k < cfg_ver_min_length; k++)
		{
			printk("0x%2.2X,",CFG_VER_MIN_buff[k]);
		}
		printk("\n");
		return true;
	firmware_read_fail:
		memset(FW_VER_MAJ_buff, 0x00, FW_VER_MAJ_FLASH_LENG);
		memset(FW_VER_MIN_buff, 0x00, FW_VER_MIN_FLASH_LENG);
		memset(CFG_VER_MAJ_buff, 0x00, CFG_VER_MAJ_FLASH_LENG);
		memset(CFG_VER_MIN_buff, 0x00, CFG_VER_MIN_FLASH_LENG);
		return false;
	}
	//----[firmware version read]-----------------------------------------------------------------------------end

	void himax_touch_information(void)
	{
		static unsigned char temp_buffer[6];
		char data[12] = {0};
		
		if(IC_TYPE == HX_85XX_A_SERIES_PWON)
		{
			HX_RX_NUM				= 0;
			HX_TX_NUM				= 0;
			HX_BT_NUM				= 0;
			HX_X_RES				= 0;
			HX_Y_RES				= 0;
			HX_MAX_PT				= 0;
			HX_XY_REVERSE 	= false;
			HX_INT_IS_EDGE	= false;
		}	
		else if(IC_TYPE == HX_85XX_B_SERIES_PWON)
		{
			HX_RX_NUM				= 0;
			HX_TX_NUM				= 0;
			HX_BT_NUM				= 0;
			HX_X_RES				= 0;
			HX_Y_RES				= 0;
			HX_MAX_PT				= 0;
			HX_XY_REVERSE 	= false;
			HX_INT_IS_EDGE	= false;
		}
		else if(IC_TYPE == HX_85XX_C_SERIES_PWON)
		{
			//sensor off & sleep in

			i2c_himax_write_command(touch_i2c, 0x82, DEFAULT_RETRY_CNT);
			msleep(120);
			
			//XY_REVERSE //RF1
			i2c_himax_read(touch_i2c, 0xF1, data, 2, DEFAULT_RETRY_CNT);
			
			if((data[1] & 0x01) == 0x01)
			{
					HX_INT_IS_EDGE = true;	
			}
			else
			{
					HX_INT_IS_EDGE = false;
			}
			
			if((data[0] & 0x04) == 0x04)
			{	
					i2c_himax_read(touch_i2c, 0xC7, data, 4, DEFAULT_RETRY_CNT);
					HX_X_RES = data[2]*256 + data[3];
					HX_Y_RES = data[0]*256 + data[1];			
			}
			else
			{
					i2c_himax_read(touch_i2c, 0xC7, data, 4, DEFAULT_RETRY_CNT);
					HX_X_RES = data[0]*256 + data[1];
					HX_Y_RES = data[2]*256 + data[3];			
			}
			
			//RX/TX/BT channel
			i2c_himax_read(touch_i2c, 0xEA, data, 3, DEFAULT_RETRY_CNT);
			HX_RX_NUM = data[0];
			HX_TX_NUM = data[1];
			HX_BT_NUM = data[2] & 0x0F;
			
			//PT num
			i2c_himax_read(touch_i2c, 0xF0, data, 3, DEFAULT_RETRY_CNT);
			HX_MAX_PT = data[0] >> 4;
			
			i2c_himax_write_command(touch_i2c, 0x83, DEFAULT_RETRY_CNT);
			msleep(120);
		}
		else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
		{
			//sensor off

			i2c_himax_write_command(touch_i2c, 0x82, DEFAULT_RETRY_CNT);
			msleep(120);

			//Update himax information from configuration
			//FE(70)
			data[0] = 0xE1;
			data[1] = 0x15;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);

			data[0] = 0xD8;
			data[1] = 0x00;
			data[2] = 0x70;
			i2c_himax_master_write(touch_i2c, &data[0],3,DEFAULT_RETRY_CNT);
			msleep(10);

			i2c_himax_read(touch_i2c, 0x5A, data, 12, DEFAULT_RETRY_CNT);

			HX_RX_NUM = data[1];               // FE(70)
			HX_TX_NUM = data[2];               // FE(71)
			HX_MAX_PT = (data[3] & 0xF0) >> 4; // FE(72)

			#ifdef HX_EN_BUTTON
			HX_BT_NUM = (data[3] & 0x0F); //FE(72)
			#endif
		
			if((data[5] & 0x04) == 0x04) {//FE(74)
				HX_XY_REVERSE = true;
				HX_Y_RES = data[7]*256 + data[8]; //FE(76),FE(77)
				HX_X_RES = data[9]*256 + data[10]; //FE(78),FE(79)
			} else {
				HX_XY_REVERSE = false;
				HX_X_RES = data[7]*256 + data[8]; //FE(76),FE(77)
				HX_Y_RES = data[9]*256 + data[10]; //FE(78),FE(79)
			}

			data[0] = 0xE1;
			data[1] = 0x00;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);
			
			#ifdef HX_EN_MUT_BUTTON
			//FE(64)
			data[0] = 0xE1;
			data[1] = 0x15;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);

			data[0] = 0xD8;
			data[1] = 0x00;
			data[2] = 0x64;
			i2c_himax_master_write(touch_i2c, &data[0],3,DEFAULT_RETRY_CNT);
			msleep(10);

			i2c_himax_read(touch_i2c, 0x5A, data, 4, DEFAULT_RETRY_CNT);

			HX_BT_NUM = (data[0] & 0x03);

			data[0] = 0xE1;
			data[1] = 0x00;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);
			#endif

			//FE(02)
			data[0] = 0xE1;
			data[1] = 0x15;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);
  		
			data[0] = 0xD8;
			data[1] = 0x00;
			data[2] = 0x02;
			i2c_himax_master_write(touch_i2c, &data[0],3,DEFAULT_RETRY_CNT);
			msleep(10);
  		
			i2c_himax_read(touch_i2c, 0x5A, data, 10, DEFAULT_RETRY_CNT);
  		
			if((data[2] & 0x01) == 1) {//FE(02)
				HX_INT_IS_EDGE = true;
			} else {
				HX_INT_IS_EDGE = false;
			}
  		
			data[0] = 0xE1;
			data[1] = 0x00;
			i2c_himax_master_write(touch_i2c, &data[0],2,DEFAULT_RETRY_CNT);
			msleep(10);
  		
			i2c_himax_write_command(touch_i2c, 0x83, DEFAULT_RETRY_CNT);
			msleep(120);
		}
		else if(IC_TYPE == HX_85XX_E_SERIES_PWON)
		{
			//Shouchung modify, initial porting touch in TF103CG
			//For ME303 
			/*HX_RX_NUM				= 32;
			HX_TX_NUM				= 50;
			HX_BT_NUM				= 0;
			HX_X_RES				= 1200;
			HX_Y_RES				= 1920;*/
			//For TF103CG
			HX_RX_NUM				= 27;
			HX_TX_NUM				= 44;
			HX_BT_NUM				= 0;
			HX_X_RES				= 1280;
			HX_Y_RES				= 800;
			//Shouchung end
			HX_MAX_PT				= 10;
			HX_INT_IS_EDGE	= false;
		}
		else
		{
			HX_RX_NUM				= 0;
			HX_TX_NUM				= 0;
			HX_BT_NUM				= 0;
			HX_X_RES				= 0;
			HX_Y_RES				= 0;
			HX_MAX_PT				= 0;
			HX_XY_REVERSE		= false;
			HX_INT_IS_EDGE	= false;
		}
	}
	//----[HX_FW_UPDATE_BY_I_FILE]--------------------------------------------------------------------------start
		#ifdef HX_FW_UPDATE_BY_I_FILE
		int fts_ctpm_fw_upgrade_with_i_file(void)
		{
			unsigned char* ImageBuffer = i_CTPM_FW;
			int fullFileLength = sizeof(i_CTPM_FW_Himax); //Paul Check
			
			int i, j;
			uint8_t cmd[5], last_byte, prePage;
			int FileLength;
			uint8_t checksumResult = 0;
			
			//Try 3 Times
			for (j = 0; j < 3; j++) 
			{
				if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_CRC)
				{
					FileLength = fullFileLength;
				}
				else
				{
					FileLength = fullFileLength - 2;
				}
				
				#ifdef HX_RST_PIN_FUNC
					himax_HW_reset();
				#endif
			
				if( i2c_himax_write(touch_i2c, 0x81 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
		
				mdelay(120);
			
				himax_unlock_flash();  //ok
			
				cmd[0] = 0x05;cmd[1] = 0x00;cmd[2] = 0x02;
				if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
		
				if( i2c_himax_write(touch_i2c, 0x4F ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				mdelay(50);
			
				himax_ManualMode(1);                                                 
				himax_FlashMode(1);                                                     
			
				FileLength = (FileLength + 3) / 4;
				for (i = 0, prePage = 0; i < FileLength; i++) 
				{
					last_byte = 0;
					
					cmd[0] = i & 0x1F;
					if (cmd[0] == 0x1F || i == FileLength - 1)
					{
						last_byte = 1;
					}
					cmd[1] = (i >> 5) & 0x1F;
					cmd[2] = (i >> 10) & 0x1F;
					if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
			
					if (prePage != cmd[1] || i == 0) 
					{
						prePage = cmd[1];
						
						cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						cmd[0] = 0x01;cmd[1] = 0x0D;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
			
						cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					}
			
					memcpy(&cmd[0], &ImageBuffer[4*i], 4);//Paul
					if( i2c_himax_write(touch_i2c, 0x45 ,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
			
					cmd[0] = 0x01;cmd[1] = 0x0D;//cmd[2] = 0x02;
					if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
			
					cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
					if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
			
					if (last_byte == 1) 
					{
						cmd[0] = 0x01;cmd[1] = 0x01;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
						
						cmd[0] = 0x01;cmd[1] = 0x05;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
			
						cmd[0] = 0x01;cmd[1] = 0x01;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
			
						cmd[0] = 0x01;cmd[1] = 0x00;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
			
						mdelay(10);
						if (i == (FileLength - 1)) 
						{
							himax_FlashMode(0);
							himax_ManualMode(0);
							checksumResult = himax_calculateChecksum(ImageBuffer, fullFileLength);
						
							himax_lock_flash();
						
							if (checksumResult) //Success
							{
								return 1;
							} 
							else //Fail
							{
								return 0;
							} 
						}
					}
				}
			}
			return 0;
		}
		
		static bool i_Check_FW_Version()
		{
			int i;
			u32 thisVersion = 0;
			u32 nextVersion = 0;

			if(FW_VER_MAJ_buff[0]== 0xFF || FW_VER_MIN_buff[0] == 0xFF ||
				CFG_VER_MAJ_buff[11]== 0xFF || CFG_VER_MIN_buff[11] == 0xFF)
			{
				return true;
			}
			else
			{
				/*compare FW ID version */
				thisVersion = 0;
				nextVersion = 0;
				thisVersion += CFG_VER_MAJ_buff[CFG_VER_MAJ_FLASH_LENG-1];
				nextVersion += i_CTPM_FW[CFG_VER_MAJ_FLASH_ADDR+CFG_VER_MAJ_FLASH_LENG-1];
				printk("[himax]FW ID, thisversion:%d,nextversion:%d\n",thisVersion,nextVersion);
				if(nextVersion != thisVersion)
				{
					return true;
				}
				else if(thisVersion == nextVersion)
				{
					/*compare FW maj version */
					thisVersion = 0;  //fw_version_int();
					nextVersion = 0;
					for(i=0; i<FW_VER_MAJ_FLASH_LENG; i++)
					{
						if(i != 0)
						{
							nextVersion = nextVersion << 8;
							thisVersion = thisVersion << 8;
						}
						thisVersion += FW_VER_MAJ_buff[i];
						nextVersion += i_CTPM_FW[FW_VER_MAJ_FLASH_ADDR+i];
					}
					printk("[himax]FW MAJ, thisversion:%d,nextversion:%d\n",thisVersion,nextVersion);
					if(nextVersion > thisVersion)
					{
						return true;
					}
					else if(thisVersion == nextVersion)
					{
						/*compare FW min version */
						thisVersion = 0;
						nextVersion = 0;
						for(i=0; i<FW_VER_MIN_FLASH_LENG; i++)
						{
							if(i != 0)
							{
								nextVersion = nextVersion << 8;
								thisVersion = thisVersion << 8;
							}
							thisVersion += FW_VER_MIN_buff[i];
							nextVersion += i_CTPM_FW[FW_VER_MIN_FLASH_ADDR+i];
						}
						printk("[himax]FW MIN, thisversion:%d,nextversion:%d\n",thisVersion,nextVersion);
						if(nextVersion > thisVersion)
						{
							return true;
						}
						else if(thisVersion == nextVersion)
						{
							/*compare CFG min version */
							thisVersion = 0;
							nextVersion = 0;
							thisVersion += CFG_VER_MIN_buff[CFG_VER_MIN_FLASH_LENG-1];
							nextVersion += i_CTPM_FW[CFG_VER_MIN_FLASH_ADDR+CFG_VER_MIN_FLASH_LENG-1];
							printk("[himax]CFG MIN, thisversion:%d,nextversion:%d\n",thisVersion,nextVersion);
							if(nextVersion > thisVersion)
							{
								return true;
							}
							else
							{
								return false;
							}
						}
						else
						{
							return false;
						}
					}
					else
					{
						return false;
					}
				}
				else
				{
					return false;
				}

			}
		}
		
		static void i_update_func(void)
		{
			unsigned char* ImageBuffer = i_CTPM_FW;
			int fullFileLength = sizeof(i_CTPM_FW_Himax);
			bool update_flag = i_Check_FW_Version();

			printk("[himax]update_flag = %d.\n",update_flag);

			if (update_flag)//|| himax_calculateChecksum(ImageBuffer, fullFileLength) == 0 )
			{
				printk("[himax]Start to update\n");
				disable_irq(private_ts->client->irq);
				if(fts_ctpm_fw_upgrade_with_i_file() == 0)
				{
					printk(KERN_ERR "TP upgrade error, line: %d\n", __LINE__);
				}
				else
				{
					printk(KERN_ERR "TP upgrade OK, line: %d\n", __LINE__);
					int i;
					for(i=0; i<FW_VER_MAJ_FLASH_LENG; i++)
						FW_VER_MAJ_buff[i] = i_CTPM_FW[FW_VER_MAJ_FLASH_ADDR+i];
					for(i=0; i<FW_VER_MIN_FLASH_LENG; i++)
						FW_VER_MIN_buff[i] = i_CTPM_FW[FW_VER_MIN_FLASH_ADDR+i];
					for(i=0; i<CFG_VER_MAJ_FLASH_LENG; i++)
						CFG_VER_MAJ_buff[i] = i_CTPM_FW[CFG_VER_MAJ_FLASH_ADDR+i];
					for(i=0; i<CFG_VER_MIN_FLASH_LENG; i++)
						CFG_VER_MIN_buff[i] = i_CTPM_FW[CFG_VER_MIN_FLASH_ADDR+i];
				}
				enable_irq(private_ts->client->irq);
			}
			#ifdef HX_RST_PIN_FUNC
				himax_HW_reset();
			#endif

			msleep(50);
			himax_ts_poweron(private_ts);
		}
		
		#endif
	//----[HX_FW_UPDATE_BY_I_FILE]----------------------------------------------------------------------------end

//=============================================================================================================
//
//	Segment : Himax SYS Debug Function
//
//=============================================================================================================

	//----[HX_TP_PROC_REGISTER]------------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_REGISTER
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_register_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
		static ssize_t himax_register_read(struct device *dev, struct device_attribute *attr, char *buf)
		//Shouchung end
		{
			int ret = 0;
			int base = 0;
			uint16_t loop_i,loop_j;
			uint8_t inData[128];
			uint8_t outData[5];

			memset(outData, 0x00, sizeof(outData));
			memset(inData, 0x00, sizeof(inData));

			printk(KERN_INFO "Himax multi_register_command = %d \n",multi_register_command);

			if(multi_register_command == 1)
			{
				base = 0;
			
				for(loop_i = 0; loop_i < 6; loop_i++)
				{
					if(multi_register[loop_i] != 0x00)
					{
						if(multi_cfg_bank[loop_i] == 1) //config bank register
						{
							outData[0] = 0x15;
							i2c_himax_write(touch_i2c, 0x8C ,&outData[0], 1, DEFAULT_RETRY_CNT);
							msleep(10);
			
							outData[0] = 0x00;
							outData[1] = multi_register[loop_i];
							i2c_himax_write(touch_i2c, 0x8B ,&outData[0], 2, DEFAULT_RETRY_CNT);
							msleep(10);
			
							i2c_himax_read(touch_i2c, 0x5A, inData, 128, DEFAULT_RETRY_CNT);
			
							outData[0] = 0x00;
							i2c_himax_write(touch_i2c, 0x8C ,&outData[0], 1, DEFAULT_RETRY_CNT);
			
							for(loop_j=0; loop_j<128; loop_j++)
							{
								multi_value[base++] = inData[loop_j];
							}
						}
						else //normal register
						{
							i2c_himax_read(touch_i2c, multi_register[loop_i], inData, 128, DEFAULT_RETRY_CNT);
			
							for(loop_j=0; loop_j<128; loop_j++)
							{
								multi_value[base++] = inData[loop_j];
							}
						}
					}
				}
			
				base = 0;
				for(loop_i = 0; loop_i < 6; loop_i++)
				{
					if(multi_register[loop_i] != 0x00)
					{
						if(multi_cfg_bank[loop_i] == 1)
						{
							ret += sprintf(buf + ret, "Register: FE(%x)\n", multi_register[loop_i]);
						}
						else
						{
							ret += sprintf(buf + ret, "Register: %x\n", multi_register[loop_i]);
						}
				
						for (loop_j = 0; loop_j < 128; loop_j++)
						{
							ret += sprintf(buf + ret, "0x%2.2X ", multi_value[base++]);
							if ((loop_j % 16) == 15)
							{
								ret += sprintf(buf + ret, "\n");
							}
						}
					}
				}
				return ret;
			}

			if(config_bank_reg)
			{
				printk(KERN_INFO "[TP] %s: register_command = FE(%x)\n", __func__, register_command);
				
				//Config bank register read flow.
				outData[0] = 0x15;
				i2c_himax_write(touch_i2c, 0x8C,&outData[0], 1, DEFAULT_RETRY_CNT);
				
				msleep(10);
				
				outData[0] = 0x00;
				outData[1] = register_command;
				i2c_himax_write(touch_i2c, 0x8B,&outData[0], 2, DEFAULT_RETRY_CNT);
				
				msleep(10);
				
				i2c_himax_read(touch_i2c, 0x5A, inData, 128, DEFAULT_RETRY_CNT);
				
				msleep(10);
				
				outData[0] = 0x00;
				i2c_himax_write(touch_i2c, 0x8C,&outData[0], 1, DEFAULT_RETRY_CNT);
			}
			else
			{
				if (i2c_himax_read(touch_i2c, register_command, inData, 128, DEFAULT_RETRY_CNT) < 0) 
				{
					return ret;
				}
			}
			
			if(config_bank_reg)
			{
				ret += sprintf(buf, "command: FE(%x)\n", register_command);
			}
			else
			{
				ret += sprintf(buf, "command: %x\n", register_command);
			}

			for (loop_i = 0; loop_i < 128; loop_i++) 
			{
				ret += sprintf(buf + ret, "0x%2.2X ", inData[loop_i]);
				if ((loop_i % 16) == 15)
				{
					ret += sprintf(buf + ret, "\n");
				}
			}
			ret += sprintf(buf + ret, "\n");
			return ret;
		}

		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_register_write(struct file *filp, const char *buff, unsigned long len, void *data)
		static ssize_t himax_register_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
		//Shouchung end
		{
			char buf_tmp[6], length = 0;
			unsigned long result		= 0;
			uint8_t loop_i 					= 0;
			uint16_t base 					= 5;
			uint8_t write_da[128];
			uint8_t outData[5];
			char messages[80] = {0};
			//Shouchung modify, initial porting touch in TF103CG
			//if (len >= 80)
			if (count >= 80)
			//Shouchung end
			{
				printk(KERN_INFO "%s: no command exceeds 80 chars.\n", __func__);
				return -EFAULT;
			}
			//Shouchung modify, initial porting touch in TF103CG
			/*if (copy_from_user(messages, buff, len))
			{
				return -EFAULT;
			}*/
			strcpy(messages, buf);
			//Shouchung end

			memset(buf_tmp, 0x0, sizeof(buf_tmp));
			memset(write_da, 0x0, sizeof(write_da));
			memset(outData, 0x0, sizeof(outData));
			
			if( messages[0] == 'm' && messages[1] == 'r' && messages[2] == ':')
			{
				memset(multi_register, 0x00, sizeof(multi_register));
				memset(multi_cfg_bank, 0x00, sizeof(multi_cfg_bank));
				memset(multi_value, 0x00, sizeof(multi_value));
				
				printk("himax multi register enter\n");
				
				multi_register_command = 1;
				
				base 		= 2;
				loop_i 	= 0;

				while(true)
				{
					if(messages[base] == '\n')
					{
						break;
					}
				
					if(loop_i >= 6 )
					{
						break;
					}
				
					if(messages[base] == ':' && messages[base+1] == 'x' && messages[base+2] == 'F' && messages[base+3] == 'E' && messages[base+4] != ':')
					{
						memcpy(buf_tmp, messages + base + 4, 2);
						if (!strict_strtoul(buf_tmp, 16, &result))
						{
							multi_register[loop_i] = result;
							multi_cfg_bank[loop_i++] = 1;
						}
						base += 6;
					}
					else
					{
						memcpy(buf_tmp, messages + base + 2, 2);
						if (!strict_strtoul(buf_tmp, 16, &result))
						{
							multi_register[loop_i] = result;
							multi_cfg_bank[loop_i++] = 0;
						}
						base += 4;
					}
				}

				printk(KERN_INFO "========================== \n");
				for(loop_i = 0; loop_i < 6; loop_i++)
				{
					printk(KERN_INFO "%d,%d:",multi_register[loop_i],multi_cfg_bank[loop_i]);
				}
				printk(KERN_INFO "\n");
			}
			else if ((messages[0] == 'r' || messages[0] == 'w') && messages[1] == ':') 
			{
				multi_register_command = 0;

				if (messages[2] == 'x') 	
				{
					if(messages[3] == 'F' && messages[4] == 'E') //Config bank register
					{
						config_bank_reg = true;
				
						memcpy(buf_tmp, messages + 5, 2);
						if (!strict_strtoul(buf_tmp, 16, &result))
						{
							register_command = result;
						}
						base = 7;
				
						printk(KERN_INFO "CMD: FE(%x)\n", register_command);
					}
					else
					{
						config_bank_reg = false;
						
						memcpy(buf_tmp, messages + 3, 2);
						if (!strict_strtoul(buf_tmp, 16, &result))
						{
							register_command = result;
						}
						base = 5;
						printk(KERN_INFO "CMD: %x\n", register_command);
					}

					for (loop_i = 0; loop_i < 128; loop_i++) 
					{
						if (messages[base] == '\n') 
						{
							if (messages[0] == 'w')
							{
								if(config_bank_reg)
								{
									outData[0] = 0x15;
									i2c_himax_write(touch_i2c, 0x8C, &outData[0], 1, DEFAULT_RETRY_CNT);
									
									msleep(10);
									
									outData[0] = 0x00;
									outData[1] = register_command;
									i2c_himax_write(touch_i2c, 0x8B, &outData[0], 2, DEFAULT_RETRY_CNT);
									
									msleep(10);
									i2c_himax_write(touch_i2c, 0x40, &write_da[0], length, DEFAULT_RETRY_CNT);
									
									msleep(10);
									
									outData[0] = 0x00;
									i2c_himax_write(touch_i2c, 0x8C, &outData[0], 1, DEFAULT_RETRY_CNT);
									
									printk(KERN_INFO "CMD: FE(%x), %x, %d\n", register_command,write_da[0], length);
								}
								else
								{
									i2c_himax_write(touch_i2c, register_command, &write_da[0], length, DEFAULT_RETRY_CNT);
									printk(KERN_INFO "CMD: %x, %x, %d\n", register_command,write_da[0], length);
								}
							}
						
							printk(KERN_INFO "\n");
							//Shouchung modify, initial porting touch in TF103CG
							//return len;
							return count;
							//Shouchung end
						}
						if (messages[base + 1] == 'x') 
						{
							buf_tmp[4] = '\n';
							buf_tmp[5] = '\0';
							memcpy(buf_tmp, messages + base + 2, 2);
							if (!strict_strtoul(buf_tmp, 16, &result))
							{
								write_da[loop_i] = result;
							}
							length++;
						}
						base += 4;
					}
				}
			}
			//Shouchung modify, initial porting touch in TF103CG
			//return len;
			return count;
			//Shouchung end
		}
		//Shouchung add, initial porting touch in TF103CG
		static DEVICE_ATTR(register, (S_IWUSR|S_IRUGO|S_IWGRP), himax_register_read, himax_register_write);
		//Shouchung end
		#endif
	//----[HX_TP_PROC_REGISTER]--------------------------------------------------------------------------------end

	//----[HX_TP_PROC_DEBUG_LEVEL]---------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_DEBUG_LEVEL
		
		static uint8_t getDebugLevel(void)
		{
			return debug_log_level;
		}
		
		int fts_ctpm_fw_upgrade_with_sys_fs(unsigned char *fw, int len)
		{
			unsigned char* ImageBuffer = fw;//CTPM_FW;
			int fullFileLength = len;//sizeof(CTPM_FW); //Paul Check
			int i, j;
			uint8_t cmd[5], last_byte, prePage;
			int FileLength;
			uint8_t checksumResult = 0;
			
			//Try 3 Times
			for (j = 0; j < 3; j++) 
			{
				if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_CRC)
				{
					FileLength = fullFileLength;
				}
				else
				{
					FileLength = fullFileLength - 2;
				}
				
				#ifdef HX_RST_PIN_FUNC
					himax_HW_reset();
				#endif
				
				if( i2c_himax_write(touch_i2c, 0x81 ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				
				mdelay(120);
				
				himax_unlock_flash();  //ok
				
				cmd[0] = 0x05;cmd[1] = 0x00;cmd[2] = 0x02;
				if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}
				
				if( i2c_himax_write(touch_i2c, 0x4F ,&cmd[0], 0, DEFAULT_RETRY_CNT) < 0)
				{
					printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
					return 0;
				}     
				mdelay(50);
				
				himax_ManualMode(1); 
				himax_FlashMode(1); 
				
				FileLength = (FileLength + 3) / 4;
				for (i = 0, prePage = 0; i < FileLength; i++) 
				{
					last_byte = 0;
					cmd[0] = i & 0x1F;
					if (cmd[0] == 0x1F || i == FileLength - 1)
					{
						last_byte = 1;
					}
					cmd[1] = (i >> 5) & 0x1F;
					cmd[2] = (i >> 10) & 0x1F;
					if( i2c_himax_write(touch_i2c, 0x44 ,&cmd[0], 3, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
					
					if (prePage != cmd[1] || i == 0) 
					{
						prePage = cmd[1];
						cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						cmd[0] = 0x01;cmd[1] = 0x0D;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					}
				
					memcpy(&cmd[0], &ImageBuffer[4*i], 4);//Paul
					if( i2c_himax_write(touch_i2c, 0x45 ,&cmd[0], 4, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
				
					cmd[0] = 0x01;cmd[1] = 0x0D;//cmd[2] = 0x02;
					if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
				
					cmd[0] = 0x01;cmd[1] = 0x09;//cmd[2] = 0x02;
					if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
					{
						printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
						return 0;
					}
				
					if (last_byte == 1) 
					{
						cmd[0] = 0x01;cmd[1] = 0x01;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						cmd[0] = 0x01;cmd[1] = 0x05;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						cmd[0] = 0x01;cmd[1] = 0x01;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
						
						cmd[0] = 0x01;cmd[1] = 0x00;//cmd[2] = 0x02;
						if( i2c_himax_write(touch_i2c, 0x43 ,&cmd[0], 2, DEFAULT_RETRY_CNT) < 0)
						{
							printk(KERN_ERR "[TP] %s: i2c access fail!\n", __func__);
							return 0;
						}
					
						mdelay(10);
						if (i == (FileLength - 1)) 
						{
							himax_FlashMode(0);
							himax_ManualMode(0);
							checksumResult = himax_calculateChecksum(ImageBuffer, fullFileLength);//, address, RST);
							//himax_ManualMode(0);
							himax_lock_flash();
							
							if (checksumResult) //Success
							{
								return 1;
							} 
							else //Fail
							{
								return 0;
							} 
						}
					}
				}
			}
			return 0;
		}
		
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_debug_level_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
		static ssize_t himax_debug_level_read(struct device *dev, struct device_attribute *attr, char *buf)
		//Shouchung end
		{
			int ret = 0;
			int i = 0;
			
			if(debug_level_cmd == 't')
			{
				if(fw_update_complete)
				{
					ret += sprintf(buf, "FW Update Complete \n");
				}
				else
				{
					ret += sprintf(buf, "FW Update Fail \n");
				}
			}
			else if(debug_level_cmd == 'i')
			{
				if(irq_enable)
				{
					ret += sprintf(buf, "IRQ is enable\n");
				}
				else
				{
					ret += sprintf(buf, "IRQ is disable\n");
				}
			}
			else if(debug_level_cmd == 'h')
			{
				if(handshaking_result == 0)
				{
					ret += sprintf(buf, "Handshaking Result = %d (MCU Running)\n",handshaking_result);
				}
				else if(handshaking_result == 1)
				{
					ret += sprintf(buf, "Handshaking Result = %d (MCU Stop)\n",handshaking_result);
				}
				else if(handshaking_result == 2)
				{
					ret += sprintf(buf, "Handshaking Result = %d (I2C Error)\n",handshaking_result);
				}
				else
				{
					ret += sprintf(buf, "Handshaking Result = error \n");
				}
			}
			else if(debug_level_cmd == 'v')
			{
				ret += sprintf(buf + ret, "FW_VER_MAJ_buff = ");
				ret += sprintf(buf + ret, "0x%2.2X \n",FW_VER_MAJ_buff[0]);

				ret += sprintf(buf + ret, "FW_VER_MIN_buff = ");
				ret += sprintf(buf + ret, "0x%2.2X \n",FW_VER_MIN_buff[0]);

				ret += sprintf(buf + ret, "CFG_VER_MAJ_buff = ");
				for( i=0 ; i<12 ; i++)
				{
					ret += sprintf(buf + ret, "0x%2.2X ",CFG_VER_MAJ_buff[i]);
				}
				ret += sprintf(buf + ret, "\n");

				ret += sprintf(buf + ret, "CFG_VER_MIN_buff = ");
				for( i=0 ; i<12 ; i++)
				{
					ret += sprintf(buf + ret, "0x%2.2X ",CFG_VER_MIN_buff[i]);
				}
				ret += sprintf(buf + ret, "\n");

				ret += sprintf(buf + ret, "TP_ID = 0x%2.2X\n",TP_ID);
				ret += sprintf(buf + ret, "FW_ID = 0x%2.2X\n",FW_ID);
			}
			else if(debug_level_cmd == 'd')
			{
				ret += sprintf(buf + ret, "Himax Touch IC Information :\n");
				if(IC_TYPE == HX_85XX_A_SERIES_PWON)
				{
					ret += sprintf(buf + ret, "IC Type : A\n");
				}
				else if(IC_TYPE == HX_85XX_B_SERIES_PWON)
				{
					ret += sprintf(buf + ret, "IC Type : B\n");
				}
				else if(IC_TYPE == HX_85XX_C_SERIES_PWON)
				{
					ret += sprintf(buf + ret, "IC Type : C\n");
				}
				else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
				{
					ret += sprintf(buf + ret, "IC Type : D\n");
				}
				else if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					ret += sprintf(buf + ret, "IC Type : E\n");
				}
				else 
				{
					ret += sprintf(buf + ret, "IC Type error.\n");
				}
				
				if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_SW)
				{
					ret += sprintf(buf + ret, "IC Checksum : SW\n");
				}
				else if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_HW)
				{
					ret += sprintf(buf + ret, "IC Checksum : HW\n");
				}
				else if(IC_CHECKSUM == HX_TP_BIN_CHECKSUM_CRC)
				{
					ret += sprintf(buf + ret, "IC Checksum : CRC\n");
				}
				else
				{
					ret += sprintf(buf + ret, "IC Checksum error.\n");
				}		
				
				if(HX_INT_IS_EDGE)
				{
					ret += sprintf(buf + ret, "Interrupt : EDGE TIRGGER\n");
				}
				else
				{
					ret += sprintf(buf + ret, "Interrupt : LEVEL TRIGGER\n");
				}
				
				ret += sprintf(buf + ret, "RX Num : %d\n",HX_RX_NUM);
				ret += sprintf(buf + ret, "TX Num : %d\n",HX_TX_NUM);
				ret += sprintf(buf + ret, "BT Num : %d\n",HX_BT_NUM);
				ret += sprintf(buf + ret, "X Resolution : %d\n",HX_X_RES);
				ret += sprintf(buf + ret, "Y Resolution : %d\n",HX_Y_RES);
				ret += sprintf(buf + ret, "Max Point : %d\n",HX_MAX_PT);
			}
			else if(debug_level_cmd == 'p')
			{
				if(Flag_print_point)
				{
					ret += sprintf(buf, "Enable report data log\n");
				}
				else
				{
					ret += sprintf(buf, "Disable report data log\n");
				}
			}
			else if (debug_level_cmd == 'r')
			{
				ret += sprintf(buf + ret, "RX Num : %d\n",HX_RX_NUM);
				ret += sprintf(buf + ret, "TX Num : %d\n",HX_TX_NUM);
			}
			else
			{
				ret += sprintf(buf, "%d\n", debug_log_level);
			}
			return ret;
		}
		
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_debug_level_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
		static ssize_t himax_debug_level_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
		//Shouchung end
		{
			struct file* local_filp = NULL;
			mm_segment_t oldfs;
			int result = 0;
			char fileName[128];
			char messages[80] = {0};
			//Shouchung modify, initial porting touch in TF103CG
			//if (len >= 80)
			if (count >= 80)
			//Shouchung end
			{
				printk(KERN_INFO "%s: no command exceeds 80 chars.\n", __func__);
				return -EFAULT;
			}
			//Shouchung modify, initial porting touch in TF103CG
			/*if (copy_from_user(messages, buff, len))
			{
				return -EFAULT;
			}*/
			strcpy(messages, buf);
			//Shouchung end
			if (messages[0] >= '0' && messages[0] <= '9' && messages[1] == '\n')
			{
				debug_log_level = messages[0] - '0';
				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}
			
			if (messages[0] == 'i') //irq
			{
				debug_level_cmd = messages[0];
	
				if( messages[2] == '1') //enable irq	
				{
					enable_irq(private_ts->client->irq);
					irq_enable = true;
				}
				else if(messages[2] == '0') //disable irq
				{
					disable_irq(private_ts->client->irq);
					irq_enable = false;
				}
				else
				{
					printk(KERN_ERR "[TP] %s: debug_level command = 'i' , parameter error.\n", __func__);
				}
				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}
			
			if( messages[0] == 'h') //handshaking
			{
				debug_level_cmd = messages[0];

				disable_irq(private_ts->client->irq);
	
				handshaking_result = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail 
	
				enable_irq(private_ts->client->irq);

				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}

			if( messages[0] == 'v') //firmware version
			{
				debug_level_cmd = messages[0];
				himax_read_FW_ver(true);
				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}
			
			if( messages[0] == 'd') //test
			{
				debug_level_cmd = messages[0];
				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}

			if (messages[0] == 'p') //open report data log
			{
				debug_level_cmd = messages[0];

				if( messages[2] == '1') //enable report data log
				{
					Flag_print_point = 1;
				}
				else if( messages[2] == '0') //disable report data log
				{
					Flag_print_point = 0;
				}
				else
				{
					printk(KERN_ERR "[TP] %s: debug_level command = 'p' , parameter error.\n", __func__);
				}
				return count;
			}

			if (messages[0] == 'r')
			{
				debug_level_cmd = messages[0];

				//Set X and Y channel
				HX_RX_NUM = (messages[2] - '0')*10 + messages[3] - '0';
				HX_TX_NUM = (messages[5] - '0')*10 + messages[6] - '0';
				setXChannel(HX_RX_NUM); // X channel
				setYChannel(HX_TX_NUM); // Y channel

				return count;
			}

			//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start	
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				cancel_delayed_work_sync(&private_ts->himax_chip_monitor);
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end	
			
			//Wakelock Protect start
			wake_lock(&private_ts->wake_lock);
			//Wakelock Protect end
			
			//Mutexlock Protect Start
			mutex_lock(&private_ts->mutex_lock);
			//Mutexlock Protect End
			
			if(messages[0] == 't')
			{
				debug_level_cmd 		= messages[0];
				fw_update_complete		= false;
				
				memset(fileName, 0, 128);
				// parse the file name
				//Shouchung modify, initial porting touch in TF103CG
				//snprintf(fileName, len-2, "%s", &messages[2]);
				snprintf(fileName, count-2, "%s", &messages[2]);
				//Shouchung end
				printk(KERN_INFO "[TP] %s: upgrade from file(%s) start!\n", __func__, fileName);
				// open file
				local_filp = filp_open(fileName, O_RDONLY, 0);
				if(IS_ERR(local_filp)) 
				{
					printk(KERN_ERR "[TP] %s: open firmware file failed\n", __func__);
					goto firmware_upgrade_done;
					//return count;
				}
				oldfs = get_fs();
				set_fs(get_ds());
				
				// read the latest firmware binary file
				result=local_filp->f_op->read(local_filp,upgrade_fw,sizeof(upgrade_fw), &local_filp->f_pos);
				if(result < 0) 
				{
					printk(KERN_ERR "[TP] %s: read firmware file failed\n", __func__);
					goto firmware_upgrade_done;
					//return count;
				}
				
				set_fs(oldfs);
				filp_close(local_filp, NULL);
				
				printk(KERN_INFO "[TP] %s: upgrade start,len %d: %02X, %02X, %02X, %02X\n", __func__, result, upgrade_fw[0], upgrade_fw[1], upgrade_fw[2], upgrade_fw[3]);
				
				if(result > 0)
				{
					// start to upgrade
					disable_irq(private_ts->client->irq);
					if(fts_ctpm_fw_upgrade_with_sys_fs(upgrade_fw, result) == 0)
					{
						printk(KERN_INFO "[TP] %s: TP upgrade error, line: %d\n", __func__, __LINE__);
						fw_update_complete = false;
					}
					else
					{
						printk(KERN_INFO "[TP] %s: TP upgrade OK, line: %d\n", __func__, __LINE__);
						fw_update_complete = true;
					}
					enable_irq(private_ts->client->irq);
					goto firmware_upgrade_done;
					//return count;                    
				}
			}
			
			// #ifdef HX_FW_UPDATE_BY_I_FILE
			// 	if(messages[0] == 'f')
			// 	{
			// 		printk(KERN_INFO "[TP] %s: upgrade firmware from kernel image start!, i_isTP_Updated = %d. \n", __func__, i_isTP_Updated);
			// 		if (i_isTP_Updated == 0)
			// 		{
			// 			printk("himax touch isTP_Updated: %d\n", i_isTP_Updated);
			// 			if(1)
			// 			{
			// 				disable_irq(private_ts->client->irq);
			// 				printk("himax touch firmware upgrade: %d\n", i_isTP_Updated);
			// 				if(fts_ctpm_fw_upgrade_with_i_file() == 0)
			// 				{
			// 					printk("himax_marked TP upgrade error, line: %d\n", __LINE__);
			// 					fw_update_complete = false;
			// 				}
			// 				else
			// 				{
			// 					printk("himax_marked TP upgrade OK, line: %d\n", __LINE__);
			// 					fw_update_complete = true;
			// 				}
			// 				enable_irq(private_ts->client->irq);
			// 				i_isTP_Updated = 1;
			// 				goto firmware_upgrade_done;
			// 			}
			// 		}
			// 	}
			// #endif
			
			firmware_upgrade_done:
			
			//Mutexlock Protect Start
			mutex_unlock(&private_ts->mutex_lock);
			//Mutexlock Protect End
			
			//----[ENABLE_CHIP_RESET_MACHINE]------------------------------------------------------------------start	
				#ifdef ENABLE_CHIP_RESET_MACHINE	
				queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]--------------------------------------------------------------------end
			
			//todo himax_chip->tp_firmware_upgrade_proceed = 0;
			//todo himax_chip->suspend_state = 0;
			//todo enable_irq(himax_chip->irq);
			
			//Wakelock Protect start
			wake_unlock(&private_ts->wake_lock);
			//Wakelock Protect end
			
			//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start	
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifndef ENABLE_ADV_CHIP_STATUS_MONITOR
				queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_monitor, 10*HZ);
				#endif
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end	
			
			//Shouchung modify, initial porting touch in TF103CG
			//return len;
			return count;
			//Shouchung end
		}
		//Shouchung add, initial porting touch in TF103CG
		static DEVICE_ATTR(debug_level, (S_IWUSR|S_IRUGO|S_IWGRP), himax_debug_level_read, himax_debug_level_write);
		//Shouchung end
		#endif
	//----[HX_TP_PROC_DEBUG_LEVEL]-----------------------------------------------------------------------------end

	//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_DIAG || HX_TP_RAW_DATA
		static uint8_t *getMutualBuffer(void)
		{
			return diag_mutual;
		}
		
		static uint8_t *getSelfBuffer(void)
		{
			return &diag_self[0];
		}
		
		static uint8_t getXChannel(void)
		{
			return x_channel;
		}
		
		static uint8_t getYChannel(void)
		{
			return y_channel;
		}
		
		static uint8_t getDiagCommand(void)
		{
			return diag_command;
		}
		
		static void setXChannel(uint8_t x)
		{
			x_channel = x;
		}
		
		static void setYChannel(uint8_t y)
		{
			y_channel = y;
		}
		
		static void setMutualBuffer(void)
		{
			diag_mutual = kzalloc(x_channel * y_channel * sizeof(uint8_t), GFP_KERNEL);
		}
		static void *himax_diag_seq_start(struct seq_file *s, loff_t *pos) 
		{
			if (*pos>=1) return NULL;
			return (void *)((unsigned long) *pos+1);
		}
		static void *himax_raw_data_start(struct seq_file *s, loff_t *pos)
		{
			if (*pos>=1) return NULL;
			return (void *)((unsigned long) *pos+1);
		}
		static void *himax_diag_seq_next(struct seq_file *s, void *v, loff_t *pos) 
		{
			return NULL;
		}
		static void *himax_raw_data_next(struct seq_file *s, void *v, loff_t *pos)
		{
			return NULL;
		}
		static void himax_diag_seq_stop(struct seq_file *s, void *v) 
		{ 
		}
		static void himax_raw_data_stop(struct seq_file *s, void *v)
		{
		}
		static int himax_diag_seq_read(struct seq_file *s, void *v)
		{
			size_t count = 0;
			uint32_t loop_i;
			uint16_t mutual_num, self_num, width;
			
			mutual_num 	= x_channel * y_channel;
			self_num 		= x_channel + y_channel; //don't add KEY_COUNT
			width 			= x_channel;

			seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);
			// start to show out the raw data in adb shell
			if (diag_command >= 1 && diag_command <= 6)
			{
				if (diag_command <= 3)
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_mutual[loop_i]);
						if ((loop_i % width) == (width - 1))
						{
							seq_printf(s, " %3d\n", diag_self[width + loop_i/width]);
						}
					}
					seq_printf(s, "\n");
					for (loop_i = 0; loop_i < width; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[loop_i]);
						if (((loop_i) % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}

					#ifdef HX_EN_BUTTON
					seq_printf(s, "\n");
					for (loop_i = 0; loop_i < HX_BT_NUM; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[HX_RX_NUM + HX_TX_NUM + loop_i]);
					}
					#endif
				}
				else if (diag_command > 4) 
				{
					for (loop_i = 0; loop_i < self_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[loop_i]);
						if (((loop_i - mutual_num) % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}
				}
				else
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_mutual[loop_i]);
						if ((loop_i % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}
				}
				seq_printf(s, "ChannelEnd");
				seq_printf(s, "\n");
			}
			else if (diag_command == 7)
			{
				for (loop_i = 0; loop_i < 128 ;loop_i++)
				{
					if((loop_i % 16) == 0)
					{
						seq_printf(s, "LineStart:");
					}

					seq_printf(s, "%4d", diag_coor[loop_i]);
					if((loop_i % 16) == 15)
					{
						seq_printf(s, "\n");
					}
				}
			}

			return count;
		}

		static ssize_t himax_diag_write_proc(struct file *filp, const char __user *buff, unsigned long len, void *data)
		{
			const uint8_t command_ec_128_raw_flag 		= 0x01;
			const uint8_t command_ec_24_normal_flag 	= 0x00;

			uint8_t command_ec_128_raw_baseline_flag 	= 0x02;
			uint8_t command_ec_128_raw_bank_flag 		= 0x03;

			uint8_t command_91h[2] = {0x91, 0x00};
			uint8_t command_82h[1] = {0x82};
			uint8_t command_F3h[2] = {0xF3, 0x00};
			uint8_t command_83h[1] = {0x83};
			uint8_t command_F1h[2] = {0xF1, 0x00};

			uint8_t receive[1];

			char messages[80] = {0};

			if(diag_dump_to_factory != true)
			{
				if (len >= 80)
				{
					printk(KERN_INFO "%s: no command exceeds 80 chars.\n", __func__);
					return -EFAULT;
				}
				if (copy_from_user(messages, buff, len))
				{
					printk(KERN_INFO "%s: copy_from_user(messages, buff, len).\n", __func__);
					return -EFAULT;
				}
			}

			strcpy(messages, buff);

			if ( (IC_TYPE != HX_85XX_D_SERIES_PWON) && (IC_TYPE != HX_85XX_E_SERIES_PWON))
			{
				command_ec_128_raw_baseline_flag = 0x02 | command_ec_128_raw_flag;
			}
			else
			{
				command_ec_128_raw_baseline_flag = 0x02;
				command_ec_128_raw_bank_flag = 0x03;
			}

			if (messages[0] == '1')	//IIR
			{
				if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					command_F1h[1] = command_ec_128_raw_baseline_flag; //E:0x02
					i2c_himax_write(touch_i2c, command_F1h[0] ,&command_F1h[1], 1, DEFAULT_RETRY_CNT);
				}
				else
				{
					command_91h[1] = command_ec_128_raw_baseline_flag; //A:0x03 , D:0x02
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);
				}
				diag_command = messages[0] - '0';
				printk(KERN_ERR "[Himax]diag_command=0x%x\n",diag_command);
			}
			else if (messages[0] == '2')	//DC
			{
				if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					command_F1h[1] = command_ec_128_raw_flag;	//0x01
					i2c_himax_write(touch_i2c, command_F1h[0] ,&command_F1h[1], 1, DEFAULT_RETRY_CNT);
				}
				else
				{
					command_91h[1] = command_ec_128_raw_flag;	//0x01
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);
				}
				diag_command = messages[0] - '0';
				printk(KERN_ERR "[Himax]diag_command=0x%x\n",diag_command);
			}
			else if (messages[0] == '3')	//BANK
			{
				if ( (IC_TYPE != HX_85XX_D_SERIES_PWON)&& (IC_TYPE != HX_85XX_E_SERIES_PWON))
				{
					i2c_himax_write(touch_i2c, command_82h[0] ,&command_82h[0], 0, DEFAULT_RETRY_CNT);
					msleep(50);

					i2c_himax_read(touch_i2c, command_F3h[0], receive, 1, DEFAULT_RETRY_CNT) ;
					command_F3h[1] = (receive[0] | 0x80);
					i2c_himax_write(touch_i2c, command_F3h[0] ,&command_F3h[1], 1, DEFAULT_RETRY_CNT);

					command_91h[1] = command_ec_128_raw_baseline_flag;
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);

					i2c_himax_write(touch_i2c, command_83h[0] ,&command_83h[0], 0, DEFAULT_RETRY_CNT);
					msleep(50);
				}
				else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
				{
					command_91h[1] = command_ec_128_raw_bank_flag;	//0x03
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);
				}
				else if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					command_F1h[1] = command_ec_128_raw_bank_flag;	//0x03
					i2c_himax_write(touch_i2c, command_F1h[0] ,&command_F1h[1], 1, DEFAULT_RETRY_CNT);
				}
				diag_command = messages[0] - '0';
				printk(KERN_ERR "[Himax]diag_command=0x%x\n",diag_command);
			}
			else if (messages[0] == '7')
			{
				diag_command = messages[0] - '0';
			}
			//coordinate dump start
			else if (messages[0] == '8')
			{
				diag_command = messages[0] - '0';

				coordinate_fn = filp_open(DIAG_COORDINATE_FILE,O_CREAT | O_WRONLY | O_APPEND | O_TRUNC,0666);
				if(IS_ERR(coordinate_fn))
				{
					printk(KERN_INFO "[HIMAX TP ERROR]%s: coordinate_dump_file_create error\n", __func__);
					coordinate_dump_enable = 0;
					filp_close(coordinate_fn,NULL);
				}
				coordinate_dump_enable = 1;
			}
			else if (messages[0] == '9')
			{
				coordinate_dump_enable = 0;
				diag_command = messages[0] - '0';

				if(!IS_ERR(coordinate_fn))
				{
					filp_close(coordinate_fn,NULL);
				}
			}
			//coordinate dump end
			else
			{
				if ( (IC_TYPE != HX_85XX_D_SERIES_PWON) && IC_TYPE != HX_85XX_E_SERIES_PWON )
				{
					i2c_himax_write(touch_i2c, command_82h[0] ,&command_82h[0], 0, DEFAULT_RETRY_CNT);
					msleep(50);
					command_91h[1] = command_ec_24_normal_flag;
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);
					i2c_himax_read(touch_i2c, command_F3h[0], receive, 1, DEFAULT_RETRY_CNT);
					command_F3h[1] = (receive[0] & 0x7F);
					i2c_himax_write(touch_i2c, command_F3h[0] ,&command_F3h[1], 1, DEFAULT_RETRY_CNT);
					i2c_himax_write(touch_i2c, command_83h[0] ,&command_83h[0], 0, DEFAULT_RETRY_CNT);
				}
				else if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					command_F1h[1] = command_ec_24_normal_flag;
					i2c_himax_write(touch_i2c, command_F1h[0] ,&command_F1h[1], 1, DEFAULT_RETRY_CNT);
					TOUCH_UP_COUNTER = 0;
				}
				else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
				{
					command_91h[1] = command_ec_24_normal_flag;
					i2c_himax_write(touch_i2c, command_91h[0] ,&command_91h[1], 1, DEFAULT_RETRY_CNT);
				}

				diag_command = 0;
				printk(KERN_ERR "[Himax]diag_command=0x%x\n",diag_command);
			}
			return len;
		}

		static int himax_raw_data_read(struct seq_file *s, void *v)
		{
			size_t count = 0;
			himax_raw_data_read_func(s, "/data/local");

			return count;
		}

		static ssize_t himax_raw_data_read_func(struct seq_file *s, char *PreFilePath)
		{
			uint32_t loop_i;
			uint16_t mutual_num, self_num, width;
			char filePath[50];
			struct file* filp = NULL;
			mm_segment_t oldfs;

			mutual_num 	= x_channel * y_channel;
			self_num 		= x_channel + y_channel; //don't add KEY_COUNT
			width 			= x_channel;

			strcpy(filePath, PreFilePath);
			if(diag_command == 1)
			{
				strcat(filePath,"/touch_dc.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}
			else if(diag_command == 2)
			{
				strcat(filePath,"/touch_iir.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}
			else if(diag_command == 3)
			{
				strcat(filePath,"/touch_bank.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}

			int flag = 0;

			seq_printf(s, "ChannelStart: %4d, %4d\n\n", x_channel, y_channel);
			// start to show out the raw data in adb shell
			if (diag_command >= 1 && diag_command <= 6)
			{
				if (diag_command <= 3)
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_mutual[loop_i]);
						if ((loop_i % width) == (width - 1))
						{
							seq_printf(s, " %3d\n", diag_self[width + loop_i/width]);
						}
					}
					seq_printf(s, "\n");
					for (loop_i = 0; loop_i < width; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[loop_i]);
						if (((loop_i) % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}

					#ifdef HX_EN_BUTTON
					seq_printf(s, "\n");
					for (loop_i = 0; loop_i < HX_BT_NUM; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[HX_RX_NUM + HX_TX_NUM + loop_i]);
					}
					#endif
				}
				else if (diag_command > 4)
				{
					for (loop_i = 0; loop_i < self_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_self[loop_i]);
						if (((loop_i - mutual_num) % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}
				}
				else
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						seq_printf(s, "%4d", diag_mutual[loop_i]);
						if ((loop_i % width) == (width - 1))
						{
							seq_printf(s, "\n");
						}
					}
				}
				seq_printf(s, "ChannelEnd");
				seq_printf(s, "\n");
			}
			else if (diag_command == 7)
			{
				for (loop_i = 0; loop_i < 128 ;loop_i++)
				{
					if((loop_i % 16) == 0)
					{
						seq_printf(s, "LineStart:");
					}

					seq_printf(s, "%4d", diag_coor[loop_i]);
					if((loop_i % 16) == 15)
					{
						seq_printf(s, "\n");
					}
				}
			}

			if(diag_command >= 1 && diag_command <= 3)
			{
				filp->f_op->write(filp, s->buf, s->count, &filp->f_pos);
				set_fs(oldfs);
				filp_close(filp, NULL);
			}
		}

		//start to store files
		static ssize_t himax_chip_raw_data_store_func(char *buf, char *PreFilePath)
		{
			size_t count = 0;
			uint32_t loop_i, i;
			uint16_t mutual_num, self_num, width;
			char filePath[50];
			struct file* filp = NULL;
			mm_segment_t oldfs;

			mutual_num = x_channel * y_channel;
			self_num = x_channel + y_channel;
			width = x_channel;
			strcpy(filePath, PreFilePath);
			if(diag_command == 1)
			{
				strcat(filePath,"/touch_dc.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}
			else if(diag_command == 2)
			{
				strcat(filePath,"/touch_iir.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}
			else if(diag_command == 3)
			{
				strcat(filePath,"/touch_bank.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}
			count += sprintf(buf + count, "Channel: %4d, %4d\n\n", x_channel, y_channel);
			if (diag_command >= 1 && diag_command <= 6)
			{
				if (diag_command < 4)
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						count += sprintf(buf + count, "%4d", diag_mutual[loop_i]);

						if ((loop_i % width) == (width - 1))
						{
							count += sprintf(buf + count, " %3d\n", diag_self[width + loop_i/width]);
						}
					}
					count += sprintf(buf + count, "\n");
					for (loop_i = 0; loop_i < width; loop_i++)
					{
						count += sprintf(buf + count, "%4d", diag_self[loop_i]);
						if (((loop_i) % width) == (width - 1))
						{
							count += sprintf(buf + count, "\n");
						}
					}

					#ifdef HX_EN_BUTTON
					count += sprintf(buf + count, "\n");
					for (loop_i = 0; loop_i < HX_KEY_COUNT; loop_i++)
					{
						count += sprintf(buf + count, "%4d", diag_self[DEFAULT_X_CHANNEL+DEFAULT_Y_CHANNEL+loop_i]);
					}
					#endif
				}
				else if (diag_command > 4)
				{
					for (loop_i = 0; loop_i < self_num; loop_i++)
					{
						count += sprintf(buf + count, "%4d", diag_self[loop_i]);
						if (((loop_i - mutual_num) % width) == (width - 1))
							count += sprintf(buf + count, "\n");
					}
				}
				else
				{
					for (loop_i = 0; loop_i < mutual_num; loop_i++)
					{
						count += sprintf(buf + count, "%4d", diag_mutual[loop_i]);
						if ((loop_i % width) == (width - 1))
							count += sprintf(buf + count, "\n");
					}
				}
			}
			if(diag_command >= 1 && diag_command <= 3)
			{
				filp->f_op->write(filp, buf, count, &filp->f_pos);
				set_fs(oldfs);
				filp_close(filp, NULL);
			}
			return count;
		}
		//end to store files

		static struct seq_operations himax_diag_seq_ops =
		{
			.start 	= himax_diag_seq_start,
			.next 	= himax_diag_seq_next,
			.stop 	= himax_diag_seq_stop,
			.show 	= himax_diag_seq_read
		};

		static struct seq_operations himax_raw_data_ops =
		{
			.start 	= himax_raw_data_start,
			.next 	= himax_raw_data_next,
			.stop 	= himax_raw_data_stop,
			.show 	= himax_raw_data_read
		};

		static int himax_diag_proc_open(struct inode *inode, struct file *file)
		{
			return seq_open(file, &himax_diag_seq_ops);
		};

		static int himax_raw_proc_open(struct inode *inode, struct file *file)
		{
			return seq_open(file, &himax_raw_data_ops);
		};

		static struct file_operations himax_diag_ops =
		{
			.owner = THIS_MODULE,
			.open = himax_diag_proc_open,
			.read = seq_read,
			.write = himax_diag_write_proc,
		};

		static struct file_operations himax_raw_ops =
		{
			.owner = THIS_MODULE,
			.open = himax_raw_proc_open,
			.read = seq_read,
		};

		#endif
	//----[HX_TP_PROC_DIAG]------------------------------------------------------------------------------------end

	//----[HX_TP_PROC_FLASH_DUMP]----------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_FLASH_DUMP
		
		static uint8_t getFlashCommand(void)
		{
			return flash_command;
		}
		
		static uint8_t getFlashDumpProgress(void)
		{
			return flash_progress;
		}
		
		static uint8_t getFlashDumpComplete(void)
		{
			return flash_dump_complete;
		}
		
		static uint8_t getFlashDumpFail(void)
		{
			return flash_dump_fail;
		}
		
		static uint8_t getSysOperation(void)
		{
			return sys_operation;
		}
		
		static uint8_t getFlashReadStep(void)
		{
			return flash_read_step;
		}
		
		static uint8_t getFlashDumpSector(void)
		{
			return flash_dump_sector;
		}
		
		static uint8_t getFlashDumpPage(void)
		{
			return flash_dump_page;
		}

		static bool getFlashDumpGoing(void)
		{
			return flash_dump_going;
		}
		
		static void setFlashBuffer(void)
		{
			int i=0;
			flash_buffer = kzalloc(32768*sizeof(uint8_t), GFP_KERNEL);
			for(i=0; i<32768; i++)
			{
				flash_buffer[i] = 0x00;
			}
		}
		
		static void setSysOperation(uint8_t operation)
		{
			sys_operation = operation;
		}
		
		static void setFlashDumpProgress(uint8_t progress)
		{
			flash_progress = progress;
			printk("TPPPP setFlashDumpProgress : progress = %d ,flash_progress = %d \n",progress,flash_progress);
		}
		
		static void setFlashDumpComplete(uint8_t status)
		{
			flash_dump_complete = status;
		}
		
		static void setFlashDumpFail(uint8_t fail)
		{
			flash_dump_fail = fail;
		}
		
		static void setFlashCommand(uint8_t command)
		{
			flash_command = command;
		}
		
		static void setFlashReadStep(uint8_t step)
		{
			flash_read_step = step;
		}
		
		static void setFlashDumpSector(uint8_t sector)
		{
			flash_dump_sector = sector;
		}
		
		static void setFlashDumpPage(uint8_t page)
		{
			flash_dump_page = page;
		}		

		static void setFlashDumpGoing(bool going)
		{
			flash_dump_going = going;
		}
		
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_flash_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
		static ssize_t himax_flash_read(struct device *dev, struct device_attribute *attr, char *buf)
		//Shouchung end
		{
			int ret = 0;
			int loop_i;
			uint8_t local_flash_read_step=0;
			uint8_t local_flash_complete = 0;
			uint8_t local_flash_progress = 0;
			uint8_t local_flash_command = 0;
			uint8_t local_flash_fail = 0;
			
			local_flash_complete = getFlashDumpComplete();
			local_flash_progress = getFlashDumpProgress();
			local_flash_command = getFlashCommand();
			local_flash_fail = getFlashDumpFail();
			
			printk("TPPPP flash_progress = %d \n",local_flash_progress);
			
			if(local_flash_fail)
			{
				ret += sprintf(buf+ret, "FlashStart:Fail \n");
				ret += sprintf(buf + ret, "FlashEnd");
				ret += sprintf(buf + ret, "\n");
				return ret;
			}
			
			if(!local_flash_complete)
			{
				ret += sprintf(buf+ret, "FlashStart:Ongoing:0x%2.2x \n",flash_progress);
				ret += sprintf(buf + ret, "FlashEnd");
				ret += sprintf(buf + ret, "\n");
				return ret;
			}
			
			if(local_flash_command == 1 && local_flash_complete)
			{
				ret += sprintf(buf+ret, "FlashStart:Complete \n");
				ret += sprintf(buf + ret, "FlashEnd");
				ret += sprintf(buf + ret, "\n");
				return ret;
			}
			
			if(local_flash_command == 3 && local_flash_complete)
			{
				ret += sprintf(buf+ret, "FlashStart: \n");
				for(loop_i = 0; loop_i < 128; loop_i++)
				{
					ret += sprintf(buf + ret, "x%2.2x", flash_buffer[loop_i]);
					if((loop_i % 16) == 15)
					{
						ret += sprintf(buf + ret, "\n");
					}
				}
				ret += sprintf(buf + ret, "FlashEnd");
				ret += sprintf(buf + ret, "\n");
				return ret;
			}
			
			//flash command == 0 , report the data
			local_flash_read_step = getFlashReadStep();
			
			ret += sprintf(buf+ret, "FlashStart:%2.2x \n",local_flash_read_step);
			
			for (loop_i = 0; loop_i < 1024; loop_i++) 
			{
				ret += sprintf(buf + ret, "x%2.2X", flash_buffer[local_flash_read_step*1024 + loop_i]);
				
				if ((loop_i % 16) == 15)
				{
					ret += sprintf(buf + ret, "\n");
				}
			}
			
			ret += sprintf(buf + ret, "FlashEnd");
			ret += sprintf(buf + ret, "\n");
			return ret;
		}

		// for ATD tools start
		static ssize_t himax_get_touch_status(struct device *dev, struct device_attribute *attr, char *buf)
		{
			uint8_t buf0[3];
			int ret = -1;
			int test_count = 5;
			int i;

			buf0[0] = 0xE9;
			buf0[1] = 0x01;
			buf0[2] = 0x01;

			for (i = 0; i < test_count; i++)
			{
				ret = i2c_himax_master_write(private_ts->client, buf0, 3, DEFAULT_RETRY_CNT);
				if(ret < 0)
				{
					break;
				}
				msleep(50);
			}

			return sprintf(buf, "%d\n", (ret < 0) ? 0 : 1);
		}
		static DEVICE_ATTR(touch_status, (S_IRUGO), himax_get_touch_status, NULL);
		// for ATD tools end

		// for ATD update touch FW start
		// static ssize_t himax_update_touch_FW(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
		// {
		// 	if(buf[0] == 'b')
		// 	{
		// 		himax_debug_level_write(dev, attr, "t /data/20140213_ASUS_TF103C_E0101_00_00.bin", 45);
		// 	}
		// }
		// static DEVICE_ATTR(tp_fw_upgrade, (S_IWUSR|S_IRUGO|S_IWGRP), NULL, himax_update_touch_FW);
		// for ATD update touch FW end

		//-----------------------------------------------------------------------------------
		//himax_flash_store
		//
		//command 0 : Read the page by step number
		//command 1 : driver start to dump flash data, save it to mem
		//command 2 : driver start to dump flash data, save it to sdcard/Flash_Dump.bin
		//
		//-----------------------------------------------------------------------------------
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_flash_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
		static ssize_t himax_flash_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
		//Shouchung end
		{
			char buf_tmp[6];
			unsigned long result = 0;
			uint8_t loop_i = 0;
			int base = 0;
			char messages[80] = {0};
			
			memset(buf_tmp, 0x0, sizeof(buf_tmp));
			
			//Shouchung modify, initial porting touch in TF103CG
			//if (len >= 80)
			if (count >= 80)
			//Shouchung end
			{
				printk(KERN_INFO "%s: no command exceeds 80 chars.\n", __func__);
				return -EFAULT;
			}
			//Shouchung modify, initial porting touch in TF103CG
			/*if (copy_from_user(messages, buff, len))
			{
				return -EFAULT;
			}*/
			strcpy(messages, buf);
			//Shouchung end
			printk(KERN_INFO "[TP] %s: buf[0] = %s\n", __func__, messages);
			
			if(getSysOperation() == 1)
			{
				printk("[TP] %s: SYS is busy , return!\n", __func__);
				//Shouchung modify, initial porting touch in TF103CG
				//return len;
				return count;
				//Shouchung end
			}
			
			if(messages[0] == '0')
			{
				setFlashCommand(0);
				if(messages[1] == ':' && messages[2] == 'x')
				{
					memcpy(buf_tmp, messages + 3, 2);
					printk(KERN_INFO "[TP] %s: read_Step = %s\n", __func__, buf_tmp);
					if (!strict_strtoul(buf_tmp, 16, &result))
					{
						printk("[TP] %s: read_Step = %lu \n", __func__, result);
						setFlashReadStep(result);
					}
				}
			}
			else if(messages[0] == '1')
			{
				setSysOperation(1);
				setFlashCommand(1);
				setFlashDumpProgress(0);
				setFlashDumpComplete(0);
				setFlashDumpFail(0);
				queue_work(private_ts->flash_wq, &private_ts->flash_work);
			}
			else if(messages[0] == '2')
			{
				setSysOperation(1);
				setFlashCommand(2);
				setFlashDumpProgress(0);
				setFlashDumpComplete(0);
				setFlashDumpFail(0);
				
				queue_work(private_ts->flash_wq, &private_ts->flash_work);
			}
			else if(messages[0] == '3')
			{
				setSysOperation(1);
				setFlashCommand(3);
				setFlashDumpProgress(0);
				setFlashDumpComplete(0);
				setFlashDumpFail(0);
				
				memcpy(buf_tmp, messages + 3, 2);
				if (!strict_strtoul(buf_tmp, 16, &result))
				{
					setFlashDumpSector(result);
				}
				
				memcpy(buf_tmp, messages + 7, 2);
				if (!strict_strtoul(buf_tmp, 16, &result))
				{
					setFlashDumpPage(result);
				}
				
				queue_work(private_ts->flash_wq, &private_ts->flash_work);
			}
			else if(messages[0] == '4')
			{
				printk(KERN_INFO "[TP] %s: command 4 enter.\n", __func__);
				setSysOperation(1);
				setFlashCommand(4);
				setFlashDumpProgress(0);
				setFlashDumpComplete(0);
				setFlashDumpFail(0);
				
				memcpy(buf_tmp, messages + 3, 2);
				if (!strict_strtoul(buf_tmp, 16, &result))
				{
					setFlashDumpSector(result);
				}
				else
				{
					printk(KERN_INFO "[TP] %s: command 4 , sector error.\n", __func__);
					//Shouchung modify, initial porting touch in TF103CG
					//return len;
					return count;
					//Shouchung end
				}
				
				memcpy(buf_tmp, messages + 7, 2);
				if (!strict_strtoul(buf_tmp, 16, &result))
				{
					setFlashDumpPage(result);
				}
				else
				{
					printk(KERN_INFO "[TP] %s: command 4 , page error.\n", __func__);
					//Shouchung modify, initial porting touch in TF103CG
					//return len;
					return count;
					//Shouchung end
				}
				
				base = 11;
				
				printk(KERN_INFO "=========Himax flash page buffer start=========\n");
				for(loop_i=0;loop_i<128;loop_i++)
				{
					memcpy(buf_tmp, messages + base, 2);
					if (!strict_strtoul(buf_tmp, 16, &result))
					{
						flash_buffer[loop_i] = result;
						printk(" %d ",flash_buffer[loop_i]);
						if(loop_i % 16 == 15)
						{
							printk("\n");
						}
					}
					base += 3;
				}
				printk(KERN_INFO "=========Himax flash page buffer end=========\n");
				
				queue_work(private_ts->flash_wq, &private_ts->flash_work);		
			}
			//Shouchung modify, initial porting touch in TF103CG
			//return len;
			return count;
			//Shouchung end
		}
			
		static void himax_ts_flash_work_func(struct work_struct *work)
		{
			struct himax_ts_data *ts = container_of(work, struct himax_ts_data, flash_work);
			
			uint8_t page_tmp[128];
			uint8_t x59_tmp[4] = {0,0,0,0};
			int i=0, j=0, k=0, l=0,/* j_limit = 0,*/ buffer_ptr = 0, flash_end_count = 0;
			uint8_t local_flash_command = 0;
			uint8_t sector = 0;
			uint8_t page = 0;
			
			uint8_t x81_command[2] = {0x81,0x00};
			uint8_t x82_command[2] = {0x82,0x00};
			uint8_t x35_command[2] = {0x35,0x00};
			uint8_t x42_command[2] = {0x42,0x00};
			uint8_t x43_command[4] = {0x43,0x00,0x00,0x00};
			uint8_t x44_command[4] = {0x44,0x00,0x00,0x00};
			uint8_t x45_command[5] = {0x45,0x00,0x00,0x00,0x00};
			uint8_t x46_command[2] = {0x46,0x00};
			uint8_t x4A_command[2] = {0x4A,0x00};
			uint8_t x4D_command[2] = {0x4D,0x00};
			/*uint8_t x59_command[2] = {0x59,0x00};*/
		
			disable_irq(ts->client->irq);
			setFlashDumpGoing(true);	

			#ifdef HX_RST_PIN_FUNC
				himax_HW_reset();
			#endif

			sector = getFlashDumpSector();
			page = getFlashDumpPage();
			
			local_flash_command = getFlashCommand();
			
			if( i2c_himax_master_write(ts->client, x81_command, 1, 3) < 0 )//sleep out
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 81 fail.\n",__func__);
				goto Flash_Dump_i2c_transfer_error;
			}
			msleep(120);

			if( i2c_himax_master_write(ts->client, x82_command, 1, 3) < 0 )
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 82 fail.\n",__func__);
				goto Flash_Dump_i2c_transfer_error;
			}
			msleep(100);

			printk(KERN_INFO "[TP] %s: local_flash_command = %d enter.\n", __func__,local_flash_command);
			printk(KERN_INFO "[TP] %s: flash buffer start.\n", __func__);
			for(i=0;i<128;i++)
			{
				printk(KERN_INFO " %2.2x ",flash_buffer[i]);
				if((i%16) == 15)
				{
					printk("\n");
				}
			}
			printk(KERN_INFO "[TP] %s: flash buffer end.\n", __func__);

			if(local_flash_command == 1 || local_flash_command == 2)
			{
				x43_command[1] = 0x01;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 1, DEFAULT_RETRY_CNT) < 0)
				{
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(100);
				
				for( i=0 ; i<8 ;i++)
				{
					for(j=0 ; j<32 ; j++)
					{
						printk("TPPPP Step 2 i=%d , j=%d %s\n",i,j,__func__);
						//read page start
						for(k=0; k<128; k++)
						{
							page_tmp[k] = 0x00;
						}
						for(k=0; k<32; k++)
						{
							x44_command[1] = k;
							x44_command[2] = j;
							x44_command[3] = i;
							if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
							{
								printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
								goto Flash_Dump_i2c_transfer_error;
							}
				
							if( i2c_himax_write_command(ts->client, x46_command[0], DEFAULT_RETRY_CNT) < 0)
							{
								printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 46 fail.\n",__func__);
								goto Flash_Dump_i2c_transfer_error;
							}
							//msleep(2);
							if( i2c_himax_read(ts->client, 0x59, x59_tmp, 4, DEFAULT_RETRY_CNT) < 0)
							{
								printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 59 fail.\n",__func__);
								goto Flash_Dump_i2c_transfer_error;
							}
							//msleep(2);
							for(l=0; l<4; l++)
							{
								page_tmp[k*4+l] = x59_tmp[l];
							}
							//msleep(10);
						}
						//read page end 
				
						for(k=0; k<128; k++)
						{
							flash_buffer[buffer_ptr++] = page_tmp[k];
							
							if(page_tmp[k] == 0xFF)
							{
								flash_end_count ++;
								if(flash_end_count == 32)
								{
									flash_end_count = 0;
									buffer_ptr = buffer_ptr -32;
									goto FLASH_END;
								}
							}
							else
							{
								flash_end_count = 0;
							}
						}
						setFlashDumpProgress(i*32 + j);
					}
				}
			}
			else if(local_flash_command == 3)
			{
				x43_command[1] = 0x01;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(100);
				
				for(i=0; i<128; i++)
				{
					page_tmp[i] = 0x00;
				}
				
				for(i=0; i<32; i++)
				{
					x44_command[1] = i;
					x44_command[2] = page;
					x44_command[3] = sector;
					
					if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					
					if( i2c_himax_write_command(ts->client, x46_command[0], DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 46 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					//msleep(2);
					if( i2c_himax_read(ts->client, 0x59, x59_tmp, 4, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 59 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					//msleep(2);
					for(j=0; j<4; j++)
					{
						page_tmp[i*4+j] = x59_tmp[j];
					}
					//msleep(10);
				}
				//read page end
				for(i=0; i<128; i++)
				{
					flash_buffer[buffer_ptr++] = page_tmp[i];
				}
			}
			else if(local_flash_command == 4)
			{
				//page write flow.
				printk(KERN_INFO "[TP] %s: local_flash_command = 4, enter.\n", __func__);

				//-----------------------------------------------------------------------------------------------
				// unlock flash
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x00; 
				x43_command[3] = 0x06;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x44_command[1] = 0x03; 
				x44_command[2] = 0x00; 
				x44_command[3] = 0x00;
				if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x45_command[1] = 0x00; 
				x45_command[2] = 0x00; 
				x45_command[3] = 0x3D; 
				x45_command[4] = 0x03;
				if( i2c_himax_write(ts->client, x45_command[0],&x45_command[1], 4, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 45 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				if( i2c_himax_write_command(ts->client, x4A_command[0], DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 4A fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(50);

				//-----------------------------------------------------------------------------------------------
				// page erase
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x00; 
				x43_command[3] = 0x02;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				x44_command[1] = 0x00; 
				x44_command[2] = page; 
				x44_command[3] = sector;
				if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				if( i2c_himax_write_command(ts->client, x4D_command[0], DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 4D fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(100);
				
				//-----------------------------------------------------------------------------------------------
				// enter manual mode
				//-----------------------------------------------------------------------------------------------
				if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					x35_command[1] = 0x01;
					if( i2c_himax_write(ts->client, x35_command[0],&x35_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 35 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
				}
				else
				{
					x42_command[1] = 0x01;
					if( i2c_himax_write(ts->client, x42_command[0],&x42_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 42 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
				}
				msleep(100);
				
				//-----------------------------------------------------------------------------------------------
				// flash enable
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x00;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				//-----------------------------------------------------------------------------------------------
				// set flash address
				//-----------------------------------------------------------------------------------------------
				x44_command[1] = 0x00; 
				x44_command[2] = page; 
				x44_command[3] = sector;
				if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				//-----------------------------------------------------------------------------------------------
				// manual mode command : 47 to latch the flash address when page address change.
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x09; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				x43_command[1] = 0x01; 
				x43_command[2] = 0x0D; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x43_command[1] = 0x01; 
				x43_command[2] = 0x09; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				for(i=0; i<32; i++)
				{
					printk(KERN_INFO "himax :i=%d \n",i);
					x44_command[1] = i; 
					x44_command[2] = page; 
					x44_command[3] = sector;
					if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					msleep(10);
					
					x45_command[1] = flash_buffer[i*4 + 0];
					x45_command[2] = flash_buffer[i*4 + 1];
					x45_command[3] = flash_buffer[i*4 + 2];
					x45_command[4] = flash_buffer[i*4 + 3];
					if( i2c_himax_write(ts->client, x45_command[0],&x45_command[1], 4, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 45 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					msleep(10);
					
					//-----------------------------------------------------------------------------------------------
					// manual mode command : 48 ,data will be written into flash buffer
					//-----------------------------------------------------------------------------------------------
					x43_command[1] = 0x01; 
					x43_command[2] = 0x0D; 
					if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					msleep(10);
					
					x43_command[1] = 0x01; 
					x43_command[2] = 0x09; 
					if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					msleep(10);
				}

				//-----------------------------------------------------------------------------------------------
				// manual mode command : 49 ,program data from flash buffer to this page
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x01;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x43_command[1] = 0x01; 
				x43_command[2] = 0x05; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x43_command[1] = 0x01; 
				x43_command[2] = 0x01; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x43_command[1] = 0x01; 
				x43_command[2] = 0x00; 
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 2, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				//-----------------------------------------------------------------------------------------------
				// flash disable
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x00;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);
				
				//-----------------------------------------------------------------------------------------------
				// leave manual mode
				//-----------------------------------------------------------------------------------------------
				if(IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					x35_command[1] = 0x01;
					if( i2c_himax_write(ts->client, x35_command[0],&x35_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 35 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
				}
				else
				{
					x42_command[1] = 0x00;
					if( i2c_himax_write(ts->client, x42_command[0],&x42_command[1], 1, DEFAULT_RETRY_CNT) < 0 )
					{
						printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
						goto Flash_Dump_i2c_transfer_error;
					}
					msleep(10);
				}
				msleep(10);
				
				//-----------------------------------------------------------------------------------------------
				// lock flash
				//-----------------------------------------------------------------------------------------------
				x43_command[1] = 0x01; 
				x43_command[2] = 0x00; 
				x43_command[3] = 0x06;
				if( i2c_himax_write(ts->client, x43_command[0],&x43_command[1], 3, DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 43 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				msleep(10);

				x44_command[1] = 0x03; 
				x44_command[2] = 0x00; 
				x44_command[3] = 0x00;
				if( i2c_himax_write(ts->client, x44_command[0],&x44_command[1], 3, DEFAULT_RETRY_CNT) < 0 ) 
				{   
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 44 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}   
				msleep(10);				
				
				x45_command[1] = 0x00; 
				x45_command[2] = 0x00; 
				x45_command[3] = 0x7D; 
				x45_command[4] = 0x03;
				if( i2c_himax_write(ts->client, x45_command[0],&x45_command[1], 4, DEFAULT_RETRY_CNT) < 0 ) 
				{   
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 45 fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}   
				msleep(10);

				if( i2c_himax_write_command(ts->client, x4A_command[0], DEFAULT_RETRY_CNT) < 0 )
				{
					printk(KERN_ERR "[TP]TOUCH_ERR: %s i2c write 4D fail.\n",__func__);
					goto Flash_Dump_i2c_transfer_error;
				}
				
				msleep(50);

				buffer_ptr = 128;
				printk(KERN_INFO "Himax: Flash page write Complete~~~~~~~~~~~~~~~~~~~~~~~\n");
			}

			FLASH_END:

			printk("Complete~~~~~~~~~~~~~~~~~~~~~~~\n");

			printk(" buffer_ptr = %d \n",buffer_ptr);
			
			for (i = 0; i < buffer_ptr; i++) 
			{
				printk("%2.2X ", flash_buffer[i]);
			
				if ((i % 16) == 15)
				{
					printk("\n");
				}
			}
			printk("End~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

			i2c_himax_master_write(ts->client, x43_command, 1, 3);
			msleep(50);

			if(local_flash_command == 2)
			{
				struct file *fn;
				
				fn = filp_open(FLASH_DUMP_FILE,O_CREAT | O_WRONLY ,0);
				if(!IS_ERR(fn))
				{
					fn->f_op->write(fn,flash_buffer,buffer_ptr*sizeof(uint8_t),&fn->f_pos);
					filp_close(fn,NULL);
				}
			}

			#ifdef ENABLE_CHIP_RESET_MACHINE
				if(private_ts->init_success)
				{
					queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
				}
			#endif

			enable_irq(ts->client->irq);
			setFlashDumpGoing(false);
		
			setFlashDumpComplete(1);
			setSysOperation(0);
			return;

			Flash_Dump_i2c_transfer_error:
				
			#ifdef ENABLE_CHIP_RESET_MACHINE
				if(private_ts->init_success)
				{
					queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
				}
			#endif

			enable_irq(ts->client->irq);
			setFlashDumpGoing(false);
			setFlashDumpComplete(0);
			setFlashDumpFail(1);
			setSysOperation(0);
			return;
		}
		//Shouchung add, initial porting touch in TF103CG
		static DEVICE_ATTR(flash_dump, (S_IWUSR|S_IRUGO|S_IWGRP), himax_flash_read, himax_flash_write);
		//Shouchung end
		#endif
	//----[HX_TP_PROC_FLASH_DUMP]------------------------------------------------------------------------------end

	static ssize_t himax_get_touch_panel_id(struct device *dev, struct device_attribute *attr, char *buf)
	{
		return sprintf(buf, "%d\n", TP_ID);
	}
	static DEVICE_ATTR(tp_id, (S_IRUGO),himax_get_touch_panel_id, NULL);

	//----[HX_TP_PROC_SELF_TEST]-----------------------------------------------------------------------------start
		#ifdef HX_TP_PROC_SELF_TEST
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_self_test_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
		static ssize_t himax_self_test_read(struct device *dev, struct device_attribute *attr, char *buf)
		//Shouchung end
		{

			char filePath[50];
			struct file* filp = NULL;
			mm_segment_t oldfs;

			int val=0x00;
			val = himax_chip_self_test();

			if(diag_dump_to_factory == true)
			{
				char filePath[50] = "/factory/";
				strcat(filePath,"/Self_test_result.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());

				//touch dc
				himax_diag_write_proc(NULL, TOUCH_DC, 2, NULL);
				msleep(2000);
				himax_chip_raw_data_store_func(buf, "/factory/");

				//touch iir
				/*himax_diag_write_func(TOUCH_IIR);
				msleep(2000);
				himax_chip_raw_data_store_func(buf, "/factory/");*/

				//touch bank
				himax_diag_write_proc(NULL, TOUCH_BANK, 2, NULL);
				msleep(2000);
				himax_chip_raw_data_store_func(buf, "/factory/");

				diag_dump_to_factory = false;
			}else
			{
				char filePath[50] = "/data/local";
				strcat(filePath,"/Self_test_result.txt");
				filp = filp_open(filePath, O_RDWR|O_CREAT,S_IRUSR);
				if(IS_ERR(filp))
				{
					printk(KERN_ERR "[Himax] %s: open %s failed\n", __func__, filePath);
					return 0;
				}
				oldfs = get_fs();
				set_fs(get_ds());
			}

			if(val == 0)
			{
				filp->f_op->write(filp, "Pass\n", 5, &filp->f_pos);
				set_fs(oldfs);
				filp_close(filp, NULL);

				return sprintf(buf, "Pass\n");
			}
			else
			{
				filp->f_op->write(filp, "Fail\n", 5, &filp->f_pos);
				set_fs(oldfs);
				filp_close(filp, NULL);

				return sprintf(buf, "Fail\n");
			}
		}
		
		static int himax_chip_self_test(void)
		{
			uint8_t cmdbuf[11];
			int ret = 0;
			uint8_t valuebuf[16];
			int i=0, pf_value=0x00;
			
			//----[HX_RST_PIN_FUNC]-----------------------------------------------------------------------------start
				#ifdef HX_RST_PIN_FUNC
				himax_HW_reset();
				#endif
			//----[HX_RST_PIN_FUNC]-------------------------------------------------------------------------------end
			
			himax_ts_poweron(private_ts);
			
			if (IC_TYPE == HX_85XX_C_SERIES_PWON)
			{		
				//sense off to write self-test parameters
				cmdbuf[0] = HX_CMD_TSSOFF;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 1, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write TSSOFF failed line: %d \n",__LINE__);
				} 
				mdelay(120); //120ms
				
				cmdbuf[0] = HX_CMD_SELFTEST_BUFFER;
				cmdbuf[1] = 0xB4; //180
				cmdbuf[2] = 0x64; //100
				cmdbuf[3] = 0x36; 
				cmdbuf[4] = 0x09; 
				cmdbuf[5] = 0x2D; 
				cmdbuf[6] = 0x09; 
				cmdbuf[7] = 0x32; 
				cmdbuf[8] = 0x08;//0x19
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 9, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write HX_CMD_SELFTEST_BUFFER failed line: %d \n",__LINE__);
				}
				
				udelay(100);
				
				ret = i2c_himax_read(private_ts->client, HX_CMD_SELFTEST_BUFFER, valuebuf, 9, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:read HX_CMD_SELFTEST_BUFFER failed line: %d \n",__LINE__);
				}
				printk("[Himax]:0x8D[0] = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0x8D[1] = 0x%x\n",valuebuf[1]);
				printk("[Himax]:0x8D[2] = 0x%x\n",valuebuf[2]);
				printk("[Himax]:0x8D[3] = 0x%x\n",valuebuf[3]);
				printk("[Himax]:0x8D[4] = 0x%x\n",valuebuf[4]);
				printk("[Himax]:0x8D[5] = 0x%x\n",valuebuf[5]);
				printk("[Himax]:0x8D[6] = 0x%x\n",valuebuf[6]);
				printk("[Himax]:0x8D[7] = 0x%x\n",valuebuf[7]);
				printk("[Himax]:0x8D[8] = 0x%x\n",valuebuf[8]);
				
				cmdbuf[0] = 0xE9;
				cmdbuf[1] = 0x00;
				cmdbuf[2] = 0x06;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 3, DEFAULT_RETRY_CNT);//write sernsor to self-test mode
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write sernsor to self-test mode failed line: %d \n",__LINE__);
				} 
				udelay(100);
				
				ret = i2c_himax_read(private_ts->client, 0xE9, valuebuf, 3, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:read 0xE9 failed line: %d \n",__LINE__);
				}
				printk("[Himax]:0xE9[0] = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0xE9[1] = 0x%x\n",valuebuf[1]);
				printk("[Himax]:0xE9[2] = 0x%x\n",valuebuf[2]);
				
				cmdbuf[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write HX_CMD_TSSON failed line: %d \n",__LINE__);
				} 
				mdelay(1500); //1500ms
				
				cmdbuf[0] = HX_CMD_TSSOFF;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 1, DEFAULT_RETRY_CNT);
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write TSSOFF failed line: %d \n",__LINE__);
				} 
				mdelay(120); //120ms
				
				memset(valuebuf, 0x00 , 16);
				ret = i2c_himax_read(private_ts->client, HX_CMD_SELFTEST_BUFFER, valuebuf, 16, DEFAULT_RETRY_CNT);
			
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:read HX_CMD_FW_VERSION_ID failed line: %d \n",__LINE__);
				}
				else
				{
					if(valuebuf[0]==0xAA)
					{
						printk("[Himax]: self-test pass\n");
						pf_value = 0x0;
					}
					else
					{
						printk("[Himax]: self-test fail\n");
						pf_value = 0x1;
						for(i=0;i<16;i++)
						{
							printk("[Himax]:0x8D buff[%d] = 0x%x\n",i,valuebuf[i]);
						}
					}
				}
				mdelay(120); //120ms
				
				cmdbuf[0] = 0xE9;
				cmdbuf[1] = 0x00;
				cmdbuf[2] = 0x00;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 3, DEFAULT_RETRY_CNT);//write sensor to normal mode
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write sensor to normal mode failed line: %d \n",__LINE__);
				} 
				mdelay(120); //120ms
				
				cmdbuf[0] = HX_CMD_TSSON;
				ret = i2c_himax_master_write(private_ts->client, cmdbuf, 1, DEFAULT_RETRY_CNT);//sense on
				if(ret < 0) 
				{
					printk(KERN_ERR "[Himax]:write HX_CMD_TSSON failed line: %d \n",__LINE__);
				}
				msleep(120); //120ms
			}
			else if(IC_TYPE == HX_85XX_D_SERIES_PWON)
			{
				//Step 0 : sensor off
				i2c_himax_write(private_ts->client, 0x82,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);	
				msleep(120);
				
				//Step 1 : Close Re-Calibration FE02
				//-->Read 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);		
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02 
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				msleep(30);
				
				printk("[Himax]:0xFE02_0 = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0xFE02_1 = 0x%x\n",valuebuf[1]);
				
				valuebuf[0] = valuebuf[1] & 0xFD; // close re-calibration  , shift first byte of config bank register read issue.
				
				printk("[Himax]:0xFE02_valuebuf = 0x%x\n",valuebuf[0]);
				
				//-->Write 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02 
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = valuebuf[0];
				i2c_himax_write(private_ts->client, 0x40,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				msleep(30);
				//0xFE02 Read Back
				
				//-->Read 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);		
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02 
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(30);
				
				printk("[Himax]:0xFE02_0_back = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0xFE02_1_back = 0x%x\n",valuebuf[1]);
				
				//Step 2 : Close Flash-Reload
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE3,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				msleep(30);
				
				i2c_himax_read(private_ts->client, 0xE3, valuebuf, 1, DEFAULT_RETRY_CNT);
				
				printk("[Himax]:0xE3_back = 0x%x\n",valuebuf[0]);
				
				//Step 4 : Write self_test parameter to FE96~FE9D
				//-->Write FE96~FE9D
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96 
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				

				//Edit for ATD self-test setting function start
				//-->Modify the initial value of self_test.
				cmdbuf[0] = 0xB4;
				cmdbuf[1] = 0x64;
				cmdbuf[2] = 0x3F;
				cmdbuf[3] = 0x3F;
				cmdbuf[4] = 0x3C;
				cmdbuf[5] = 0x00;
				cmdbuf[6] = 0x3C;
				cmdbuf[7] = 0x00;

				// cmdbuf[0] = rFE96_setting[0];
				// cmdbuf[1] = rFE96_setting[1];
				// cmdbuf[2] = rFE96_setting[2];
				// cmdbuf[3] = rFE96_setting[3];
				// cmdbuf[4] = rFE96_setting[4];
				// cmdbuf[5] = rFE96_setting[5];
				// cmdbuf[6] = rFE96_setting[6];
				// cmdbuf[7] = rFE96_setting[7];
				//Edit for ATD self-test setting function end

				i2c_himax_write(private_ts->client, 0x40,&cmdbuf[0], 8, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				msleep(30);
				
				//Read back
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);		
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 16, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				for(i=1;i<16;i++)
				{
					printk("[Himax]:0xFE96 buff_back[%d] = 0x%x\n",i,valuebuf[i]);
				}
				
				msleep(30);
				
				//Step 5 : Enter self_test mode
				cmdbuf[0] = 0x16;
				i2c_himax_write(private_ts->client, 0x91,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				i2c_himax_read(private_ts->client, 0x91, valuebuf, 1, DEFAULT_RETRY_CNT);
				
				printk("[Himax]:0x91_back = 0x%x\n",valuebuf[0]);
				msleep(10);
				
				//Step 6 : Sensor On
				i2c_himax_write(private_ts->client, 0x83,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
				
				//Edit for ATD self-test setting function start
				mdelay(self_test_delay_time * 1000);
				//Edit for ATD self-test setting function end
				
				//Step 7 : Sensor Off
				i2c_himax_write(private_ts->client, 0x82,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
				
				msleep(30);
				
				//Step 8 : Get self_test result
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);		
				msleep(10);
				
				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96 
				i2c_himax_write(private_ts->client, 0xD8,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);
				
				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 16, DEFAULT_RETRY_CNT);
				msleep(10);
				
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				//Final : Leave self_test mode
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x91,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				
				if(valuebuf[1]==0xAA) //get the self_test result , shift first byte for config bank read issue.
				{
					printk("[Himax]: self-test pass\n");
					pf_value = 0x0;
				}
				else
				{
					printk("[Himax]: self-test fail\n");
					pf_value = 0x1;
					for(i=1;i<16;i++)
					{
						printk("[Himax]:0xFE96 buff[%d] = 0x%x\n",i,valuebuf[i]);
					}
				}

				//HW reset and power on again.
				//----[HX_RST_PIN_FUNC]-----------------------------------------------------------------------------start
					#ifdef HX_RST_PIN_FUNC
					himax_HW_reset();
					#endif
				//----[HX_RST_PIN_FUNC]-------------------------------------------------------------------------------end
				
				himax_ts_poweron(private_ts);

			}
			else if(IC_TYPE == HX_85XX_E_SERIES_PWON)
			{
				//Step 0 : sensor off
				i2c_himax_write(private_ts->client, 0x82,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
				msleep(120);

				i2c_himax_write(private_ts->client, 0x80,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
				msleep(120);

				cmdbuf[0] = 0xA5;
				i2c_himax_write(private_ts->client, 0xCA,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(100);

				cmdbuf[0] = 0xCA;
				i2c_himax_write(private_ts->client, 0xCE,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(100);

				//Step 1 : Close Re-Calibration FE02
				//-->Read 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 2, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				msleep(30);

				printk("[Himax]:0xFE02_0 = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0xFE02_1 = 0x%x\n",valuebuf[1]);

				//valuebuf[0] = valuebuf[1] & 0xFD; // close re-calibration  , shift first byte of config bank register read issue.
				valuebuf[0] = valuebuf[0] & 0xFD; // close re-calibration  , shift first byte of config bank register read issue.

				printk("[Himax]:0xFE02_valuebuf = 0x%x\n",valuebuf[0]);

				//-->Write 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = valuebuf[0];
				i2c_himax_write(private_ts->client, 0x40,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				msleep(30);
				//0xFE02 Read Back

				//-->Read 0xFE02
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x02; //FE02
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 2, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(30);

				printk("[Himax]:0xFE02_0_back = 0x%x\n",valuebuf[0]);
				printk("[Himax]:0xFE02_1_back = 0x%x\n",valuebuf[1]);

				//Step 2 : Close Flash-Reload
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xE3,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				msleep(30);

				i2c_himax_read(private_ts->client, 0xE3, valuebuf, 1, DEFAULT_RETRY_CNT);

				printk("[Himax]:0xE3_back = 0x%x\n",valuebuf[0]);

				//Step 4 : Write self_test parameter to FE96~FE9D
				//-->Write FE96~FE9D
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				//Edit for ATD self-test setting function start
				//-->Modify the initial value of self_test.
				// cmdbuf[0] = 0xB4;
				// cmdbuf[1] = 0x64;
				// cmdbuf[2] = 0x3F;
				// cmdbuf[3] = 0x3F;
				// cmdbuf[4] = 0x3C;
				// cmdbuf[5] = 0x00;
				// cmdbuf[6] = 0x3C;
				// cmdbuf[7] = 0x00;

				cmdbuf[0] = rFE96_setting[0];
				cmdbuf[1] = rFE96_setting[1];
				cmdbuf[2] = rFE96_setting[2];
				cmdbuf[3] = rFE96_setting[3];
				cmdbuf[4] = rFE96_setting[4];
				cmdbuf[5] = rFE96_setting[5];
				cmdbuf[6] = rFE96_setting[6];
				cmdbuf[7] = rFE96_setting[7];
				//Edit for ATD self-test setting function start

				i2c_himax_write(private_ts->client, 0x40,&cmdbuf[0], 8, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				msleep(30);

				//Read back
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 16, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				for(i=0; i<15; i++)//for(i=1; i<16; i++)
				{
				    printk("[Himax]:0xFE96 buff_back[%d] = 0x%x\n",i,valuebuf[i]);
				}

				msleep(30);

				//Step 5 : Enter self_test mode
				cmdbuf[0] = 0x06;//0x16;
				i2c_himax_write(private_ts->client, 0xF1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				i2c_himax_read(private_ts->client, 0xF1, valuebuf, 1, DEFAULT_RETRY_CNT);

				printk("[Himax]:0x91_back = 0x%x\n",valuebuf[0]);
				msleep(10);

				//Step 6 : Sensor On
				i2c_himax_write(private_ts->client, 0x83,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);
				msleep(120);
				i2c_himax_write(private_ts->client, 0x81,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);

				//Edit for ATD self-test setting function start
				mdelay(self_test_delay_time * 1000);
				//Edit for ATD self-test setting function end

				//Step 7 : Sensor Off
				i2c_himax_write(private_ts->client, 0x82,&cmdbuf[0], 0, DEFAULT_RETRY_CNT);

				msleep(30);

				//Step 8 : Get self_test result
				cmdbuf[0] = 0x15;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				cmdbuf[1] = 0x96; //FE96
				i2c_himax_write(private_ts->client, 0x8B,&cmdbuf[0], 2, DEFAULT_RETRY_CNT);
				msleep(10);

				i2c_himax_read(private_ts->client, 0x5A, valuebuf, 16, DEFAULT_RETRY_CNT);
				msleep(10);

				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0x8C,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				//Final : Leave self_test mode
				cmdbuf[0] = 0x00;
				i2c_himax_write(private_ts->client, 0xF1,&cmdbuf[0], 1, DEFAULT_RETRY_CNT);

				//if(valuebuf[1]==0xAA) //get the self_test result , shift first byte for config bank read issue.
				if(valuebuf[0]==0xAA) //get the self_test result , shift first byte for config bank read issue.
				{
					printk("[Himax]: E self-test pass\n");
					pf_value = 0x0;
					for(i=0; i<7; i++)//for(i=1; i<16; i++)
					{
						printk("[Himax]:0xFE96 buff[%d] = 0x%x\n",i,valuebuf[i]);
					}
				}
				else
				{
					printk("[Himax]: E self-test fail\n");
					pf_value = 0x1;
					for(i=0; i<15; i++)
					{
						printk("[Himax]:0xFE96 buff[%d] = 0x%x\n",i,valuebuf[i]);
					}
				}

				//HW reset and power on again.
				//----[HX_RST_PIN_FUNC]-----------------------------------------------------------------------------start
					#ifdef HX_RST_PIN_FUNC
					himax_HW_reset();
					#endif
				//----[HX_RST_PIN_FUNC]-------------------------------------------------------------------------------end

				himax_ts_poweron(private_ts);

			}
			return pf_value;
		}

		//add by josh setting self_test config
		/*
		* time : t:x12 (12s)
		* fe96 : w:x96:x03:x02......
		*/
		static ssize_t himax_self_test_setting(struct device *dev,struct device_attribute *attr, const char *buf, size_t count)
		{
			char buf_tmp[6], length = 0;
			uint8_t veriLen = 0;
			uint8_t write_da[100];
			unsigned long result = 0;
			static uint8_t himax_command = 0;

			memset(buf_tmp, 0x0, sizeof(buf_tmp));
			memset(write_da, 0x0, sizeof(write_da));
			if (buf[0] == 't' && buf[1] == ':' && buf[2] == 'x')
			{
				if(buf[3] > 47 && buf[3] < 58 && buf[4] > 47 && buf[4] < 58)
				{
					self_test_delay_time = ( buf[3] - 48 ) * 10 + buf[4] - 48;
					printk(KERN_INFO "self_test_delay_time: %d", self_test_delay_time);
				}
				return count;
			}

			// for store function start
			if (buf[0] == 's')
			{
				diag_dump_to_factory = true;
				if (buf[2] == 'w' && buf[3] == ':')
				{
					if (buf[4] == 'x')
					{
						uint8_t loop_i;
						uint16_t base = 7;
						memcpy(buf_tmp, buf + 3, 2);
						if (!strict_strtoul(buf_tmp, 16, &result))
							himax_command = result;
						for (loop_i = 0; loop_i < 100; loop_i++)
						{
							if (buf[base] == '\n')
							{
								if (buf[2] == 'w')
									printk(KERN_INFO "CMD: %x, %x, %d\n", himax_command,write_da[0], length);
								for (veriLen = 0; veriLen < length; veriLen++)
								{
									printk(KERN_INFO "%x ", *((&write_da[0])+veriLen));
									rFE96_setting[veriLen] = *((&write_da[0])+veriLen);
									printk(KERN_INFO "rFE96_setting[%d] : %x \n",veriLen ,rFE96_setting[veriLen]);
								}

								printk(KERN_INFO "\n");
								return count;
							}
							if (buf[base + 1] == 'x')
							{
								buf_tmp[4] = '\n';
								buf_tmp[5] = '\0';
								memcpy(buf_tmp, buf + base + 2, 2);
								if (!strict_strtoul(buf_tmp, 16, &result))
									write_da[loop_i] = result;
								length++;
							}
							base += 4;
						}
					}
				}
				return count;
			}
			// for store function end

			if (buf[0] == 'w' && buf[1] == ':')
			{
				if (buf[2] == 'x')
				{
					uint8_t loop_i;
					uint16_t base = 5;
					memcpy(buf_tmp, buf + 3, 2);
					if (!strict_strtoul(buf_tmp, 16, &result))
						himax_command = result;
					for (loop_i = 0; loop_i < 100; loop_i++)
					{
						if (buf[base] == '\n')
						{
							if (buf[0] == 'w')
								printk(KERN_INFO "CMD: %x, %x, %d\n", himax_command, write_da[0], length);
							for (veriLen = 0; veriLen < length; veriLen++)
							{
								printk(KERN_INFO "%x ", *((&write_da[0])+veriLen));
								rFE96_setting[veriLen] = *((&write_da[0])+veriLen);
								printk(KERN_INFO "rFE96_setting[%d] : %x \n",veriLen ,rFE96_setting[veriLen]);
							}
							printk(KERN_INFO "\n");
							return count;
						}
						if (buf[base + 1] == 'x')
						{
							buf_tmp[4] = '\n';
							buf_tmp[5] = '\0';
							memcpy(buf_tmp, buf + base + 2, 2);
							if (!strict_strtoul(buf_tmp, 16, &result))
								write_da[loop_i] = result;
							length++;
						}
						base += 4;
					}
				}
			}

			return count;
		}

		//Shouchung add, initial porting touch in TF103CG
		static DEVICE_ATTR(tp_self_test, (S_IWUSR|S_IRUGO|S_IWGRP), himax_self_test_read, himax_self_test_setting); // for self-test setting function
		//Shouchung end
		#endif
	//----[HX_TP_PROC_SELF_TEST]-------------------------------------------------------------------------------end

		#ifdef HX_TP_PROC_RESET
		//Shouchung modify, initial porting touch in TF103CG
		//static ssize_t himax_reset_write(struct file *filp, const char __user *buff, unsigned long len, void *data)
		static ssize_t himax_reset_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
		//Shouchung end
		{
			//----[ENABLE_CHIP_RESET_MACHINE]-------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
					if(private_ts->init_success)
					{
						queue_delayed_work(private_ts->himax_wq, &private_ts->himax_chip_reset_work, 0);
					}
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]--------------------------------------------------------------------end

			//Shouchung modify, initial porting touch in TF103CG
			//return len;
			return count;
			//Shouchung end
		}
		//Shouchung add, initial porting touch in TF103CG
		static DEVICE_ATTR(reset, (S_IWUSR|S_IRUGO|S_IWGRP), NULL, himax_reset_write);
		//Shouchung end
		#endif
	//----[HX_TP_PROC_RESET]----------------------------------------------------------------------------------end	

	//Shouchung add, show the firmware version in Settings
	static ssize_t himax_touch_switch_name(struct switch_dev *sdev, char *buf)
	{
		himax_read_FW_ver(true);
		mdelay(1000);
		int count = 0;
		count += sprintf(buf + count, "Himax:MAJ");
		int i;

		count += sprintf(buf + count, "-0x%2.2X", FW_VER_MAJ_buff[0]);
		count += sprintf(buf + count, ":MIN");
		count += sprintf(buf + count, "-0x%2.2X", FW_VER_MIN_buff[0]);
		count += sprintf(buf + count, ":CFG");
		count += sprintf(buf + count,  "-0x%2.2X", CFG_VER_MIN_buff[CFG_VER_MIN_FLASH_LENG-1]);
		count += sprintf(buf + count, ":TP");
		//count += sprintf(buf + count,  "-0x%2.2X \n", CFG_VER_MAJ_buff[CFG_VER_MAJ_FLASH_LENG-1]);
		count += sprintf(buf + count,  "-0x%2.2X \n", TP_ID);
		return count;
	}
	//Shouchung end

	static int himax_touch_proc_init(void)
	{
		int ret;
		//Shouchung modify, initial porting touch in TF103CG
		struct proc_dir_entry *entry = NULL;

		#ifdef HX_TP_PROC_DIAG || HX_TP_RAW_DATA
			entry = proc_create(HIMAX_PROC_DIAG_FILE, 0664, NULL, &himax_diag_ops);
			if (!entry) {
				printk("[Himax] %s: proc himax_diag_ops file create failed!\n", __FUNCTION__);
				return -EINVAL;
			}

			entry = proc_create(HIMAX_RAW_DATA_FILE, 0664, NULL, &himax_raw_ops);
			if (!entry) {
				printk("[Himax] %s: proc himax_raw_ops file create failed!\n", __FUNCTION__);
				return -EINVAL;
			}
		#endif
		
		//----[HX_TP_PROC_REGISTER]----------------------------------------------------------------------------start
		/*#ifdef HX_TP_PROC_REGISTER
			himax_proc_register_file = create_proc_entry(HIMAX_PROC_REGISTER_FILE, 0666, NULL);
			if(himax_proc_register_file)
			{
				himax_proc_register_file->read_proc 	= himax_register_read;
				himax_proc_register_file->write_proc = himax_register_write;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc register file create failed!\n", __func__);
			}
		#endif
		
		#ifdef HX_TP_PROC_DIAG
			himax_proc_diag_file = create_proc_entry(HIMAX_PROC_DIAG_FILE, 0666, NULL);
			if(himax_proc_diag_file)
			{
				himax_proc_diag_file->proc_fops = &himax_diag_ops;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc diag file create failed!\n", __func__);
			}
		#endif
		
		#ifdef HX_TP_PROC_DEBUG_LEVEL
			himax_proc_debug_file = create_proc_entry(HIMAX_PROC_DEBUG_FILE, 0666, NULL);
			if(himax_proc_debug_file)
			{
				himax_proc_debug_file->read_proc 	= himax_debug_level_read;
				himax_proc_debug_file->write_proc = himax_debug_level_write;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc debug file create failed!\n", __func__);
			}
		#endif
		
		#ifdef HX_TP_PROC_FLASH_DUMP
			himax_proc_flash_file = create_proc_entry(HIMAX_PROC_FLASH_FILE, 0666, NULL);
			if(himax_proc_flash_file)
			{
				himax_proc_flash_file->read_proc 	= himax_flash_read;
				himax_proc_flash_file->write_proc = himax_flash_write;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc flash file create failed!\n", __func__);
			}    
		#endif
		
		#ifdef HX_TP_PROC_SELF_TEST
			himax_proc_selftest_file = create_proc_entry(HIMAX_PROC_SELFTEST_FILE, 0666, NULL);
			if(himax_proc_selftest_file)
			{
				himax_proc_selftest_file->read_proc 	= himax_self_test_read;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc self test file create failed!\n", __func__);
			}
		#endif

		#ifdef HX_TP_PROC_RESET
			himax_proc_reset_file = create_proc_entry(HIMAX_PROC_RESET_FILE, 0666, NULL);
			if(himax_proc_reset_file)
			{
				//himax_proc_reset_file->read_proc 	= himax_reset_read;
				himax_proc_reset_file->write_proc = himax_reset_write;
			}
			else
			{
				printk(KERN_ERR "[Himax] %s: proc reset file create failed!\n", __func__);
			}
		#endif*/
		//Shouchung end
		return 0 ;
	}

	static void himax_touch_proc_deinit(void)
	{
	}

	//Shouchung add, initial porting touch in TF103CG
	static struct attribute *himax_attr[] =
	{
		&dev_attr_register.attr,
		&dev_attr_debug_level.attr,
		// &dev_attr_diag.attr,
		&dev_attr_tp_self_test.attr,
		&dev_attr_touch_status.attr,	// for ATD tools
		//&dev_attr_tp_fw_upgrade.attr,	// for ATD fw upgrade
		//&dev_attr_touch_switch.attr, 
		&dev_attr_flash_dump.attr,
		&dev_attr_reset.attr,
		&dev_attr_tp_id.attr,
		NULL
	};

	static int himax_touch_sysfs_init(void)
	{
		int ret;
		android_touch_kobj = kobject_create_and_add("android_touch", NULL);
		if (android_touch_kobj == NULL)
		{
			printk(KERN_ERR "[TP]TOUCH_ERR: subsystem_register failed\n");
			ret = -ENOMEM;
			return ret;
		}

		#ifdef HX_TP_PROC_DEBUG_LEVEL
			ret = sysfs_create_file(android_touch_kobj, &dev_attr_debug_level.attr);
			if (ret)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: create_file debug_level failed\n");
				return ret;
			}
		#endif

		#ifdef HX_TP_PROC_REGISTER
			register_command = 0;
			ret = sysfs_create_file(android_touch_kobj, &dev_attr_register.attr);
			if (ret)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: create_file register failed\n");
				return ret;
			}
		#endif

		#ifdef HX_TP_PROC_SELF_TEST
			ret = sysfs_create_file(android_touch_kobj, &dev_attr_tp_self_test.attr);
			if (ret)
			{
				printk(KERN_ERR "[Himax]TOUCH_ERR: sysfs_create_file dev_attr_tp_self_test failed\n");
				return ret;
			}
		#endif

		#ifdef HX_TP_PROC_FLASH_DUMP
			ret = sysfs_create_file(android_touch_kobj, &dev_attr_flash_dump.attr);
			if (ret)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: sysfs_create_file failed\n");
				return ret;
			}
		#endif

		#ifdef HX_TP_PROC_RESET
			ret = sysfs_create_file(android_touch_kobj, &dev_attr_reset.attr);
			if (ret)
			{
				printk(KERN_ERR "[TP]TOUCH_ERR: sysfs_create_file failed\n");
				return ret;
			}
		#endif

		// for ATD tools start
		ret = sysfs_create_file(android_touch_kobj, &dev_attr_touch_status.attr);
		if (ret)
		{
			printk(KERN_ERR "[Himax]TOUCH_ERR: sysfs_create_file dev_attr_touch_status failed\n");
			return ret;
		}
		// for ATD tools end

		// for ATD tp_fw_upgrade start
		// ret = sysfs_create_file(android_touch_kobj, &dev_attr_tp_fw_upgrade.attr);
		// if (ret)
		// {
		// 	printk(KERN_ERR "[Himax]TOUCH_ERR: sysfs_create_file ev_attr_tp_fw_upgrade failed\n");
		// 	return ret;
		// }
		// for ATD tp_fw_upgrade end

		ret = sysfs_create_file(android_touch_kobj, &dev_attr_tp_id.attr);
		if (ret)
		{
			printk(KERN_ERR "[Himax]TOUCH_ERR: sysfs_create_file dev_attr_tp_id failed\n");
			return ret;
		}

		return 0;
	}

	static void himax_touch_sysfs_deinit(void)
	{
		#ifdef HX_TP_PROC_DEBUG_LEVEL
			sysfs_remove_file(android_touch_kobj, &dev_attr_debug_level.attr);
		#endif

		#ifdef HX_TP_PROC_REGISTER
			sysfs_remove_file(android_touch_kobj, &dev_attr_register.attr);
		#endif

		#ifdef HX_TP_PROC_SELF_TEST
			sysfs_remove_file(android_touch_kobj, &dev_attr_tp_self_test.attr);
		#endif
		
		#ifdef HX_TP_PROC_FLASH_DUMP	
			sysfs_remove_file(android_touch_kobj, &dev_attr_flash_dump.attr);
		#endif

		#ifdef HX_TP_PROC_RESET
			sysfs_remove_file(android_touch_kobj, &dev_attr_reset.attr);
		#endif
		sysfs_remove_file(android_touch_kobj, &dev_attr_touch_status.attr);	// for ATD tools
		//sysfs_remove_file(android_touch_kobj, &dev_attr_tp_fw_upgrade.attr);	// for ATD tp_fw_upgrade tools
		sysfs_remove_file(android_touch_kobj, &dev_attr_tp_id.attr);
		//sysfs_remove_file(android_touch_kobj, &dev_attr_touch_switch.attr);

		kobject_del(android_touch_kobj);
	}
//=============================================================================================================
//
//	Segment : Himax Touch Work Function
//
//=============================================================================================================

	static void himax_ts_work_func(struct work_struct *work)
	{
		int ret, i, temp1, temp2;
		unsigned int x=0, y=0, area=0, press=0;
		const unsigned int x_res = HX_X_RES;
		const unsigned int y_res = HX_Y_RES;
		struct himax_ts_data *ts = container_of(work, struct himax_ts_data, work);
		unsigned char check_sum_cal = 0;
		struct i2c_msg msg[2];
		uint8_t start_reg;
		uint8_t buf[128] = {0};
		int RawDataLen = 0;
		unsigned int temp_x[HX_MAX_PT], temp_y[HX_MAX_PT];
		
		#ifdef ENABLE_ADV_CHIP_STATUS_MONITOR
		unsigned char polling_status;
		#endif

		//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------start
			#ifdef HX_TP_PROC_DIAG
			uint8_t *mutual_data;
			uint8_t *self_data;
			uint8_t diag_cmd;
			int 		mul_num; 
			int 		self_num;
			int 		index = 0;
		
			//coordinate dump start
			char coordinate_char[15+(HX_MAX_PT+5)*2*5+2];
			struct timeval t;
			struct tm broken;
			//coordinate dump end
			#endif
		//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------end

		//Calculate the raw data length
		//Bizzy added for common RawData
		int raw_cnt_max = HX_MAX_PT/4;
		int raw_cnt_rmd = HX_MAX_PT%4;
		int hx_touch_info_size;
		if(raw_cnt_rmd != 0x00) //more than 4 fingers
		{
			if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
			{	
				RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+3)*4) - 1;
			}
			else
			{
				RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+3)*4);
			}		
			
			hx_touch_info_size = (HX_MAX_PT+raw_cnt_max+2)*4;
		}
		else //less than 4 fingers
		{
			if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
			{
				RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+2)*4) - 1;
			}
			else
			{
				RawDataLen = 128 - ((HX_MAX_PT+raw_cnt_max+2)*4);
			}			
			
			hx_touch_info_size = (HX_MAX_PT+raw_cnt_max+1)*4;
		}
		
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
			#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifdef ENABLE_ADV_CHIP_STATUS_MONITOR
					if(ts->running_status == 0)
							polling_status = 1;
					else
							polling_status = 0;
				#endif
			ts->running_status = 1;
			cancel_delayed_work_sync(&ts->himax_chip_monitor);
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end	
		
		start_reg = HX_CMD_RAE;
		msg[0].addr = ts->client->addr;
		msg[0].flags = 0;
		msg[0].len = 1;
		msg[0].buf = &start_reg;
		
		msg[1].addr = ts->client->addr;
		msg[1].flags = I2C_M_RD;
		
		#ifdef HX_TP_PROC_DIAG
		if(diag_command) //count the i2c read length
		#else
		if(false)
		#endif
		{
			#ifdef HX_TP_PROC_DIAG
				if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					msg[1].len =  128;//hx_touch_info_size + RawDataLen + 4 + 1;	//4: RawData Header
				}
				else
				{
					msg[1].len =  128;//hx_touch_info_size + RawDataLen + 4;	//4: RawData Header
				}
			#else
				msg[1].len =  hx_touch_info_size;
			#endif
		}
		else
		{
			//Bizzy modify for E version
			if(ESD_RESET_ACTIVATE)
			{
				msg[1].len =  128;
				printk(KERN_INFO "[HIMAX]:ESD_RESET_ACTIVATE = %d, 0x86 128 bytes.\n", ESD_RESET_ACTIVATE);
			}
			else
				msg[1].len =  hx_touch_info_size;
		}
		msg[1].buf = buf;
		
		#ifdef HX_PORTING_DEB_MSG
		printk("[HIMAX PORTING MSG]%s Touch Controller Trigger ISR enter.\n",__func__);	
		#endif	


		//Mutexlock Protect Start
		mutex_lock(&ts->mutex_lock);
		//Mutexlock Protect End
		
		//read 0x86 all event
		ret = i2c_transfer(ts->client->adapter, msg, 2); 
		if (ret < 0) 
		{
			printk(KERN_INFO "[HIMAX TP ERROR]:%s:i2c_transfer fail.\n", __func__);
			memset(buf, 0xff , 128);
			
			//----[ENABLE_CHIP_RESET_MACHINE]-------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				//Mutexlock Protect Start
				mutex_unlock(&ts->mutex_lock);
				//Mutexlock Protect End
				
				enable_irq(ts->client->irq);
				goto work_func_send_i2c_msg_fail;
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------end
		}

		if(ESD_RESET_ACTIVATE)
		{
			printk(KERN_INFO "[HIMAX]:ESD_RESET_ACTIVATE=%d, Resd %d bytes.\n", ESD_RESET_ACTIVATE, msg[1].len);
		}
		//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
			#ifdef HX_ESD_WORKAROUND
			for(i = 0; i < hx_touch_info_size; i++)
			{
				if(buf[i] == 0x00)
				{
					check_sum_cal = 1;
				}
				else if(buf[i] == 0xED)
				{
					check_sum_cal = 2;
				}
				else
				{
					check_sum_cal = 0;
					i = hx_touch_info_size;
				}
			}
			
			//IC status is abnormal ,do hand shaking
			//----[HX_TP_PROC_DIAG]------------------------------------------------------------------------------start
			#ifdef HX_TP_PROC_DIAG
			diag_cmd = getDiagCommand();
				#ifdef HX_ESD_WORKAROUND
					if((check_sum_cal != 0 || TOUCH_UP_COUNTER > 10) && ESD_RESET_ACTIVATE == 0 && diag_cmd == 0)  //ESD Check
				#else 
					if(check_sum_cal != 0 && diag_cmd == 0)
				#endif
			#else
				#ifdef HX_ESD_WORKAROUND
					if((check_sum_cal != 0 || TOUCH_UP_COUNTER > 10) && ESD_RESET_ACTIVATE == 0 )  //ESD Check
				#else
					if(check_sum_cal !=0)
				#endif
			#endif
			//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------end
			{
				//Mutexlock Protect Start
				mutex_unlock(&ts->mutex_lock);
				//Mutexlock Protect End
				
				//Shouchung modify for ESD solution
				#ifdef HX_ESD_WORKAROUND_HANDSHAKING
					ret = himax_hang_shaking(); //0:Running, 1:Stop, 2:I2C Fail
				#else
					ret = 1; //return STOP
				#endif
				//Shouchung end
				enable_irq(ts->client->irq);
				
				if(ret == 2)
				{
					goto work_func_send_i2c_msg_fail;
				}
				
				if((ret == 1) && (check_sum_cal == 1))
				{
					printk("[HIMAX TP MSG]: ESD event checked - ALL Zero.\n");
					//Bizzy remove for E version
					//ESD_HW_REST();

				}
				else if(check_sum_cal == 2)
				{
					printk("[HIMAX TP MSG]: ESD event checked - ALL 0xED.\n");
					ESD_HW_REST();
				}
				else if(TOUCH_UP_COUNTER > 10)
				{
					printk("[HIMAX TP MSG]: TOUCH UP COUNTER > 10.\n");
					ESD_HW_REST();
				}

				//Bizzy clear for TOUCH_UP issue
				if(TOUCH_UP_COUNTER > 10)
					TOUCH_UP_COUNTER = 0;

				//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------start
					#ifdef ENABLE_CHIP_STATUS_MONITOR
					ts->running_status = 0;
					queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
					printk("[HIMAX TP MSG]: ESD Polling Start 1.\n");
					#endif
				//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------end
				return;
			}
			else if(ESD_RESET_ACTIVATE)
			{
				ESD_RESET_ACTIVATE--;
				printk(KERN_INFO "[HIMAX TP MSG]:%s: Back from ESD reset, ready to serve., ESD_RESET_ACTIVATE=%d\n", __func__, ESD_RESET_ACTIVATE);
				//Mutexlock Protect Start
				mutex_unlock(&ts->mutex_lock);
				//Mutexlock Protect End
				enable_irq(ts->client->irq);
				
				//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------start
					#ifdef ENABLE_CHIP_STATUS_MONITOR
					#ifdef ENABLE_ADV_CHIP_STATUS_MONITOR
						if(polling_status == 1)
						{
								ts->running_status = 0;
								queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
								printk("[HIMAX TP MSG]: ESD Polling Start 2.\n");
						}
					#else
						ts->running_status = 0;
						queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
						printk("[HIMAX TP MSG]: ESD Polling Start 2.\n");
					#endif
					#endif
				//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------end
				return;
			}
			#endif
		//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end
		
		//calculate the checksum
		for(i = 0; i < hx_touch_info_size; i++)
		{
			check_sum_cal += buf[i];
		}
		
		//check sum fail
		if ((check_sum_cal != 0x00) || (buf[HX_TOUCH_INFO_POINT_CNT] & 0xF0 )!= 0xF0)
		{
			printk(KERN_INFO "[HIMAX TP MSG] checksum fail : check_sum_cal: 0x%02X\n", check_sum_cal);
		
			//Mutexlock Protect Start
			mutex_unlock(&ts->mutex_lock);
			//Mutexlock Protect End
			
			enable_irq(ts->client->irq);
			
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
#ifdef HX_ESD_WORKAROUND
				ESD_COUNTER++;
				printk("[HIMAX TP MSG]: ESD event checked - check_sum_cal, ESD_COUNTER = %d.\n", ESD_COUNTER);
				if(ESD_COUNTER > ESD_COUNTER_SETTING)
				{
					ESD_HW_REST();
			
					//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end

					//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start
					#ifdef ENABLE_CHIP_STATUS_MONITOR
					ts->running_status = 0;
					queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
					printk("[HIMAX TP MSG]: ESD Polling Start 3.\n");
					#endif
					//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end
				}
#endif
			return;
		}
		
		//debug , printk the i2c packet data
		//----[HX_TP_PROC_DEBUG_LEVEL]-------------------------------------------------------------------------start
			#ifdef HX_TP_PROC_DEBUG_LEVEL
			if (getDebugLevel() & 0x1) 
			{
				printk(KERN_INFO "[HIMAX TP MSG]%s: raw data:\n", __func__);
				for (i = 0; i < 128; i=i+8) 
				{
					printk(KERN_INFO "%d: 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X, 0x%2.2X \n", i, buf[i], buf[i+1], buf[i+2], buf[i+3], buf[i+4], buf[i+5], buf[i+6], buf[i+7]);
				}
			}
			#endif
		//----[HX_TP_PROC_DEBUG_LEVEL]---------------------------------------------------------------------------end
		
		//touch monitor raw data fetch
		//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------start
			#ifdef HX_TP_PROC_DIAG
			diag_cmd = getDiagCommand();
			if (diag_cmd >= 1 && diag_cmd <= 6) 
			{
				
				if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
				{
					//Check 128th byte CRC
					for (i = hx_touch_info_size, check_sum_cal = 0; i < 128; i++)
					{
						check_sum_cal += buf[i];
					}
					
					if (check_sum_cal % 0x100 != 0)
					{
						goto bypass_checksum_failed_packet;
					}
				}				
				
				mutual_data = getMutualBuffer();
				self_data 	= getSelfBuffer();
				
				// initiallize the block number of mutual and self
				mul_num = getXChannel() * getYChannel();
				
				#ifdef HX_EN_BUTTON
				self_num = getXChannel() + getYChannel() + HX_BT_NUM;
				#else
				self_num = getXChannel() + getYChannel();
				#endif        
			
				//Himax: Check Raw-Data Header
				if(buf[hx_touch_info_size] == buf[hx_touch_info_size+1] && buf[hx_touch_info_size+1] == buf[hx_touch_info_size+2] 
				&& buf[hx_touch_info_size+2] == buf[hx_touch_info_size+3] && buf[hx_touch_info_size] > 0) 
				{
					index = (buf[hx_touch_info_size] - 1) * RawDataLen;
					//printk("Header[%d]: %x, %x, %x, %x, mutual: %d, self: %d\n", index, buf[56], buf[57], buf[58], buf[59], mul_num, self_num);
					for (i = 0; i < RawDataLen; i++) 
					{
						if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
						{
							temp1 = index + i;
						}
						else
						{
							temp1 = index;
						}
						
						if(temp1 < mul_num)
						{ //mutual
							mutual_data[index + i] = buf[i + hx_touch_info_size+4];	//4: RawData Header
						} 
						else 
						{//self
							if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
							{
								temp1 = i + index;
								temp2 = self_num + mul_num;
							}
							else
							{
								temp1 = i;
								temp2 = self_num;
							}
							if(temp1 >= temp2)
							{
								break;
							}
							
							if (IC_TYPE == HX_85XX_D_SERIES_PWON || IC_TYPE == HX_85XX_E_SERIES_PWON)
							{
								self_data[i+index-mul_num] = buf[i + hx_touch_info_size+4];	//4: RawData Header
							}
							else									
							{
								self_data[i] = buf[i + hx_touch_info_size+4];	//4: RawData Header
							}			    		
						}
					}
				}
				else
				{
					printk(KERN_INFO "[HIMAX TP MSG]%s: header format is wrong!\n", __func__);
				}
			}
			else if(diag_cmd == 7)
			{
				memcpy(&(diag_coor[0]), &buf[0], 128);
			}
			//coordinate dump start   
			if(coordinate_dump_enable == 1)
			{	
				for(i=0; i<(15 + (HX_MAX_PT+5)*2*5); i++)
				{
					coordinate_char[i] = 0x20;
				}
				coordinate_char[15 + (HX_MAX_PT+5)*2*5] = 0xD;
				coordinate_char[15 + (HX_MAX_PT+5)*2*5 + 1] = 0xA;		
			}
			//coordinate dump end	
			#endif	
		//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------end 	
		
		bypass_checksum_failed_packet:
		
		#if defined(HX_EN_BUTTON) || defined(HX_EN_MUT_BUTTON)
		tpd_key = (buf[HX_TOUCH_INFO_POINT_CNT+2]>>4);
		if(tpd_key == 0x0F)
		{
			tpd_key = 0xFF;
		}	 
		//printk("TPD BT:  %x\r\n", tpd_key);
		#else
		tpd_key = 0xFF;
		#endif 
		
		p_point_num = hx_point_num;
		
		if(buf[HX_TOUCH_INFO_POINT_CNT] == 0xff)
		{
			hx_point_num = 0;
		}
		else
		{
			hx_point_num= buf[HX_TOUCH_INFO_POINT_CNT] & 0x0f;
		}   
		
		// Touch Point information
		if(hx_point_num != 0 && tpd_key == 0xFF)
		{
			// parse the point information
			for(i=0; i<HX_MAX_PT; i++)
			{
				if(buf[4*i] != 0xFF)
				{
					// x and y axis
					x = buf[4 * i + 1] | (buf[4 * i] << 8) ;
					y = buf[4 * i + 3] | (buf[4 * i + 2] << 8);

					if(x > x_res)
					{
						x = x_res;
					}

					if(y > y_res)
					{
						y = y_res;
					}
					
					temp_x[i] = x;
					temp_y[i] = y;
				
					if((x <= x_res) && (y <= y_res))
					{
						// caculate the pressure and area
						press = buf[4*HX_MAX_PT+i];
						area = press;
						if(area > 31)
						{
							area = (area >> 3);
						}
				
						// kernel call for report point area, pressure and x-y axis
						input_report_key(ts->input_dev, BTN_TOUCH, 1);             // touch down
						input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, i);     //ID of touched point
						input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, area); //Finger Size
						input_report_abs(ts->input_dev, ABS_MT_PRESSURE, press);   // Pressure
						input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x);     // X axis
						input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y);     // Y axis
						
						input_mt_sync(ts->input_dev);

						if (Touch_flag == 0)
						{
							printk("[HIMAX PORTING MSG]%s TOUCH DOWN!.\n",__func__);
							Touch_flag = 1;
						}
						
						#ifdef HX_PORTING_DEB_MSG
						printk("[HIMAX PORTING MSG]%s Touch DOWN x = %d, y = %d, area = %d, press = %d.\n",__func__, x, y, area, press);                               
						#endif

						if (Flag_print_point == 1)
						{
							printk("[HIMAX PORTING MSG]%s x = %d, y = %d, area = %d, press = %d.\n",__func__, x, y, area, press);
						}
						
						//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------start
							#ifdef HX_TP_PROC_DIAG	
							//coordinate dump start	
							if(coordinate_dump_enable == 1)
							{					
								do_gettimeofday(&t);
								time_to_tm(t.tv_sec, 0, &broken);
							
								sprintf(&coordinate_char[0], "%2d:%2d:%2d:%3li,", broken.tm_hour, broken.tm_min, broken.tm_sec, t.tv_usec/1000);				
							
								sprintf(&coordinate_char[15 + (i*2)*5], "%4d,", x);			
								sprintf(&coordinate_char[15 + (i*2)*5 + 5], "%4d,", y);
	
								coordinate_fn->f_op->write(coordinate_fn,&coordinate_char[0],15 + (HX_MAX_PT+5)*2*sizeof(char)*5 + 2,&coordinate_fn->f_pos);
							} 
							//coordinate dump end  
							#endif
						//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------end                  
					} 
				}
				else
				{
					temp_x[i] = 0xFFFF;
					temp_y[i] = 0xFFFF; 
					//input_mt_sync(ts->input_dev);
				}
			} 
			input_sync(ts->input_dev);   
		
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
				#ifdef HX_ESD_WORKAROUND
				ESD_COUNTER = 0;
				TOUCH_UP_COUNTER = 0;
				#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
		}
		else if(hx_point_num == 0 && tpd_key != 0xFF)
		{
			temp_x[0] = 0xFFFF;
			temp_y[0] = 0xFFFF; 
			temp_x[1] = 0xFFFF;
			temp_y[1] = 0xFFFF; 
		
			if( tpd_key == 1)
			{
				input_report_key(ts->input_dev, tpd_keys_local[0], 1);
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);
				//printk("Press BT1*** \r\n");	
			}
			
			if( tpd_key == 2)
			{
				input_report_key(ts->input_dev, tpd_keys_local[1], 1);
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);
				//printk("Press BT2*** \r\n");	
			}
			
			if( tpd_key == 3)
			{
				input_report_key(ts->input_dev, tpd_keys_local[2], 1);
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);
				//printk("Press BT3*** \r\n");	
			}
			
			if( tpd_key == 4)
			{
				input_report_key(ts->input_dev, tpd_keys_local[3], 1);
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);
				//printk("Press BT4*** \r\n");	
			}
		
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
				#ifdef HX_ESD_WORKAROUND
				ESD_COUNTER = 0;
				TOUCH_UP_COUNTER = 0;
				#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
		}
		else if(hx_point_num == 0 && tpd_key == 0xFF)
		{
			temp_x[0] = 0xFFFF;
			temp_y[0] = 0xFFFF; 
			temp_x[1] = 0xFFFF;
			temp_y[1] = 0xFFFF; 
			
			if (tpd_key_old != 0xFF)
			{
				input_report_key(ts->input_dev, tpd_keys_local[tpd_key_old-1], 0);
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);
			}
			else
			{
#ifdef HX_ESD_WORKAROUND
				if (diag_cmd == 0)
					TOUCH_UP_COUNTER++;
#endif
				// leave event
				input_report_key(ts->input_dev, BTN_TOUCH, 0);  // touch up
				input_mt_sync(ts->input_dev);
				input_sync(ts->input_dev);

				if (Touch_flag == 1)
				{
					printk("[HIMAX PORTING MSG]%s TOUCH UP!.\n",__func__);
					Touch_flag = 0;
				}
			
				#ifdef HX_PORTING_DEB_MSG
				printk("[HIMAX PORTING MSG]%s Touch UP.\n",__func__);                               
				#endif
	
				//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------start
					#ifdef HX_TP_PROC_DIAG	
					//coordinate dump start	
					if(coordinate_dump_enable == 1)
					{				
						do_gettimeofday(&t);
						time_to_tm(t.tv_sec, 0, &broken);
					
						sprintf(&coordinate_char[0], "%2d:%2d:%2d:%lu,", broken.tm_hour, broken.tm_min, broken.tm_sec, t.tv_usec/1000);	
						sprintf(&coordinate_char[15], "Touch up!");
						coordinate_fn->f_op->write(coordinate_fn,&coordinate_char[0],15 + (HX_MAX_PT+5)*2*sizeof(char)*5 + 2,&coordinate_fn->f_pos);
					}
					//coordinate dump end
					#endif
				//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------end
			}
			//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start
				#ifdef HX_ESD_WORKAROUND
				ESD_COUNTER = 0;
				#endif
			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end
		}
		/*	
		//----[HX_EN_GESTURE]---------------------------------------------------------------------------------start
			#ifdef HX_EN_GESTURE
			if (hx_point_num == 2 && p_point_num == 2 && temp_x[0] != 0xFFFF && temp_y[0] != 0xFFFF && temp_x[1] != 0xFFFF && temp_y[1] != 0xFFFF)
			{		  		
				if (temp_x[0] > temp_x[1] && temp_y[0] > temp_y[1]) 
				{
					Dist_Cal_Now = (temp_x[0] - temp_x[1]) + (temp_y[0] - temp_y[1]);
				}
				else if (temp_x[0] > temp_x[1] && temp_y[1] > temp_y[0]) 
				{
					Dist_Cal_Now = (temp_x[0] - temp_x[1]) + (temp_y[1] - temp_y[0]);
				}	
				else if (temp_x[1] > temp_x[0] && temp_y[0] > temp_y[1]) 
				{
					Dist_Cal_Now = (temp_x[1] - temp_x[0]) + (temp_y[0] - temp_y[1]);
				}
				else if (temp_x[1] > temp_x[0] && temp_y[1] > temp_y[0]) 
				{
					Dist_Cal_Now = (temp_x[1] - temp_x[0]) + (temp_y[1] - temp_y[0]);	
				}
			
				if (Dist_Cal_Now - Dist_Cal_EX > 10)
				{
					ZoomInCnt++;
				}
				else if (Dist_Cal_EX - Dist_Cal_Now > 10)
				{
					ZoomOutCnt++;
				}
			
				//EX_x[0]= x[0];
				//EX_y[0]= y[0];
				//EX_x[1]= x[1];
				//EX_y[1]= y[1];
			
				if (ZoomInCnt > 2)
				{
					ZoomInFlag = 1;
					ZoomOutFlag = 0;
				}
				else if (ZoomOutCnt > 2)
				{
					ZoomOutFlag =1;
					ZoomInFlag = 0; 
				}
				Dist_Cal_EX = Dist_Cal_Now;
			}
			else if (hx_point_num == 2 && p_point_num != 2 && temp_x[0] != 0xFFFF && temp_y[0] != 0xFFFF && temp_x[1] != 0xFFFF && temp_y[1] != 0xFFFF)
			{
				Dist_Cal_EX = 0;			
			}
			else
			{
				Dist_Cal_EX = 0xFFFF;
				Dist_Cal_Now = 0xFFFF;
				ZoomInCnt = 0;
				ZoomOutCnt = 0;
				p_point_num = 0xFFFF;
			}
			#endif	
		//----[HX_EN_GESTURE]---------------------------------------------------------------------------------start
		*/	

		tpd_key_old = tpd_key;
		
		//Mutexlock Protect Start
		mutex_unlock(&ts->mutex_lock);
		//Mutexlock Protect End
		
		enable_irq(ts->client->irq);
		
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
			#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifdef ENABLE_ADV_CHIP_STATUS_MONITOR
					if(polling_status == 1)
					{
							ts->running_status = 0;
							queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
					}
				#else
					ts->running_status = 0;
					queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
				#endif
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end	
		
		return;
		
		work_func_send_i2c_msg_fail:
		
		printk(KERN_ERR "[HIMAX TP ERROR]:work_func_send_i2c_msg_fail: %d \n",__LINE__);
		
		//----[ENABLE_CHIP_RESET_MACHINE]-------------------------------------------------------------------start
			#ifdef ENABLE_CHIP_RESET_MACHINE
			if(private_ts->init_success)
			{
				queue_delayed_work(ts->himax_wq, &ts->himax_chip_reset_work, 0);
			}
		#endif
		//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------end

		//----[ENABLE_CHIP_STATUS_MONITOR]------------------------------------------------------------------start	
			#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifdef ENABLE_ADV_CHIP_STATUS_MONITOR
					if(polling_status == 1)
					{
							ts->running_status = 0;
							queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
					}
				#else
					ts->running_status = 0;
					queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 10*HZ);
				#endif
			#endif
		//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------end	
		
		return;
	}
		
//=============================================================================================================
//
//	Segment : Himax Linux Driver Probe Function
//
//=============================================================================================================

	//----[ interrupt ]---------------------------------------------------------------------------------------start
		static irqreturn_t himax_ts_irq_handler(int irq, void *dev_id)
		{
			struct himax_ts_data *ts = dev_id;
			struct i2c_client *client = ts->client;

			dev_dbg(&client->dev, "[HIMAX TP MSG] %s\n", __func__);
			disable_irq_nosync(ts->client->irq);
			queue_work(ts->himax_wq, &ts->work);

			return IRQ_HANDLED;
		}

		static int himax_ts_register_interrupt(struct i2c_client *client)
		{
			struct himax_ts_data *ts = i2c_get_clientdata(client);
			int err = 0;

			if(HX_INT_IS_EDGE)	//edge trigger
			{
				err = request_irq(client->irq, himax_ts_irq_handler,IRQF_TRIGGER_FALLING, client->name, ts);
			}
			else	//low level trigger
			{
				err = request_irq(client->irq, himax_ts_irq_handler,IRQF_TRIGGER_LOW, client->name, ts);
			}

			if (err)
			{
				dev_err(&client->dev, "[himax] %s: request_irq %d failed\n",__func__, client->irq);
			}
			else
			{
				printk("[himax]%s request_irq ok \r\n",__func__);  
			}
			return err;
		}
	//----[ interrupt ]-----------------------------------------------------------------------------------------end

	//----[ i2c ]---------------------------------------------------------------------------------------------start
		static int himax_ts_probe(struct i2c_client *client,const struct i2c_device_id *id)
		{
			int err = 0;
			struct himax_ts_data *ts;

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s enter\n",__func__);
			#endif

			//*********************************************************************************************************
			// Allocate the himax_ts_data
			//*********************************************************************************************************
			ts = kzalloc(sizeof(struct himax_ts_data), GFP_KERNEL);
			if (ts == NULL) 
			{
				printk(KERN_ERR "[HIMAX TP ERROR] %s: allocate himax_ts_data failed\n", __func__);
				err = -ENOMEM;
				goto err_alloc_data_failed;
			}

			ts->client 				= client;
			ts->init_success 	= 0;
			i2c_set_clientdata(client, ts);
			touch_i2c 	= client; //global variable
			private_ts 	= ts;
	//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
#ifdef HX_ESD_WORKAROUND
			ESD_RESET_ACTIVATE = 2;
#endif
	//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end

			#ifdef HX_PORTING_DEB_MSG
				printk("[HIMAX PORTING MSG]%s S1 : himax_ts_data allocate OK. \n",__func__);
			#endif

			//*********************************************************************************************************
			// TODO Power Pin 
			// If power control by baseband , please marked follows.
			// If power control by driver , please porting follows.
			//*********************************************************************************************************
			//Shouchung remove, initial porting touch in TF103CG
			/*tegra_gpio_enable(HIMAX_PWR_GPIO);
			if( gpio_request(HIMAX_PWR_GPIO, "himax-pwn") !=0 )
			{
				printk("[HIMAX PORTING ERROR] power gpio %d request fail.\n",HIMAX_PWR_GPIO);
				err = -ENODEV;
				goto err_check_functionality_failed;
			}
			gpio_direction_output(HIMAX_PWR_GPIO, 1);
			
			#ifdef HX_PORTING_DEB_MSG
				printk("[HIMAX PORTING MSG]%s S2 : Power Pin OK. \n",__func__);
			#endif*/
			//Shouchung end

			//*********************************************************************************************************
			// TODO Interrupt Pin
			//*********************************************************************************************************
			//Shouchung remove, initial porting touch in TF103CG
			//tegra_gpio_enable(HIMAX_INT_GPIO);
			//Shouchung end
			if( gpio_request(HIMAX_INT_GPIO, "himax-irq") != 0 )
			{
				printk("[HIMAX PORTING ERROR] interrupt gpio %d request fail.\n",HIMAX_PWR_GPIO);
				err = -ENODEV;
				goto err_check_functionality_failed;
			}
			gpio_direction_input(HIMAX_INT_GPIO);

			ts->intr_gpio 	= HIMAX_INT_GPIO;
			ts->client->irq = gpio_to_irq(ts->intr_gpio);

			#ifdef HX_PORTING_DEB_MSG
				printk("[HIMAX PORTING MSG]%s S3 : Interrupt Pin OK. \n",__func__);
			#endif

			//*********************************************************************************************************
			// TODO Reset Pin
			//*********************************************************************************************************
			//Shouchung remove, initial porting touch in TF103CG
			//tegra_gpio_enable(HIMAX_RST_GPIO);
			//Shouchung end
			if( gpio_request(HIMAX_RST_GPIO, "himax-reset") != 0)
			{
				printk("[HIMAX PORTING ERROR] reset gpio %d request fail.\n",HIMAX_PWR_GPIO);
				err = -ENODEV;
				goto err_check_functionality_failed;
			}
			gpio_direction_output(HIMAX_RST_GPIO, 1);	//reset set high
			msleep(100);

			#ifdef HX_RST_PIN_FUNC
				ts->rst_gpio = HIMAX_RST_GPIO;
			#endif

			#ifdef HX_PORTING_DEB_MSG
				printk("[HIMAX PORTING MSG]%s S4 : Reset Pin OK. \n",__func__);
			#endif

			//*********************************************************************************************************
			// Check i2c functionality
			//*********************************************************************************************************
			if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
			{
				printk(KERN_ERR "[HIMAX TP ERROR] %s: i2c check functionality error\n", __func__);
				err = -ENODEV;
				goto err_check_functionality_failed;
			}

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S5 : i2c functionality check OK.\n",__func__);
			#endif

			//*********************************************************************************************************
			// Create Work Queue
			//*********************************************************************************************************

			//----[ HX_TP_PROC_FLASH_DUMP ]------------------------------------------------------------------------start
				#ifdef  HX_TP_PROC_FLASH_DUMP
				ts->flash_wq = create_singlethread_workqueue("himax_flash_wq");
				if (!ts->flash_wq) 
				{
					printk(KERN_ERR "[HIMAX TP ERROR] %s: create flash workqueue failed\n", __func__);
					err = -ENOMEM;
					goto err_create_wq_failed;
				}
				#endif
			//----[ HX_TP_PROC_FLASH_DUMP ]--------------------------------------------------------------------------end

			ts->himax_wq = create_singlethread_workqueue("himax_wq");
			if (!ts->himax_wq) 
			{
				printk(KERN_ERR "[HIMAX TP ERROR] %s: create workqueue failed\n", __func__);
				err = -ENOMEM;
				goto err_create_wq_failed;
			}

			//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				INIT_DELAYED_WORK(&ts->himax_chip_reset_work, himax_chip_reset_function);
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				INIT_DELAYED_WORK(&ts->himax_chip_monitor, himax_chip_monitor_function); //for ESD solution
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	

			//----[HX_TP_PROC_FLASH_DUMP]--------------------------------------------------------------------------start	
				#ifdef HX_TP_PROC_FLASH_DUMP
				INIT_WORK(&ts->flash_work, himax_ts_flash_work_func);
				#endif
			//----[HX_TP_PROC_FLASH_DUMP]----------------------------------------------------------------------------end	

			INIT_WORK(&ts->work, himax_ts_work_func);

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S6 : Create Work Queue OK. \n",__func__);
			#endif

			//Mutexlock Protect Start
			mutex_init(&ts->mutex_lock);
			//Mutexlock Protect End

			//Wakelock Protect Start
			wake_lock_init(&ts->wake_lock, WAKE_LOCK_SUSPEND, "himax_touch_wake_lock");
			//Wakelock Protect End

			//*********************************************************************************************************
			// Himax Information / Initial, I2C must be ready
			//*********************************************************************************************************
			himax_ic_package_check(ts);

			//*********************************************************************************************************
			// *.i FW update
			//*********************************************************************************************************
			// #ifdef HX_FW_UPDATE_BY_I_FILE
			// if(i_Needupdate)
			// {
			// 	i_update_func();
			// }
			// #endif

			// for ATD tools start
			private_ts->attrs.attrs = himax_attr;
			err = sysfs_create_group(&client->dev.kobj, &private_ts->attrs);
			if (err)
			{
				dev_err(&client->dev, "[TP] %s: Not able to create the sysfs\n", __func__);
			}
			// for ATD tools end

			//*********************************************************************************************************
			// Himax Touch IC Power on
			//*********************************************************************************************************

			//HW Reset
			//----[HX_RST_PIN_FUNC]-------------------------------------------------------------------------------start	
				#ifdef HX_RST_PIN_FUNC
					#ifdef HX_RST_BY_POWER
						//TODO power off by different platform's API
						msleep(100);
						//TODO power on by different platform's API
						msleep(100);
					#else
						err = gpio_direction_output(ts->rst_gpio, 1);
						if (err)
						{
							printk(KERN_ERR "Failed to set reset direction, error=%d\n", err);
							//gpio_free(himax_chip->rst_gpio);
						}

						//----[HX_ESD_WORKAROUND]---------------------------------------------------------------------------start	
							#ifdef HX_ESD_WORKAROUND
							ESD_RESET_ACTIVATE = 2;
							#endif
						//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------end	

						gpio_set_value(ts->rst_gpio, 0);
						msleep(100);
						gpio_set_value(ts->rst_gpio, 1);
						msleep(100);		
					#endif
				#endif     
			//----[HX_RST_PIN_FUNC]---------------------------------------------------------------------------------end	

			//PowerOn
			himax_ts_poweron(ts);     

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S7 : Himax Power on OK. \n",__func__);
			#endif

			himax_touch_information();
			calculate_point_number();

			//----[HX_TP_PROC_FLASH_DUMP]--------------------------------------------------------------------------start
				#ifdef HX_TP_PROC_FLASH_DUMP
				setSysOperation(0);
				setFlashBuffer();
				#endif
			//----[HX_TP_PROC_FLASH_DUMP]----------------------------------------------------------------------------end

			//----[HX_TP_PROC_DIAG]--------------------------------------------------------------------------------start
				#ifdef HX_TP_PROC_DIAG
				setXChannel(HX_RX_NUM); // X channel
				setYChannel(HX_TX_NUM); // Y channel

				setMutualBuffer();
				if (getMutualBuffer() == NULL) 
				{
					printk(KERN_ERR "[HIMAX TP ERROR] %s: mutual buffer allocate fail failed\n", __func__);
					return -1; 
				}
				#endif
			//----[HX_TP_PROC_DIAG]----------------------------------------------------------------------------------end

			//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				ts->retry_time = 0;
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

			//----[HX_ESD_WORKAROUND]-----------------------------------------------------------------------------start
				#ifdef HX_ESD_WORKAROUND   
				ESD_RESET_ACTIVATE = 2;
				#endif
			//----[HX_ESD_WORKAROUND]-------------------------------------------------------------------------------end

			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start	
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifndef ENABLE_ADV_CHIP_STATUS_MONITOR
					ts->running_status = 0;	//Polling: default open
				#else
					ts->running_status = 1;	//ADV Polling: default close
				#endif
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S8 : Himax Information / Initial OK. \n",__func__);
			#endif

			//*********************************************************************************************************
			// Allocate Input Device
			//*********************************************************************************************************
			ts->input_dev = input_allocate_device();

			if (ts->input_dev == NULL) 
			{
				err = -ENOMEM;
				dev_err(&client->dev, "[HIMAX TP ERROR] Failed to allocate input device\n");
				goto err_input_dev_alloc_failed;
			}

			ts->input_dev->name		= INPUT_DEV_NAME;  
			ts->abs_x_max 				= HX_X_RES;
			ts->abs_y_max 				= HX_Y_RES;

			//----[HX_EN_XXX_BUTTON]----------------------------------------------------------------------------------start	
				#if defined(HX_EN_BUTTON) || defined(HX_EN_MUT_BUTTON)
				for(i=0; i<HX_BT_NUM; i++)
				{
					set_bit(tpd_keys_local[i], ts->input_dev->keybit);
				}
				#endif
			//----[HX_EN_XXX_BUTTON]------------------------------------------------------------------------------------end	

			__set_bit(EV_KEY, ts->input_dev->evbit);
			__set_bit(EV_ABS, ts->input_dev->evbit);
			__set_bit(BTN_TOUCH, ts->input_dev->keybit);

			input_set_abs_params(ts->input_dev, ABS_MT_TRACKING_ID, 0, HX_MAX_PT, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, HX_X_RES, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, HX_Y_RES, 0, 0);
			input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 31, 0, 0); //Finger Size
			input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 31, 0, 0); //Touch Size
			input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0, 0xFF, 0, 0);

			err = input_register_device(ts->input_dev);
			if (err) 
			{
				dev_err(&client->dev,"[HIMAX TP ERROR]%s: unable to register %s input device\n",__func__, ts->input_dev->name);
				goto err_input_register_device_failed;
			}

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S9 : Input Device Reigster OK. \n",__func__);
			#endif

			//TODO check the interrupt's API function by differrnt platform 
			himax_ts_register_interrupt(ts->client);

			//if (gpio_get_value(ts->intr_gpio) == 0) 
			//{
			//	printk(KERN_INFO "[HIMAX TP ERROR]%s: handle missed interrupt\n", __func__);
			//	himax_ts_irq_handler(client->irq, ts);
			//}

			//*********************************************************************************************************
			// Remain work of Probe
			//*********************************************************************************************************

			//----[CONFIG_HAS_EARLYSUSPEND]-----------------------------------------------------------------------start
				#ifdef CONFIG_HAS_EARLYSUSPEND
				ts->early_suspend.level = EARLY_SUSPEND_LEVEL_DISABLE_FB + 1;
				ts->early_suspend.suspend = himax_ts_early_suspend;
				ts->early_suspend.resume = himax_ts_late_resume;
				register_early_suspend(&ts->early_suspend);
				#endif
			//----[CONFIG_HAS_EARLYSUSPEND]-------------------------------------------------------------------------end

			//Shouchung add, show the firmware version in Settings
			/* Register Switch file */
			private_ts->touch_sdev.name = "touch";
			private_ts->touch_sdev.print_name = himax_touch_switch_name;
			if(switch_dev_register(&private_ts->touch_sdev) < 0)
			{
				dev_info(&client->dev, "switch_dev_register for touch failed!\n");
			}
			switch_set_state(&private_ts->touch_sdev, 0);
			//Shouchung end

			//Shouchung add, initial porting touch in TF103CG
			err = himax_touch_proc_init();
			if (err)
			{
				printk(KERN_ERR "Failed to create debug node in Proc FS, error=%d\n", err);
			}
			err = himax_touch_sysfs_init();
			if (err)
			{
				printk(KERN_ERR "Failed to create debug node in Sys FS, error=%d\n", err);
			}
			ts->attrs.attrs = himax_attr;
			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s S10 : Create debug node OK. \n",__func__);
			#endif
			//Shouchung end
			
			ts->init_success = 1;

			//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				ts->retry_time = 0;
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				#ifndef ENABLE_ADV_CHIP_STATUS_MONITOR
					queue_delayed_work(ts->himax_wq, &ts->himax_chip_monitor, 60*HZ);   //for ESD solution
				#else
					cancel_delayed_work(&private_ts->himax_chip_monitor);
				#endif
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end


			i_Check_FW_Version_func();
			#ifdef HX_RST_PIN_FUNC
				himax_HW_reset();
			#endif
				msleep(50);
				// himax_ts_poweron(private_ts);

			TP_ID = Read_TP_ID();
			if(TP_ID == TP_HIMAX)
			{
				i_CTPM_FW = i_CTPM_FW_Himax;
				FW_ID = TP_HIMAX_FW;
			}else
			{
				i_CTPM_FW = i_CTPM_FW_Ofilm;
				FW_ID = TP_OFILM_FW;
			}

			#ifdef HX_FW_UPDATE_BY_I_FILE
			if(i_Needupdate)
			{
				i_update_func();
			}
			#endif

			ac_work_queue = create_singlethread_workqueue("ac_workqueue");
			INIT_DELAYED_WORK(&ac_work, himax_cable_status);

			cable_status_register_client(&cable_status_notifier);

			#ifdef HX_PORTING_DEB_MSG
			printk("[HIMAX PORTING MSG]%s complete. \n",__func__);
			#endif

			return 0;

			err_input_register_device_failed:
			if (ts->input_dev)
			{
				input_free_device(ts->input_dev);
			}

			err_input_dev_alloc_failed:
			//Mutexlock Protect Start
			mutex_destroy(&ts->mutex_lock);
			//Mutexlock Protect End

			//Wakelock Protect Start
			wake_lock_destroy(&ts->wake_lock);
			//Wakelock Protect End

			//----[ENABLE_CHIP_RESET_MACHINE]---------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_RESET_MACHINE
				cancel_delayed_work(&ts->himax_chip_reset_work);
				#endif
			//----[ENABLE_CHIP_RESET_MACHINE]-----------------------------------------------------------------------end

			//----[ENABLE_CHIP_STATUS_MONITOR]--------------------------------------------------------------------start
				#ifdef ENABLE_CHIP_STATUS_MONITOR
				cancel_delayed_work(&ts->himax_chip_monitor);
				#endif
			//----[ENABLE_CHIP_STATUS_MONITOR]----------------------------------------------------------------------end

			if (ts->himax_wq)
			{
				destroy_workqueue(ts->himax_wq);
			}

			err_create_wq_failed:
			kfree(ts);

			err_alloc_data_failed:
			err_check_functionality_failed:

			return err;
		}

		static int himax_ts_remove(struct i2c_client *client)
		{
			struct himax_ts_data *ts = i2c_get_clientdata(client);

			himax_touch_proc_deinit();
			//Shouchung add, initial porting touch in TF103CG
			himax_touch_sysfs_deinit();
			//Shouchung end

			//Shouchung add, show the firmware version in Settings
			switch_dev_unregister(&private_ts->touch_sdev);
			//Shouchung end

			unregister_early_suspend(&ts->early_suspend);
			free_irq(client->irq, ts);

			//Mutexlock Protect Start
			mutex_destroy(&ts->mutex_lock);
			//Mutexlock Protect End

			if (ts->himax_wq)
			{
				destroy_workqueue(ts->himax_wq);
			}
			input_unregister_device(ts->input_dev);

			//Wakelock Protect Start
			wake_lock_destroy(&ts->wake_lock);
			//Wakelock Protect End

			kfree(ts);

			printk(KERN_INFO "[himax]%s: cable_status_unregister_client\n", __func__);
			cable_status_unregister_client(&cable_status_notifier);

			return 0;
		}

		static struct himax_i2c_setup_data himax_ts_info = {1, HIMAX_I2C_ADDR, 0, HIMAX_TS_NAME};

		static const struct i2c_device_id himax_ts_id[] = 
		{
			{ HIMAX_TS_NAME, 0 },
			{ }
		};

		static struct i2c_driver himax_ts_driver = 
		{
			.probe		= himax_ts_probe,
			.remove		= himax_ts_remove,
			#ifndef CONFIG_HAS_EARLYSUSPEND
			.suspend	= himax_ts_suspend,
			.resume		= himax_ts_resume,
			#endif
			.id_table    = himax_ts_id,
			.driver        = 
			{
				.name = HIMAX_TS_NAME,
			},
		};

		int himax_add_i2c_device(struct himax_i2c_setup_data *i2c_set_data, struct i2c_driver *driver)
		{	
			struct i2c_board_info info;
			struct i2c_adapter *adapter;
			struct i2c_client *client;
			int ret,err;

			printk("%s : i2c_bus=%d; slave_address=0x%x; i2c_name=%s",__func__,i2c_set_data->i2c_bus,i2c_set_data->i2c_address, i2c_set_data->name);

			memset(&info, 0, sizeof(struct i2c_board_info));
			info.addr = i2c_set_data->i2c_address;
			strlcpy(info.type, i2c_set_data->name, I2C_NAME_SIZE);

			if(i2c_set_data->irq > 0)
			{
				info.irq = i2c_set_data->irq;
			}

			adapter = i2c_get_adapter( i2c_set_data->i2c_bus);
			if (!adapter) 
			{
				printk("%s: can't get i2c adapter %d\n",__func__,  i2c_set_data->i2c_bus);
				err = -ENODEV;
				goto err_driver;
			}

			client = i2c_new_device(adapter, &info);
			if (!client) 
			{
				printk("%s:  can't add i2c device at 0x%x\n",__func__, (unsigned int)info.addr);
				err = -ENODEV;
				goto err_driver;
			}

			i2c_put_adapter(adapter);

			ret = i2c_add_driver(driver);
			if (ret != 0)
			{
				printk("%s: can't add i2c driver\n", __func__);
				err = -ENODEV;
				goto err_driver;
			}	

			return 0;

			err_driver:
			return err;
		}

		//Shouchung modify, initial porting touch in TF103CG
		//static int __devinit himax_ts_init(void)
		static int __init himax_ts_init(void)
		//Shouchung end
		{
			printk(KERN_INFO "[himax] %s\n", __func__);
			return i2c_add_driver(&himax_ts_driver);
			//return himax_add_i2c_device(&himax_ts_info,&himax_ts_driver);
		}

		static void __exit himax_ts_exit(void)
		{
			i2c_del_driver(&himax_ts_driver);
			return;
		}

		module_init(himax_ts_init);
		module_exit(himax_ts_exit);

		MODULE_DESCRIPTION("Himax Touchscreen Driver");
		MODULE_LICENSE("GPL");
	//----[ i2c ]-----------------------------------------------------------------------------------------------end
