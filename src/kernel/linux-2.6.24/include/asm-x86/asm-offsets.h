#ifndef __ASM_OFFSETS_H__
#define __ASM_OFFSETS_H__
/*
 * DO NOT MODIFY.
 *
 * This file was generated by Kbuild
 *
 */

#define SIGCONTEXT_eax 44 /* offsetof(struct sigcontext, eax)	# */
#define SIGCONTEXT_ebx 32 /* offsetof(struct sigcontext, ebx)	# */
#define SIGCONTEXT_ecx 40 /* offsetof(struct sigcontext, ecx)	# */
#define SIGCONTEXT_edx 36 /* offsetof(struct sigcontext, edx)	# */
#define SIGCONTEXT_esi 20 /* offsetof(struct sigcontext, esi)	# */
#define SIGCONTEXT_edi 16 /* offsetof(struct sigcontext, edi)	# */
#define SIGCONTEXT_ebp 24 /* offsetof(struct sigcontext, ebp)	# */
#define SIGCONTEXT_esp 28 /* offsetof(struct sigcontext, esp)	# */
#define SIGCONTEXT_eip 56 /* offsetof(struct sigcontext, eip)	# */

#define CPUINFO_x86 0 /* offsetof(struct cpuinfo_x86, x86)	# */
#define CPUINFO_x86_vendor 1 /* offsetof(struct cpuinfo_x86, x86_vendor)	# */
#define CPUINFO_x86_model 2 /* offsetof(struct cpuinfo_x86, x86_model)	# */
#define CPUINFO_x86_mask 3 /* offsetof(struct cpuinfo_x86, x86_mask)	# */
#define CPUINFO_hard_math 6 /* offsetof(struct cpuinfo_x86, hard_math)	# */
#define CPUINFO_cpuid_level 8 /* offsetof(struct cpuinfo_x86, cpuid_level)	# */
#define CPUINFO_x86_capability 12 /* offsetof(struct cpuinfo_x86, x86_capability)	# */
#define CPUINFO_x86_vendor_id 44 /* offsetof(struct cpuinfo_x86, x86_vendor_id)	# */

#define TI_task 0 /* offsetof(struct thread_info, task)	# */
#define TI_exec_domain 4 /* offsetof(struct thread_info, exec_domain)	# */
#define TI_flags 8 /* offsetof(struct thread_info, flags)	# */
#define TI_status 12 /* offsetof(struct thread_info, status)	# */
#define TI_preempt_count 20 /* offsetof(struct thread_info, preempt_count)	# */
#define TI_addr_limit 24 /* offsetof(struct thread_info, addr_limit)	# */
#define TI_restart_block 32 /* offsetof(struct thread_info, restart_block)	# */
#define TI_sysenter_return 28 /* offsetof(struct thread_info, sysenter_return)	# */
#define TI_cpu 16 /* offsetof(struct thread_info, cpu)	# */

#define GDS_size 0 /* offsetof(struct Xgt_desc_struct, size)	# */
#define GDS_address 2 /* offsetof(struct Xgt_desc_struct, address)	# */
#define GDS_pad 6 /* offsetof(struct Xgt_desc_struct, pad)	# */

#define PT_EBX 0 /* offsetof(struct pt_regs, ebx)	# */
#define PT_ECX 4 /* offsetof(struct pt_regs, ecx)	# */
#define PT_EDX 8 /* offsetof(struct pt_regs, edx)	# */
#define PT_ESI 12 /* offsetof(struct pt_regs, esi)	# */
#define PT_EDI 16 /* offsetof(struct pt_regs, edi)	# */
#define PT_EBP 20 /* offsetof(struct pt_regs, ebp)	# */
#define PT_EAX 24 /* offsetof(struct pt_regs, eax)	# */
#define PT_DS 28 /* offsetof(struct pt_regs, xds)	# */
#define PT_ES 32 /* offsetof(struct pt_regs, xes)	# */
#define PT_FS 36 /* offsetof(struct pt_regs, xfs)	# */
#define PT_ORIG_EAX 40 /* offsetof(struct pt_regs, orig_eax)	# */
#define PT_EIP 44 /* offsetof(struct pt_regs, eip)	# */
#define PT_CS 48 /* offsetof(struct pt_regs, xcs)	# */
#define PT_EFLAGS 52 /* offsetof(struct pt_regs, eflags)	# */
#define PT_OLDESP 56 /* offsetof(struct pt_regs, esp)	# */
#define PT_OLDSS 60 /* offsetof(struct pt_regs, xss)	# */

#define EXEC_DOMAIN_handler 4 /* offsetof(struct exec_domain, handler)	# */
#define RT_SIGFRAME_sigcontext 164 /* offsetof(struct rt_sigframe, uc.uc_mcontext)	# */

#define pbe_address 0 /* offsetof(struct pbe, address)	# */
#define pbe_orig_address 4 /* offsetof(struct pbe, orig_address)	# */
#define pbe_next 8 /* offsetof(struct pbe, next)	# */
#define TSS_sysenter_esp0 -8700 /* offsetof(struct tss_struct, x86_tss.esp0) - sizeof(struct tss_struct)	# */
#define PAGE_SIZE_asm 4096 /* PAGE_SIZE	# */
#define PAGE_SHIFT_asm 12 /* PAGE_SHIFT	# */
#define PTRS_PER_PTE 1024 /* PTRS_PER_PTE	# */
#define PTRS_PER_PMD 1 /* PTRS_PER_PMD	# */
#define PTRS_PER_PGD 1024 /* PTRS_PER_PGD	# */
#define VDSO_PRELINK_asm 0 /* VDSO_PRELINK	# */
#define crypto_tfm_ctx_offset 32 /* offsetof(struct crypto_tfm, __crt_ctx)	# */

#define PARAVIRT_enabled 8 /* offsetof(struct pv_info, paravirt_enabled)	# */
#define PARAVIRT_PATCH_pv_cpu_ops 40 /* offsetof(struct paravirt_patch_template, pv_cpu_ops)	# */
#define PARAVIRT_PATCH_pv_irq_ops 168 /* offsetof(struct paravirt_patch_template, pv_irq_ops)	# */
#define PV_IRQ_irq_disable 12 /* offsetof(struct pv_irq_ops, irq_disable)	# */
#define PV_IRQ_irq_enable 16 /* offsetof(struct pv_irq_ops, irq_enable)	# */
#define PV_CPU_iret 116 /* offsetof(struct pv_cpu_ops, iret)	# */
#define PV_CPU_irq_enable_sysexit 112 /* offsetof(struct pv_cpu_ops, irq_enable_sysexit)	# */
#define PV_CPU_read_cr0 12 /* offsetof(struct pv_cpu_ops, read_cr0)	# */

#define BP_scratch 484 /* offsetof(struct boot_params, scratch)	# */
#define BP_loadflags 529 /* offsetof(struct boot_params, hdr.loadflags)	# */
#define BP_hardware_subarch 572 /* offsetof(struct boot_params, hdr.hardware_subarch)	# */
#define BP_version 518 /* offsetof(struct boot_params, hdr.version)	# */

#endif