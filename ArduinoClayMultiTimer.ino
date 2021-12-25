#include <LiquidCrystal.h>
#include <EncButton.h>

// ========= SETTINGS =========
#define DISPLAY_WIDTH 16
#define DISPLAY_HEIGHT 2
#define DISPLAY_RS 12
#define DISPLAY_EN 11
#define DISPLAY_D4 7
#define DISPLAY_D5 8
#define DISPLAY_D6 9
#define DISPLAY_D7 10

#define ENCODER_S1 2
#define ENCODER_S2 5
#define ENCODER_KEY 3

#define MAX_TIMERS 64
// ========= SETTINGS =========

#define SELECTED_TIMER_ZERO 127

enum TimerStatus {
	OFF = 0,
	ON = 1,
};

enum ScreenType {
	MAIN,
	TIMER_MAIN,
	QUICK_MENU,
	TIMER_SELECT_TO_RESET,
	TIMER_RESET_CONFIRM,
	TIMER_RESET_ALL_CONFIRM
};

byte uiPointer = SELECTED_TIMER_ZERO; // будет использоватся в разных граф интерфейсах наверное .-_,

byte selectedTimer1 = SELECTED_TIMER_ZERO;
byte selectedTimer2 = SELECTED_TIMER_ZERO+1;
uint32_t timers[MAX_TIMERS];
TimerStatus timersStates[MAX_TIMERS];

ScreenType screen = ScreenType::MAIN;

LiquidCrystal *lcd;
EncButton<EB_TICK, ENCODER_S1, ENCODER_S2, ENCODER_KEY> *enc;

void setup() {
	Serial.begin(9600);

	screen = ScreenType::MAIN;

	LiquidCrystal _lcd(DISPLAY_RS, DISPLAY_EN, DISPLAY_D4, DISPLAY_D5, DISPLAY_D6, DISPLAY_D7);
  	EncButton<EB_TICK, ENCODER_S1, ENCODER_S2, ENCODER_KEY> _enc;

  	lcd = &_lcd;
  	enc = &_enc;

  	lcd->begin(DISPLAY_WIDTH, DISPLAY_HEIGHT);
}

void loop() {
	enc->tick();
	
	static uint32_t tmr;
	if (millis() - tmr >= 1000) {
		for (int i = 0; i < MAX_TIMERS; ++i)
		{
			TimerStatus status = timersStates[i];
			if (status == ON) {
				timers[i] = timers[i] + 1;
			}
		}
		tmr = millis();
	}

	if (screen == ScreenType::MAIN) {
		loopScreenMain();
	} else if (screen == ScreenType::TIMER_MAIN) {
		loopScreenTimerMain();
	} else if (screen == ScreenType::QUICK_MENU) {
		loopScreenQuickMenu();
	} else if (screen == ScreenType::TIMER_SELECT_TO_RESET) {
		loopScreenTimerSelectToReset();
	} else if (screen == ScreenType::TIMER_RESET_CONFIRM) {
		loopScreenTimerResetConfirm();
	} else if (screen == ScreenType::TIMER_RESET_ALL_CONFIRM) {
		loopScreenTimerResetAllConfirm();
	}
}

void loopScreenMain() {
	screen = ScreenType::TIMER_MAIN;
}

void loopScreenTimerMain() {
	if (enc->click()) {
		TimerStatus status = timersStates[sttn(selectedTimer1)];
	    if (status == TimerStatus::ON) {
	    	timersStates[sttn(selectedTimer1)] = TimerStatus::OFF;
	    } else if (status == TimerStatus::OFF) {
	    	timersStates[sttn(selectedTimer1)] = TimerStatus::ON;
	    }
	}

	if (enc->held()) {
		printClear();
		screen = ScreenType::QUICK_MENU;
		return;
	}

	short coff = 0;
	if (enc->right()) coff = -1;
	if (enc->left()) coff = 1;

	if (coff != 0) {
		scroll(selectedTimer1, coff);
		scroll(selectedTimer2, coff);
	}
	printChar(0, 0, '~');
	printTimerLine(1, 0, sttn(selectedTimer1));
	printTimerLine(1, 1, sttn(selectedTimer2));
}

