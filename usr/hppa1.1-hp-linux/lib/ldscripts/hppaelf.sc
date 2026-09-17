#
# Unusual variables checked by this code:
#	NOP - two byte opcode for no-op (defaults to 0)
#	DATA_ADDR - if end-of-text-plus-one-page isn't right for data start
#	OTHER_READONLY_SECTIONS - other than .text .init .rodata ...
#		(e.g., .PARISC.milli)
#	OTHER_READWRITE_SECTIONS - other than .data .bss .ctors .sdata ...
#		(e.g., .PARISC.global)
#	OTHER_SECTIONS - at the end
#	EXECUTABLE_SYMBOLS - symbols that must be defined for an
#		executable (e.g., _DYNAMIC_LINK)
#	TEXT_START_SYMBOLS - symbols that appear at the start of the
#		.text section.
#	DATA_START_SYMBOLS - symbols that appear at the start of the
#		.data section.
#	OTHER_BSS_SYMBOLS - symbols that appear at the start of the
#		.bss section besides __bss_start.
#	DATA_PLT - .plt should be in data segment, not text segment.
#
# When adding sections, do note that the names of some sections are used
# when specifying the start address of the next.
#
o
test "$LD_FLAG" = "N" && DATA_ADDR=.
cat <<EOF
OUTPUT_FORMAT("${OUTPUT_FORMAT}")
OUTPUT_ARCH(${ARCH})
ENTRY("\$START\$")

${RELOCATING+${LIB_SEARCH_DIRS}}
SECTIONS
{

  .text 0x1000 ${RELOCATING++${TEXT_START_ADDR}}:
  {
    ${RELOCATING+__text_start = .};
    *(.PARISC.stubs)
    *(.text)
    ${RELOCATING+etext = .};
    ${RELOCATING+_etext = .};
    ${RELOCATING+${OTHER_READONLY_SECTIONS}}	
  } =${NOP-0}

  .data  ${RELOCATING-0} :
  {
    ${RELOCATING+ . = . + 0x1000 };
    ${RELOCATING+__data_start = .};
    *(.data)
    ${CONSTRUCTING+CONSTRUCTORS}
    ${RELOCATING+edata = .};
    ${RELOCATING+_edata = .};
  }

  .bss  ${RELOCATING-0} :
  {
   *(.bss)
   *(COMMON)
   ${RELOCATING+end = . };
   ${RELOCATING+_end = . };
  }

  /* These must appear regardless of ${RELOCATING}.  */
  ${OTHER_SECTIONS}
}
EOF
