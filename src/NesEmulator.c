/*

Laboratario Sistemas Embebidos
Embedded Systems Laboratory

NES port to EVK1105 by Ripagood, //Elias Ventura

This NES emulator requires an SDCARD formatted in FAT16 with atleast 4 .nes games and 2 .png images 320*240
The tested roms are Super Mario Bros, Volleyball, Bomberman and official NES Test. Other games may or may not work.

To remove graphical glitches at the cost of speed, remove the added parts in ppu_step

This project makes use of external SDRAM, FAT for SDCARD and the screen.
For any contact try ripagood@gmail.com


Original NES implementation by Rupert Shuttleworth


// This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "board.h"
#include "gpio.h"
#include "power_clocks_lib.h"
#include "et024006dhu.h"
#include "delay.h"
#include "avr32_logo.h"
#include "conf_clock.h"
#include <stdio.h>
#include "iggy.h"
#include "tc.h"
#include "conf_clock.h"
#include "usart.h"
#include <stdlib.h>
#include "Practica4Lab.h"
#include "spi.h"
#include "conf_sd_mmc_spi.h"
#include "sd_mmc_spi.h"
#include "pdca.h"
#include "inc6502.h"
#include "fs_com.h"
#include "navigation.h"
#include "nav_filterlist.h"
#include "NES1\nes.h"
#include "sdramc.h"
#include "file.h"
#include "flashc.h"
#include "NES1\nes_type.h"

#include "lodepng.h"



#define PBA_HZ FOSC0
#define PRACTICA 3

volatile int enter=0;
volatile int end_of_transfer=0;

volatile char password[6];



// PDCA Channel pointer
volatile avr32_pdca_channel_t* pdca_channel_usart ;

// PDCA Channel pointer
volatile avr32_pdca_channel_t* pdca_channelrx ;
volatile avr32_pdca_channel_t* pdca_channeltx ;

// Dummy char table
const char dummy_data[] =
#include "dummy.h"
;
volatile char ram_buffer[516];
/*! \brief Initialize PDCA (Peripheral DMA Controller A) resources for the SPI transfer and start a dummy transfer
 */
char mensaje[]={""};
char msj[] = "A       TXT";
char *ptr_msj = &msj[0];
static char str_buff[40] = "A       TXT";

#define FREQ 66000000 //80 MDIPS! 
//#define FREQ 132000000
#define  RGB(r,g,b) r<<11|g<<5|b 

#  define EXAMPLE_USART                 (&AVR32_USART0)
#  define EXAMPLE_USART_RX_PIN          AVR32_USART0_RXD_0_0_PIN
#  define EXAMPLE_USART_RX_FUNCTION     AVR32_USART0_RXD_0_0_FUNCTION
#  define EXAMPLE_USART_TX_PIN          AVR32_USART0_TXD_0_0_PIN
#  define EXAMPLE_USART_TX_FUNCTION     AVR32_USART0_TXD_0_0_FUNCTION
#  define EXAMPLE_USART_CLOCK_MASK      AVR32_USART0_CLK_PBA
#  define EXAMPLE_PDCA_CLOCK_HSB        AVR32_PDCA_CLK_HSB
#  define EXAMPLE_PDCA_CLOCK_PB         AVR32_PDCA_CLK_PBA

#define PDCA_CHANNEL_USART_EXAMPLE 2
#define AVR32_PDCA_CHANNEL_SPI_TX 1 //CANAL 1 para SPI TX
#define AVR32_PDCA_CHANNEL_SPI_RX 0 //CANAL 2 para SPI RX

//extern uint8_t read6502(uint16_t address);
extern uint16_t pc;
 extern uint8_t sp, a, x, y, status;

 #define PIXEL uint16_t
 #define AVERAGE(a, b)   (PIXEL)( (a) == (b) ? (a) \
 : (((a) & 0xf7dfU) + ((b) & 0xf7dfU)) >> 1 )
void scale_by_denom(PIXEL *Target, PIXEL *Source, int SrcWidth, int SrcHeight,int denom);
void lodepng_decode_display(unsigned char* table,size_t buffersize,int scale);



