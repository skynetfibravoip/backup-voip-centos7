/*
 * include/linux/cpu.h - generic cpu definition
 *
 * This is mainly for topological representation. We define the 
 * basic 'struct cpu' here, which can be embedded in per-arch 
 * definitions of processors.
 *
 * Basic handling of the devices is done in drivers/base/cpu.c
 * and system devices are handled in drivers/base/sys.c. 
 *
 * CPUs are exported via sysfs in the class/cpu/devices/
 * directory. 
 */
#ifndef _LINUX_CPU_H_
#define _LINUX_CPU_H_

#include <linux/node.h>
#include <linux/compiler.h>
#include <linux/cpumask.h>

struct device;

struct cpu {
	int node_id;		/* The node which contains the CPU */
	int hotpluggable;	/* creates sysfs control file if hotpluggable */
	struct device dev;
};

extern void boot_cpu_state_init(void);

extern int register_cpu(struct cpu *cpu, int num);
extern struct device *get_cpu_device(unsigned cpu);
extern bool cpu_is_hotpluggable(unsigned cpu);

extern int cpu_add_dev_attr(struct device_attribute *attr);
extern void cpu_remove_dev_attr(struct device_attribute *attr);

extern int cpu_add_dev_attr_group(struct attribute_group *attrs);
extern void cpu_remove_dev_attr_group(struct attribute_group *attrs);

