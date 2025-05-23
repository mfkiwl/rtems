Motorola PowerPC
================

The code in this directory has been taken WITH PERMISSION from 
Gabriel Paubert, paubert@iram.es. The main reason for having
a separate bootloader for PreP compliant firmware is that the 
initial code is relocated by firmware at an unknow address 
(actually  0x5000 on motorola MCP750) and that as Gabriel I 
think having a relocatable bootloder code is a must.

So the way of building a binary executable that can be booted via
hard disk or network boot goes like this :

	- make a RTEMS executable,
	- put is as data section in the bootloder binary,
	- relink the loader (see make-exe macros for details),

I would like to thank Gabriel for his support and his code.
The original code can be found in form of a patch to official linux 
kernel at (I insist not vger ppc kernel or Imac ppc kernels!!) :

<ftp://vlab1.iram.es/pub/linux-2.2/> 

After applying the patch, the code is located in a new directory 
called prepboot.

(NB : note use ftp not netscape...)

Note that the actual code differs a lot since Gabriel choose to use
a CHRP compliant mapping instead of a Prep Mapping to save
BATs. I had no time to upgrade the code to its new one allthough
I agree it should be done...

I have also splitted the original code to have a more modular
design enabling to reuse code between the loader and RTEMS
initialization (e.g printk, ...).

Eric Valette (eric.valette@free.fr)


2003/5/7, Greg Menke, gregory.menke@gsfc.nasa.gov

Reworked the pci bus 0 initialization a little and added support for
configuring an arbitrary number of other busses & their respective
bridges.  Also added support for configuring IO ranges below 0x10000,
which I think is reasonable given this is a PowerPC bsp.