extern uint8_t opcode;
uint8_t readROM(uint16_t address);
static void init_sys_clocks(void);
#if BOARD == EVK1105
#include "pwm.h"
#include <string.h>
avr32_pwm_channel_t pwm_channel6 = {
/*
  .cmr = ((PWM_MODE_LEFT_ALIGNED << AVR32_PWM_CMR_CALG_OFFSET)
    | (PWM_POLARITY_HIGH << AVR32_PWM_CMR_CPOL_OFFSET)
    | (PWM_UPDATE_DUTY << AVR32_PWM_CMR_CPD_OFFSET)
    | AVR32_PWM_CMR_CPRE_MCK_DIV_2),
    */
  //.cdty = 0,
  .cdty = 0,
  .cprd = 100
};


__attribute__((__section__(".bss_sdram")))
NES nes;
//__attribute__((__section__(".bss_sdram")))
char* imagen;
//__attribute__((__section__(".bss_sdram")))
char* imagen1;

 



static void tft_bl_init(void)
{

  pwm_opt_t opt = {
    .diva = 0,
    .divb = 0,
    .prea = 0,
    .preb = 0
  };
  /* MCK = OSC0 = 12MHz
   * Desired output 60kHz
   * Chosen MCK_DIV_2
   * CPRD = 12MHz / (60kHz * 2) = 100
   *
   * The duty cycle is 100% (CPRD = CDTY)
   * */
  pwm_init(&opt);
  pwm_channel6.CMR.calg = PWM_MODE_LEFT_ALIGNED;
  pwm_channel6.CMR.cpol = PWM_POLARITY_HIGH; //PWM_POLARITY_LOW;//PWM_POLARITY_HIGH;
  pwm_channel6.CMR.cpd = PWM_UPDATE_DUTY;
  pwm_channel6.CMR.cpre = AVR32_PWM_CMR_CPRE_MCK_DIV_2;

  pwm_channel_init(6, &pwm_channel6);
  pwm_start_channels(AVR32_PWM_ENA_CHID6_MASK);

}
#endif






volatile U32 tc_tick = 0;

volatile int segundos =0;
volatile int minutos =0;
volatile int horas=0;

volatile int gana2=1;
__attribute__((__interrupt__))
static void pdca_int_handler_USART(void){
	Disable_global_interrupt();
	
	usart_write_line(&AVR32_USART0,"recibido");
	
	pdca_disable_interrupt_transfer_complete(PDCA_CHANNEL_USART_EXAMPLE);
	pdca_disable(PDCA_CHANNEL_USART_EXAMPLE);
	end_of_transfer=1;
	Enable_global_interrupt();
	
}



__attribute__((__interrupt__))
static void pdca_int_handler_SD(void)
{
  // Disable all interrupts.
  Disable_global_interrupt();

  // Disable interrupt channel.
  pdca_disable_interrupt_transfer_complete(AVR32_PDCA_CHANNEL_SPI_RX);

  sd_mmc_spi_read_close_PDCA();//unselects the SD/MMC memory.
  wait();
  // Disable unnecessary channel
  pdca_disable(AVR32_PDCA_CHANNEL_SPI_TX);
  pdca_disable(AVR32_PDCA_CHANNEL_SPI_RX);

  // Enable all interrupts.
  Enable_global_interrupt();

  end_of_transfer = 1;
}
__attribute__((__interrupt__))
static void tc_irq(void)
{
  // Increment the ms seconds counter
  tc_tick++;

  // Clear the interrupt flag. This is a side effect of reading the TC SR.
  tc_read_sr(EXAMPLE_TC, TC_CHANNEL);

  // Toggle a GPIO pin (this pin is used as a regular GPIO pin).
  if (tc_tick>=1000)
  {		
	   gpio_tgl_gpio_pin(LED0_GPIO);
		  tc_tick=0;
  }
  

}

 volatile avr32_tc_t *tc =  (&AVR32_TC);
