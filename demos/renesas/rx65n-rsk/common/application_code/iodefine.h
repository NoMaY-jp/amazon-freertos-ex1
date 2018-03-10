/* 
In case of Renesas RX MCUs, FreeRTOS kernel's "port.c" includes Renesas BSP's "iodefine.h".
For example, it is located at "src/smc_gen/r_bsp/board/generic_rx65n/register_access/".
Therefore it needs an include path "src/smc_gen/r_bsp/board/generic_rx65n/register_access".
But e2 studio removes deeper include path than "r_bsp/" after code generation by built-in smart configurator.
This "iodefine.h" is a workaround for such trouble of e2 studio and smart configurator. Because 
"platform.h" includes "board/generic_rx65n/r_bsp.h"(i.e. "src/smc_gen/r_bsp/board/generic_rx65n/r_bsp.h") then 
"r_bsp.h" includes "mcu/rx65n/register_access/iodefine.h"(i.e. "src/smc_gen/r_bsp/mcu/rx65n/register_access/iodefine.h").
On the other hand, CS+ and stand-alone smart configurator doesn't remove such deeper include path.
To be exact, "src/smc_gen/r_bsp/board/generic_rx65n/register_access" and so on are added forcibly.
This "iodefine.h" works for such case of CS+ and smart configurator too.
*/
#include "platform.h"
