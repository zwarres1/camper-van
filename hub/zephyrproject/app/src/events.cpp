#include <events.hpp>

/* Actually allocate the memory for the semaphore */
K_SEM_DEFINE(ui_wake_sem, 0, 1);