// Main function
int main(void)
{
  U32 i;
  
    // Set CPU and PBA clock
  //  pcl_switch_to_osc(PCL_OSC0, FOSC0, OSC0_STARTUP);
  init_sys_clocks();
  //volatile avr32_tc_t *tc = EXAMPLE_TC;
  //volatile avr32_tc_t *tc =  (&AVR32_TC);

/*
  // Options for waveform genration.
  static const tc_waveform_opt_t WAVEFORM_OPT =
  {
	  .channel  = TC_CHANNEL,                        // Channel selection. 0 

	  .bswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOB.
	  .beevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOB.
	  .bcpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOB.
	  .bcpb     = TC_EVT_EFFECT_NOOP,                // RB compare effect on TIOB.

	  .aswtrg   = TC_EVT_EFFECT_NOOP,                // Software trigger effect on TIOA.
	  .aeevt    = TC_EVT_EFFECT_NOOP,                // External event effect on TIOA.
	  .acpc     = TC_EVT_EFFECT_NOOP,                // RC compare effect on TIOA: toggle.
	  .acpa     = TC_EVT_EFFECT_NOOP,                  // RA compare effect on TIOA: toggle (other possibilities are none, set and clear).

	  .wavsel   = TC_WAVEFORM_SEL_UP_MODE_RC_TRIGGER,// Waveform selection: Up mode with automatic trigger(reset) on RC compare.
	  .enetrg   = FALSE,                                          // External event trigger enable.
	  .eevt     = 0,                                                    // External event selection.
	  .eevtedg  = TC_SEL_NO_EDGE,                   // External event edge selection.
	  .cpcdis   = FALSE,                                         // Counter disable when RC compare.
	  .cpcstop  = FALSE,                                        // Counter clock stopped with RC compare.

	  .burst    = FALSE,                                           // Burst signal selection.
	  .clki     = FALSE,                                            // Clock inversion.
	  .tcclks   = TC_CLOCK_SOURCE_TC3         // Internal source clock 3, connected to fPBA / 8.
  };

  static const tc_interrupt_t TC_INTERRUPT =
  {
	  .etrgs = 0,
	  .ldrbs = 0,
	  .ldras = 0,
	  .cpcs  = 1,   // Habilitar interrupción por comparación con RC
	  .cpbs  = 0,
	  .cpas  = 0,
	  .lovrs = 0,
	  .covfs = 0
  };


*/

  gpio_enable_gpio_pin(LED0_GPIO);
  gpio_enable_gpio_pin(LED1_GPIO);
  gpio_enable_gpio_pin(LED2_GPIO);
  gpio_enable_gpio_pin(LED3_GPIO);

 // et024006_Init( FOSC0, FOSC0 );
  et024006_Init( FREQ, FREQ );

#if BOARD == EVK1105
  /* PWM is fed by PBA bus clock which is by default the same
   * as the CPU speed. We set a 0 duty cycle and thus keep the
   * display black*/
  tft_bl_init();
#elif BOARD == EVK1104 || BOARD == UC3C_EK
  gpio_set_gpio_pin(ET024006DHU_BL_PIN);
#endif

  // Clear the display i.e. make it black
  et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
  
  //Interrupciones
  
  Disable_global_interrupt();

  INTC_init_interrupts();
 // INTC_register_interrupt(&tc_irq, AVR32_TC_IRQ0, AVR32_INTC_INT0);
  init_Usart_DMA_RX();
  init_SD_DMA_RX();
  
  //sdramc_init(FOSC0);
  sdramc_init(FREQ);
  Enable_global_interrupt();
  
 // tc_init_waveform(tc, &WAVEFORM_OPT);         // Initialize the timer/counter waveform.

  // Set the compare triggers.
  // Remember TC counter is 16-bits, so counting second is not possible with fPBA = 12 MHz.
  // We configure it to count ms.
  // We want: (1/(fPBA/8)) * RC = 0.001 s, hence RC = (fPBA/8) / 1000 = 1500 to get an interrupt every 1 ms.
  //tc_write_rc(tc, TC_CHANNEL, 1500);            // Set RC value.

 // tc_configure_interrupts(tc, TC_CHANNEL, &TC_INTERRUPT);

  // Start the timer/counter.
 // tc_start(tc, TC_CHANNEL);                    // And start the timer/counter.
  


  while(pwm_channel6.cdty < pwm_channel6.cprd)
  {
	  pwm_channel6.cdty++;
	  pwm_channel6.cupd = pwm_channel6.cdty;
	  //pwm_channel6.cdty--;
	  pwm_async_update_channel(AVR32_PWM_ENA_CHID6, &pwm_channel6);
	  delay_ms(10);
  }
  
  
  // Initialize SD/MMC driver resources: GPIO, SPI and SD/MMC.
  sd_mmc_resources_init();

  // Wait for a card to be inserted
  //while (!sd_mmc_spi_mem_check());
 
 
 uint8_t hayClave=0;
  uint32_t sector =10;
  char disp[50];
  char clave[7];
  
  // Mount file system.
  nav_reset();

  // Display # of drives
  int drives = nav_drive_nb();
  sprintf(mensaje,"%d",drives);
  et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,50,70,BLUE,-1);

  // Set Drive #
  nav_drive_set(0);

  // Display Drive #
  drives = nav_drive_get();
  sprintf(mensaje,"%d",drives);
  et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,70,70,BLUE,-1);

  while(!nav_partition_mount()); // If we're unable to mount the partition, halt here.

  nav_checkdisk_disable(); // Skip fat_check_device() for every read.

  // Display # of files in drive
  drives = nav_filelist_nb(FS_FILE);
  sprintf(mensaje,"%d",drives);
  et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,90,70,BLUE,-1);

  // Display # of bytes in file
  if(nav_filelist_first(FS_FILE))
  {
	  drives = nav_file_lgt();
	  sprintf(mensaje,"%d",drives);
	  et024006_PrintString(mensaje,(const unsigned char*)&FONT8x8,110,70,BLUE,-1);
  }
  
  
   //Apply a filter for “*.png” files and display the all the files’ names.
   //Muestra primer imagen .png
  
 

 
   
   nav_filterlist_setfilter("png");
   nav_filterlist_root();
    nav_filterlist_goto(0);
    nav_file_getname((FS_STRING) str_buff, 40);
	uint32_t size1= nav_file_lgt(); //tamaño de la imagen a cargar
	file_bof();
	file_open(FOPEN_MODE_R);
	imagen=(char*)malloc(size1);
	file_read_buf(imagen,size1);
	lodepng_decode_display((unsigned char *)imagen,size1,1);//decodifca la PNG
	file_close();
	
	nav_filterlist_goto(1);
	nav_file_getname((FS_STRING) str_buff, 40);
	uint32_t size2= nav_file_lgt(); //tamaño de la imagen a cargar
	file_bof();
	file_open(FOPEN_MODE_R);
	imagen1=(char*)malloc(size2);
	file_read_buf(imagen1,size2);
	file_close();


  //Apply a filter for “*.txt” files and display number of files with this file extension
  int wave = nav_filterlist_nb(FL_FILE,"nes");
  nav_filterlist_setfilter("nes");
  
  
   
  
  
  //et024006_DrawFilledRect(0,0,320,240,BLACK);
  int l=0;
 

  nav_filterlist_root();
