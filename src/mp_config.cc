/*
 * guzhoudiaoke@126.com
 * 2018-02-03
 */

#include "mp_config.h"
#include "babyos.h"
#include "string.h"

void mp_floating_pointer_t::dump()
{
    char* signature = (char *) &m_signature;
    console()->kprintf(YELLOW, "mp floating pointer: \n");
    console()->kprintf(YELLOW, "m_signature:    %c%c%c%c\n", signature[0], signature[1], 
        signature[2], signature[3]);
    console()->kprintf(YELLOW, "physical addr:  %x\n", m_phy_addr);
    console()->kprintf(YELLOW, "length:         %x\n", m_length);
    console()->kprintf(YELLOW, "spec rev:       %x\n", m_spec_rev);
    console()->kprintf(YELLOW, "type:           %x\n", m_type);
    console()->kprintf(YELLOW, "imcrp:          %x\n", m_imcrp);
}

void mp_config_table_header_t::dump()
{
    char* signature = (char *) &m_signature;
    console()->kprintf(YELLOW, "mp config table header: \n");
    console()->kprintf(YELLOW, "m_signature:    %c%c%c%c\n", signature[0], signature[1], 
        signature[2], signature[3]);
    console()->kprintf(YELLOW, "table length:   %x\n", m_table_length);
    console()->kprintf(YELLOW, "spec rev        %x\n", m_spec_rev);
    console()->kprintf(YELLOW, "oem id          %s\n", m_oem_id);
    console()->kprintf(YELLOW, "product id      %s\n", m_product_id);
    console()->kprintf(YELLOW, "product pointer %x\n", m_oem_table_pointer);
    console()->kprintf(YELLOW, "entry count:    %x\n", m_entry_count);
}

void mp_config_processor_entry_t::dump()
{
    console()->kprintf(GREEN, "mp config process entry: \n");
    console()->kprintf(PINK, "local apic id:      %x\n", m_local_apic_id);
    console()->kprintf(PINK, "local apic version: %x\n", m_local_apic_version);
    console()->kprintf(PINK, "cpu flags:          %x\n", m_cpu_flags);
    console()->kprintf(PINK, "signature:          %x\n", m_signature);
    console()->kprintf(PINK, "feature flags:      %x\n", m_feature_flags);
}

void mp_config_bus_entry_t::dump()
{
    char type[7] = {0};
    strncpy(type, (char *) m_bus_type, 6);
    console()->kprintf(GREEN, "mp config bus entry: \n");
    console()->kprintf(PINK, "bus id:      %x\n", m_bus_id);
    console()->kprintf(PINK, "bus type:    %s\n", type);
}

void mp_config_io_apic_entry_t::dump()
{
    console()->kprintf(GREEN, "mp config io apic entry: \n");
    console()->kprintf(PINK, "io apic id:       %x\n", m_io_apic_id);
    console()->kprintf(PINK, "io apic version:  %x\n", m_io_apic_version);
    console()->kprintf(PINK, "io apic flags:    %x\n", m_io_apic_flags);
    console()->kprintf(PINK, "mem map address:  %x\n", m_mem_map_addr);
}

void mp_config_io_interrupt_entry_t::dump()
{
    //console()->kprintf(GREEN, "mp config io interrupt entry: \n");
    //console()->kprintf(PINK, "interrupt type:     %x\n", m_interrupt_type);
    //console()->kprintf(PINK, "io interrupt flag:  %x\n", m_io_interrupt_flag);
    //console()->kprintf(PINK, "source bus id:      %x\n", m_source_bus_id);
    //console()->kprintf(PINK, "source bus irq:     %x\n", m_source_bus_irq);
    //console()->kprintf(PINK, "dest io apic id:    %x\n", m_destination_io_apic_id);
    //console()->kprintf(PINK, "dest io apic intin: %x\n", m_destination_io_apic_intin);
    
    console()->kprintf(PINK, "interrupt type:     %x,", m_interrupt_type);
    console()->kprintf(PINK, "io interrupt flag:  %x,", m_io_interrupt_flag);
    console()->kprintf(PINK, "source bus id:      %x,", m_source_bus_id);
    console()->kprintf(PINK, "source bus irq:     %x,", m_source_bus_irq);
    console()->kprintf(PINK, "dest io apic id:    %x,", m_destination_io_apic_id);
    console()->kprintf(PINK, "dest io apic intin: %x\n", m_destination_io_apic_intin);
}

void mp_config_local_interrupt_entry_t::dump()
{
    console()->kprintf(GREEN, "mp config local interrupt entry: \n");
    console()->kprintf(PINK, "interrupt type:        %x\n", m_interrupt_type);
    console()->kprintf(PINK, "local interrupt flag:  %x\n", m_local_interrupt_flag);
    console()->kprintf(PINK, "source bus id:         %x\n", m_source_bus_id);
    console()->kprintf(PINK, "source bus irq:        %x\n", m_source_bus_irq);
    console()->kprintf(PINK, "dest local apic id:    %x\n", m_destination_local_apic_id);
    console()->kprintf(PINK, "dest local apic intin: %x\n", m_destination_local_apic_lintin);
}

uint8 mp_config_t::check_sum(void* addr, int len)
{
    uint8* p = (uint8 *) addr;
    uint8 sum = 0;
    for (int i = 0; i < len; i++) {
        sum += p[i];
    }
    return sum;
}

