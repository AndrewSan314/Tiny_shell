#include "core.h"
#include "process_manager.h"
#include "colors.h"
#include "history.h"
#include "alias.h"
#include "config.h"
#include "hacker.h"

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    /* Initialize all subsystems */
    colors_init();
    history_init();
    alias_init();
    
    if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
        print_warning("Could not set control handler");
    }
    
    init_process_manager();
    
    /* Load user configuration (.mshrc) */
    config_load();
    
    /* HOLLYWOOD HACKER BOOT SEQUENCE */
    hacker_boot_sequence();
    
    msh_loop();
    
    return EXIT_SUCCESS;
}