//soccer , donkey , test , mario
  for(i=0;i<wave;i++)
  {
	  nav_filterlist_goto(i);
	  if(nav_file_getname((FS_STRING) str_buff, 40))
	  et024006_PrintString(str_buff,(const unsigned char*)&FONT8x8,40,80+(l+=10),BLUE,-1);
  }
  
  
  while (!gpio_get_pin_value(QT1081_TOUCH_SENSOR_ENTER))
  {
	  
	  if (debounce2(QT1081_TOUCH_SENSOR_UP))
	  {
		  CLR_disp();
		  lodepng_decode_display((unsigned char *)imagen,size1,2);//decodifca la PNG
		  nav_filterlist_goto(0);
		  if(nav_file_getname((FS_STRING) str_buff, 40))
		  et024006_PrintString(str_buff,(const unsigned char*)&FONT8x8,40,80,RED,-1);
	  }
	  
	   if (debounce2(QT1081_TOUCH_SENSOR_DOWN))
	   {
		   CLR_disp();
		   lodepng_decode_display((unsigned char *)imagen1,size2,1);//decodifca la PNG
		   nav_filterlist_goto(1);
		   if(nav_file_getname((FS_STRING) str_buff, 40))
		   et024006_PrintString(str_buff,(const unsigned char*)&FONT8x8,40,80,RED,-1);
	   }
	    if (debounce2(QT1081_TOUCH_SENSOR_RIGHT))
	    {
		    CLR_disp();
			lodepng_decode_display((unsigned char *)imagen1,size2,1);//decodifca la PNG
		    nav_filterlist_goto(2);
		    if(nav_file_getname((FS_STRING) str_buff, 40))
		    et024006_PrintString(str_buff,(const unsigned char*)&FONT8x8,40,80,RED,-1);
	    }
		 if (debounce2(QT1081_TOUCH_SENSOR_LEFT))
		 {
			 CLR_disp();
			 lodepng_decode_display((unsigned char *)imagen,size1,2);//decodifca la PNG
			 nav_filterlist_goto(3);
			 if(nav_file_getname((FS_STRING) str_buff, 40))
			 et024006_PrintString(str_buff,(const unsigned char*)&FONT8x8,40,80,RED,-1);
		 }
	  
	  
	  
  }
  
  free(imagen);
  free(imagen1);

  
  
  
  
 
 char z;

 usart_write_char(&AVR32_USART0,'d');
 

 file_bof();
