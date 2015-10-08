//rozdzielczość wyświetlacza (wiersze/kolumny)
#define LCD_Y 2
#define LCD_X 16

//0 - pin RW zwarty do GND, 1 - pin RW do uC
#define USE_RW 1

//konfiguracja portów i pinów dla D7-D4
#define LCD_D7PORT C
#define LCD_D7 5
#define LCD_D6PORT C
#define LCD_D6 4
#define LCD_D5PORT C
#define LCD_D5 3
#define LCD_D4PORT C
#define LCD_D4 2

//RS, RW, E
#define LCD_RSPORT B
#define LCD_RS 2
#define LCD_RWPORT B
#define LCD_RW 1
#define LCD_EPORT B
#define LCD_E 0

//definicja adresów w DDRAM
#if ( (LCD_Y == 4) && (LCD_X == 20))
#define LCD_LINE1 0x00 //adres 1. znaku 1. wiersza
#define LCD_LINE2 0x28 //adres 1. znaku 2. wiersza
#define LCD_LINE3 0x14
#define LCD_LINE4 0x54
#else
#define LCD_LINE1 0x00
#define LCD_LINE2 0x40
#define LCD_LINE3 0x10
#define LCD_LINE4 0x50
#endif

// makra upraszczające dostęp do portów
// PORT
#define PORT(x) SPORT(x)
#define SPORT(x) (PORT##x)
// PIN
#define PIN(x) SPIN(x)
#define SPIN(x) (PIN##x)
// DDR
#define DDR(x) SDDR(x)
#define SDDR(x) (DDR##x)

//ustawia kursor na wybranej pozycji Y, X
#define USE_LCD_LOCATE 1
#define USE_LCD_STR_P 0 	//wysyła string umieszczony w pamięci FLASH
#define USE_LCD_STR_E 0 	//wysyła string umieszczony w pamięci EEPROM
#define USE_LCD_INT 0 		//wyświetla liczbę DEC na LCD
#define USE_LCD_HEX 0 		//wyświetla liczbę HEX na LCD
#define USE_LCD_DEFCHAR 0 	//wysyła zdefiniowany znak z pamięci RAM
#define USE_LCD_DEFCHAR_P 0 //wysyła zdefiniowany znak z pamięci FLASH
#define USE_LCD_DEFCHAR_E 0 //wysyła zdefiniowany znak z pamięci EEPROM
#define USE_LCD_CURSOR_ON 0 //obsługa wł/wył kursora
#define USE_LCD_CURSOR_BLINK 0 	//obsługa wł/wył migania kursora
#defina USE_LCD_CURSOR_HOME 0  	//ustawia kursor na pozycji początkowej

//inicjowanie pinów portów ustalonych do podłączenia z wyświetlaczem LCD
//ustawienie wszystkich jako wyjścia
DDR(LCD_D7PORT) |= (1<<LCD_D7);
DDR(LCD_D6PORT) |= (1<<LCD_D6);
DDR(LCD_D5PORT) |= (1<<LCD_D5);
DDR(LCD_D4PORT) |= (1<<LCD_D4);
DDR(LCD_RSPORT) |= (1<<LCD_RS);
DDR(LCD_EPORT) |= (1<<LCD_E);
#if USE_RW == 1 //kompilacja warunkowa, gdy używamy RW
	DDR(LCD_RWPORT) |= (1<<LCD_RW);
#endif

//wyzerowanie wszystkich linii sterujących
PORT(LCD_RSPORT) &= ~(1<<LCD_RS);
PORT(LCD_EPORT) &= ~(1<<LCD_E);
#if USE_RW == 1
	PORT(LCD_RWPORT) &= ~(1<<LCD_RW);
#endif

//makrodefinicje operacji na sygnałach sterujących RS, RW i E
//stan wysoki na linii RS
#define SET_RS PORT(LCD_RSPORT) |= (1<<LCD_RS)
//stan niski na linii RS
#define CLR_RS PORT(LCD_RSPORT) &= ~(1<<LCD_RS)
//stan wysoki na linii RW
#define SET_RW PORT(LCD_RWPORT) |= (1<<LCD_RW)
//stan niski na linii RW
#define CLR_RW PORT(LCD_RWPORT) &= ~(1<<LCD_RW)
//stan wysoki na linii E
#define SET_E PORT(LCD_EPORT) |= (1<<LCD_E)
//stan niski na linii E
#define CLR_E PORT(LCD_EPORT) &= ~(1<<LCD_E)

