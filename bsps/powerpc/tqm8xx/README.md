tqm8xx
======

Summary
-------
```
BSP NAME:           tqm8xx
BOARD:              various boards based on TQ Components TQM8xx modules
BUS:                No backplane. 
CPU FAMILY:         PowerPC
CPU:                PowerPC MPC860 or MPC866
COPROCESSORS:       Built-in Motorola QUICC
MODE:               32 bit mode

BOOT MONITOR:       TQMon 
```

PERIPHERALS
-----------
```
TIMERS:             PIT / Timebase
  RESOLUTION:       1 microsecond / frequency = clock-speed / 16
SERIAL PORTS:       1-4 SCCs, 1-2 SMC
REAL-TIME CLOCK:    <none>
DMA:                Each SCC and SMC.
VIDEO:              <none>
SCSI:               <none>
NETWORKING:         Ethernet 10 Mbps on SCC1 and/or 
                    10/100Mbps on FEC (for MPC866T)		    
```
	   

DRIVER INFORMATION
------------------
```
CLOCK DRIVER:       yes
CONSOLE DRIVER:     yes
SHMSUPP:            N/A
TIMER DRIVER:       yes
NETWORK DRIVER:	    yes
```

NOTES
-----
```
On-chip resources:
	SCC1		network or serial port
	SCC2		serial port
	SCC3		serial port
	SCC4		serial port
	SMC1		serial port
	SMC2		serial port
	CLK1		network
	CLK2		network
	CLK3
	CLK4
	CLK5
	CLK6
	CLK7
	CLK8
	BRG1		console
	BRG2		console
	BRG3		console
	BRG4		console
	RTC
	PIT		clock
	TB
	DEC
	SWT	
	UPMA
	UPMB
	IRQ0
	IRQ1
	IRQ2		
	IRQ3		
	IRQ4		
	IRQ5	
	IRQ6
	IRQ7
	IRQ_LVL0	
	IRQ_LVL1
	IRQ_LVL2
	IRQ_LVL3
	IRQ_LVL4
	IRQ_LVL5
	IRQ_LVL6
	IRQ_LVL7
```

Board description
-----------------
```
Clock rate:     50MHz - 133MHz.
Bus width:      32 bit Flash, 32 bit DRAM
FLASH:	        2-8MB
RAM:            32-256MB SDRAM
```

Console driver
---------------
This BSP contains a console driver for polled and interrupt-driven
operation. It supports SCCs and SMCs.
During BSP configuration, various variables can be set to activate a
certain channels and to specify the console channel:

CONS_SMC1_MODE, CONS_SMC2_MODE, CONS_SCC[1-4]_MODE can be set to
CONS_MODE_UNUSED, CONS_MODE_POLLED or CONS_MODE_IRQ

The driver always uses termios. 


Floating-point
--------------
The MPC8xx do not have floating-point units. All code should
get compiled with the appropriate -mcpu flag. The nof variants of the gcc
runtime libraries should be used for linking.



Miscellaneous
-------------
All development was based on the mbx8xx and gen68360 port.

Test Configuration
------------------
```
Board:                pghplus
CPU:                  Motorola MPC866T
Clock Speed:          133MHz
RAM:                  64MByte
Cache Configuration:  Instruction cache on; data cache on, copyback mode.
```