file_open(FOPEN_MODE_R);
usart_write_char(&AVR32_USART0,fs_g_status);

 int filename =3;
 #define DEFAULT_RESOLUTION_WIDTH 256
 #define DEFAULT_RESOLUTION_HEIGHT 240
 int width = DEFAULT_RESOLUTION_WIDTH;
 int height = DEFAULT_RESOLUTION_HEIGHT;
  nes = nes_init(filename, width, height);
  while (1)
  {
	   nes_run(nes);
  }
  
 
  nes_destroy(nes);

  }



void CLR_disp(void)
{
	// Clear the display i.e. make it black
	et024006_DrawFilledRect(0 , 0, ET024006_WIDTH, ET024006_HEIGHT, BLACK );
}


uint32_t debounce2( uint32_t GPIO_PIN ){//regresar se presiono el boton o no
	if(gpio_get_pin_value(GPIO_PIN)==1){// se presiono el boton?, sino salir de la funcion
		delay_ms(10);
		if (gpio_get_pin_value(GPIO_PIN)==0){//Si ya se libero, es ruido, salir sin hacer nada
			goto salir;
		}
		espera://espera a que suelte el botón
		while (gpio_get_pin_value(GPIO_PIN)==1){}
		delay_ms(10);
		if (gpio_get_pin_value(GPIO_PIN)==1) {//si ya lo presiono otra vez , es ruido, regresa a esperar
			goto espera;
		}
		return 1;//debounce completo regresa 1
	}
	salir:
	return 0;
}

static void sd_mmc_resources_init(void)
{
	// GPIO pins used for SD/MMC interface
	static const gpio_map_t SD_MMC_SPI_GPIO_MAP =
	{
		{SD_MMC_SPI_SCK_PIN,  SD_MMC_SPI_SCK_FUNCTION },  // SPI Clock.
		{SD_MMC_SPI_MISO_PIN, SD_MMC_SPI_MISO_FUNCTION},  // MISO.
		{SD_MMC_SPI_MOSI_PIN, SD_MMC_SPI_MOSI_FUNCTION},  // MOSI.
		{SD_MMC_SPI_NPCS_PIN, SD_MMC_SPI_NPCS_FUNCTION}   // Chip Select NPCS.
	};

	// SPI options.
	spi_options_t spiOptions =
	{
		.reg          = SD_MMC_SPI_NPCS,
		.baudrate     = SD_MMC_SPI_MASTER_SPEED,  // Defined in conf_sd_mmc_spi.h.
		.bits         = SD_MMC_SPI_BITS,          // Defined in conf_sd_mmc_spi.h.
		.spck_delay   = 0,
		.trans_delay  = 0,
		.stay_act     = 1,
		.spi_mode     = 0,
		.modfdis      = 1
	};

	// Assign I/Os to SPI.
	gpio_enable_module(SD_MMC_SPI_GPIO_MAP,
	sizeof(SD_MMC_SPI_GPIO_MAP) / sizeof(SD_MMC_SPI_GPIO_MAP[0]));

	// Initialize as master.
	spi_initMaster(SD_MMC_SPI, &spiOptions);

	// Set SPI selection mode: variable_ps, pcs_decode, delay.
	spi_selectionMode(SD_MMC_SPI, 0, 0, 0);

	// Enable SPI module.
	spi_enable(SD_MMC_SPI);

	// Initialize SD/MMC driver with SPI clock (PBA).
	//sd_mmc_spi_init(spiOptions, PBA_HZ);
	sd_mmc_spi_init(spiOptions, FREQ);
}




