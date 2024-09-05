
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
#include <ncsi.h>
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
    {.identifier = 'p', .access_letters = "p",  .access_name = "packet",.value_name = "VALUE",  .description = "Set pcket size in bytes."},
    {.identifier = 'v', .access_letters = "v",  .access_name = "ver",   .value_name = NULL,     .description = "Print the version and exit."},
    {.identifier = 'c', .access_letters = "c",  .access_name = "cgi",   .value_name = NULL,     .description = "Enable web CGI mode."},
    {.identifier = 'h', .access_letters = "h?", .access_name = "help",  .value_name = NULL,     .description = "Print usage."},
};
/* clang-format on */

/* clang-format off */ 
static test_launcher_item_info tests_info[] = {

    /*   Init                       Prolog                          Test function               Epilogue            Description                 Args            Repetitions */
    /* ---------------------------------------------------------------------------------------------------------------------------------------------------------------- */
/* 0 */ { NULL,                     NULL,                           hal_useless_function,       NULL,               test_useless_desc,          0,     0,       0,  0,  1    },
/* 1 */ { NULL,                     NULL,                           test_exec_memcpy,           NULL,               test_memcpy_desc_xtensa,    0,     0,       0,  0,  1    },
/* 2 */ { NULL,                     NULL,                           test_exec_memcpy,           NULL,               test_memcpy_desc_hal,       0,     0,       1,  0,  1    },
/* 3 */ { NULL,                     test_msgq_prologue,             test_exec_msgq,             NULL,               test_msgq_desc,             0,     0,       0,  0,  1    },
/* 4 */ { test_defrag_init,         test_defrag_prologue,           test_exec_defrag,           test_defrag_epilog, test_defrag_desc,           0,     1500,    0,  0,  1    },
/* 5 */ { test_defrag_mctplib_init, test_defrag_mctplib_prologue,   test_exec_defrag_mctplib,   NULL,               test_defrag_mctplib_desc,   0,     0,       0,  0,  1    },
/* 6 */ { test_frag_init,           test_frag_prologue,             test_exec_frag,             test_frag_epilog,   test_frag_desc,             0,     1500,    0,  0,  1    }

};
/* clang-format on */

/* Shortcut for seting text color whne in CGI mode. */
static void cgi_set_color(bool mode, const char *color)
{
    if ( mode == true )
    {
        printf("</span><span style=\"color: %s; font-size: 14px;\">\n", color);
        fflush(stdout);
    }
}

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
 * @brief ToDo
 * @return none
 */

