#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <byteswap.h>

#include "m68k.h"
long count=0;
long chipcount=0;
long fastcount=0;

unsigned int fast_start = 0x40000000; // location of zorro 384MB
unsigned int rtg_start = 0x2000000;
unsigned int fast_span = 384*1024*1024;

unsigned char volatile * chipram;
unsigned char volatile * chiprom;
unsigned char volatile * fastram;
unsigned char volatile * rtgram;
unsigned char volatile * irqs;

unsigned int addr = 0xc0000000;
unsigned int chip_span = 0x1000000;
unsigned int rtg_span = 0x800000;

//unsigned int fast_start = 0x40000000; // location of zorro 384MB
//unsigned int fast_span = 384*1024*1024;

//unsigned int fast_base = 0x28000000; // physical ddr address, shared with f2h bridge

unsigned int handle_irq(unsigned int level)
{
	unsigned char irq = *irqs;
	m68k_set_irq(7&(~irq));
	return M68K_INT_ACK_AUTOVECTOR;
}

char* m68ki_disassemble_quick(unsigned int pc, unsigned int cpu_type);
void m68hook(int pc)
{
	//printf("PC:%x:%s\n",pc,m68ki_disassemble_quick(pc,M68K_CPU_TYPE_68EC020));
}

int main(void)
{
	// map chipram
	unsigned int hpsbridgeaddr = 0xc0000000;
	int fduncached = open("/dev/mem",(O_RDWR|O_SYNC));
	int fdcached = open("/sys/kernel/debug/minimig_irq/mmap_cached",(O_RDWR));
	
	int chipram_bytes = 15*1024*1024;
	chipram = mmap(NULL,chipram_bytes,(PROT_READ|PROT_WRITE),MAP_SHARED,fduncached,hpsbridgeaddr+0); //cached?
	int rom_bytes = 1*1024*1024;
	chiprom = mmap(NULL,rom_bytes,(PROT_READ|PROT_WRITE),MAP_SHARED,fdcached,hpsbridgeaddr+0xf00000);
	chiprom = chiprom-0xf00000;
	int z3fastram_bytes = 384*1024*1024;
	//int z3fastram_bytes = 8*1024*1024;
	unsigned int z3fast_physical = 0x28000000; // physical ddr address, shared with f2h bridge
	fastram = mmap(NULL,z3fastram_bytes,(PROT_READ|PROT_WRITE),MAP_SHARED,fdcached,z3fast_physical);

	unsigned int rtg_physical = 0x27000000; 
	int rtgram_bytes = 8*1024*1024;
	rtgram = mmap(NULL,rtgram_bytes,(PROT_READ|PROT_WRITE),MAP_SHARED,fduncached,rtg_physical);
	//
	unsigned int irqoffset = 0x1000000;
	irqs = mmap(NULL,1,(PROT_READ|PROT_WRITE),MAP_SHARED,fduncached,hpsbridgeaddr+irqoffset); //cached?
	fprintf(stderr,"%p:%p:%p:%p:%p\n",chipram,chiprom,irqs,fastram,rtgram);

if (0)
{
	int now = clock();
	for (unsigned int i=0x100000;i!=(0x200000);++i)
	{
		void volatile * loc;
		if (!validate_addr(i,&loc))
		{
			fprintf(stderr,"CHIP:Validate failed:%d\n",i);
			exit(-1);
		}

		*((unsigned char volatile *)(loc)) = i&0xff;
		unsigned int stored = *((unsigned char volatile *)(loc));
		if (stored != (i&0xff))
		{
			unsigned int stored2 = *((unsigned char volatile *)(loc));
			fprintf(stderr,"CHIP:Save/load failed:%d  %d:%d:%d\n",i,i&0xff,stored,stored2);
			exit(-1);
		}
	}
	int now2 = clock();
	fprintf(stderr,"Passed\n");
        double elapsed = ((double)(now2-now))/CLOCKS_PER_SEC;
        printf("Elapsed:%f\n",elapsed);
        int bytes = 0x100000;
        printf("Bytes/sec:%f\n",bytes/elapsed);
}

if (0)
{
	int now = clock();
	for (unsigned int i=fast_start;i!=(fast_start+fast_span);i+=4)
	{
		m68k_write_memory_32(i,i);
		unsigned int exp = m68k_read_memory_32(i);
		if (exp!=i)
		{
			fprintf(stderr,"FAST:Save/load failed:%d:%d\n",i,exp);
			exit(-1);
		}
	}
	int now2 = clock();
	fprintf(stderr,"Passed\n");
        double elapsed = ((double)(now2-now))/CLOCKS_PER_SEC;
        printf("Elapsed:%f\n",elapsed);
        int bytes = fast_span;
        printf("Bytes/sec:%f\n",bytes/elapsed);

	for (unsigned int i=fast_start;i!=(fast_start+fast_span);i+=4)
	{
		unsigned int exp = m68k_read_memory_32(i);
		if (exp!=i)
		{
			fprintf(stderr,"VERIFY FAST:Save/load failed:%d:%d\n",i,exp);
			exit(-1);
		}
	}

}

	// set cpu
	m68k_init();
	m68k_set_cpu_type(M68K_CPU_TYPE_68020);
 	m68k_pulse_reset();
	while (1)
	{
		m68k_execute(10);
		unsigned char irq = *irqs;
		if ((irq&8)==0)
		{
			printf("RESET\n");
 			m68k_pulse_reset();
			continue;
		}
		irq = irq&0x7;
	//	if (irq!=7)
	//		printf("irq:%x\n",irq);
		m68k_set_irq(7&(~irq));
	}

//You can use the functions m68k_cycles_run(), m68k_cycles_remaining(),
//m68k_modify_timeslice(), and m68k_end_timeslice() to do this.
}


unsigned int m68k_read_disassembler_32(unsigned int address)
{
	return m68k_read_memory_32(address);
}
unsigned int m68k_read_disassembler_16(unsigned int address)
{
	return m68k_read_memory_16(address);
}