extern ssize_t cpu_show_meltdown(struct device *dev,
				 struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_spectre_v1(struct device *dev,
				   struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_spectre_v2(struct device *dev,
				   struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_spec_store_bypass(struct device *dev,
					  struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_l1tf(struct device *dev,
			     struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_mds(struct device *dev,
			    struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_tsx_async_abort(struct device *dev,
					struct device_attribute *attr,
					char *buf);
extern ssize_t cpu_show_itlb_multihit(struct device *dev,
				      struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_srbds(struct device *dev, struct device_attribute *attr, char *buf);
extern ssize_t cpu_show_mmio_stale_data(struct device *dev,
					struct device_attribute *attr,
					char *buf);
extern ssize_t cpu_show_retbleed(struct device *dev,
				 struct device_attribute *attr, char *buf);

#ifdef CONFIG_HOTPLUG_CPU
extern void unregister_cpu(struct cpu *cpu);
extern ssize_t arch_cpu_probe(const char *, size_t);
extern ssize_t arch_cpu_release(const char *, size_t);
#endif
struct notifier_block;

#ifdef CONFIG_ARCH_HAS_CPU_AUTOPROBE
extern int arch_cpu_uevent(struct device *dev, struct kobj_uevent_env *env);
extern ssize_t arch_print_cpu_modalias(struct device *dev,
				       struct device_attribute *attr,
				       char *bufptr);
#endif

/*
 * CPU notifier priorities.
 */
enum {
	/*
	 * SCHED_ACTIVE marks a cpu which is coming up active during
	 * CPU_ONLINE and CPU_DOWN_FAILED and must be the first
	 * notifier.  CPUSET_ACTIVE adjusts cpuset according to
	 * cpu_active mask right after SCHED_ACTIVE.  During
	 * CPU_DOWN_PREPARE, SCHED_INACTIVE and CPUSET_INACTIVE are
	 * ordered in the similar way.
	 *
	 * This ordering guarantees consistent cpu_active mask and
	 * migration behavior to all cpu notifiers.
	 */
	CPU_PRI_SCHED_ACTIVE	= INT_MAX,
	CPU_PRI_CPUSET_ACTIVE	= INT_MAX - 1,
	CPU_PRI_SCHED_INACTIVE	= INT_MIN + 1,
	CPU_PRI_CPUSET_INACTIVE	= INT_MIN,

	/* migration should happen before other stuff but after perf */
	CPU_PRI_PERF		= 20,
	CPU_PRI_MIGRATION	= 10,
	/* bring up workqueues before normal notifiers and down after */
	CPU_PRI_WORKQUEUE_UP	= 5,
	CPU_PRI_WORKQUEUE_DOWN	= -5,
};

#define CPU_ONLINE		0x0002 /* CPU (unsigned)v is up */
#define CPU_UP_PREPARE		0x0003 /* CPU (unsigned)v coming up */
#define CPU_UP_CANCELED		0x0004 /* CPU (unsigned)v NOT coming up */
#define CPU_DOWN_PREPARE	0x0005 /* CPU (unsigned)v going down */
#define CPU_DOWN_FAILED		0x0006 /* CPU (unsigned)v NOT going down */
#define CPU_DEAD		0x0007 /* CPU (unsigned)v dead */
#define CPU_DYING		0x0008 /* CPU (unsigned)v not running any task,
					* not handling interrupts, soon dead.
					* Called on the dying cpu, interrupts
					* are already disabled. Must not
					* sleep, must not fail */
#define CPU_POST_DEAD		0x0009 /* CPU (unsigned)v dead, cpu_hotplug
					* lock is dropped */
#define CPU_STARTING		0x000A /* CPU (unsigned)v soon running.
					* Called on the new cpu, just before
					* enabling interrupts. Must not sleep,
					* must not fail */
#define CPU_BROKEN		0x000C /* CPU (unsigned)v did not die properly,
					* perhaps due to preemption. */

/* Used for CPU hotplug events occurring while tasks are frozen due to a suspend
 * operation in progress
 */
#define CPU_TASKS_FROZEN	0x0010

#define CPU_ONLINE_FROZEN	(CPU_ONLINE | CPU_TASKS_FROZEN)
#define CPU_UP_PREPARE_FROZEN	(CPU_UP_PREPARE | CPU_TASKS_FROZEN)
#define CPU_UP_CANCELED_FROZEN	(CPU_UP_CANCELED | CPU_TASKS_FROZEN)
#define CPU_DOWN_PREPARE_FROZEN	(CPU_DOWN_PREPARE | CPU_TASKS_FROZEN)
#define CPU_DOWN_FAILED_FROZEN	(CPU_DOWN_FAILED | CPU_TASKS_FROZEN)
#define CPU_DEAD_FROZEN		(CPU_DEAD | CPU_TASKS_FROZEN)
#define CPU_DYING_FROZEN	(CPU_DYING | CPU_TASKS_FROZEN)
#define CPU_STARTING_FROZEN	(CPU_STARTING | CPU_TASKS_FROZEN)


#ifdef CONFIG_SMP
/* Need to know about CPUs going up/down? */
#if defined(CONFIG_HOTPLUG_CPU) || !defined(MODULE)
#define cpu_notifier(fn, pri) {					\
	static struct notifier_block fn##_nb =			\
		{ .notifier_call = fn, .priority = pri };	\
	register_cpu_notifier(&fn##_nb);			\
}

#define __cpu_notifier(fn, pri) {				\
	static struct notifier_block fn##_nb =			\
		{ .notifier_call = fn, .priority = pri };	\
	__register_cpu_notifier(&fn##_nb);			\
}
#else /* #if defined(CONFIG_HOTPLUG_CPU) || !defined(MODULE) */
#define cpu_notifier(fn, pri)	do { (void)(fn); } while (0)
#define __cpu_notifier(fn, pri)	do { (void)(fn); } while (0)
#endif /* #else #if defined(CONFIG_HOTPLUG_CPU) || !defined(MODULE) */

#ifdef CONFIG_HOTPLUG_CPU
extern int register_cpu_notifier(struct notifier_block *nb);
extern int __register_cpu_notifier(struct notifier_block *nb);
extern void unregister_cpu_notifier(struct notifier_block *nb);
extern void __unregister_cpu_notifier(struct notifier_block *nb);
#else

#ifndef MODULE
extern int register_cpu_notifier(struct notifier_block *nb);
extern int __register_cpu_notifier(struct notifier_block *nb);
#else
static inline int register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int __register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}
#endif

static inline void unregister_cpu_notifier(struct notifier_block *nb)
{
}

static inline void __unregister_cpu_notifier(struct notifier_block *nb)
{
}
#endif

int cpu_up(unsigned int cpu);
void notify_cpu_starting(unsigned int cpu);
extern void cpu_maps_update_begin(void);
extern void cpu_maps_update_done(void);

#define cpu_notifier_register_begin	cpu_maps_update_begin
#define cpu_notifier_register_done	cpu_maps_update_done

#else	/* CONFIG_SMP */

#define cpu_notifier(fn, pri)	do { (void)(fn); } while (0)
#define __cpu_notifier(fn, pri)	do { (void)(fn); } while (0)

static inline int register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int __register_cpu_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline void unregister_cpu_notifier(struct notifier_block *nb)
{
}

static inline void __unregister_cpu_notifier(struct notifier_block *nb)
{
}

static inline void cpu_maps_update_begin(void)
{
}

static inline void cpu_maps_update_done(void)
{
}

static inline void cpu_notifier_register_begin(void)
{
}

static inline void cpu_notifier_register_done(void)
{
}

#endif /* CONFIG_SMP */
extern struct bus_type cpu_subsys;

#ifdef CONFIG_HOTPLUG_CPU
/* Stop CPUs going up and down. */

extern void get_online_cpus(void);
extern bool try_get_online_cpus(void);
extern void put_online_cpus(void);
extern void cpu_hotplug_disable(void);
extern void cpu_hotplug_enable(void);
#define hotcpu_notifier(fn, pri)	cpu_notifier(fn, pri)
#define __hotcpu_notifier(fn, pri)	__cpu_notifier(fn, pri)
#define register_hotcpu_notifier(nb)	register_cpu_notifier(nb)
#define __register_hotcpu_notifier(nb)	__register_cpu_notifier(nb)
#define unregister_hotcpu_notifier(nb)	unregister_cpu_notifier(nb)
#define __unregister_hotcpu_notifier(nb)	__unregister_cpu_notifier(nb)
void clear_tasks_mm_cpumask(int cpu);
int cpu_down(unsigned int cpu);

#else		/* CONFIG_HOTPLUG_CPU */

#define get_online_cpus()	do { } while (0)
#define try_get_online_cpus()	true
#define put_online_cpus()	do { } while (0)
#define cpu_hotplug_disable()	do { } while (0)
#define cpu_hotplug_enable()	do { } while (0)
#define hotcpu_notifier(fn, pri)	do { (void)(fn); } while (0)
#define __hotcpu_notifier(fn, pri)	do { (void)(fn); } while (0)
/* These aren't inline functions due to a GCC bug. */
#define register_hotcpu_notifier(nb)	({ (void)(nb); 0; })
#define __register_hotcpu_notifier(nb)	({ (void)(nb); 0; })
#define unregister_hotcpu_notifier(nb)	({ (void)(nb); })
#define __unregister_hotcpu_notifier(nb)	({ (void)(nb); })
static inline int cpu_down(unsigned int cpu) { return -ENOSYS; }
#endif		/* CONFIG_HOTPLUG_CPU */

#ifdef CONFIG_PM_SLEEP_SMP
extern int disable_nonboot_cpus(void);
extern void enable_nonboot_cpus(void);
#else /* !CONFIG_PM_SLEEP_SMP */
static inline int disable_nonboot_cpus(void) { return 0; }
static inline void enable_nonboot_cpus(void) {}
#endif /* !CONFIG_PM_SLEEP_SMP */

enum cpuhp_state {
	CPUHP_OFFLINE,
	CPUHP_ONLINE,
};

void cpu_startup_entry(enum cpuhp_state state);
void cpu_idle(void);

void cpu_idle_poll_ctrl(bool enable);

/* Attach to any functions which should be considered cpuidle. */
#define __cpuidle	__attribute__((__section__(".cpuidle.text")))

bool cpu_in_idle(unsigned long pc);

void arch_cpu_idle(void);
void arch_cpu_idle_prepare(void);
void arch_cpu_idle_enter(void);
void arch_cpu_idle_exit(void);
void arch_cpu_idle_dead(void);

DECLARE_PER_CPU(bool, cpu_dead_idle);

int cpu_report_state(int cpu);
int cpu_check_up_prepare(int cpu);
void cpu_set_state_online(int cpu);
#ifdef CONFIG_HOTPLUG_CPU
bool cpu_wait_death(unsigned int cpu, int seconds);
bool cpu_report_death(void);
#endif /* #ifdef CONFIG_HOTPLUG_CPU */

enum cpuhp_smt_control {
	CPU_SMT_ENABLED,
	CPU_SMT_DISABLED,
	CPU_SMT_FORCE_DISABLED,
	CPU_SMT_NOT_SUPPORTED,
	CPU_SMT_NOT_IMPLEMENTED,
};

#if defined(CONFIG_SMP) && defined(CONFIG_HOTPLUG_SMT)
extern enum cpuhp_smt_control cpu_smt_control;
extern void cpu_smt_disable(bool force);
extern void cpu_smt_check_topology(void);
#else
# define cpu_smt_control		(CPU_SMT_NOT_IMPLEMENTED)
static inline void cpu_smt_disable(bool force) { }
static inline void cpu_smt_check_topology(void) { }
#endif

#ifdef CONFIG_HOTPLUG_SMT
bool cpu_smt_allowed(unsigned int cpu);
#else
static inline bool cpu_smt_allowed(unsigned int cpu) { return true; }
#endif

extern bool cpu_mitigations_off(void);
extern bool cpu_mitigations_auto_nosmt(void);

#endif /* _LINUX_CPU_H_ */
