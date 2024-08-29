
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

#include <hal.h>
#include <test_launcher.h>
#include <tests.h>
#include <stdio.h>
#include <ctype.h>
#include <cargs.h>

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

/* clang-format off */
static test_launcher_item_info tests_info[] = {

    /* Prolog               Test function         Epilogue       Description           Prolog args  Test arg  Epilogue args  Repetitions */
    /* --------------------------------------------------------------------------------------------------------------------------------- */
    { NULL,                 hal_useless_function, NULL,          test_useless_desc,    0,           0,        0,             1           },
    { NULL,                 test_exec_memcpy,     NULL,          test_memcpy_desc,     0,           0,        0,             1           },
    { NULL,                 test_exec_msgq,       NULL,          test_msgq_desc,       0,           0,        0,             1           },
    { test_mctplib_prolog,  test_exec_mctplib,    NULL,          test_mctplib_desc,    0,           0,        0,             1           },
    { test_frag_prolog,     test_exec_frag,       NULL,          test_frag_desc,       0,           0,        0,             1           },

};
/* clang-format on */

/**
 * @brief Initializes the test launcher and registers all tests.
 * 
 * This function initializes the test launcher module and registers all tests 
 * defined in the `tests_info` array. It returns 1 on success and 0 on failure.
 * 
 * @return int Returns 1 on successful initialization and registration, 0 on failure.
 */

static int init_register_tests(void)
{
    /* Initialize the tests launcher module */
    if ( test_launcher_init() != 0 )
        return 0;

    /* Register each test from the tests_info array */
    for ( size_t i = 0; i < sizeof(tests_info) / sizeof(tests_info[0]); i++ )
    {
        if ( test_launcher_register_test(&tests_info[i]) != 0 )
        {
            /* Handle registration failure */
            return 0; /* Return 0 on failure */
        }
    }

    return 1; /* Return 1 on successful initialization and registration */
}

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
    int                test_index       = -1;    /* Local argumnet */
    bool               cgi_mode         = false; /* Local argumnet */
    bool               exit_fetch       = false; /* Exit the arguments fearch loop */
    char *             long_description;

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

    /* Some tests require dedicated initialization routines, so: */

    /* 
     * Initializes the transport layer, this call will assert on any error and
     * consequently cause the emulator to exit back to the shell. 
     */
    test_mctplib_init(MCTP_USB__DEST_EID);

    /* Initilizes the 'frag' logic test */
    test_frag_init();

    /* Now initilize the tets launcher module*/
    init_register_tests();

    /* Retrieve argc and argv passed to main */
    hal_get_argcv(&argc, &argv);

    if ( argc > 1 )
    {

        /* 
         * Use libcargs to handle arguments.
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
                    test_index = 0xff;
                    value      = cag_option_get_value(&context);
                    if ( value != NULL )
                    {
                        if ( isdigit(*value) )
                            test_index = atol(value);
                    }
                    break;

                case 'c': /* Assume running as CGI - allow for some additional html related prinouts. */
                    cgi_mode = true;
                    printf("<span>\n");
                    fflush(stdout);
                    break;

                case 'v': /* Version */
                    printf("%s version %s\r\n", HAL_APP_NAME, HAL_APP_VERSION);
                    exit_fetch = true;
                    break;

                case 'h': /* Help */
                    printf("%s\n", HAL_APP_NAME);
                    printf("\nUsage: %s [OPTION]...\r\n", argv[0]);
                    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
                    exit_fetch = true;
                    break;
            }
        }

        if ( exit_fetch == false )
        {
            /* Execute the action specified by the input flgas */
            if ( test_index >= 0 )
            {
                measured_cycles = test_launcher_execute(test_index);
                if ( measured_cycles > 0 )
                {
                    long_description = test_launcher_long_help(test_index);
                    printf("Cycles count : %lld.\n\n", measured_cycles);

                    if ( cgi_mode == true )
                    {
                        /* Running as a CGI: This will turn the text color to white to 
                         * better differentiate our printouts from the xt-run summary.
                         */

                        printf("<span style=\"color: white; font-size: 12px;\">\n");
                    }
                    printf("Description:\n%s\n", long_description);
                }
            }
        }
    }

    if ( cgi_mode == true )
    {
        printf("\nRequest completed.\n");
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
