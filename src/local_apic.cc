/*
 * guzhoudiaoke@126.com
 * 2018-01-25
 */

#include "local_apic.h"
#include "babyos.h"
#include "x86.h"
#include "delay.h"

#define CALIBRATE_LOOP              (HZ/10)
#define MSR_IA32_APICBASE		    (0x1b)
#define MSR_IA32_APICBASE_ENABLE	(1<<11)

#define	APIC_ID		0x20
#define	APIC_LVR	0x30
#define APIC_TPR    0x80
#define	APIC_EOI	0xb0
#define	APIC_SPIV	0xf0

/* Local Vector Table */
#define APIC_LVT_CMCI   0x2f0   /* LVT Machine Check */
#define APIC_ICR_LOW    0x300   /* Interrupt Command Register 0-31 */
#define APIC_ICR_HIGH   0x310   /* Interrupt Command Register 32-63 */
#define	APIC_LVT_TIMER	0x320   /* LVT Timer register */
#define	APIC_LVT_THMR	0x330   /* LVT Thermal Sensor register */
#define	APIC_LVT_PMCR	0x340   /* LVT Performance Monitoring Counter register */
#define	APIC_LVT_LINT0	0x350   /* LVT LINT0 register */
#define	APIC_LVT_LINT1	0x360   /* LVT LINT1 register */
#define	APIC_LVT_ERROR	0x370   /* LVT Error register */

/* Timer */
#define	APIC_TIMER_ICT	0x380   /* Timer initial count register */
#define	APIC_TIMER_CCT	0x390   /* Timer current count register */
#define	APIC_TIMER_DCR	0x3E0   /* Timer divide configuration register */

#define MASKED          0x00010000   /* Interrupt masked */

/* ICR */
#define APIC_DM_INIT        0x500
#define APIC_DM_STARTUP     0x600
#define APIC_INT_ASSERT     0x4000
#define APIC_INT_LEVEL_TRIG 0x8000

/* SPIV */
#define APIC_SOFT_ENABLE    (0x100)

static __inline void apic_write(uint32 reg, uint32 v)
{
	*((volatile uint32 *)(APIC_BASE+reg)) = v;
}

static __inline void apic_write64(uint64 reg, uint64 v)
{
	*((volatile uint64 *)(APIC_BASE+reg)) = v;
}

static __inline uint32 apic_read(uint32 reg)
{
	return *((volatile uint32 *)(APIC_BASE+reg));
}

static __inline uint64 apic_read64(uint32 reg)
{
	return *((volatile uint64 *)(APIC_BASE+reg));
}


int local_apic_t::check()
{
    uint32 edx = 0, ecx = 0;
    __asm__ volatile("cpuid" : "=d" (edx), "=c" (ecx) : "a" (0x1) : "memory", "cc");

    //console()->kprintf(YELLOW, "**************** check apic **********************\n");
    //console()->kprintf(YELLOW, "local_apic check, ecx: %x, edx: %x\n", ecx, edx);
    //console()->kprintf(YELLOW, "support APIC:   %s\n", (edx & (1 << 9))  ? "YES" : "NO");
    //console()->kprintf(YELLOW, "support x2APIC: %s\n", (ecx & (1 << 21)) ? "YES" : "NO");
    //console()->kprintf(YELLOW, "**************** check apic **********************\n");
    //console()->kprintf(YELLOW, "\n");

    //console()->kprintf(CYAN, "**************** MSR IA_32_APICBASE ***************\n");
    //uint32 l, h;
    //rdmsr(MSR_IA32_APICBASE, l, h);
    //console()->kprintf(CYAN, "MSR IA_32_APICBASE: %x, %x\n", h, l);
    //console()->kprintf(CYAN, "**************** MSR IA_32_APICBASE ***************\n");
    //console()->kprintf(CYAN, "\n");

    //console()->kprintf(WHITE, "******************** local APIC register *****************\n");
    //uint32 val = 0;
    m_id = apic_read(APIC_ID) >> 24;
    //console()->kprintf(WHITE, "APIC ID:                                 %x\n", m_id);
    //val = apic_read(APIC_LVR);
    //console()->kprintf(WHITE, "APIC version:                            %x\n", val);
    //val = apic_read(APIC_SPIV);
    //console()->kprintf(WHITE, "APIC Spurious interrupt vertor register: %x\n", val);
    //console()->kprintf(WHITE, "******************** local APIC register *****************\n");
    //console()->kprintf(WHITE, "\n");

    //console()->kprintf(GREEN, "******************** local vector table *****************\n");
    //val = apic_read(APIC_LVT_CMCI);
    //console()->kprintf(GREEN, "lvt machine check:       %x\n", val);
    //val = apic_read(APIC_LVT_TIMER);
    //console()->kprintf(GREEN, "lvt timer:               %x\n", val);
    //val = apic_read(APIC_LVT_THMR);
    //console()->kprintf(GREEN, "lvt thermal sensor       %x\n", val);
    //val = apic_read(APIC_LVT_PMCR);
    //console()->kprintf(GREEN, "lvt performance monitor: %x\n", val);
    //val = apic_read(APIC_LVT_LINT0);
    //console()->kprintf(GREEN, "lvt lint0:               %x\n", val);
    //val = apic_read(APIC_LVT_LINT1);
    //console()->kprintf(GREEN, "lvt lint1:               %x\n", val);
    //val = apic_read(APIC_LVT_ERROR);
    //console()->kprintf(GREEN, "lvt error:               %x\n", val);
    //console()->kprintf(GREEN, "******************** local vector table *****************\n");
    //console()->kprintf(GREEN, "\n");

    //console()->kprintf(PINK, "*************************** timer ************************\n");
    //val = apic_read(APIC_TIMER_ICT);
    //console()->kprintf(PINK, "timer initial count register:        %x\n", val); 
    //val = apic_read(APIC_TIMER_CCT);
    //console()->kprintf(PINK, "timer current count register:        %x\n", val);
    //val = apic_read(APIC_TIMER_DCR);
    //console()->kprintf(PINK, "timer divide configuration register: %x\n", val);
    //console()->kprintf(PINK, "*************************** timer ************************\n");
    //console()->kprintf(PINK, "\n");

    /* support APIC */
    if (edx & (1 << 9)) {
        return 0;
    }

    return -1;
}

