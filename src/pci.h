/*
 * guzhoudiaoke@126.com
 * 2018-03-01
 */

#ifndef _PCI_H_
#define _PCI_H_

#include "types.h"
#include "pool.h"
#include "list.h"

class pci_device_bar_t {
public:
    void init(uint32 addr_reg_val, uint32 len_reg_val);
    void dump();

    enum type_e {
        TYPE_INVALID = -1,
        TYPE_MEM = 0,
        TYPE_IO,
    };

public:
    uint32 m_type;
    uint32 m_base_addr;
    uint32 m_length;
};

class pci_device_t {
public:
    void    init(uint8 bus, uint8 dev, uint8 function, uint16 vendor_id, uint16 device_id, 
                 uint32 class_code, uint8 revision, bool multi_function);
    void    dump();
    uint32  get_io_addr();
    uint32  get_interrupt_line();

public:
    uint8               m_bus;
    uint8               m_dev;
    uint8               m_function;

    uint16              m_vendor_id;
    uint16              m_device_id;
    uint32              m_class_code;
    bool                m_multi_function;
    uint8               m_interrupt_line;
    uint8               m_interrupt_pin;
    uint8               m_revision;
    pci_device_bar_t    m_bar[6];
};

class pci_t {
public:
    void init();
    pci_device_t* get_device(uint16 vendor_id, uint16 device_id);
    void enable_bus_mastering(pci_device_t* device);

private:
    void enum_buses();
    void check_device(uint8 bus, uint8 device);
    uint32 read(uint32 bus, uint32 device, uint32 function, uint32 addr);
    void write(uint32 bus, uint32 device, uint32 function, uint32 addr, uint32 val);

private:
    object_pool_t           m_device_pool;
    list_t<pci_device_t*>   m_devices;
};

#endif
