
/**
  ******************************************************************************
  * @file    test_launcher.c
  * @author  IMCv2 Team
  * @brief   Allow to register and execute various test and report Cycles 
  *          asociated with them.
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
#include <hal_llist.h>
#include <test_launcher.h>
#include <string.h>

/**
 * @brief The message queue item structure.
 */

typedef struct __launched_item_t
{
    struct __launched_item_t *next;      /* Pointer to the next item in the list */
    test_launcher_item_info * item_info; /* Pointer to the data for the launched item */

} launched_item;

/**
 * @brief Type for managing the test launcher context.
 */

typedef struct _test_launcher_session_t
{
    launched_item *items_head;  /* Pointer to the head of the launched items list */
    size_t         items_count; /* Number of items in the list */
    size_t         cgi_mode;

} test_launcher_session;

/* Global session pointer */
static test_launcher_session *p_launcher = NULL;

/**
 * @brief Registers a test item with the launcher.
 * @param item_info A pointer to the test item to be registered.
 * @return int Returns 0 on success, 1 on error.
 */

int test_launcher_register_test(test_launcher_item_info *item_info)
{
    launched_item *item = NULL;

    do
    {
        if ( p_launcher == NULL )
            break; /* Module not initialized */

        if ( ! item_info || ! item_info->test_func )
            break; /* Error: Invalid input or missing required function */

        if ( p_launcher->items_count >= TEST_LAUNCHER_MAX_ITEMS )
            break; /* Error: Maximum number of test items reached */

        /* Allocate memory for the outer structure */
        item = hal_alloc(sizeof(launched_item));
        if ( item == NULL )
            break; /* Memory allocation error */

        /* Allocate memory for the test item */
        item->item_info = hal_alloc(sizeof(test_launcher_item_info));
        if ( item->item_info == NULL )
            break; /* Memory allocation error */

        /* Copy the test item to the allocated memory */
        memcpy(item->item_info, item_info, sizeof(test_launcher_item_info));

        /* Attach to the head of the list */
        LL_APPEND(p_launcher->items_head, item);

        p_launcher->items_count++;
        return 0; /* Success */

    } while ( 0 );

    /* Note: We're using an allocator that does not allow freeing */
    return 1; /* Error */
}

/**
 * @brief Executes a registered test based on its index.
 * @param test_index The index of the test to execute.
 * @return int Cycles count associated with the test.
 */

uint64_t test_launcher_execute(size_t test_index)
{
    launched_item *item         = NULL;
    uint64_t       cycles_count = 0;
    int            i            = 0;
    uint64_t       ret          = 0;

    do
    {
        if ( p_launcher == NULL )
            break; /* Module not initialized */

        if ( test_index >= p_launcher->items_count )
        {
            test_launcher_help();
            break; /* Error: Invalid test index */
        }

        /* Finds list item at the specified index */
        LL_FOREACH(p_launcher->items_head, item)
        {
            if ( i == test_index )
                break;
            i++;
        }

        if ( item == NULL )
            break;

        /* Execute the init function if it exists */
        if ( item->item_info->init )
        {
            if ( item->item_info->init(item->item_info->init_arg) )
            {
                printf("Launcher error: init() function failed.\n");
                break;
            }
        }

        /* Execute the prolog function if it exists */
        if ( item->item_info->prolog )
        {
            if ( item->item_info->prolog(item->item_info->prolog_arg) )
            {
                printf("Launcher error: prolog() function failed.\n");
                break;
            }
        }

        /* Execute the test function the specified number of times */
        for ( uint8_t j = 0; j < item->item_info->repetitions; j++ )
        {
            cycles_count = hal_measure_cycles(item->item_info->test_func, item->item_info->test_arg);
        }

        /* Execute the epilogue function if it exists */
        if ( item->item_info->epilogue )
        {
            if ( item->item_info->epilogue(item->item_info->epilogue_arg) )
            {
                printf("Launcher error: epilogue() function failed.\n");
                break;
            }
        }

        ret = cycles_count; /* Success */

    } while ( 0 );

    return cycles_count;
}

/**
 * @brief Prints the description of all registered tests.
 * @return int Always returns 0.
 */

int test_launcher_help(void)
{
    char *test_description;

    if ( p_launcher == NULL )
        return 1; /* Module not initialized */

    launched_item *item = p_launcher->items_head;

    if ( p_launcher->cgi_mode == 1 )
        printf("</span>");

    for ( size_t i = 0; i < p_launcher->items_count; i++ )
    {
        if ( item->item_info->desc != NULL )
            test_description = item->item_info->desc(0);
        else
            test_description = "Test description not available";

        if ( p_launcher->cgi_mode == 1 )
        {
            printf("<span style=\"color: %s; font-size: 15px;\">%zu:  </span>", "yellow", i);
            printf("<span style=\"color: %s; font-size: 15px;\">%s</span>\n", "white", test_description);
        }
        else
            printf("%zu: %s\n", i, test_description);

        item = item->next;
    }

    return 0;
}

/**
 * @brief Prints the long description for a registered test at a given index.
 * 
 * This function returns the long description of a test registered in the launcher 
 * at the specified index. If the test or description cannot be found, it returns 
 * an error message.
 * 
 * @param test_index The index of the test whose long description is to be retrieved.
 * @param type 0 for short description, 1 for long.
 * @return A pointer to the description string, or an error message if the test 
 *         or description is not found.
 */

char *test_launcher_get_desc(size_t test_index, size_t type)
{
    launched_item *item = NULL;
    size_t         i    = 0;
    char *         test_description;

    /* Find the list item at the specified index */
    LL_FOREACH(p_launcher->items_head, item)
    {
        if ( i == test_index )
            break;
        i++;
    }

    if ( item == NULL || item->item_info->desc == NULL )
        return "Error: can't locate specified test";

    if ( type > 1 )
        type = 1;

    test_description = item->item_info->desc(type);

    return test_description;
}

/**
 * @brief Initializes the test launcher module.
 * @return int Returns 0 on success, 1 on error.
 */

int test_launcher_init(size_t cgi_mode)
{
    if ( p_launcher != NULL )
        return 1; /* Multiple initializations are not allowed */

    p_launcher = hal_alloc(sizeof(test_launcher_session));
    if ( p_launcher == NULL )
        return 1; /* Memory allocation error */

    hal_zero_buf(p_launcher, sizeof(test_launcher_session));
    p_launcher->cgi_mode = cgi_mode;

    return 0;
}