/*
 * When IA32_APIC_BASE[11] is set to 0, processor APICs based on the 3-wire APIC bus 
 * cannot be generally re-enabled until a system hardware reset.
 */
void local_apic_t::global_enable()
{
    uint32 l, h;
    rdmsr(MSR_IA32_APICBASE, l, h);
    l |= MSR_IA32_APICBASE_ENABLE;
    wrmsr(MSR_IA32_APICBASE, l, h);
}

void local_apic_t::global_disable()
{
    uint32 l, h;
    rdmsr(MSR_IA32_APICBASE, l, h);
    l &= ~MSR_IA32_APICBASE_ENABLE;
    wrmsr(MSR_IA32_APICBASE, l, h);
}

void local_apic_t::software_enable()
{
    uint32 val = apic_read(APIC_SPIV);
    val |= (APIC_SOFT_ENABLE);
    apic_write(APIC_SPIV, val);
}

void local_apic_t::software_disable()
{
    uint32 val = apic_read(APIC_SPIV);
    val &= ~(APIC_SOFT_ENABLE);
    apic_write(APIC_SPIV, val);
}

void local_apic_t::test()
{
    check();
}

void setup_lvt_timer(uint32 clocks)
{
    /* (1 << 17) means timer mode Periodic */
    apic_write(APIC_LVT_TIMER, (1 << 17) | VEC_LOCAL_TIMER);

    /* set timer divide configuration as 111b(0xb's 0,1,3 bit)
     * so the divide value is 1, 
     * so timer's rate will = processor's bus clock / 1
     */
    apic_write(APIC_TIMER_DCR, 0xb);

    /* set timer's initial count as clocks */
    apic_write(APIC_TIMER_ICT, clocks);
}

/* get 8254 timer count, until wraparound */
static void wait_8254_wraparound()
{
    uint32 current_count, prev_count;
    int32 delta = 0;

    current_count = os()->get_arch()->get_8254()->get_timer_count();
    do {
        prev_count = current_count;
        current_count = os()->get_arch()->get_8254()->get_timer_count();
        delta = current_count - prev_count;
    } while (delta < 300);
}

uint32 local_apic_t::calibrate_clock()
{
    uint64 tsc_begin, tsc_end;

    /* write a max value to APIC timeout */
    setup_lvt_timer(0xffffffff);

    /* wait 8254 start a new round */
    wait_8254_wraparound();

    /* begin */
    rdtsc64(tsc_begin);
    uint32 apic_begin = apic_read(APIC_TIMER_CCT);

    /* wait */
    for (int i = 0; i < CALIBRATE_LOOP; i++) {
        wait_8254_wraparound();
    }

    /* end */
    rdtsc64(tsc_end);
    uint32 apic_end = apic_read(APIC_TIMER_CCT);

    uint32 clocks = (uint32) (apic_begin - apic_end);
    uint32 tsc_delta = (uint32) ((tsc_end - tsc_begin));
    delay_t::init(tsc_delta / CALIBRATE_LOOP * HZ);

    console()->kprintf(CYAN, "********** calibrate local APIC clock *********\n");
    console()->kprintf(CYAN, "tsc speed: %u.%u MHz.\n", 
            (tsc_delta/CALIBRATE_LOOP) / (1000000/HZ),
            (tsc_delta/CALIBRATE_LOOP) % (1000000/HZ));
    
    console()->kprintf(CYAN, "bus clock speed: %u.%u MHz.\n", 
            (clocks / CALIBRATE_LOOP) / (1000000/HZ),
            (clocks / CALIBRATE_LOOP) % (1000000/HZ));
    console()->kprintf(CYAN, "********** calibrate local APIC clock *********\n");

    return clocks / CALIBRATE_LOOP;
}

