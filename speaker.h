#include <stdlib.h>
#include <stdio.h>
#include <x86/inout.h>
#include <sys/neutrino.h>
#include <unistd.h>
#include <stdint.h>

unsigned long kbd, clk;
int system_control = 0;

void speaker_enable() {
   ThreadCtl(_NTO_TCTL_IO, 0);
      
   kbd = mmap_device_io(4, 0x60);
   clk = mmap_device_io(4, 0x40);

   system_control = in8(kbd + 1);
   out8(kbd + 1, system_control | 0x03);

   out8(clk + 3, 0xb7); 
   out8(clk + 2, 0x02);
   out8(clk + 2, 0x27);                
}

void speaker_disable() {
   system_control = in8(kbd + 1);
   out8(kbd + 1, system_control & 0xfc);    
}