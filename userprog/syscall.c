#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/loader.h"
#include "userprog/gdt.h"
#include "threads/flags.h"
#include "intrinsic.h"
#include <debug.h>
#include <string.h>

// TBD chobae : project2 api add
#include <filesys/filesys.h>
#include <init.h>
#include <process.h>
#include <file.h>

void syscall_entry (void);
void syscall_handler (struct intr_frame *);

/* System call.
 *
 * Previously system call services was handled by the interrupt handler
 * (e.g. int 0x80 in linux). However, in x86-64, the manufacturer supplies
 * efficient path for requesting the system call, the `syscall` instruction.
 *
 * The syscall instruction works by reading the values from the the Model
 * Specific Register (MSR). For the details, see the manual. */

#define MSR_STAR 0xc0000081         /* Segment selector msr */
#define MSR_LSTAR 0xc0000082        /* Long mode SYSCALL target */
#define MSR_SYSCALL_MASK 0xc0000084 /* Mask for the eflags */

void
syscall_init (void) {
	write_msr(MSR_STAR, ((uint64_t)SEL_UCSEG - 0x10) << 48  |
			((uint64_t)SEL_KCSEG) << 32);
	write_msr(MSR_LSTAR, (uint64_t) syscall_entry);

	/* The interrupt service rountine should not serve any interrupts
	 * until the syscall_entry swaps the userland stack to the kernel
	 * mode stack. Therefore, we masked the FLAG_FL. */
	write_msr(MSR_SYSCALL_MASK,
			FLAG_IF | FLAG_TF | FLAG_DF | FLAG_IOPL | FLAG_AC | FLAG_NT);
}

/* The main system call interface */
void
syscall_handler (struct intr_frame *f UNUSED) {
	// TODO: Your implementation goes here.
	// 유저 스택에 저장되어 있는 시스템 콜 넘버를 이용해 시스템 콜 핸들러 구현
	// 스택 포인터가 유저 영역인지 확인
	check_address(f->rsp);
	printf ("system call!\n");

	switch (f->R.rax)
	{
	// 0 -> HALT() :
	case SYS_HALT:
		halt();
		break;
	case SYS_EXIT:
		exit();
		break;
	case SYS_FORK:
		fork();
		break;
	case SYS_EXEC:
		exec(f->R.rdi);
		break;
	case SYS_WAIT:
		wait();
		break;
	case SYS_CREATE:
		create();
		break;
	case SYS_REMOVE:
		remove();
		break;
	case SYS_OPEN:
		open();
		break;
	case SYS_FILESIZE:
		filesize();
		break;
	case SYS_READ:
		read();
		break;
	case SYS_WRITE:
		write();
		break;
	case SYS_SEEK:
		seek();
		break;
	case SYS_TELL:
		tell();
		break;
	case SYS_CLOSE:
		close();
		break;
	
	default:
		break;
	}
	thread_exit ();
}


// TBD chobae : pintos syscall func add
void check_address(void *addr)	{
	/*  포인터가 가리키는 주소가 유저영역의 주소인지 확인
		잘못된 접근일 경우 프로세스 종료
		유효한 주소 (0x8048000 ~ 0x0000000)인지 확인
	*/
	if (is_kernel_vaddr(addr)) {
		process_exit();
	}

}

// syscall func section
// halt() -> pintos 종료
void halt(void) {
	// 거의 사용하지 않는게 좋음.
	power_off();
}
// exit() -> 실행중인 스레드? 프로세스? 종료 후 status 리턴
void exit(int status) {
	// TODO : 부모 스레드가 기다릴때 fork?
	struct thread *cur = thread_current();
	printf("%s : exit", cur->tid);
	thread_exit();
	process_exit();
}
// fork() ->  💩💩왕중요💩💩
// pid_t fork (const char *thread_name) {
// 	thread_name = thread_current();

// }
// exec() -> 파일 실행
int exec (const char *cmd_line)	{
	// TODO: exec 이 호출되는 동안 파일 디스크립터들이 계속 열려있는 상태를 유지하도록 주의
	process_exec(cmd_line);
}
// wait() -> 자식 스레드가 일을 마칠때까지 기다림  💩💩왕중요💩💩
// int wait (pid_t pid) {
	
// }

// create() -> 파일 이름과 크기에 해당하는 파일 생성
bool create(const char *file, unsigned initial_size) {
	return filesys_create(file, initial_size);
}
// remove() -> 파일 이에 해당하는 파일 제거
bool remove(const char *file) {
	return filesys_remove(file);
}
// open() -> 파일 열기
int open (const char *file) {
	return filesys_open(file);
}
// filesize() -> fd가 가리키는 열려있는 파일의 사이즈를 리턴
int filesize (int fd) {
	return file_length(fd);
}
// read() -> fd가 가르키는 file에서 size 바이트만큼 buffer로 읽음.
int read (int fd, void *buffer, unsigned size) {
	return file_read(fd, buffer, size);
}
// write() -> buffer의 내용을 size 바이트만큼 fd에 write
int write (int fd, const void *buffer, unsigned size) {
	return file_write(fd, buffer, size);
}
// seek() -> 처음부터 센 것을 기준으로, 다음에 읽거나 쓸 바이트를 position으로 변경
void seek (int fd, unsigned position) {
	return file_seek(fd, position);
}
// tell() -> 일의 처음부터 센 것을 기준으로, 다음에 읽을 바이트 혹은 fd에 쓸 다음 바이트의 포지션을 리턴
unsigned tell (int fd) {
	return file_tell(fd);
}

// close() -> fd가 가르키는 파일 닫기
void close (int fd) {
	return file_close(fd);
}

