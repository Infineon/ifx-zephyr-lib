/*
 * TinyUSB DWC2 (Infineon PSE84 high-speed) controller integration for Zephyr.
 *
 * Copyright (c) 2026 Infineon Technologies AG,
 * or an affiliate of Infineon Technologies AG.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Connects the on-chip USBHS (DWC2) controller interrupt - taken from the
 * devicetree node labelled 'usbhs' - to TinyUSB's tusb_int_handler() at boot.
 *
 * This lets a Zephyr application drive the TinyUSB stack (tusb_init() +
 * tud_task()/tuh_task()) WITHOUT pulling in a TinyUSB hw/bsp board layer
 * (hw/bsp/board.c + hw/bsp/<family>/family.c) just to wire the controller IRQ.
 * The DWC2 register base, peripheral clocking and PHY bring-up are handled by
 * the driver itself (src/portable/synopsys/dwc2/dwc2_ifx.h), so only the
 * interrupt is connected here.
 *
 * Board-specific concerns that are NOT the controller's job - LED/button GPIOs,
 * USB host VBUS/mux enable, console routing - remain the application's
 * responsibility (via Zephyr devicetree/GPIO/console APIs), and are
 * intentionally out of scope for this file.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/irq.h>
#include <zephyr/init.h>

#include "tusb.h"

/* Single on-chip high-speed controller. The SoC devicetree provides its
 * interrupt line; the register base comes from the Infineon HAL (dwc2_ifx.h). */
#define TINYUSB_DWC2_NODE DT_NODELABEL(usbhs)

#if !DT_NODE_EXISTS(TINYUSB_DWC2_NODE)
#error \
	"CONFIG_TINYUSB_DWC2_IFX_IRQ_CONNECT requires a devicetree node labelled 'usbhs' with an 'interrupts' property"
#endif

/* TinyUSB roothub port index for the single USBHS controller. */
#define TINYUSB_DWC2_RHPORT 0

/* ISR trampoline: tusb_int_handler() services the compiled role (device dcd
 * and/or host hcd) for the given roothub port. */
static void tinyusb_dwc2_ifx_isr(const void *arg)
{
	ARG_UNUSED(arg);
	tusb_int_handler(TINYUSB_DWC2_RHPORT, true);
}

/*
 * Connect and enable the USBHS interrupt. Run after the kernel is up but before
 * the application's main() calls tusb_init(): the DWC2 core does not assert
 * interrupts until it is initialized, so enabling the line early is safe and
 * mirrors the previous hw/bsp board_init() ordering.
 */
static int tinyusb_dwc2_ifx_init(void)
{
	IRQ_CONNECT(DT_IRQN(TINYUSB_DWC2_NODE),
		    DT_IRQ(TINYUSB_DWC2_NODE, priority),
		    tinyusb_dwc2_ifx_isr, NULL, 0);

	irq_enable(DT_IRQN(TINYUSB_DWC2_NODE));

	return 0;
}

SYS_INIT(tinyusb_dwc2_ifx_init, POST_KERNEL,
	 CONFIG_TINYUSB_DWC2_IFX_INIT_PRIORITY);
