
extern "C" void kernel_main2(void);

extern "C" void tfp_printf(char *fmt, ...);

void cpp_kernel() {
    kernel_main2();
}