void loopScreenQuickMenu() {
	lcd->setCursor(1, 0);
    lcd->print("BACK");

    lcd->setCursor(8, 0);
    lcd->print("RESET");

    lcd->setCursor(1, 1);
    lcd->print("RESET_ALL");

    auto clearAllPointers = []() { 
        printChar(0, 0, ' ');
        printChar(7, 0, ' ');
        printChar(0, 1, ' ');
    };

    if (sttn(uiPointer) == 0) {
    	printChar(0, 0, '~');
    } else if (sttn(uiPointer) == 1) {
    	printChar(7, 0, '~');
    } else if (sttn(uiPointer) == 2) {
    	printChar(0, 1, '~');
    }

	if (enc->click()) {
	    if (sttn(uiPointer) == 0) {
	    	screen = ScreenType::TIMER_MAIN;
	    } else if (sttn(uiPointer) == 1) {
	    	screen = ScreenType::TIMER_SELECT_TO_RESET;
	    } else if (sttn(uiPointer) == 2) {
	    	screen = ScreenType::TIMER_RESET_ALL_CONFIRM;
	    } 
	    uiPointer = SELECTED_TIMER_ZERO;
	    printClear();
	    return;
	}

	short coff = 0;
	if (enc->right()) coff = -1;
	if (enc->left()) coff = 1;

	if (coff != 0) {
		scroll(uiPointer, coff, 3);
		clearAllPointers();
	}
}

void loopScreenTimerSelectToReset() {
	if (enc->click()) {
		printClear();
	    screen = ScreenType::TIMER_RESET_CONFIRM;
	    uiPointer = SELECTED_TIMER_ZERO;
	    return;
	}

	short coff = 0;
	if (enc->right()) coff = -1;
	if (enc->left()) coff = 1;

	if (coff != 0) {
		scroll(selectedTimer1, coff);
		scroll(selectedTimer2, coff);
	}

	lcd->setCursor(0, 0);
    lcd->print("Select to Reset");

	printTimerLine(1, 1, sttn(selectedTimer1));
	printChar(0, 1, '~');
}

void loopScreenTimerResetConfirm() {
	lcd->setCursor(0, 0);
    lcd->print("Reset?");
    printChar(0, 1, '#');
	print2num(1, 1, sttn(selectedTimer1));
	lcd->setCursor(11, 0);
    lcd->print(" NO");
    lcd->setCursor(11, 1);
    lcd->print("YES");
    lcd->setCursor(5, 1);
    lcd->print("BACK");

    auto clearAllPointers = []() { 
        printChar(10, 0, ' ');
		printChar(10, 1, ' ');
		printChar(4, 1, ' ');
    };

	short coff = 0;
	if (enc->right()) coff = -1;
	if (enc->left()) coff = 1;

	if (coff != 0) {
		scroll(uiPointer, coff, 3);
    	clearAllPointers();
	}

	if (sttn(uiPointer) == 0) {
    	printChar(10, 0, '~');
    } else if (sttn(uiPointer) == 1) {
		printChar(10, 1, '~');
    } else if (sttn(uiPointer) == 2) {
		printChar(4, 1, '~');
    }

	if (enc->click()) {
		if (sttn(uiPointer) == 1) {
			timers[sttn(selectedTimer1)] = 0;
			timersStates[sttn(selectedTimer1)] = TimerStatus::OFF;
		}
		if (sttn(uiPointer) == 2) {
			screen = ScreenType::TIMER_SELECT_TO_RESET;
		} else {
			screen = ScreenType::TIMER_MAIN;
		}
		uiPointer = SELECTED_TIMER_ZERO;
		printClear();
	}
}