void init_Usart_DMA_RX(void){
	
	const gpio_map_t usart_gpio_map = {
		{EXAMPLE_USART_RX_PIN, EXAMPLE_USART_RX_FUNCTION},
		{EXAMPLE_USART_TX_PIN, EXAMPLE_USART_TX_FUNCTION}
	};

	const usart_options_t usart_options = {
		.baudrate     = 57600,
		.charlength   = 8,
		.paritytype   = USART_NO_PARITY,
		.stopbits     = USART_1_STOPBIT,
		.channelmode  = USART_NORMAL_CHMODE,
	};

	const pdca_channel_options_t PDCA_OPTIONS = {
		/* Select peripheral - data is transmitted on USART RX line */
		.pid = AVR32_PDCA_PID_USART0_RX,
		/* Select size of the transfer */
		.transfer_size = PDCA_TRANSFER_SIZE_BYTE,

		/* Memory address */
		.addr = (void*)password,
		/* Transfer counter */
		.size = sizeof(password), //6 

		/* Next memory address */
		.r_addr = NULL,
		/* Next transfer counter */
		.r_size = 0,
	};

	/* Assign GPIO pins to USART. */
	gpio_enable_module(usart_gpio_map,
			sizeof(usart_gpio_map) / sizeof(usart_gpio_map[0]));

	/* Initialize the USART in RS232 mode. */
	//usart_init_rs232(EXAMPLE_USART, &usart_options,FOSC0);
	usart_init_rs232(EXAMPLE_USART, &usart_options,FREQ);

	//usart_write_line(EXAMPLE_USART, "PDCA Example.\r\n");

	/* Initialize the PDCA channel with the requested options. */
	pdca_init_channel(PDCA_CHANNEL_USART_EXAMPLE, &PDCA_OPTIONS);
	
	INTC_register_interrupt(&pdca_int_handler_USART, AVR32_PDCA_IRQ_2,
	AVR32_INTC_INT3);
    
	/* Enable PDCA interrupt each time the reload counter reaches zero, i.e.
	 * each time half of the ASCII animation (either anim1 or anim2) is
	 * transferred. */
	//pdca_enable_interrupt_reload_counter_zero(PDCA_CHANNEL_USART_EXAMPLE);
	
	//pdca_enable_interrupt_transfer_complete(PDCA_CHANNEL_USART_EXAMPLE);

}


void init_SD_DMA_RX(void)
{
	
  // this PDCA channel is used for data reception from the SPI
  pdca_channel_options_t pdca_options_SPI_RX ={ // pdca channel options

    .addr = ram_buffer,//RAM
    // memory address. We take here the address of the string dummy_data. This string is located in the file dummy.h

    .size = 512,                              // transfer counter: here the size of the string
    .r_addr = NULL,                           // next memory address after 1st transfer complete
    .r_size = 0,                              // next transfer counter not used here
    .pid = AVR32_PDCA_PID_SPI0_RX,        // select peripheral ID - data are on reception from SPI1 RX line
    .transfer_size = 8 // select size of the transfer: 8,16,32 bits
  };

  // this channel is used to activate the clock of the SPI by sending a dummy variables
  pdca_channel_options_t pdca_options_SPI_TX ={ // pdca channel options

    .addr = (void *)&dummy_data,              // memory address.
                                              // We take here the address of the string dummy_data.
                                              // This string is located in the file dummy.h
    .size = 512,                              // transfer counter: here the size of the string
    .r_addr = NULL,                           // next memory address after 1st transfer complete
    .r_size = 0,                              // next transfer counter not used here
    .pid = AVR32_PDCA_PID_SPI0_TX,        // select peripheral ID - data are on reception from SPI1 RX line
    .transfer_size = 8 // select size of the transfer: 8,16,32 bits
  };

  // Init PDCA transmission channel
  pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_TX, &pdca_options_SPI_TX);

  // Init PDCA Reception channel
  pdca_init_channel(AVR32_PDCA_CHANNEL_SPI_RX, &pdca_options_SPI_RX);

  //! \brief Enable pdca transfer interrupt when completed
  INTC_register_interrupt(&pdca_int_handler_SD, AVR32_PDCA_IRQ_0, AVR32_INTC_INT1);  // pdca_channel_spi1_RX = 0

}


