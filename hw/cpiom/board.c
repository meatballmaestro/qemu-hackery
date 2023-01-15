#include <stdint.h>

#include "qemu/compiler.h"
#include "qemu/osdep.h"

#include "hw/core/cpu.h"
#include "qemu/typedefs.h"

#include "hw/boards.h"
/* #include "qemu/typedefs.h" */


#include "target/ppc/cpu.h"

/* #include "exec/memory.h" */
/* #include "sysemu/blockdev.h" */
/* #include "sysemu/accel.h" */
/* #include "qapi/qapi-types-machine.h" */
/* #include "qemu/module.h" */
/* #include "qom/object.h" */

void cpiom_machine_init(MachineClass *mc);

DEFINE_MACHINE("cpiom", cpiom_machine_init)
;


static void cpiom_init(MachineState *mcs);
static void cpiom_init_cpu(cpiom_t *cpiom, MachineState *mcs);

void cpiom_machine_init(MachineClass *mc)
{
    mc->desc = "CPIOM board";
    mc->init = cpiom_init;
    mc->default_cpu_type = POWERPC_CPU_TYPE_NAME("755_v2.8");
    mc->default_ram_size = CPIOM_MMAP_SDRAM_SIZE;
}

static void cpiom_init(MachineState *mcs)
{
    cpiom_t *cpiom = g_new0(cpiom_t, 1);

    cpiom_init_cpu(cpiom, mcs);
    cpiom_init_dev(cpiom);
    cpiom_init_boot(cpiom, mcs);
}

static void cpiom_init_cpu(cpiom_t *cpiom, MachineState *mcs)
{
    cpiom->cpu = POWERPC_CPU(cpu_create(mcs->cpu_type));

    /* Set time-base frequency to 16.6 Mhz  */
    cpu_ppc_tb_init(&cpiom->cpu->env, CPIOM_TBFREQ);

    cpiom->cpu->env.spr[SPR_HID2] = 0x00040000;

    CPUClass *cc = CPU_GET_CLASS( CPU(cpiom->cpu) );
    PowerPCCPUClass *pcc = POWERPC_CPU_CLASS(cc);
    CPUPPCState *env = &cpiom->cpu->env;

    /* Default mmu_model or MPC755 is POWERPC_MMU_SOFT_6xx (no softmmu)  */
    pcc->mmu_model = POWERPC_MMU_32B;
    env->mmu_model = POWERPC_MMU_32B;
    pcc->handle_mmu_fault = ppc_hash32_handle_mmu_fault;    
}
