#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <stdio.h>
#include <ucontext.h>
#include <sys/types.h>
#include <bits/sigcontext.h>
#include <features.h>
#include <stdbool.h>
#include <setjmp.h>
#include <sys/mman.h>
#include <errno.h>

char * digits[16] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"a",
	"b",
	"c",
	"d",
	"e",
	"f"
};

char * registers[] = {
	"R8  = ",
	"R9  = ",
	"R10 = ",
	"R11 = ",
	"R12 = ",
	"R13 = ",
	"R14 = ",
	"R15 = ",
	"RDI = ",
	"RSI = ",
	"RBP = ",
	"RBX = ",
	"RDX = ",
	"RAX = ",
	"RCX = ",
	"RSP = "
};

void convert_ull(unsigned long long n, char s[]) {
	s[0] = '0';
	s[1] = 'x';
	for (int i = 0; i < 16; i++) {
		s[i + 2] = digits[n % 16][0];
		n /= 16;
	}
	s[18] = '\n';
	s[19] = '\0';
}

void convert_to_hex(unsigned char c) {
	write(1, digits[c / 16], 1);
	write(1, digits[c % 16], 1);
}

sigjmp_buf j_buffer;
void sighandler_2(int signum) {
	siglongjmp(j_buffer, 1);
}

void dump_memory(unsigned long long address, int length) {
	struct sigaction new_action;
	memset(&new_action, 0, sizeof(new_action));
	new_action.sa_handler = sighandler_2;
	new_action.sa_flags = SA_NODEFER;
	if(sigaction(SIGSEGV, &new_action, NULL) == -1) {
		write(1, strerror(errno), strlen(strerror(errno)));
		exit(0);
	}

    for (int i = -length; i < length; i++) {
    	if(i % 16 == 0) {
        	write(1, "\n", 1);
        }
    	if (i == 0) {
    		write (1, "[", 1);
    	}
	if (address + i <= 0) {
		write (1, "  ", 2);
	} else {
    	`	if(sigsetjmp(j_buffer, 0)) {
    			write(1, "**", 2);
    		} else {
    			unsigned char * pc = (unsigned char *) (address + i);
        		convert_to_hex(pc[0]);
    		}
	}
    	if (i == 0) {
    		write (1, "]", 1);
    	}
    	write (1, " ", 1);
    }
}

void sighandler(int signum, siginfo_t * si, void * arg) {
	ucontext_t context, *cp = &context;
	getcontext(cp);

	write(1, "Segmentation fault occured at address ", 38);
	char add[20];
	unsigned long long address = (unsigned long long)si->si_addr;
	convert_ull(address, add);
	write (1, add, strlen(add));
	write(1, "\n\nRegisters:\n\n", 14);
	for(int i = 0;i < 16;i++) {
		char val[20];
		convert_ull(cp->uc_mcontext.gregs[i], val);
		write(1, registers[i], strlen(registers[i]));
		write(1, val, strlen(val));
	}
	write(1, "\n", 1);

	write(1, "Memory dump:", 12);
	dump_memory(address, 64);
	write(1, "\nMemory dump end\n", 17);
	exit(0);
}

int main(int argc, char * argv[]) {
	struct sigaction action;
	memset(&action, 0, sizeof(action));

	action.sa_sigaction = sighandler;
	action.sa_flags = SA_SIGINFO | SA_NODEFER;
	
	if(sigaction(SIGSEGV, &action, NULL) == -1) {
		write(1, strerror(errno), strlen(strerror(errno)));
		exit(0);
	} 
	char * mem = (mmap(NULL, 4096, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1,0));
	mem[0] = 'x';
	return 0;
}