void getClave(char* clave){
		for( int i = 0; i < 6; i++)
		{
			
			clave[i]= ( (U8)(*(ram_buffer + i)));
		}
		clave[6]='\0';
}

// Software wait
void wait(void)
{
	volatile int i;
	for(i = 0 ; i < 5000; i++);
}


static void init_sys_clocks(void)
{
	// Switch to OSC0 to speed up the booting
	pm_switch_to_osc0(&AVR32_PM, FOSC0, OSC0_STARTUP);

	
	// Set PLL0 (fed from OSC0 = 12 MHz) to 132 MHz
	// We use OSC1 since we need a correct master clock for the SSC module to generate
	// the correct sample rate
	pm_pll_setup(&AVR32_PM, 0,  // pll.
	10,  // mul.
	1,   // div.
	0,   // osc.
	16); // lockcount.

	// Set PLL operating range and divider (fpll = fvco/2)
	// -> PLL0 output = 66 MHz
	pm_pll_set_option(&AVR32_PM, 0, // pll.
	1,  // pll_freq.
	1,  // pll_div2.
	0); // pll_wbwdisable.

	// start PLL0 and wait for the lock
	pm_pll_enable(&AVR32_PM, 0);
	pm_wait_for_pll0_locked(&AVR32_PM);
	// Set all peripheral clocks torun at master clock rate
	pm_cksel(&AVR32_PM,
	0,   // pbadiv.
	0,   // pbasel.
	0,   // pbbdiv.
	0,   // pbbsel.
	0,   // hsbdiv.
	0);  // hsbsel.

	// Set one waitstate for the flash
	flashc_set_wait_state(1);

	// Switch to PLL0 as the master clock
	pm_switch_to_clock(&AVR32_PM, AVR32_PM_MCCTRL_MCSEL_PLL0);

}


