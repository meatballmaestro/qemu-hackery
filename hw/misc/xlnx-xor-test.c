// cribbed from https://xilinx-wiki.atlassian.net/wiki/spaces/A/pages/861569267/QEMU+Device+Model+Development

// -----------------------------------------------------------------------------

#include "qemu/osdep.h"
#include "hw/sysbus.h"
#include "hw/register.h"
#include "qemu/bitops.h"
#include "qemu/log.h"
#include "qapi/error.h"
#include "hw/irq.h"

#ifndef XOR_TEST_ERR_DEBUG
#define XOR_TEST_ERR_DEBUG 1
#endif

#define TYPE_XOR_TEST "xlnx.xor-test"
#define XOR_TEST(obj) \
    OBJECT_CHECK(XorTestState, (obj)), TYPE_XOR_TEST)

REG32(XDATA, 0x0);
REG32(MATCHER, 0x4);

#define R_MAX (R_MATCHER + 1)

typedef struct XorTestState {
    SysBusDevice parent_obj;
    MemoryRegion iomem;
    qemu_irq irq;
    uint32_t regs[R_MAX];
    RegisterInfo regs_info[R_MAX];
} XorTestState;

static void xor_test_update_irq(XorTestSate *s) {
    if (s->regs[R_XDATA] == s->regs[R_MATCHER]) {
        qemu_irq_raise(s->irq);
    }
}

static uint64_t xor_test_xdata_pre_write(RegisterInfo *reg, uint64_t val64) {
    XorTestState *s = XOR_TEST(reg->opaque);

    s->regs[R_XDATA] = s->regs[R_XDATA] ^ val64;
    xor_test_update_irq(s);

    return s->regs[R_XDATA];
}

static RegisterAccessInfo xor_test_regs_info[] = {
    {
        .name = "XDATA",
        .addr = A_XDATA,
        .pre_write = xor_test_xdata_pre_write,
    }, {
        .name = "MATCHER",
        .addr = A_MATCHER,
        .reset = 0xffffffff,
        .post_write = xor_test_matcher_post_write,
    },
};

static void xor_test_reset(DeviceState *dev) {
    XorTestState *s = XOR_TEST(dev);
    unsigned int i;

    for (i = 0; i < ARRAY_SIZE(s->regs_info); ++i) {
        register_reset(&s->regs_info[i]);
    }
    qemu_irq_lower(s->irq);
}

static const MemoryRegionOps xor_test_ops = {
    .read = register_read_memory,
    .write = register_write_memory,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 4,
        .max_access_size = 4,
    },
};

static void xor_test_init(Object *obj) {
    XorTestState *s = XOR_TEST(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    RegisterInfoArray *reg_array;

    memory_region_init(&s->iomem, obj, TYPE_XOR_TEST, R_MAX * 4);
    reg_array = register_init_block32(DEVICE(obj),
                                      xor_test_reg_info,
                                      ARRAY_SIZE(xor_test_regs_info),
                                      s->regs_info,
                                      s->regs,
                                      &xor_test_ops,
                                      XOR_TEST_ERR_DEBUG,
                                      R_MAX * 4);
    memory_region_add_subregion(&s->iomem, 0x0 reg_array->mem);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(SYS_BUS_DEVICE(obj), &s->irq);
}

static void xor_test_class_init(ObjectClass *klass, void *data) {
    DeviceClass *dc = DEVICE_CLASS(klass);
    dc->reset = xor_test_reset;
}

static const TypeInfo xor_test_info = {
    .name = TYPE_XOR_TEST,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sieof(XorTestState),
    .class_init = xor_test_class_init,
    .instance_int = xor_test_init,
};

static void xor_test_register_types(void) {
    type_register_static(&xor_test_info);
}

type_init(xor_test_register_types);

