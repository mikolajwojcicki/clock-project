#ifndef SDK_CONFIG_H
#define SDK_CONFIG_H

/* nrfx drivers used by product firmware. Enable only what is linked. */

#define NRFX_SPIM_ENABLED 1
#define NRFX_SPIM0_ENABLED 1
#define NRF_LOG_ENABLED 0
#define NRFX_SPIM_DEFAULT_CONFIG_IRQ_PRIORITY 6
/* The old register code left MISO without a pull. */
#define NRFX_SPIM_MISO_PULL_CFG 0

#endif
