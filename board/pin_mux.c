/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Pins v7.0
processor: MK64FN1M0xxx12
package_id: MK64FN1M0VLQ12
mcu_data: ksdk2_0
processor_version: 7.0.1
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

#include "fsl_common.h"
#include "fsl_port.h"
#include "pin_mux.h"

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitBootPins
 * Description   : Calls initialization functions.
 *
 * END ****************************************************************************************************************/
void BOARD_InitBootPins(void)
{
    BOARD_InitPins();
}

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_InitPins:
- options: {callFromInitBoot: 'true', coreID: core0, enableClock: 'true'}
- pin_list:
  - {pin_num: '1', peripheral: SDHC, signal: 'DATA, 1', pin_signal: ADC1_SE4a/PTE0/SPI1_PCS1/UART1_TX/SDHC0_D1/TRACE_CLKOUT/I2C1_SDA/RTC_CLKOUT, slew_rate: fast,
    open_drain: disable, drive_strength: high, pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '2', peripheral: SDHC, signal: 'DATA, 0', pin_signal: ADC1_SE5a/PTE1/LLWU_P0/SPI1_SOUT/UART1_RX/SDHC0_D0/TRACE_D3/I2C1_SCL/SPI1_SIN, slew_rate: fast,
    open_drain: disable, drive_strength: high, pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '3', peripheral: SDHC, signal: DCLK, pin_signal: ADC0_DP2/ADC1_SE6a/PTE2/LLWU_P1/SPI1_SCK/UART1_CTS_b/SDHC0_DCLK/TRACE_D2, slew_rate: fast, open_drain: disable,
    drive_strength: high, pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '4', peripheral: SDHC, signal: CMD, pin_signal: ADC0_DM2/ADC1_SE7a/PTE3/SPI1_SIN/UART1_RTS_b/SDHC0_CMD/TRACE_D1/SPI1_SOUT, slew_rate: fast, open_drain: disable,
    drive_strength: high, pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '7', peripheral: SDHC, signal: 'DATA, 3', pin_signal: PTE4/LLWU_P2/SPI1_PCS0/UART3_TX/SDHC0_D3/TRACE_D0, slew_rate: fast, open_drain: disable, drive_strength: high,
    pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '8', peripheral: SDHC, signal: 'DATA, 2', pin_signal: PTE5/SPI1_PCS2/UART3_RX/SDHC0_D2/FTM3_CH0, slew_rate: fast, open_drain: disable, drive_strength: high,
    pull_select: up, pull_enable: enable, passive_filter: disable}
  - {pin_num: '105', peripheral: GPIOC, signal: 'GPIO, 2', pin_signal: ADC0_SE4b/CMP1_IN0/PTC2/SPI0_PCS2/UART1_CTS_b/FTM0_CH1/FB_AD12/I2S0_TX_FS}
  - {pin_num: '106', peripheral: GPIOC, signal: 'GPIO, 3', pin_signal: CMP1_IN1/PTC3/LLWU_P7/SPI0_PCS1/UART1_RX/FTM0_CH2/CLKOUT/I2S0_TX_BCLK}
  - {pin_num: '128', peripheral: SPI0, signal: SCK, pin_signal: ADC0_SE5b/PTD1/SPI0_SCK/UART2_CTS_b/FTM3_CH1/FB_CS0_b}
  - {pin_num: '130', peripheral: SPI0, signal: SIN, pin_signal: PTD3/SPI0_SIN/UART2_TX/FTM3_CH3/FB_AD3/I2C0_SDA}
  - {pin_num: '129', peripheral: SPI0, signal: SOUT, pin_signal: PTD2/LLWU_P13/SPI0_SOUT/UART2_RX/FTM3_CH2/FB_AD4/I2C0_SCL, drive_strength: low}
  - {pin_num: '127', peripheral: SPI0, signal: PCS0_SS, pin_signal: PTD0/LLWU_P12/SPI0_PCS0/UART2_RTS_b/FTM3_CH0/FB_ALE/FB_CS1_b/FB_TS_b, drive_strength: low}
  - {pin_num: '103', peripheral: GPIOC, signal: 'GPIO, 0', pin_signal: ADC0_SE14/PTC0/SPI0_PCS4/PDB0_EXTRG/USB_SOF_OUT/FB_AD14/I2S0_TXD1}
  - {pin_num: '104', peripheral: GPIOC, signal: 'GPIO, 1', pin_signal: ADC0_SE15/PTC1/LLWU_P6/SPI0_PCS3/UART1_RTS_b/FTM0_CH0/FB_AD13/I2S0_TXD0}
  - {pin_num: '97', peripheral: FTM2, signal: 'QD_PH, A', pin_signal: PTB18/CAN0_TX/FTM2_CH0/I2S0_TX_BCLK/FB_AD15/FTM2_QD_PHA, pull_select: up, pull_enable: enable}
  - {pin_num: '98', peripheral: FTM2, signal: 'QD_PH, B', pin_signal: PTB19/CAN0_RX/FTM2_CH1/I2S0_TX_FS/FB_OE_b/FTM2_QD_PHB, pull_select: up, pull_enable: enable}
  - {pin_num: '45', peripheral: GPIOE, signal: 'GPIO, 24', pin_signal: ADC0_SE17/PTE24/UART4_TX/I2C0_SCL/EWM_OUT_b}
  - {pin_num: '46', peripheral: GPIOE, signal: 'GPIO, 25', pin_signal: ADC0_SE18/PTE25/UART4_RX/I2C0_SDA/EWM_IN}
  - {pin_num: '19', peripheral: USB0, signal: DP, pin_signal: USB0_DP}
  - {pin_num: '20', peripheral: USB0, signal: DM, pin_signal: USB0_DM}
  - {pin_num: '38', peripheral: DAC0, signal: OUT, pin_signal: DAC0_OUT/CMP1_IN3/ADC0_SE23}
  - {pin_num: '39', peripheral: DAC1, signal: OUT, pin_signal: DAC1_OUT/CMP0_IN4/CMP2_IN3/ADC1_SE23}
  - {pin_num: '125', peripheral: GPIOC, signal: 'GPIO, 18', pin_signal: PTC18/UART3_RTS_b/ENET0_1588_TMR2/FB_TBST_b/FB_CS2_b/FB_BE15_8_BLS23_16_b, open_drain: enable}
  - {pin_num: '50', peripheral: JTAG, signal: JTAG_TCLK_SWD_CLK, pin_signal: PTA0/UART0_CTS_b/UART0_COL_b/FTM0_CH5/JTAG_TCLK/SWD_CLK/EZP_CLK}
  - {pin_num: '60', peripheral: FTM1, signal: 'QD_PH, A', pin_signal: ADC0_SE11/PTA8/FTM1_CH0/FTM1_QD_PHA/TRACE_D2, pull_select: up, pull_enable: enable}
  - {pin_num: '61', peripheral: FTM1, signal: 'QD_PH, B', pin_signal: PTA9/FTM1_CH1/MII0_RXD3/FTM1_QD_PHB/TRACE_D1, pull_select: up, pull_enable: enable}
  - {pin_num: '53', peripheral: JTAG, signal: JTAG_TMS_SWD_DIO, pin_signal: PTA3/UART0_RTS_b/FTM0_CH0/JTAG_TMS/SWD_DIO}
  - {pin_num: '52', peripheral: JTAG, signal: TDO, pin_signal: PTA2/UART0_TX/FTM0_CH7/JTAG_TDO/TRACE_SWO/EZP_DO}
  - {pin_num: '64', peripheral: I2C2, signal: SCL, pin_signal: CMP2_IN0/PTA12/CAN0_TX/FTM1_CH0/RMII0_RXD1/MII0_RXD1/I2C2_SCL/I2S0_TXD0/FTM1_QD_PHA}
  - {pin_num: '63', peripheral: I2C2, signal: SDA, pin_signal: PTA11/FTM2_CH1/MII0_RXCLK/I2C2_SDA/FTM2_QD_PHB}
  - {pin_num: '117', peripheral: GPIOC, signal: 'GPIO, 12', pin_signal: PTC12/UART4_RTS_b/FB_AD27/FTM3_FLT0}
  - {pin_num: '118', peripheral: GPIOC, signal: 'GPIO, 13', pin_signal: PTC13/UART4_CTS_b/FB_AD26}
  - {pin_num: '119', peripheral: GPIOC, signal: 'GPIO, 14', pin_signal: PTC14/UART4_RX/FB_AD25}
  - {pin_num: '120', peripheral: GPIOC, signal: 'GPIO, 15', pin_signal: PTC15/UART4_TX/FB_AD24}
  - {pin_num: '62', peripheral: GPIOA, signal: 'GPIO, 10', pin_signal: PTA10/FTM2_CH0/MII0_RXD2/FTM2_QD_PHA/TRACE_D0}
  - {pin_num: '126', peripheral: GPIOC, signal: 'GPIO, 19', pin_signal: PTC19/UART3_CTS_b/ENET0_1588_TMR3/FB_CS3_b/FB_BE7_0_BLS31_24_b/FB_TA_b, open_drain: enable}
  - {pin_num: '131', peripheral: GPIOD, signal: 'GPIO, 4', pin_signal: PTD4/LLWU_P14/SPI0_PCS1/UART0_RTS_b/FTM0_CH4/FB_AD2/EWM_IN/SPI1_PCS0}
  - {pin_num: '132', peripheral: GPIOD, signal: 'GPIO, 5', pin_signal: ADC0_SE6b/PTD5/SPI0_PCS2/UART0_CTS_b/UART0_COL_b/FTM0_CH5/FB_AD1/EWM_OUT_b/SPI1_SCK}
  - {pin_num: '133', peripheral: GPIOD, signal: 'GPIO, 6', pin_signal: ADC0_SE7b/PTD6/LLWU_P15/SPI0_PCS3/UART0_RX/FTM0_CH6/FB_AD0/FTM0_FLT0/SPI1_SOUT}
  - {pin_num: '85', peripheral: GPIOB, signal: 'GPIO, 4', pin_signal: ADC1_SE10/PTB4/ENET0_1588_TMR2/FTM1_FLT0, pull_select: up, pull_enable: enable}
  - {pin_num: '86', peripheral: GPIOB, signal: 'GPIO, 5', pin_signal: ADC1_SE11/PTB5/ENET0_1588_TMR3/FTM2_FLT0, pull_select: up, pull_enable: enable}
  - {pin_num: '87', peripheral: GPIOB, signal: 'GPIO, 6', pin_signal: ADC1_SE12/PTB6/FB_AD23, pull_select: up, pull_enable: enable}
  - {pin_num: '99', peripheral: GPIOB, signal: 'GPIO, 20', pin_signal: PTB20/SPI2_PCS0/FB_AD31/CMP0_OUT, pull_select: up, pull_enable: enable}
  - {pin_num: '65', peripheral: GPIOA, signal: 'GPIO, 13', pin_signal: CMP2_IN1/PTA13/LLWU_P4/CAN0_RX/FTM1_CH1/RMII0_RXD0/MII0_RXD0/I2C2_SDA/I2S0_TX_FS/FTM1_QD_PHB,
    pull_select: up, pull_enable: enable}
  - {pin_num: '65', peripheral: LLWU, signal: 'P, 4', pin_signal: CMP2_IN1/PTA13/LLWU_P4/CAN0_RX/FTM1_CH1/RMII0_RXD0/MII0_RXD0/I2C2_SDA/I2S0_TX_FS/FTM1_QD_PHB}
  - {pin_num: '111', peripheral: I2S0, signal: MCLK, pin_signal: CMP0_IN0/PTC6/LLWU_P10/SPI0_SOUT/PDB0_EXTRG/I2S0_RX_BCLK/FB_AD9/I2S0_MCLK}
  - {pin_num: '110', peripheral: I2S0, signal: RXD0, pin_signal: PTC5/LLWU_P9/SPI0_SCK/LPTMR0_ALT2/I2S0_RXD0/FB_AD10/CMP0_OUT/FTM0_CH2}
  - {pin_num: '112', peripheral: I2S0, signal: RX_FS, pin_signal: CMP0_IN1/PTC7/SPI0_SIN/USB_SOF_OUT/I2S0_RX_FS/FB_AD8}
  - {pin_num: '116', peripheral: I2S0, signal: RXD1, pin_signal: ADC1_SE7b/PTC11/LLWU_P11/I2C1_SDA/FTM3_CH7/I2S0_RXD1/FB_RW_b}
  - {pin_num: '9', peripheral: GPIOE, signal: 'GPIO, 6', pin_signal: PTE6/SPI1_PCS3/UART3_CTS_b/I2S0_MCLK/FTM3_CH1/USB_SOF_OUT, pull_select: up, pull_enable: enable}
  - {pin_num: '101', peripheral: GPIOB, signal: 'GPIO, 22', pin_signal: PTB22/SPI2_SOUT/FB_AD29/CMP2_OUT, open_drain: enable}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_InitPins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
