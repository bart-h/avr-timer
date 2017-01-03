CFLAGS=-Os -mmcu=attiny2313 -std=gnu99 -flto
#CFLAGS=-Os -mmcu=attiny2313 -std=gnu99
CC=avr-gcc
LD=avr-ld
OBJCOPY=avr-objcopy
TARGET=timer
OBJS=timer.o charlieplex.o event.o buzzer.o rotary.o song.o
AVRDUDEFLAGS=-p t2313 -c usbasp

all: $(TARGET).hex

$(TARGET).avr: $(OBJS)
	$(CC) $(CFLAGS) -Wl,--relax,--gc-sections -o $@ $^
	size $@

$(TARGET).hex: $(TARGET).avr
	 $(OBJCOPY) -j.text -j.data -Oihex $< $@

flash: $(TARGET).hex
	avrdude $(AVRDUDEFLAGS) -U flash:w:$< -Evcc

start:
	avrdude $(AVRDUDEFLAGS) -Enoreset

clean:
	rm -f $(TARGET).avr $(TARGET).hex $(OBJS) $(OBJS:.o=.S) 


timer.o: timer.c avr-config.h charlieplex.h 

charlieplex.o: charlieplex.c avr-config.h charlieplex.h


song.o: song.c song-dat.inc 

song-dat.inc:
	midicsv song.mid | perl song_csvtoc.pl > $@

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.S: %.c
	$(CC) $(CFLAGS) -fverbose-asm -o $@ -S $<
