
/** @file
 *
 * @defgroup app_error Common application error handler
 * @{
 * @ingroup app_common
 *
 * @brief Common application error handler.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f1xx.h"
#include "app_util.h"
#define APP_LOG_MODULE_NAME "[app_error]"
#include "app_log.h"
#include "app_error.h"

/**@brief       Function for printing all error info (using app_log).
 *
 * @details     app_log library must be initialized using APP_LOG_INIT macro before calling
 *              this function.
 *
 * @param[in] id    Fault identifier. See @ref APP_FAULT_IDS.
 * @param[in] pc    The program counter of the instruction that triggered the fault, or 0 if
 *                  unavailable.
 * @param[in] info  Optional additional information regarding the fault. Refer to each fault
 *                  identifier for details.
 */
static __INLINE void app_error_log(uint32_t id, uint32_t pc, uint32_t info)
{
    switch (id)
    {
        case APP_FAULT_ID_ASSERT:
            APP_LOG_ERROR("\r\n*** ASSERTION FAILED ***\r\n");
            if (((assert_info_t *)(info))->p_file_name)
            {
                APP_LOG_ERROR("Line Number: %u\r\n", (unsigned int) ((assert_info_t *)(info))->line_num);
                APP_LOG_ERROR("File Name:   %s\r\n",(uint8_t*)((assert_info_t *)(info))->p_file_name);
            }
                APP_LOG_ERROR("\r\n");
            break;

        case APP_FAULT_ID_ERROR:
            APP_LOG_ERROR("\r\n*** APPLICATION ERROR *** \r\n" APP_LOG_COLOR_CODE_WHITE);
            if (((error_info_t *)(info))->p_file_name)
            {
                APP_LOG_ERROR("Line Number: %u\r\n", (unsigned int) ((error_info_t *)(info))->line_num);
                APP_LOG_ERROR("File Name:   %s\r\n", (uint8_t*)((error_info_t *)(info))->p_file_name);
            }
                APP_LOG_ERROR("Error Code:  0x%X\r\n", (unsigned int) ((error_info_t *)(info))->err_code);
            break;
    }
}

__WEAK void app_assert_handler(uint16_t line_num, const uint8_t * file_name)
{
    assert_info_t assert_info =
    {
        .line_num    = line_num,
        .p_file_name = file_name,
    };
    app_error_fault_handler(APP_FAULT_ID_ASSERT, 0, (uint32_t)(&assert_info));

    UNUSED_VARIABLE(assert_info);
}


/**
 * Function is implemented as weak so that it can be overwritten by custom application error handler
 * when needed.
 */
__WEAK void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    APP_LOG_ERROR("Fatal!\r\n");
    // On assert, the system can only recover with a reset.
#if APP_LOG_ENABLED == 0
    UNUSED_VARIABLE(id);
    UNUSED_VARIABLE(pc);
    UNUSED_VARIABLE(info);
    NVIC_SystemReset();
#else
    app_error_log(id, pc, info);
    NVIC_SystemReset();
#endif // APP_DEBUG
}


/**@brief Function for error handling, which is called when an error has occurred.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of error.
 *
 * @param[in] error_code  Error code supplied to the handler.
 * @param[in] line_num    Line number where the handler is called.
 * @param[in] p_file_name Pointer to the file name.
 */
/*lint -save -e14 */
void app_error_handler(ret_code_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    error_info_t error_info =
    {
        .line_num    = line_num,
        .p_file_name = p_file_name,
        .err_code    = error_code,
    };
    app_error_fault_handler(APP_FAULT_ID_ERROR, 0, (uint32_t)(&error_info));

    UNUSED_VARIABLE(error_info);
}



