#ifndef _MONA_GLOBAL_
#define _MONA_GLOBAL_

#ifdef GLOBAL_VALUE_DEFINED
#define GLOBAL /* */
#define GLOBAL_VAL(v) = (v)
#else
#define GLOBAL extern "C"
#define GLOBAL_VAL(v) /* */
#endif

#include<info.h>
#include<kernel.h>
#include<VirtualConsole.h>
#include<FDCDriver.h>
#include<Semaphore.h>
#include<GDTUtil.h>
#include<IDTUtil.h>
#include<PagingUtil.h>
#include<Process.h>
#include<string.h>
#include<types.h>
#include<Scheduler.h>
#include<ProcessManager.h>
#include <sysresource.h> /* (expr) */
#include <MessageServer.h>

GLOBAL VirtualConsole* g_console;                    /*< pointer to console    */
GLOBAL StackView g_stack_view;                       /*< struct for stack view */

GLOBAL FDCDriver* g_fdcdriver;

GLOBAL dword g_demo_step GLOBAL_VAL(0);

/* semaphore */
GLOBAL semaphore g_semaphore_console GLOBAL_VAL(1);

/* expr:sysresource */
GLOBAL BitMap *g_irqMap;
GLOBAL IRQHandler g_irqHandlers[16];

GLOBAL int  g_info_level GLOBAL_VAL(DEV_ERROR);

/* GLOBAL DESCRIPTOR TABLE */
GLOBAL SegDesc* g_gdt;

/* INTERRUPT DESCRIPTOR TABLE */
GLOBAL GateDesc* g_idt;

GLOBAL dword g_total_system_memory;

/* Paging */
GLOBAL PTE* g_page_dir;
GLOBAL PTE* g_page_tbl;

/* Process */
GLOBAL TSS g_tss;
GLOBAL Process** g_process;
GLOBAL ProcessInfo* g_current_process;
GLOBAL class ProcessManager* g_process_manager;

/* MessageServer */
GLOBAL MessageServer* g_message_server;

#endif
