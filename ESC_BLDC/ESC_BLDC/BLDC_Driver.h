
// Ansteuerung für einen 3Phasen BLDC Treiber

/*   - - \ _ _ /
	Phase 1 = Phase A -
	Phase 2 = Phase A -
	Phase 3 = Phase A \
	Phase 4 = Phase A _
	Phase 5 = Phase A _
	Phase 6 = Phase A /
*/


// Hier die Pinbelegung eintragen

/////////////////////////////////////////////////////////////////
// Motor_Driver

#define PWM_Port	PORTB
#define PWM			PIN0_bm

#define	Phase_Port	PORTC
#define INHA		PIN4_bm
#define INLA		PIN5_bm
#define INHB		PIN2_bm
#define INLB		PIN3_bm
#define INHC		PIN0_bm
#define INLC		PIN1_bm

void BLDC_set_reverse (uint8_t reverse_flag);

void BLDC_PWM_Init (void);

void BLDC_PWM_Set (uint16_t PWM_Wert); // Muss zwischen 0 und 511 liegen

void BLDC_Driver_init (void);

void BLDC_Stop (void);

void BLDC_Break (void);

uint8_t BLDC_get_ready (uint8_t Phase);

uint8_t BLDC_next_Phase (uint8_t Phase);

void BLDC_change_Phase (uint8_t Phase);


/////////////////////////////////////////////////////////////////
// Motor_BEMF

#define ADC_POS_MID		ADC_MUXPOS_AIN0_gc
#define ADC_POS_BEMF_A	ADC_MUXPOS_AIN3_gc
#define ADC_POS_BEMF_B	ADC_MUXPOS_AIN2_gc
#define ADC_POS_BEMF_C	ADC_MUXPOS_AIN1_gc

#define BEMF_Port		PORTD
#define BEMF_Mid		PIN0_bm
#define BEMF_A			PIN3_bm
#define BEMF_B			PIN2_bm
#define BEMF_C			PIN1_bm

#define BEMF_Mid_PCTRL	PIN0CTRL
#define BEMF_A_PCTRL	PIN3CTRL
#define BEMF_B_PCTRL	PIN2CTRL
#define BEMF_C_PCTRL	PIN1CTRL


void BEMF_ADC_init (void);

uint8_t ADC_read_Mid_V (void);

uint8_t ADC_read_BEMF_V (uint8_t Phase);

uint8_t BEMF_ADC_compare (uint8_t Phase, uint8_t Mid_V, uint8_t BEMF_V);

