
/**
  ******************************************************************************
  * @file    main.c
  * @author  IMCv2 Team
  * @brief   Playground for facilitating accurate measurement of the operations
  *          required for supporting MCTP over USB.
  *    
  ******************************************************************************
  * 
  * @copyright
  * @par Copyright (c) 2024 Intel Corporation.
  * All rights reserved.
  * 
  * This code is proprietary to Intel Corporation and may not be used, modified,
  * or distributed without the express written permission of Intel Corporation.
  * 
  ******************************************************************************
  */

#include <stdio.h>
#include <hal.h>
#include <ctype.h>
#include <mctp_usb.h>
#include <cargs.h>
#include <cycles_eval.h>

/* Known console input arguments table. */
/* clang-format off */

static struct cag_option options[] =
{
    {.identifier = 't', .access_letters = "t",  .access_name = "test",  .value_name = "VALUE",  .description = "Execute a cycle test."},
    {.identifier = 'r', .access_letters = "r",  .access_name = "rept",  .value_name = "VALUE",  .description = "Set the number of test repetitions."},
    {.identifier = 'v', .access_letters = "v",  .access_name = "ver",   .value_name = NULL,     .description = "Print the version and exit."},
    {.identifier = 'c', .access_letters = "c",  .access_name = "cgi",   .value_name = NULL,     .description = "Enable web CGI mode."},
    {.identifier = 'h', .access_letters = "h?", .access_name = "help",  .value_name = NULL,     .description = "Print usage."},
};

/* clang-format on */

/**
 * @brief Initial startup thread that initializes the system and processes 
 *        command-line arguments.
 *
 * This function serves as the initial startup thread for the application. It 
 * initializes the necessary components, such as the transport layer, and then 
 * processes command-line arguments passed to the application using the libcargs 
 * library. Based on the provided arguments, it can execute specific tests, 
 * display version information, or print usage help. If specific arguments are 
 * provided, the function may terminate the simulation immediately after 
 * processing them.
 *
 * @param arg    Pointer to the XOS thread arguments (unused).
 * @param unused Unused parameter.
 * 
 * @return Always returns 0.
 */
static int init_thread(void *arg, int32_t unused)
{
    uint64_t           measured_cycles  = 0;     /* Cycles related to any of our tests */
    cag_option_context context          = {0};   /* libcargs context */
    int                argc             = 0;     /* Arguments count passed to main() */
    char **            argv             = NULL;  /* Arguments array passed to main() */
    char               identifier       = 0;     /* libcargs identifier */
    bool               run_and_exit     = true;  /* Specify to terminate immediately */
    const char *       value            = NULL;  /* Points to an extrcated argumnet */
    int                test_repetitions = 1;     /* Local argumnet */
    int                test_type        = -1;    /* Local argumnet */
    bool               cgi_mode         = false; /* Local argumnet */
    bool               exit_fetch       = false; /* Exit the arguments fearch loop */

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

    /* Initializes the transport layer, this call will assert on any error and
     * consequently cause the emulator to exit back to the shell. */
    mctp_usb_init(MCTP_USB__DEST_EID);

    /* Retrieve argc and argv passed to main */
    hal_get_argcv(&argc, &argv);

    if ( argc > 1 )
    {

        /* Use libcargs to handle arguments.
         * Here we're making use of the handy feature that the emulator could be invoked
         * with command-line arguments, allowing us to execute different paths based 
         * on external arguments.
         * Example: Retrieve the version using: xt-run build/release/firmware.elf -v
         */

        cag_option_prepare(&context, options, CAG_ARRAY_SIZE(options), argc, argv);
        while ( cag_option_fetch(&context) )
        {
            identifier = cag_option_get(&context);
            switch ( identifier )
            {

                case 'r': /* Sets test repetiotions */
                    value = cag_option_get_value(&context);
                    if ( value != NULL )
                        test_repetitions = atol(value);
                    break;

                case 't': /* Execute our basic 'useless cycles' test */
                    test_type = 0xff;
                    value     = cag_option_get_value(&context);
                    if ( value != NULL )
                    {
                        if ( isdigit(*value) )
                            test_type = atol(value);
                    }
                    break;

                case 'c': /* Assume running as CGI - allow for some additional html related prinouts. */
                    cgi_mode = true;
                    printf("<span>\n");
                    fflush(stdout);
                    break;

                case 'v': /* Version */
                    printf("%s version %s\r\n", MCTP_USB_APP_NAME, MCTP_USB_APP_VERSION);
                    exit_fetch = true;
                    break;

                case 'h': /* Help */
                    printf("%s\n", MCTP_USB_APP_NAME);
                    printf("\nUsage: %s [OPTION]...\r\n", argv[0]);
                    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
                    exit_fetch = true;
                    break;
            }
        }

        if ( exit_fetch == false )
        {
            /* Execute the action specified by the input flgas */
            if ( test_type >= 0 )
                measured_cycles = run_cycles_test(test_type, test_repetitions);
        }
    }

    /* Running as a CGI: This will turn the text color to white to 
     * better differentiate our printouts from the xt-run summary.
     */

    if ( cgi_mode == true )
    {
        printf("\nRequest completed.\n");
        printf("<span style=\"color: white; font-size: 12px;\">\n");
    }

    if ( run_and_exit == true )
        hal_terminate_simulation(EXIT_SUCCESS);

#ifdef HAL_START_XOS_KERNAL

    printf("Starting XOS Kernel..\n");
    while ( 1 )
    {
        /* Loop indefinitely */
        hal_delay_ms(1000);
    }
#endif

    return 0;
}

/**
 * @brief System initialization and startup.
 *
 * This is the entry point for initializing the emulated LX7 environment, 
 * starting the background tick timer interrupt, spawning the initial 
 * main thread, and launching  the XOS scheduler. Once this function is 
 * called, the system will be up and running, and control will be handed 
 * over to the XOS kernel.
 *
 * @return This function does not return under normal circumstances, 
 *         as `hal_sys_init` starts the XOS kernel, which takes over 
 *         system control. If it returns, an error has occurred, 
 *         and the return value is 1.
 */

int main(int argc, char **argv)
{
    /* Initialize system and start XOS kernel.*/

    hal_sys_init(init_thread, argc, argv);

    /* If we reach here, something went wrong */
    return 1;
}