int local_apic_t::init_timer()
{
    if (os()->get_arch()->get_current_cpu()->is_bsp()) {
        m_clocks = calibrate_clock();
    }
    else {
        m_clocks = os()->get_arch()->get_boot_processor()->get_local_apic()->get_clocks();
    }
    setup_lvt_timer(m_clocks);

    return 0;
}

int local_apic_t::init()
{
    m_tick = 0;

    if (check() != 0) {
        return -1;
    }

    if (init_timer() != 0) {
        return -1;
    }

    apic_write(APIC_SPIV, APIC_SOFT_ENABLE | VEC_SPURIOUS);

    /* mask performance monitor counter, LINT0, LINT1 */
    apic_write(APIC_LVT_PMCR,  (1 << 16));
    apic_write(APIC_LVT_LINT0, (1 << 16));
    apic_write(APIC_LVT_LINT1, (1 << 16));

    /* error */
    apic_write(APIC_LVT_ERROR, VEC_ERROR);

    /* ack outstanding interrupts */
    eoi();

    /* enable interrupt on APIC */
    apic_write(APIC_TPR, 0);

    return 0;
}

void local_apic_t::do_timer_irq()
{
    m_tick++;
    if ((uint32)m_tick % 100 == 0) {
        m_id = apic_read(APIC_ID) >> 24;
        //console()->kprintf(CYAN, "apic %u, tick %u\n", m_id, m_tick);
    }

    os()->update(m_tick);
}

void local_apic_t::eoi()
{
    apic_write(APIC_EOI, 0);
}

void local_apic_t::start_ap(uint32 id, uint32 addr)
{
    /* 
     * The operating system places the address of a HLT
     * instruction in the warm-reset vector (40:67), sets the CMOS shut-down code to 0Ah, then sends an
     * INIT IPI to the AP. The INIT IPI causes the AP to enter the BIOS POST routine, where it
     * immediately jumps to the warm-reset vector and executes the operating system’s HLT instruction.
     * Only one processor can execute the shutdown routine at any given time, due to the use of the
     * shutdown code. 
     */
    cmos_write(0xf, 0xa);
    uint32* warm_reset_vec = (uint32 *) PA2VA(0x467);
    *warm_reset_vec = (addr >> 4) << 16;

    /* 
     * An AP may be started either by the BSP or by another active AP. The operating system causes
     * application processors to start executing their initial tasks in the operating system code by using the
     * following universal algorithm. The algorithm detailed below consists of a sequence of
     * interprocessor interrupts and short programmatic delays to allow the APs to respond to the wakeup
     * commands. The algorithm shown here in pseudo-code assumes that the BSP is starting an AP for
     * documentation convenience. The BSP must initialize BIOS shutdown code to 0AH and the warm
     * reset vector (DWORD based at 40:67) to point to the AP startup code prior to executing the
     * following sequence:
     *      BSP sends AP an INIT IPI
     *      BSP DELAYs (10mSec)
     *      If (APIC_VERSION is not an 82489DX) {
     *          BSP sends AP a STARTUP IPI
     *          BSP DELAYs (200μSEC)
     *          BSP sends AP a STARTUP IPI
     *          BSP DELAYs (200μSEC)
     *      }
     *      BSP verifies synchronization with executing AP 
     */
    apic_write(APIC_ICR_HIGH, id << 24);
    apic_write(APIC_ICR_LOW,  APIC_INT_LEVEL_TRIG | APIC_INT_ASSERT | APIC_DM_INIT);
    delay_t::ms_delay(10);

    apic_write(APIC_ICR_HIGH, id << 24);
    apic_write(APIC_ICR_LOW,  APIC_INT_LEVEL_TRIG | APIC_DM_INIT);

    for (int i = 0; i < 2; i++) {
        apic_write(APIC_ICR_HIGH, id << 24);
        apic_write(APIC_ICR_LOW,  APIC_DM_STARTUP | (addr >> 12));
        delay_t::us_delay(200);
    }
}

uint32 local_apic_t::get_apic_id()
{
    uint32 id = apic_read(APIC_ID) >> 24;
    return id;
}

uint32 local_apic_t::get_clocks()
{
    return m_clocks;
}

uint64 local_apic_t::get_tick()
{
    return m_tick;
}

