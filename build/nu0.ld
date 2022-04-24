OUTPUT_ARCH (mips)
SECTIONS
{
   __romPos = 0;
   _bootSegmentStart = ADDR(.boot); _bootSegmentRomStart = __romPos; .boot 0x04000000 : AT(__romPos)
   {
      build/asm/rom_header.o(.text);
      build/boot.6102.o(.data);
   }
   _bootSegmentEnd = ADDR(.boot) + SIZEOF(.boot); _bootSegmentRomEnd = __romPos + SIZEOF(.boot); __romPos += SIZEOF(.boot);
   _codeSegmentStart = ADDR(.code); _codeSegmentRomStart = __romPos; .code 0x80025c00 : AT(__romPos) SUBALIGN(16)
   {
  build/asm/entry.o(.start);
      /usr/lib/n64/nusys/nusys_rom.o(.text);
      build/src/main*.o(.text);
      */libultra_rom.a:*.o(.text);
      */libnusys.a:*.o(.text);
      */libgcc.a:*.o(.text);
      */libnustd.a:*.o(.text);
      /usr/lib/n64/PR/rspboot.o(.text);
      /usr/lib/n64/PR/gspF3DEX2.fifo.o(.text);
      /usr/lib/n64/PR/gspL3DEX2.fifo.o(.text);
      /usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o(.text);
      /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.text);
      /usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o(.text);
      /usr/lib/n64/PR/gspS2DEX2.fifo.o(.text);
      /usr/lib/n64/nusys/nusys_rom.o(.data*);
      build/src/main*.o(.data*);
      */libultra_rom.a:*.o(.data*);
      */libnusys.a:*.o(.data*);
      /usr/lib/n64/PR/rspboot.o(.data*);
      /usr/lib/n64/PR/gspF3DEX2.fifo.o(.data*);
      /usr/lib/n64/PR/gspL3DEX2.fifo.o(.data*);
      /usr/lib/n64/PR/gspF3DEX2.Rej.fifo.o(.data*);
      /usr/lib/n64/PR/gspF3DEX2.NoN.fifo.o(.data*);
      /usr/lib/n64/PR/gspF3DLX2.Rej.fifo.o(.data*);
      /usr/lib/n64/PR/gspS2DEX2.fifo.o(.data*);
      /usr/lib/n64/nusys/nusys_rom.o(.*rodata*);
      build/src/main*.o(.*rodata*);
      */libultra_rom.a:*.o(.*rodata*);
      */libnusys.a:*.o(.*rodata*);
      */libgcc.a:*.o(.*rodata*);
   }
   _codeSegmentEnd = ADDR(.code) + SIZEOF(.code); _codeSegmentRomEnd = __romPos + SIZEOF(.code); __romPos += SIZEOF(.code);
   _codeSegmentBssStart = ADDR(.code.noload); .code.noload (NOLOAD) :
   {
      /usr/lib/n64/nusys/nusys_rom.o(COMMON);
      /usr/lib/n64/nusys/nusys_rom.o(.scommon);
      /usr/lib/n64/nusys/nusys_rom.o(.*bss*);
      build/src/main*.o(.*bss*);
      */libultra_rom.a:*.o(COMMON);
      */libultra_rom.a:*.o(.scommon);
      */libultra_rom.a:*.o(.*bss*);
      */libnusys.a:*.o(COMMON);
      */libnusys.a:*.o(.scommon);
      */libnusys.a:*.o(.*bss*);
   }
   _codeSegmentBssEnd = ADDR(.code.noload) + SIZEOF(.code.noload); _codeSegmentBssSize = SIZEOF(.code.noload);
   /DISCARD/ :
   {
      *(*);
   }
}