static inline void lcd_sendHalf(uint8_t data)
{
	if(data&(1<<0)) PORT(LCD_D4PORT) |= (1<<LCD_D4);
	else PORT(LCD_D4PORT) &= ~(1<<LCD_D4);
	
	if(data&(1<<1)) PORT(LCD_D5PORT) |= (1<<LCD_D5);
	else PORT(LCD_D5PORT) &= ~(1<<LCD_D5);

	if(data&(1<<2)) PORT(LCD_D6PORT) |= (1<<LCD_D6);
	else PORT(LCD_D6PORT) &= ~(1<<LCD_D6);
	
	if(data&(1<<3)) PORT(LCD_D7PORT) |= (1<<LCD_D7);
	else PORT(LCD_D7PORT) &= ~(1<<LCD_D7);
}

void _lcd_write_byte(unsigned char _data)
{
	//ustawienie pinów portu LCD D4-D7 jako wyjścia
	data_dir_out();
	
#if USE_RW == 1
	CLR_RW;
#endif
	SET_E;
	lcd_sendHalf(_data>>4); //wysłanie starszej części bajtu danych D7-D4
	CLR_E;
	
	SET_E;
	lcd_sendHalf(_data); // wysłanie młodszej części bajtu danych D3-D0
	CLR_E;
	
#if USE_RW == 1
	while( check_BF() & (1<<7)); //oczekiwanie na flagę zajętości (busy flag) sterownika LCD. Jeśli 1, to można przesyłać następne dane.
#else
	_delay_us(120);
#endif
}

void lcd_write_cmd(uint8_t cmd)
{
	CLR_RS;
	_lcd_write_byte(cmd);
}

void lcd_write_data(uint8_t data)
{
	SET_RS;
	_lcd_write_byte(data);
}

#if USE_RW == 1
static inline uint8_t lcd_readHalf(void)
{
	uint8_t result = 0;
	if(PIN(LCD_D4PORT)&(1<<LCD_D4)) result |= (1<<0);
	if(PIN(LCD_D5PORT)&(1<<LCD_D5)) result |= (1<<1);
	if(PIN(LCD_D6PORT)&(1<<LCD_D6)) result |= (1<<2);
	if(PIN(LCD_D7PORT)&(1<<LCD_D7)) result |= (1<<3);
	return result;
}
#endif

#if USE_RW == 1
uint8_t _lcd_read_byte(void)
{
	uint8_t result = 0;
	data_dir_in();
	SET_RW;
	SET_E;
	//odczyt starszej części bajtu z LCD D7-D4
	result |= (lcd_readHalf() << 4);
	CLR_E;
	SET_E;
	//odczyt młodszej części bajtu
	result |= lcd_readHalf();
	CLR_E;
	
	return result;
}
#endif

#if USE_RW == 1
uint8_t check_BF(void)
{
	CLR_RS;
	return _lcd_read_byte();
}
#endif

void lcd_init(void)
{
	data_dir_out();
	DDR(LCD_RSPORT) |= (1<<LCD_RS);
	DDR(LCD_EPORT) |= (1<<LCD_E);
	#if USE_RW == 1
		DDR(LCD_RWPORT) |= (1<<LCD_RW);
	#endif
	
	PORT(LCD_RSPORT) |= (1<<LCD_RS);
	PORT(LCD_EPORT) |= (1<<LCD_E);
	#if USE_RW == 1
		PORT(LCD_RWPORT) |= (1<<LCD_RW);
	#endif
	_delay_ms(15);
	PORT(LCD_EPORT) &= ~(1<<LCD_E);
	PORT(LCD_RSPORT) &= ~(1<<LCD_RS);
	#if USE_RW == 1
		PORT(LCD_RWPORT) &= ~(1<<LCD_RW);
	#endif
	SET_E;
	lcd_sendHalf(0x03); //tryb 8-bitowy DL=1
	CLR_E;
	_delay_ms(4.1);
	SET_E;
	lcd_sendHalf(0x03);
	CLR_E;
	_delay_us(100);
	SET_E;
	lcd_sendHalf(0x03);
	CLR_E;
	_delay_us(100);
	SET_E;
	lcd_sendHalf(0x02); //tryb 4-bitowy DL=0
	CLR_E;
	_delay_us(100);
	//można używać BusyFlag - używamy funkcji do wysyłąnia komend
	//tryb 4-bitowy, 2 wiersze, znak 5x7
	lcd_write_cmd(LCDC_FUNC|LCDC_FUNC4B|LCDC_FUNC2l|LCDC_FUNC5x7);
	//wyłączenie kursora
	lcd_write_cmd(LCDC_ONOFF|LCDC_CURSOROFF);
	//włączenie wyświetlacza
	lcd_write_cmd(LCDC_ONOFF|LCDC_DISPLAYON);
	//przesuwanie kursora w prawo bez przesuwania zawartości ekranu
	lcd_write_cmd(LCDC_ENTRY|LCDC_ENTRYR);
	//wyczyszczenie ekranu
	lcd_cls();
}

