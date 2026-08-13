#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x70dd6e66, "input_free_device" },
	{ 0x70dd6e66, "input_unregister_device" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x57de5c31, "input_close_device" },
	{ 0x57de5c31, "input_unregister_handle" },
	{ 0xcb8b6ec6, "kfree" },
	{ 0xe028781e, "input_unregister_handler" },
	{ 0x549de754, "input_event" },
	{ 0xbd03ed67, "random_kmalloc_seed" },
	{ 0x58bfb3ec, "kmalloc_caches" },
	{ 0xa7a90c10, "__kmalloc_cache_noprof" },
	{ 0x7805ff58, "input_register_handle" },
	{ 0x7805ff58, "input_open_device" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x363d6b25, "input_register_handler" },
	{ 0x6f7a2db2, "input_allocate_device" },
	{ 0x39fc1a5a, "input_register_device" },
	{ 0xd95bcd7f, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x70dd6e66,
	0x70dd6e66,
	0xd272d446,
	0x57de5c31,
	0x57de5c31,
	0xcb8b6ec6,
	0xe028781e,
	0x549de754,
	0xbd03ed67,
	0x58bfb3ec,
	0xa7a90c10,
	0x7805ff58,
	0x7805ff58,
	0xd272d446,
	0xe8213e80,
	0x363d6b25,
	0x6f7a2db2,
	0x39fc1a5a,
	0xd95bcd7f,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"input_free_device\0"
	"input_unregister_device\0"
	"__x86_return_thunk\0"
	"input_close_device\0"
	"input_unregister_handle\0"
	"kfree\0"
	"input_unregister_handler\0"
	"input_event\0"
	"random_kmalloc_seed\0"
	"kmalloc_caches\0"
	"__kmalloc_cache_noprof\0"
	"input_register_handle\0"
	"input_open_device\0"
	"__fentry__\0"
	"_printk\0"
	"input_register_handler\0"
	"input_allocate_device\0"
	"input_register_device\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("input:b0003v1A2Cp4C5Ee*-e*k*r*a*m*l*s*f*w*");

MODULE_INFO(srcversion, "0ED241C191037795C17E6CD");