void exec_multi_size(size_t test_index, size_t min, size_t max)
{

    uint64_t measured_cycles;
    int      i;
    char *   test_desc = test_launcher_get_desc(test_index, 0);

    printf("// Test #%d: %s\n", test_index, test_desc);
    printf("// Packet size %d:%d\n", min, max);
    printf("let cyclesArray = [\n");

    for ( i = 1302; i < max; i++ )
    {
        tests_info[4].prologue_arg = i + (NCSI_INTEL_PRE_BYTE - 1);
        tests_info[6].prologue_arg = i + (NCSI_INTEL_PRE_BYTE - 1);

        test_launcher_update_test(4, &tests_info[4]);
        test_launcher_update_test(6, &tests_info[6]);

        measured_cycles = test_launcher_execute(test_index);
        printf("       [%-6u],\t// %d bytes\n", (size_t) measured_cycles, i);
        xos_thread_sleep(10);
    }

    printf("];\n");
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

    cag_option_context context      = {0};   /* libcargs context */
    int                argc         = 0;     /* Arguments count passed to main() */
    char **            argv         = NULL;  /* Arguments array passed to main() */
    char               identifier   = 0;     /* libcargs identifier */
    bool               run_and_exit = true;  /* Specify to terminate immediately */
    bool               got_command  = false; /* Have we got any command to execute? */
    const char *       value        = NULL;  /* Points to an extrcated argumnet */
    int                test_index   = -1;    /* Local argumnet */
    int                packet_size  = -1;    /* External forced packet size */
    bool               cgi_mode     = false; /* Local argumnet */
    bool               exit_fetch   = false; /* Exit the arguments fearch loop */

    HAL_UNUSED(arg);
    HAL_UNUSED(unused);

/* Allow for easer debugging */
#ifdef DEBUG
    xos_disable_interrupts();
#endif

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

                case 't': /* Execute our basic 'useless cycles' test */
                    test_index = 0xff;
                    value      = cag_option_get_value(&context);
                    if ( value != NULL )
                    {
                        if ( isdigit(*value) )
                            test_index = atol(value);
                        got_command = true;
                    }
                    break;

                case 'p': /* Sets NC-SI packet size for frag / defga tests */
                    value = cag_option_get_value(&context);
                    if ( value != NULL )
                    {
                        if ( isdigit(*value) )
                        {
                            packet_size = atol(value);

                            /* Patch the test table with the forced values, 
                             * compensating for the addition of the 32-bit extra bytes */
                            tests_info[4].prologue_arg = packet_size + (NCSI_INTEL_PRE_BYTE - 1);
                            tests_info[6].prologue_arg = packet_size + (NCSI_INTEL_PRE_BYTE - 1);
                        }
                    }
                    break;

                case 'c': /* Assume running as CGI - allow for some additional html related prinouts. */
                    cgi_mode = true;
                    printf("<span>\n");
                    fflush(stdout);
                    break;

                case 'v': /* Version */
                    printf("%s version %s\r\n", HAL_APP_NAME, HAL_APP_VERSION);
                    exit_fetch  = true;
                    got_command = true;
                    break;

                case 'h': /* Help */
                    printf("%s\n", HAL_APP_NAME);
                    printf("\nUsage: %s [OPTION]...\r\n", argv[0]);
                    cag_option_print(options, CAG_ARRAY_SIZE(options), stdout);
                    exit_fetch  = true;
                    got_command = true;
                    break;
            }
        }

        if ( exit_fetch == false )
        {
            /* Execute the action specified by the input flgas */
            if ( test_index >= 0 )
            {

                /* Initialize the tests launcher module */
                test_launcher_init(cgi_mode);

                /* Now initilize the tets launcher module*/
                init_register_tests();

#if ( TEST_CONTINUOUS_MODE > 0 )
                exec_multi_size(test_index, 24, 1500);
#else
                char *   test_desc       = NULL; /* Test descriptive test */
                uint64_t measured_cycles = 0;    /* Cycles related to any of our tests */

                measured_cycles = test_launcher_execute(test_index);

                if ( measured_cycles > 0 )
                {
                    test_desc = test_launcher_get_desc(test_index, 0);
                    if ( test_desc != NULL )
                    {
                        /*: Change text color whne in cgi mode */
                        cgi_set_color(cgi_mode, "yellow");
                        printf("Test %d: %s.\n", test_index, test_desc);
                    }

#ifdef DEBUG
                    cgi_set_color(cgi_mode, "red");
                    printf("Cycles count [DEBUG]: %lld.\n\n", measured_cycles);
#else
                    cgi_set_color(cgi_mode, "cyan");
                    printf("Cycles count: %lld.\n\n", measured_cycles);
#endif

                    cgi_set_color(cgi_mode, "white");
                    test_desc = test_launcher_get_desc(test_index, 1);
                    if ( test_desc != NULL )
                        printf("Description:\n%s\n", test_desc);
                }

#endif /* TEST_CONTIMUOS_MODE */
            }
        }
    }

    /* If no valid command was detected */
    if ( got_command == false )
    {
        cgi_set_color(cgi_mode, "red");
        printf("Error: did not get valid command to execute.\n");
    }

    cgi_set_color(cgi_mode, "white");

    if ( run_and_exit == true )
        exit(EXIT_SUCCESS);

#ifdef HAL_START_XOS_KERNAL

    printf("Starting XOS Kernel..\n");
    while ( 1 )
    {
        /* Loop indefinitely */
        hal_delay_ms(1000);
    }
#endif

    return EXIT_SUCCESS;
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