/*
  * \brief lodepng_decode_display function : This function initialise the
  * PNG decoder, decodes the .PNG image and display it on ET024006 display device
  * with various scaling factors.
  */
 
 void lodepng_decode_display(unsigned char* table,size_t buffersize,int scale)
 {
     unsigned char* image;
     uint8_t r,g,b;
     size_t  imagesize;
     LodePNG_Decoder decoder;
     unsigned int i,j;
     uint16_t color;
     uint16_t* temp16;
 
     LodePNG_Decoder_init(&decoder);
     //decode the png
     LodePNG_decode(&decoder, &image, &imagesize, table, buffersize);
     usart_write_line(&AVR32_USART0,"Decoding completed \r\n");
 
     /* if there's an error, display it, otherwise display
      * information about the image
      */
     if(decoder.error) {
         usart_write_line(&AVR32_USART0,"error: Decoding\r\n");
         return;
     }
     else {
 #if (defined __GNUC__)
         usart_write_line(&AVR32_USART0,"Decoding Successful\r\n");
 #elif (defined __ICCAVR32__)
         printf("w: %d\n", decoder.infoPng.width);
         printf("h: %d\n", decoder.infoPng.height);
         printf("bitDepth: %d\n", decoder.infoPng.color.bitDepth);
         printf("bpp: %d\n", LodePNG_InfoColor_getBpp(&decoder.infoPng.color));
         printf("colorChannels: %d\n", LodePNG_InfoColor_getChannels(
             &decoder.infoPng.color));
         printf("paletteSize: %d\n", decoder.infoPng.color.palettesize);
         printf("colorType: %d\n", decoder.infoPng.color.colorType);
         printf("compressionMethod: %d\n", decoder.infoPng.compressionMethod);
         printf("filterMethod: %d\n", decoder.infoPng.filterMethod);
         printf("interlaceMethod: %d\n", decoder.infoPng.interlaceMethod);
         for(i = 0; i < decoder.infoPng.text.num; i++)
         printf("%s: %s\n",  decoder.infoPng.text.keys[i],
             decoder.infoPng.text.strings[i]);
         for(i = 0; i < decoder.infoPng.itext.num; i++)
         printf("%s (%s %s) : %s\n",  decoder.infoPng.itext.keys[i],
             decoder.infoPng.itext.langtags[i], decoder.infoPng.itext.transkeys[i],
             decoder.infoPng.itext.strings[i]);
 
         if(decoder.infoPng.time_defined) {
             printf("modification time: %d-%d-%d %d:%d:%d\n",
                 decoder.infoPng.time.year, decoder.infoPng.time.month,
                 decoder.infoPng.time.day, decoder.infoPng.time.hour,
                 decoder.infoPng.time.minute, decoder.infoPng.time.second);
         }
 
         if(decoder.infoPng.phys_defined) {
             printf("physical size: %d %d %d\n", decoder.infoPng.phys_x,
                 decoder.infoPng.phys_y, (int)decoder.infoPng.phys_unit);
         }
 #endif
 
     }
    usart_write_line(&AVR32_USART0,"sending to display\r\n");
 
     // changing to 16-bit 565
     temp16 = (uint16_t *) image;
     for(i=0,j=0;i<(decoder.infoPng.width*decoder.infoPng.height*4);i=i+4,j++) {
         r= (uint8_t)*(image+i);
         g = (uint8_t)*(image+i+1);
         b = (uint8_t)*(image+i+2);
         color = et024006_Color(r,g,b);
         temp16[j] = color;
     }
 
     //call scaling if required -supports only 1/2,1/4,1/8
     if(scale>1) {
         scale_by_denom(temp16,temp16,320,240,scale);
     }
 
     et024006_DrawFilledRect(0,0,ET024006_WIDTH,ET024006_HEIGHT,0x2458 );
 
     usart_write_line(&AVR32_USART0,"sending to display\r\n");
     et024006_PutPixmap( (uint16_t *)temp16, 320/scale, 0, 0, 0, 0, 320/scale,
         240/scale );
     free(image);
 
     // cleanup decoder
     LodePNG_Decoder_cleanup(&decoder);
     return;
 }

 
 
 
 // supports scaling by 2,4,8 in denominator to 1
 void scale_by_denom(PIXEL *Target, PIXEL *Source, int SrcWidth,
     int SrcHeight,int denom)
 {
     int x, y, x2, y2;
     int TgtWidth, TgtHeight;
     PIXEL p,p1,q,q1,r,r1,s,s1;
 
     TgtWidth = SrcWidth / denom;
     TgtHeight = SrcHeight / denom;
 
     // Box filter method
     for (y = 0; y < TgtHeight; y++) {
         y2 = denom * y;
         for (x = 0; x < TgtWidth; x++) {
             x2 = denom * x;
             p = AVERAGE(Source[y2*SrcWidth + x2], Source[y2*SrcWidth +
                 x2 + 1]);
             q = AVERAGE(Source[(y2+1)*SrcWidth + x2], Source[(y2+1)*SrcWidth +
                 x2 + 1]);
 
             if (denom==4) {
                 r = AVERAGE(Source[(y2+2)*SrcWidth + x2],
                     Source[(y2+2)*SrcWidth + x2 + 1]);
                 s = AVERAGE(Source[(y2+3)*SrcWidth + x2],
                     Source[(y2+3)*SrcWidth + x2 + 1]);
                 p = AVERAGE(p,r);
                 q = AVERAGE(q,s);
             }
             else if(denom==8) {
                 r = AVERAGE(Source[(y2+2)*SrcWidth + x2],
                     Source[(y2+2)*SrcWidth + x2 + 1]);
                 s = AVERAGE(Source[(y2+3)*SrcWidth + x2],
                     Source[(y2+3)*SrcWidth + x2 + 1]);
                 p = AVERAGE(p,r);
                 q = AVERAGE(q,s);
                 p1 = AVERAGE(Source[(y2+4)*SrcWidth + x2],
                     Source[(y2+4)*SrcWidth + x2 + 1]);
                 q1 = AVERAGE(Source[(y2+5)*SrcWidth + x2],
                     Source[(y2+5)*SrcWidth + x2 + 1]);
                 r1 = AVERAGE(Source[(y2+6)*SrcWidth + x2],
                     Source[(y2+6)*SrcWidth + x2 + 1]);
                 s1 = AVERAGE(Source[(y2+7)*SrcWidth + x2],
                     Source[(y2+7)*SrcWidth + x2 + 1]);
                 p1 = AVERAGE(p1,r1);
                 q1 = AVERAGE(q1,s1);
                 p =  AVERAGE(p,p1);
                 q = AVERAGE(q,q1);
             }
             Target[y*TgtWidth + x] = AVERAGE(p, q);
         } // for
     } // for
 }

 