mp_floating_pointer_t* mp_config_t::find_floating_pointer(uint32 paddr, uint32 len)
{
    uint8* va = (uint8 *) PA2VA(paddr);
    uint32 sz = sizeof(mp_floating_pointer_t);
    if (sz != 16) {
        os()->panic("the size of mp_floating_pointer_t is error!");
    }

    for (char* p = (char *) va; p < (char *) va + len; p += sz) {
        if (strncmp(p, "_MP_", 4) != 0) {
            continue;
        }

        uint8 sum = check_sum(p, sz);
        if (sum == 0) {
            return (mp_floating_pointer_t *) p;
        }
        console()->kprintf(GREEN, "sum = %d\n", sum);
    }

    return NULL;
}

/* a. In the first kilobyte of Extended BIOS Data Area (EBDA), or
 * b. Within the last kilobyte of system base memory (e.g., 639K-640K for systems with 640
 *    KB of base memory or 511K-512K for systems with 512 KB of base memory) if the
 *    EBDA segment is undefined, or
 * c. In the BIOS ROM address space between 0F0000h and 0FFFFFh.
 */
mp_floating_pointer_t* mp_config_t::find_floating_pointer()
{
    const uint32 c_kb = 1024;
    mp_floating_pointer_t* fp = NULL;

    /* 400- 4FF BIOS data area (BDA) */
    uint8* bios_data_area = (uint8 *) PA2VA(0x400);

    /* 40:0E	word	Extended BIOS Data Area segment */
    uint32 extended_bios_data_area = (*(uint16 *)(bios_data_area + 0x0e)) << 4;
    //console()->kprintf(GREEN, "extended_bios_data_area: %p\n", extended_bios_data_area);

    /* a. first kb of ebda */
    if (extended_bios_data_area != NULL) {
        fp = find_floating_pointer(extended_bios_data_area, c_kb);
        //console()->kprintf(GREEN, "look for floating pointer form extended_bios_data_area: %p\n", fp);
        if (fp != NULL) {
            return fp;
        }
    }
    else {
        /* 40:13	word	Memory size in Kbytes */
        uint32 base_memory = (*(uint16 *)(bios_data_area + 0x13)) * c_kb;
        //uint32 base_memory = 640*c_kb;
        //console()->kprintf(GREEN, "system base memory: %p, \n", base_memory);

        /* b. last kb of system base memory */
        fp = find_floating_pointer(base_memory, c_kb);
        //console()->kprintf(GREEN, "look for floating pointer form system base memory: %p\n", fp);
        if (fp != NULL) {
            return fp;
        }
    }

    /* c. In the BIOS ROM address space between 0F0000h and 0FFFFFh. */
    fp = find_floating_pointer(0xf0000, 0x10000);
    //console()->kprintf(GREEN, "look for floating pointer form 0xf0000-0x100000: %p\n", fp);

    return fp;
}

mp_config_table_header_t* mp_config_t::find_config_table()
{
    m_floating_pointer = find_floating_pointer();
    console()->kprintf(PINK, "m_floating_pointer: %p\n", m_floating_pointer);
    if (m_floating_pointer == NULL) {
        return NULL;
    }

    m_floating_pointer->dump();
    m_config_table_header = (mp_config_table_header_t *) PA2VA(m_floating_pointer->m_phy_addr);
    if (m_config_table_header == NULL) {
        return NULL;
    }
    if (strncmp((char *)&m_config_table_header->m_signature, "PCMP", 4) != 0) {
        return NULL;
    }

    uint8 sum = check_sum(m_config_table_header, m_config_table_header->m_table_length);
    if (sum == 0) {
        return m_config_table_header;
    }

    //console()->kprintf(GREEN, "config table header sum = %d\n", sum);
    return NULL;
}

void mp_config_t::parse_config_entries()
{
    uint8* config = (uint8 *) (m_config_table_header + 1);
    uint8* end = (uint8 *) (m_config_table_header) + m_config_table_header->m_table_length;
    while (config < end) {
        switch (*config) {
        case entry_processor:
            {
                mp_config_processor_entry_t* e = (mp_config_processor_entry_t *) config;
                //e->dump();
                os()->get_arch()->add_processor(e->m_local_apic_id, e->m_cpu_flags & 0x2);
                config += sizeof(mp_config_processor_entry_t);
            }
            break;
        case entry_bus:
            {
                mp_config_bus_entry_t* e = (mp_config_bus_entry_t *) config;
                //e->dump();
                config += sizeof(mp_config_bus_entry_t);
            }
            break;
        case entry_io_apic:
            {
                mp_config_io_apic_entry_t* e = (mp_config_io_apic_entry_t *) config;
                //e->dump();
                config += sizeof(mp_config_io_apic_entry_t);
            }
            break;
        case entry_io_interrupt:
            {
                mp_config_io_interrupt_entry_t* e = (mp_config_io_interrupt_entry_t *) config;
                //e->dump();
                config += sizeof(mp_config_io_interrupt_entry_t);
            }
            break;
        case entry_local_interrupt:
            {
                mp_config_local_interrupt_entry_t* e = (mp_config_local_interrupt_entry_t *) config;
                //e->dump();
                config += sizeof(mp_config_local_interrupt_entry_t);
            }
            break;
        default:
            console()->kprintf(RED, "unknown mp config entry type %u!\n", *config);
            break;
        }
    }
}

void mp_config_t::init()
{
    m_floating_pointer = NULL;
    m_config_table_header = NULL;

    console()->kprintf(YELLOW, "---------------------init mp_config...----------------\n");
    m_config_table_header = find_config_table();
    console()->kprintf(PINK, "m_config_table_header: %p\n", m_config_table_header);
    if (m_config_table_header != NULL) {
        m_config_table_header->dump();
        parse_config_entries();
    }

    console()->kprintf(YELLOW, "---------------------init mp_config...----------------\n");
}

