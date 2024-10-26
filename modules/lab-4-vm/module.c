#include <linux/module.h>
#include <linux/mmzone.h>
#include <linux/mm.h>
#include <linux/node.h>
#include <linux/topology.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Carlos Ruiz");

#define PN(x) ((void *)(unsigned long long)(x) >> PAGE_SHIFT)

static void *buffer;

static void print_zones(void) {
    int node_id = numa_node_id();
    int zone_id;
    struct pglist_data *pgdat = NODE_DATA(node_id);
    pr_info("Memory zones for NUMA Node %d:\n", node_id);
    for (zone_id = 0; zone_id < MAX_NR_ZONES; zone_id++) {
        struct zone *zone = &pgdat->node_zones[zone_id];
        unsigned long start_pfn = zone->zone_start_pfn;
        unsigned long end_pfn = zone_end_pfn(zone);
        pr_info("Zone %d - Start PFN: 0x%lx, End PFN: 0x%lx\n", zone_id, start_pfn, end_pfn);
    }
}

static void kmalloc_alloc(int n) {
    size_t buffer_size = n * PAGE_SIZE;
    buffer = kmalloc(buffer_size, GFP_KERNEL);
    if (buffer) {
        pr_info("kmalloc - VPN %px -> PPN: %px\n", PN(buffer), PN(virt_to_phys(buffer)));
        kfree(buffer);
    } else {
        pr_err("kmalloc failed\n");
    }
}

static void vmalloc_alloc(int n) {
    size_t buffer_size = n * PAGE_SIZE;
    int i;
    buffer = vmalloc(buffer_size);
    if (buffer) {
        for (i = 0; i < n; i++) {
            struct page *page = vmalloc_to_page(buffer + i * PAGE_SIZE);
            unsigned long ppn = page_to_pfn(page);  // Typo correction: logn -> long
            pr_info("vmalloc - VPN: %px -> PPN %lx\n", PN(buffer + i * PAGE_SIZE), ppn);
        }
        vfree(buffer);
    } else {
        pr_err("vmalloc failed\n");
    }
}

static int __init memalloc_init(void) {
    print_zones();
    pr_info("Kernel logical base VPN: %px\n", PN(PAGE_OFFSET));
    pr_info("Kernel virtual range (VPN - VPN): %px - %px\n", PN(VMALLOC_START), PN(VMALLOC_END));
    kmalloc_alloc(4);
    vmalloc_alloc(4);
    return 0;
}

static void __exit memalloc_cleanup(void) {
    pr_info("Module cleanup\n");
}

module_init(memalloc_init);
module_exit(memalloc_cleanup);

