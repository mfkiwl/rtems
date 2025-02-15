leon2
=====
Description of SIS as related to this BSP


```
BSP NAME:           sis
BOARD:              any based on the European Space Agency's ERC32
BUS:                N/A
CPU FAMILY:         sparc
CPU:                ERC32 (SPARC V7 + on-CPU peripherals)
                    based on Cypress 601/602
COPROCESSORS:       on-chip 602 compatible FPU
MODE:               32 bit mode

DEBUG MONITOR:      none
```

PERIPHERALS
-----------
```
TIMERS:
  NAME:             General Purpose Timer  
  RESOLUTION:         50 nanoseconds - 12.8 microseconds
  NAME:             Real Time Clock Timer  
  RESOLUTION:         50 nanoseconds - 3.2768 milliseconds
  NAME:             Watchdog Timer  
  RESOLUTION:         XXX
SERIAL PORTS:       2 using on-chip UART
REAL-TIME CLOCK:    none
DMA:                on-chip
VIDEO:              none
SCSI:               none
NETWORKING:         none
```

DRIVER INFORMATION
------------------
```
CLOCK DRIVER:       ERC32 internal Real Time Clock Timer
IOSUPP DRIVER:      N/A
SHMSUPP:            N/A
TIMER DRIVER:       ERC32 internal General Purpose Timer
CONSOLE DRIVER:     ERC32 internal UART 
```

STDIO
-----
```
PORT:               Channel A
ELECTRICAL:         na since using simulator
BAUD:               na
BITS PER CHARACTER: na
PARITY:             na
STOP BITS:          na
```

Notes
-----
ERC32 BSP only supports single processor operations.

A nice feature of this BSP is that the RAM and PROM size are set in the 
linkcmds file and the startup code programs the Memory Configuration
Register based on those sizes.

The Watchdog Timer is disabled.

This code was developed and tested entirely using the SPARC Instruction 
Simulator (SIS) for the ERC32.  All tests were known to run correctly
against sis v1.7.


Memory Map
----------
```
0x00000000 - 0x00000000 + _PROM_SIZE      code and initialized data
0x01f80000                                on chip peripherals
0x00000000 - 0x02000000 + _RAM_SIZE       destination for initialized data
                                          BSS (i.e. unitialized data)
                                          C Heap (i.e. malloc area)
                                          RTEMS Workspace

The C heap is assigned all memory between the end of the BSS and the 
RTEMS Workspace.  The size of the RTEMS Workspace is based on that 
specified in the application's configuration table.
```
  
