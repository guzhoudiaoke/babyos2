/*
 * guzhoudiaoke@126.com
 * 2017-10-22
 */

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .text : AT(0x100000) {
        *(.text .stub .text.* .gnu.linkonce.t.*)
    }

    . = ALIGN(0x1000);

    .data : {
        *(.data)
    }
    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack)
    }
}