void loopScreenTimerResetAllConfirm() {
	lcd->setCursor(0, 0);
    lcd->print("RESET ALL?");
	lcd->setCursor(11, 0);
    lcd->print(" NO");
    lcd->setCursor(11, 1);
    lcd->print("YES");
    lcd->setCursor(5, 1);
    lcd->print("BACK");

    auto clearAllPointers = []() { 
        printChar(10, 0, ' ');
		printChar(10, 1, ' ');
		printChar(4, 1, ' ');
    };

	short coff = 0;
	if (enc->right()) coff = -1;
	if (enc->left()) coff = 1;

	if (coff != 0) {
		scroll(uiPointer, coff, 3);
    	clearAllPointers();
	}

	if (sttn(uiPointer) == 0) {
    	printChar(10, 0, '~');
    } else if (sttn(uiPointer) == 1) {
		printChar(10, 1, '~');
    } else if (sttn(uiPointer) == 2) {
		printChar(4, 1, '~');
    }

	if (enc->click()) {
		if (sttn(uiPointer) == 1) {
			for (int i = 0; i < 64; ++i) {
				timers[i] = 0;
				timersStates[i] = TimerStatus::OFF;
			}
		}
		if (sttn(uiPointer) == 2) {
			screen = ScreenType::QUICK_MENU;
		} else {
			screen = ScreenType::TIMER_MAIN;
		}
		uiPointer = SELECTED_TIMER_ZERO;
		printClear();
	}
}

// Рендерит таймерную линию (которых на экране 2) на координатах y (0/1)
void printTimerLine(byte x, byte y, byte timer) {
	// interface
	print2num(x+0, y, timer);
	printChar(x+2, y, ' ');
	printChar(x+5, y, ':');
    printChar(x+8, y, ':');

    TimerStatus status = timersStates[timer];
    if (status == TimerStatus::ON) printChar(x+11, y, '*');
    if (status == TimerStatus::OFF) printChar(x+11, y, ' ');

    // values
    print2num(x+3, y, secondsToTimer(timers[timer], 0));
    print2num(x+6, y, secondsToTimer(timers[timer], 1));
    print2num(x+9, y, secondsToTimer(timers[timer], 2));
}

void scroll(byte &v, short coff) {
	scroll(v, coff, MAX_TIMERS);
}

// меняет значение v на коффициент coff и не позволяет выйти за границ(ы/и)
void scroll(byte &v, short coff, byte max) {
	v = v + coff;
	if (v < SELECTED_TIMER_ZERO) v = SELECTED_TIMER_ZERO + max-1;
	if (v > SELECTED_TIMER_ZERO + max-1) v = SELECTED_TIMER_ZERO;
}

// Сделать из числа с 'типом' selectedTimer нормальное человеческое число
byte sttn(byte &s) {
	return s - SELECTED_TIMER_ZERO;
}

// Сырые секунды в человеческие (часы type=0) (минуты type=1) (секунды type=2)
byte secondsToTimer(uint32_t number, byte type) {
	if (type == 0) {
		return number / 60 / 60;
	} else if (type == 1) {
		return number / 60 % 60;
	} else if (type == 2) {
		return number % 60;
	}
	return 255;
}


// Вывести на дисплей двухзначние число координата x это начало тоесть если число 9 то на коодинатах x будет 0 а x+1 там будет 9
void print2num(byte x, byte y, byte number) {
	if (number < 10) {
      lcd->setCursor(x, y);
      lcd->print("0");
      x++;
    }
    lcd->setCursor(x, y);
    lcd->print(number);
}

// Вывести на дисплей символ 1 командой вместо двух
void printChar(byte x, byte y, char c) {
	lcd->setCursor(x, y);
    lcd->print(c);
}

void printClear() {
    lcd->setCursor(0, 0);
    lcd->print("                ");

    lcd->setCursor(0, 1);
    lcd->print("                ");
}