void BOARD_InitPins(void)
{
    /* Port A Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortA);
    /* Port B Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortB);
    /* Port C Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortC);
    /* Port D Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortD);
    /* Port E Clock Gate Control: Clock enabled */
    CLOCK_EnableClock(kCLOCK_PortE);

    /* PORTA0 (pin 50) is configured as JTAG_TCLK */
    PORT_SetPinMux(PORTA, 0U, kPORT_MuxAlt7);

    /* PORTA10 (pin 62) is configured as PTA10 */
    PORT_SetPinMux(PORTA, 10U, kPORT_MuxAsGpio);

    /* PORTA11 (pin 63) is configured as I2C2_SDA */
    PORT_SetPinMux(PORTA, 11U, kPORT_MuxAlt5);

    /* PORTA12 (pin 64) is configured as I2C2_SCL */
    PORT_SetPinMux(PORTA, 12U, kPORT_MuxAlt5);

    /* PORTA13 (pin 65) is configured as PTA13, LLWU_P4 */
    PORT_SetPinMux(PORTA, 13U, kPORT_MuxAsGpio);

    PORTA->PCR[13] = ((PORTA->PCR[13] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                      /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                       * corresponding PE field is set. */
                      | (uint32_t)(kPORT_PullUp));

    /* PORTA2 (pin 52) is configured as JTAG_TDO */
    PORT_SetPinMux(PORTA, 2U, kPORT_MuxAlt7);

    /* PORTA3 (pin 53) is configured as JTAG_TMS */
    PORT_SetPinMux(PORTA, 3U, kPORT_MuxAlt7);

    /* PORTA8 (pin 60) is configured as FTM1_QD_PHA */
    PORT_SetPinMux(PORTA, 8U, kPORT_MuxAlt6);

    PORTA->PCR[8] = ((PORTA->PCR[8] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));

    /* PORTA9 (pin 61) is configured as FTM1_QD_PHB */
    PORT_SetPinMux(PORTA, 9U, kPORT_MuxAlt6);

    PORTA->PCR[9] = ((PORTA->PCR[9] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));

    /* PORTB18 (pin 97) is configured as FTM2_QD_PHA */
    PORT_SetPinMux(PORTB, 18U, kPORT_MuxAlt6);

    PORTB->PCR[18] = ((PORTB->PCR[18] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                      /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                       * corresponding PE field is set. */
                      | (uint32_t)(kPORT_PullUp));

    /* PORTB19 (pin 98) is configured as FTM2_QD_PHB */
    PORT_SetPinMux(PORTB, 19U, kPORT_MuxAlt6);

    PORTB->PCR[19] = ((PORTB->PCR[19] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                      /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                       * corresponding PE field is set. */
                      | (uint32_t)(kPORT_PullUp));

    /* PORTB20 (pin 99) is configured as PTB20 */
    PORT_SetPinMux(PORTB, 20U, kPORT_MuxAsGpio);

    PORTB->PCR[20] = ((PORTB->PCR[20] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                      /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                       * corresponding PE field is set. */
                      | (uint32_t)(kPORT_PullUp));

    /* PORTB22 (pin 101) is configured as PTB22 */
    PORT_SetPinMux(PORTB, 22U, kPORT_MuxAsGpio);

    PORTB->PCR[22] = ((PORTB->PCR[22] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_ODE_MASK | PORT_PCR_ISF_MASK)))

                      /* Open Drain Enable: Open drain output is enabled on the corresponding pin, if the pin is
                       * configured as a digital output. */
                      | PORT_PCR_ODE(kPORT_OpenDrainEnable));

    /* PORTB4 (pin 85) is configured as PTB4 */
    PORT_SetPinMux(PORTB, 4U, kPORT_MuxAsGpio);

    PORTB->PCR[4] = ((PORTB->PCR[4] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));

    /* PORTB5 (pin 86) is configured as PTB5 */
    PORT_SetPinMux(PORTB, 5U, kPORT_MuxAsGpio);

    PORTB->PCR[5] = ((PORTB->PCR[5] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));

    /* PORTB6 (pin 87) is configured as PTB6 */
    PORT_SetPinMux(PORTB, 6U, kPORT_MuxAsGpio);

    PORTB->PCR[6] = ((PORTB->PCR[6] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));

    /* PORTC0 (pin 103) is configured as PTC0 */
    PORT_SetPinMux(PORTC, 0U, kPORT_MuxAsGpio);

    /* PORTC1 (pin 104) is configured as PTC1 */
    PORT_SetPinMux(PORTC, 1U, kPORT_MuxAsGpio);

    /* PORTC11 (pin 116) is configured as I2S0_RXD1 */
    PORT_SetPinMux(PORTC, 11U, kPORT_MuxAlt4);

    /* PORTC12 (pin 117) is configured as PTC12 */
    PORT_SetPinMux(PORTC, 12U, kPORT_MuxAsGpio);

    /* PORTC13 (pin 118) is configured as PTC13 */
    PORT_SetPinMux(PORTC, 13U, kPORT_MuxAsGpio);

    /* PORTC14 (pin 119) is configured as PTC14 */
    PORT_SetPinMux(PORTC, 14U, kPORT_MuxAsGpio);

    /* PORTC15 (pin 120) is configured as PTC15 */
    PORT_SetPinMux(PORTC, 15U, kPORT_MuxAsGpio);

    /* PORTC18 (pin 125) is configured as PTC18 */
    PORT_SetPinMux(PORTC, 18U, kPORT_MuxAsGpio);

    PORTC->PCR[18] = ((PORTC->PCR[18] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_ODE_MASK | PORT_PCR_ISF_MASK)))

                      /* Open Drain Enable: Open drain output is enabled on the corresponding pin, if the pin is
                       * configured as a digital output. */
                      | PORT_PCR_ODE(kPORT_OpenDrainEnable));

    /* PORTC19 (pin 126) is configured as PTC19 */
    PORT_SetPinMux(PORTC, 19U, kPORT_MuxAsGpio);

    PORTC->PCR[19] = ((PORTC->PCR[19] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_ODE_MASK | PORT_PCR_ISF_MASK)))

                      /* Open Drain Enable: Open drain output is enabled on the corresponding pin, if the pin is
                       * configured as a digital output. */
                      | PORT_PCR_ODE(kPORT_OpenDrainEnable));

    /* PORTC2 (pin 105) is configured as PTC2 */
    PORT_SetPinMux(PORTC, 2U, kPORT_MuxAsGpio);

    /* PORTC3 (pin 106) is configured as PTC3 */
    PORT_SetPinMux(PORTC, 3U, kPORT_MuxAsGpio);

    /* PORTC5 (pin 110) is configured as I2S0_RXD0 */
    PORT_SetPinMux(PORTC, 5U, kPORT_MuxAlt4);

    /* PORTC6 (pin 111) is configured as I2S0_MCLK */
    PORT_SetPinMux(PORTC, 6U, kPORT_MuxAlt6);

    /* PORTC7 (pin 112) is configured as I2S0_RX_FS */
    PORT_SetPinMux(PORTC, 7U, kPORT_MuxAlt4);

    /* PORTD0 (pin 127) is configured as SPI0_PCS0 */
    PORT_SetPinMux(PORTD, 0U, kPORT_MuxAlt2);

    PORTD->PCR[0] = ((PORTD->PCR[0] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))

                     /* Drive Strength Enable: Low drive strength is configured on the corresponding pin, if pin
                      * is configured as a digital output. */
                     | PORT_PCR_DSE(kPORT_LowDriveStrength));

    /* PORTD1 (pin 128) is configured as SPI0_SCK */
    PORT_SetPinMux(PORTD, 1U, kPORT_MuxAlt2);

    /* PORTD2 (pin 129) is configured as SPI0_SOUT */
    PORT_SetPinMux(PORTD, 2U, kPORT_MuxAlt2);

    PORTD->PCR[2] = ((PORTD->PCR[2] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_DSE_MASK | PORT_PCR_ISF_MASK)))

                     /* Drive Strength Enable: Low drive strength is configured on the corresponding pin, if pin
                      * is configured as a digital output. */
                     | PORT_PCR_DSE(kPORT_LowDriveStrength));

    /* PORTD3 (pin 130) is configured as SPI0_SIN */
    PORT_SetPinMux(PORTD, 3U, kPORT_MuxAlt2);

    /* PORTD4 (pin 131) is configured as PTD4 */
    PORT_SetPinMux(PORTD, 4U, kPORT_MuxAsGpio);

    /* PORTD5 (pin 132) is configured as PTD5 */
    PORT_SetPinMux(PORTD, 5U, kPORT_MuxAsGpio);

    /* PORTD6 (pin 133) is configured as PTD6 */
    PORT_SetPinMux(PORTD, 6U, kPORT_MuxAsGpio);

    const port_pin_config_t porte0_pin1_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_D1 */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE0 (pin 1) is configured as SDHC0_D1 */
    PORT_SetPinConfig(PORTE, 0U, &porte0_pin1_config);

    const port_pin_config_t porte1_pin2_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_D0 */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE1 (pin 2) is configured as SDHC0_D0 */
    PORT_SetPinConfig(PORTE, 1U, &porte1_pin2_config);

    const port_pin_config_t porte2_pin3_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_DCLK */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE2 (pin 3) is configured as SDHC0_DCLK */
    PORT_SetPinConfig(PORTE, 2U, &porte2_pin3_config);

    /* PORTE24 (pin 45) is configured as PTE24 */
    PORT_SetPinMux(PORTE, 24U, kPORT_MuxAsGpio);

    /* PORTE25 (pin 46) is configured as PTE25 */
    PORT_SetPinMux(PORTE, 25U, kPORT_MuxAsGpio);

    const port_pin_config_t porte3_pin4_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_CMD */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE3 (pin 4) is configured as SDHC0_CMD */
    PORT_SetPinConfig(PORTE, 3U, &porte3_pin4_config);

    const port_pin_config_t porte4_pin7_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_D3 */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE4 (pin 7) is configured as SDHC0_D3 */
    PORT_SetPinConfig(PORTE, 4U, &porte4_pin7_config);

    const port_pin_config_t porte5_pin8_config = {/* Internal pull-up resistor is enabled */
                                                  kPORT_PullUp,
                                                  /* Fast slew rate is configured */
                                                  kPORT_FastSlewRate,
                                                  /* Passive filter is disabled */
                                                  kPORT_PassiveFilterDisable,
                                                  /* Open drain is disabled */
                                                  kPORT_OpenDrainDisable,
                                                  /* High drive strength is configured */
                                                  kPORT_HighDriveStrength,
                                                  /* Pin is configured as SDHC0_D2 */
                                                  kPORT_MuxAlt4,
                                                  /* Pin Control Register fields [15:0] are not locked */
                                                  kPORT_UnlockRegister};
    /* PORTE5 (pin 8) is configured as SDHC0_D2 */
    PORT_SetPinConfig(PORTE, 5U, &porte5_pin8_config);

    /* PORTE6 (pin 9) is configured as PTE6 */
    PORT_SetPinMux(PORTE, 6U, kPORT_MuxAsGpio);

    PORTE->PCR[6] = ((PORTE->PCR[6] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PS_MASK | PORT_PCR_PE_MASK | PORT_PCR_ISF_MASK)))

                     /* Pull Select: Internal pullup resistor is enabled on the corresponding pin, if the
                      * corresponding PE field is set. */
                     | (uint32_t)(kPORT_PullUp));
}
/***********************************************************************************************************************
 * EOF
 **********************************************************************************************************************/
