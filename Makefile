             programs := bart-demo blink echo green-glow heaters hello \
                         LEDs LEDs2 LEDs-off limit-switches microbench \
                         null powerup scale SPI talk timers visible-laser \
                         xengine xhome xintr xmotor xyhome yhome ymotor

                 dirs := one-axis-random

     bart-demo_cfiles := bart-demo.c LED-util.c motor-util.c serial.c \
                         stdio_util.c timer.c
     bart-demo_ldlibs := -Wl,-u,vfprintf -lprintf_flt -lm
         blink_cfiles := blink.c timer.c
          echo_cfiles := echo.c serial.c
       heaters_cfiles := heaters.c serial.c
    green-glow_cfiles := green-glow.c LED-util.c serial.c stdio_util.c timer.c
    green-glow_ldlibs := -Wl,-u,vfprintf -lprintf_flt -lm
         hello_cfiles := hello.c
limit-switches_cfiles := limit-switches.c serial.c timer.c
          LEDs_cfiles := LEDs.c timer.c
         LEDs2_cfiles := LEDs2.c serial.c stdio_util.c timer.c
      LEDs-off_cfiles := LEDs-off.c LED-util.c timer.c
    microbench_cfiles := microbench.c serial.c stdio_util.c usec-timer.c
    microbench_ldlibs := -Wl,-u,vfprintf -lprintf_flt -lm
          null_cfiles := null.c
       powerup_cfiles := powerup.c serial.c
         scale_cfiles := scale.c LED-util.c motor-util.c motors.c serial.c \
                         serial_util.c stdio_util.c timer.c
           SPI_cfiles := SPI.c timer.c
        timers_cfiles := timers.c stdio_util.c serial.c
          talk_cfiles := talk.c stdio_util.c serial.c timer.c
 visible-laser_cfiles := visible-laser.c timer.c
       xengine_cfiles := xengine.c LED-util.c motor-util.c motors.c serial.c \
                         serial_util.c stdio_util.c timer.c
       xengine_ldlibs := -Wl,-u,vfprintf -lprintf_flt -lm
         xhome_cfiles := xhome.c motor-util.c timer.c
         xintr_cfiles := xintr.c LED-util.c motor-util.c motors.c serial.c \
                         serial_util.c stdio_util.c timer.c
        xmotor_cfiles := xmotor.c timer.c
        xyhome_cfiles := xyhome.c motor-util.c serial.c serial_util.c \
                         stdio_util.c timer.c
        xyhome_ldlibs := -Wl,-u,vfprintf -lprintf_flt -lm
         yhome_cfiles := yhome.c serial.c serial_util.c stdio_util.c timer.c
        ymotor_cfiles := ymotor.c timer.c

                  MCU := atmega2560

                    CC = avr-gcc
                   CXX = avr-g++
                    AR = avr-ar
               OBJCOPY = avr-objcopy
               AVRDUDE = ~/kerfburn/front/thruport/thruport suspend avrdude

             CPPFLAGS += -mmcu=$(MCU) -DF_CPU=16000000L
               CFLAGS += -g -Os -Wall -Werror $(CPPFLAGS) \
                         -ffunction-sections -fdata-sections -std=c99
             CXXFLAGS += $(patsubst $(CFLAGS),-std=c99,) -fno-exceptions
              libtype := static
              LDFLAGS += -Wl,--gc-sections,--relax -mmcu=$(MCU)

# Create programs as Intel Hex files

         PROGRAMS_HEX := $(programs:%=%.hex)
                 JUNK += *.hex
default: all
%.hex: %
	$(OBJCOPY) -O ihex -R .eeprom $< $@
help::
	@echo 'Build and download firmware for the Azteeg X3'
	@echo ''
	@echo 'Download Images'
	@$(foreach p, $(PROGRAMS), echo '    $(patsubst ./%,load-%,$p)';)
	@echo ''

# Rules to load the Azteeg

load-%: %.hex
	$(AVRDUDE) -qq -p$(MCU) -D -Uflash:w:$<

include makefiles/project-root.make