void lcd_cls(void)
{
	lcd_write_cmd(LCDC_CLS);
	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}

#if USE_LCD_CURSOR_HOME == 1
void lcd_home(void)
{
	lcd_write_cmd(LCDC_CLS|LCDC_HOME);
	#if USE_RW == 0
		_delay_ms(4.9);
	#endif
}
#endif

#if USE_LCD_CURSOR_ON == 1
void lcd_cursor_on(void)
{
	lcd_write_cmd(LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON);
}

void lcd_cursor_off(void)
{
	lcd_write_cmd(LCDC_ONOFF|LCDC_DISPLAYON);
}
#endif

#if USE_LCD_CURSOR_BLINK == 1
void lcd_blink_on(void)
{
	lcd_write_cmd(LCDC_ONOFF|LCDC_DISPLAYON|LCDC_CURSORON|LCDC_BLINKON);
}

void lcd_blink_off(void)
{
	lcd_write_cmd(LCDC_ONOFF|LCDC_DISPLAYON);
}
#endif

void lcd_str(char* str)
{
	register char znak;
	while((znak = *(str++)))
		lcd_write_data((znak >= 0x80 && znak <= 0x87) ? (znak & 0x07) : znak);
}

#if USE_LCD_STR_P == 1
void lcd_str_P(char* str)
{
	register char znak;
	while((znak = pgm_read_byte(str++)))
		lcd_write_data(((znak >= 0x80) && (znak <= 0x87)) ? (znak & 0x07) : znak);
}
#endif

#if USE_LCD_STR_E == 1
void lcd_str_E(char* str)
{
	register char znak;
	while(1){
		znak = eeprom_read_byte((uint8_t *)(str++));
		if(!znak || znak == 0xFF) break; //0xFF traktujemy jako 0 w pamięci EEPROM
		else lcd_write_data(((znak >= 0x80) && (znak <= 0x87)) ? (znak & 0x07) : znak);
	}
}
#endif

#if USE_LCD_INT == 1
void lcd_int(int val)
{
	char bufor[17];
	lcd_str(itoa(val, bufor, 10));
}
#endif

#if USE_LCD_HEX == 1
void lcd_hex(int val)
{
	char bufor[17];
	lcd_str(itoa(val, bufor, 16));
}
#endif

#if USE_LCD_DEFCHAR == 1
//definicja własnego znaku na LCD z pamięci RAM
//argumenty:
//nr - kod własnego znaki w pamięci CGRAM od 0x80 do 0x87
//*def_znak - wskaźnik do tablicy 7 bajtów definiujących znak
void lcd_defchar(uint8_t nr, uint8_t *def_znak)
{
	register uint8_t i, c;
	lcd_write_cmd(64+((nr & 0x07) * 8));
	for(i=0; i<8; i++)
	{
		c = *(def_znak++);
		lcd_write_data(c);
	}
}
#endif

#if USE_LCD_DEFCHAR_P == 1
void lcd_defchar_P(uint8_t nr, uint8_t *def_znak)
{
	register uint8_t i, c;
	lcd_write_cmd(64+((nr & 0x07) * 8));
	for(i=0; i<8; i++)
	{
		c = pgm_read_byte(def_znak++);
		lcd_write_data(c);
	}
}
#endif
#if USE_LCD_DEFCHAR_E == 1
void lcd_defchar_E(uint8_t nr, uint8_t *def_znak)
{
	register uint8_t i, c;
	lcd_write_cmd(64+((nr & 0x07) * 8));
	for(i=0; i<8; i++)
	{
		c = eeprom_read_byte(def_znak++);
		lcd_write_data(c);
	}
}
#endif

#if USE_LCD_LOCATE == 1
void lcd_locate(uint8_t y, uint8_t x)
{
	switch(y)
	{
		case 0: 
			y = LCD_LINE1;  //adres 1. znaku 0. wiersza
			break;
	#if (LCD_Y > 1)
		case 1:
			y = LCD_LINE2; //adres 1. znaku 1. wiersza
			break;
	#endif
	#if (LCD_Y > 2)
		case 2:
			y = LCD_LINE3;
			break;
	#endif
	#if (LCD_Y > 3)
		case 3:
			y = LCD_LINE4;
			break;
	#endif
	}
	lcd_write_cmd((0x80+y+x));
}
#endif
