#include <stdio.h>

#include "qemu/osdep.h"
#include "qapi/error.h" // error_fatal handler
#include "hw/sysbus.h" // sysbus registration

#include "hw/misc/banana_rom.h"

#define TYPE_BANANA_ROM "banana_rom"
typedef struct BananaRomState BananaRomState;
DECLARE_INSTANCE_CHECKER(BananaRomState, BANANA_ROM, TYPE_BANANA_ROM)

#define REG_ID 0x0
#define CHIP_ID 0xBA000001

struct BananaRomState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    uint64_t chip_id;
};

static uint64_t banana_rom_read(void *opaque, hwaddr addr, unsigned int size)
{
    BananaRomState *s = opaque;

    switch (addr) {
    case REG_ID:
        return s->chip_id;
        break;

    default:
        return 0xDEADBEEF;
        break;
    }

    return 0;
}

static void banana_rom_write(void *opaque, hwaddr addr, uint64_t data, unsigned size)
{
    FILE *log = fopen("/home/kmstout/log", "a");
    fprintf(log, "writing %u bytes to %lx\n", size, addr);
    fclose(log);
}
    
static const MemoryRegionOps banana_rom_ops = {
    .read = banana_rom_read,
    .write = banana_rom_write,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void banana_rom_instance_init(Object *obj)
{
    BananaRomState *s = BANANA_ROM(obj);

    memory_region_init_io(&s->iomem, obj, &banana_rom_ops, s, TYPE_BANANA_ROM, 0x100);
    sysbus_init_mmio(SYS_BUS_DEVICE(obj), &s->iomem);

    s->chip_id = CHIP_ID;
}

static const TypeInfo banana_rom_info = {
    .name = TYPE_BANANA_ROM,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(BananaRomState),
    .instance_init = banana_rom_instance_init,
};

static void banana_rom_register_types(void)
{
    type_register_static(&banana_rom_info);
}

type_init(banana_rom_register_types)

DeviceState* banana_rom_create(hwaddr addr)
{
    DeviceState *dev = qdev_new(TYPE_BANANA_ROM);
    sysbus_realize_and_unref(SYS_BUS_DEVICE(dev), &error_fatal);
    sysbus_mmio_map(SYS_BUS_DEVICE(dev), 0, addr);
    FILE *log = fopen("/home/kmstout/log", "w");
    fprintf(log, "created device\n");
    fclose(log);
    return dev;
}
