
#include <mcu/arch.h>
#include <mcu/arch/stm32/stm32f7xx/stm32f7xx_hal.h>

#define  RK043FN48H_WIDTH ((uint16_t)480)
#define  RK043FN48H_HEIGHT ((uint16_t)272)

#define  RK043FN48H_HSYNC ((uint16_t)41)
#define  RK043FN48H_HBP ((uint16_t)13)
#define  RK043FN48H_HFP ((uint16_t)32)
#define  RK043FN48H_VSYNC ((uint16_t)10)
#define  RK043FN48H_VBP ((uint16_t)2)
#define  RK043FN48H_VFP ((uint16_t)2)

#define  RK043FN48H_FREQUENCY_DIVIDER 5

#define LCD_DISP_PIN                    GPIO_PIN_12
#define LCD_DISP_GPIO_PORT              GPIOI
#define LCD_DISP_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOI_CLK_ENABLE()
#define LCD_DISP_GPIO_CLK_DISABLE()     __HAL_RCC_GPIOI_CLK_DISABLE()

#define LCD_BL_CTRL_PIN                  GPIO_PIN_3
#define LCD_BL_CTRL_GPIO_PORT            GPIOK
#define LCD_BL_CTRL_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOK_CLK_ENABLE()
#define LCD_BL_CTRL_GPIO_CLK_DISABLE()   __HAL_RCC_GPIOK_CLK_DISABLE()

void LCD_Config(void)
{
	static LTDC_HandleTypeDef hltdc_F;
	LTDC_LayerCfgTypeDef      pLayerCfg;

	/* LTDC Initialization -------------------------------------------------------*/

	/* Polarity configuration */
	/* Initialize the horizontal synchronization polarity as active low */
	hltdc_F.Init.HSPolarity = LTDC_HSPOLARITY_AL;
	/* Initialize the vertical synchronization polarity as active low */
	hltdc_F.Init.VSPolarity = LTDC_VSPOLARITY_AL;
	/* Initialize the data enable polarity as active low */
	hltdc_F.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	/* Initialize the pixel clock polarity as input pixel clock */
	hltdc_F.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

	/* The RK043FN48H LCD 480x272 is selected */
	/* Timing Configuration */
	hltdc_F.Init.HorizontalSync = (RK043FN48H_HSYNC - 1);
	hltdc_F.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
	hltdc_F.Init.AccumulatedHBP = (RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hltdc_F.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc_F.Init.AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc_F.Init.AccumulatedActiveW = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP - 1);
	hltdc_F.Init.TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
	hltdc_F.Init.TotalWidth = (RK043FN48H_WIDTH + RK043FN48H_HSYNC + RK043FN48H_HBP + RK043FN48H_HFP - 1);

	/* Configure R,G,B component values for LCD background color : all black background */
	hltdc_F.Init.Backcolor.Blue = 0;
	hltdc_F.Init.Backcolor.Green = 0;
	hltdc_F.Init.Backcolor.Red = 0;

	hltdc_F.Instance = LTDC;

	/* Layer1 Configuration ------------------------------------------------------*/

	/* Windowing configuration */
	/* In this case all the active display area is used to display a picture then :
	  Horizontal start = horizontal synchronization + Horizontal back porch = 43
	  Vertical start   = vertical synchronization + vertical back porch     = 12
	  Horizontal stop = Horizontal start + window width -1 = 43 + 480 -1
	  Vertical stop   = Vertical start + window height -1  = 12 + 272 -1      */
	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = 480;
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = 272;

	/* Pixel Format configuration*/
	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;

	/* Start Address configuration : frame buffer is located at SDRAM memory */
	pLayerCfg.FBStartAdress = 0xC0000000;

	/* Alpha constant (255 == totally opaque) */
	pLayerCfg.Alpha = 255;

	/* Default Color configuration (configure A,R,G,B component values) : no background color */
	pLayerCfg.Alpha0 = 0; /* fully transparent */
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;

	/* Configure blending factors */
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

	/* Configure the number of lines and number of pixels per line */
	pLayerCfg.ImageWidth  = 480;
	pLayerCfg.ImageHeight = 272;

	/* Configure the LTDC */
	if(HAL_LTDC_Init(&hltdc_F) != HAL_OK){
		/* Initialization Error */
		while(1){

		}
	}

	/* Configure the Layer*/
	if(HAL_LTDC_ConfigLayer(&hltdc_F, &pLayerCfg, 1) != HAL_OK){
		/* Initialization Error */
		while(1){

		}
	}
}


void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc)
{
	GPIO_InitTypeDef GPIO_Init_Structure;

	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/*##-1- Enable peripherals and GPIO Clocks #################################*/
	/* Enable the LTDC Clock */
	__HAL_RCC_LTDC_CLK_ENABLE();

	/*##-2- Configure peripheral GPIO ##########################################*/
	/******************** LTDC Pins configuration *************************/
	/* Enable GPIOs clock */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	/*** LTDC Pins configuration ***/
	/* GPIOE configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_4;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Pull      = GPIO_NOPULL;
	GPIO_Init_Structure.Speed     = GPIO_SPEED_FAST;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOE, &GPIO_Init_Structure);

	/* GPIOG configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_12;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Alternate = GPIO_AF9_LTDC;
	HAL_GPIO_Init(GPIOG, &GPIO_Init_Structure);

	/* GPIOI LTDC alternate configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_9 | GPIO_PIN_10 | \
			GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

	/* GPIOJ configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | \
			GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | \
			GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | \
			GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOJ, &GPIO_Init_Structure);

	/* GPIOK configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4 | \
			GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_Init_Structure.Mode      = GPIO_MODE_AF_PP;
	GPIO_Init_Structure.Alternate = GPIO_AF14_LTDC;
	HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);

	/* LCD_DISP GPIO configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_12;     /* LCD_DISP pin has to be manually controlled */
	GPIO_Init_Structure.Mode      = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOI, &GPIO_Init_Structure);

	/* LCD_BL_CTRL GPIO configuration */
	GPIO_Init_Structure.Pin       = GPIO_PIN_3;  /* LCD_BL_CTRL pin has to be manually controlled */
	GPIO_Init_Structure.Mode      = GPIO_MODE_OUTPUT_PP;
	HAL_GPIO_Init(GPIOK, &GPIO_Init_Structure);

	/* Assert display enable LCD_DISP pin */
	HAL_GPIO_WritePin(LCD_DISP_GPIO_PORT, LCD_DISP_PIN, GPIO_PIN_SET);

	/* Assert backlight LCD_BL_CTRL pin */
	HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_PORT, LCD_BL_CTRL_PIN, GPIO_PIN_SET);

}

/**
  * @brief LTDC MSP De-Initialization
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  * @param hltdc: LTDC handle pointer
  * @retval None
  */
void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc)
{

	/*##-1- Reset peripherals ##################################################*/
	/* Enable LTDC reset state */
	__HAL_RCC_LTDC_FORCE_RESET();

	/* Release LTDC from reset state */
	__HAL_RCC_LTDC_RELEASE_RESET();
}
