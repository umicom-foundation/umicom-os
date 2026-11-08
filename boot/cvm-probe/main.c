/*-----------------------------------------------------------------------------
 * Umicom OS
 * File: boot/cvm-probe/main.c
 *
 * PURPOSE:
 *   Reuse Umicom C through a bounded, optional user-space consumer.
 *
 * AUTHOR AND ORGANISATION:
 * Sammy Hegab
 * Umicom Foundation
 *
 * LICENCE:
 * MIT
 *---------------------------------------------------------------------------*/
#include "umicom_c/umicom_c.h"
#include "umicom_c/trace.h"
#ifndef UMC_COMPARISON_PROFILE
#error "The CVM probe requires comparison-capable Umicom C source and headers."
#endif
#include <stdio.h>
#include <string.h>

/* This Linux user-space probe uses the same runtime as the freestanding Kernel
 * program. It accepts no paths or commands and performs no privileged work. */
/* The explicit trace-check option selects a fixed built-in exercise. It does
 * not admit source, bytecode, filesystem paths or external command execution. */
/* Preserve the former entry point for engineering review. The no-argument
 * path below calls the same readiness logic; traced checks are opt-in so boot
 * consumers cannot mistake an ordinary readiness line for trace qualification. */
#if 0
int main(void)
{
    static const UmcInstruction program[] = {
        {UMC_PUSH, 7, 0U}, {UMC_PUSH, 6, 0U},
        {UMC_MULTIPLY, 0, 0U}, {UMC_RETURN, 0, 0U}
    };
    UmcResult result;
    UmcDiagnostic diagnostic;
    const UmcStatus status = umc_execute(program, 4U, 4U, &result, &diagnostic);
    if (status != UMC_OK || result.value != 42 || result.instructions_executed != 4U) {
        fputs("UMICOM_CVM_PROBE_FAILED\n", stderr);
        return 1;
    }
    puts("UMICOM_CVM_PROBE_READY value=42");
    return 0;
}
#endif

static int RunProbe(void)
{
    static const UmcInstruction program[] = {
        {UMC_PUSH, 7, 0U}, {UMC_PUSH, 6, 0U},
        {UMC_MULTIPLY, 0, 0U}, {UMC_RETURN, 0, 0U}
    };
    UmcResult result;
    UmcDiagnostic diagnostic;
    const UmcStatus status = umc_execute(program, 4U, 4U, &result, &diagnostic);
    if (status != UMC_OK || result.value != 42 || result.instructions_executed != 4U) {
        fputs("UMICOM_CVM_PROBE_FAILED\n", stderr);
        return 1;
    }
    puts("UMICOM_CVM_PROBE_READY value=42");
    return 0;
}

static int RunTraceChecks(void)
{
    for (size_t selector = 0U; selector < 4U; ++selector) {
        UmcInstruction program[] = {
            {UMC_PUSH, 7, 1U}, {UMC_PUSH, selector == 2U ? 0 : 6, 3U},
            {selector == 2U ? UMC_DIVIDE : UMC_MULTIPLY, 0, 2U}, {UMC_RETURN, 0, 0U}
        };
        UmcResult result = {19, 0U};
        UmcDiagnostic diagnostic;
        UmcTraceStep steps[4];
        size_t used = 99U;
        steps[0].instruction_index = 113U;
        steps[0].top = 17;
        const UmcStatus status = umc_execute_trace(program, 4U, selector == 1U ? 3U : 4U,
            &result, steps, selector == 3U ? 3U : 4U, &used, &diagnostic);
        int accepted;
        if (selector == 0U) {
            accepted = status == UMC_OK && used == 4U && result.value == 42 &&
                result.instructions_executed == 4U && steps[2].opcode == UMC_MULTIPLY &&
                steps[2].stack_depth == 1U && steps[2].top == 42 &&
                steps[3].opcode == UMC_RETURN && steps[3].status == UMC_OK;
        } else if (selector == 1U) {
            accepted = status == UMC_FUEL_EXHAUSTED && used == 4U &&
                steps[3].opcode == UMC_RETURN && steps[3].status == UMC_FUEL_EXHAUSTED &&
                steps[3].top == 42 && steps[3].stack_depth == 1U &&
                result.value == 19 && result.instructions_executed == 0U;
        } else if (selector == 2U) {
            accepted = status == UMC_ARITHMETIC_ERROR && used == 3U &&
                steps[2].opcode == UMC_DIVIDE && steps[2].status == UMC_ARITHMETIC_ERROR &&
                steps[2].top == 0 && steps[2].stack_depth == 2U &&
                diagnostic.source_offset == 2U && result.value == 19 && result.instructions_executed == 0U;
        } else {
            accepted = status == UMC_BUFFER_TOO_SMALL && used == 0U &&
                steps[0].instruction_index == 113U && steps[0].top == 17 &&
                result.value == 19 && result.instructions_executed == 0U;
        }
        if (!accepted) {
            fprintf(stderr, "UMICOM_CVM_TRACE_FAILED case=%zu status=%s\n",
                selector, umc_status_text(status));
            return 1;
        }
    }
    puts("UMICOM_CVM_TRACE_READY cases=4");
    return 0;
}

/* This opt-in probe exercises fixed comparison contracts through the canonical
 * runtime. It accepts no program bytes, paths, commands or privileged actions. */
static int RunComparisonChecks(void)
{
    static const UmcOpcode operations[6] = {
        UMC_LESS, UMC_LESS_EQUAL, UMC_GREATER, UMC_GREATER_EQUAL, UMC_EQUAL, UMC_NOT_EQUAL
    };
    static const int32_t answers[6] = {1, 1, 0, 0, 0, 1};
    for (size_t selector = 0U; selector < 8U; ++selector) {
        UmcInstruction program[4];
        program[0].opcode = UMC_PUSH; program[0].operand = INT32_MIN; program[0].source_offset = 0U;
        program[1].opcode = UMC_PUSH; program[1].operand = INT32_MAX; program[1].source_offset = 2U;
        program[2].opcode = selector < 6U ? operations[selector] : UMC_LESS;
        program[2].operand = 0; program[2].source_offset = 1U;
        program[3].opcode = UMC_RETURN; program[3].operand = 0; program[3].source_offset = 3U;
        uint8_t bytes[UMC_BYTECODE_HEADER_SIZE + 4U * UMC_BYTECODE_RECORD_SIZE];
        UmcInstruction decoded[4];
        UmcDiagnostic diagnostic;
        UmcResult result = {19, 0U};
        UmcTraceStep steps[4];
        size_t size = 0U, count = 99U, used = 99U;
        UmcStatus status = umc_encode(program, 4U, bytes, sizeof bytes, &size, &diagnostic);
        int accepted = status == UMC_OK && bytes[4] == 2U;
        if (accepted) {
            if (selector == 7U) bytes[4] = 1U;
            status = umc_decode(bytes, size, decoded, 4U, &count, &diagnostic);
            if (selector == 7U) accepted = status == UMC_BAD_BYTECODE && count == 0U;
            else {
                accepted = status == UMC_OK && count == 4U;
                if (accepted) {
                    status = umc_execute_trace(decoded, count, selector == 6U ? 2U : 4U,
                        &result, steps, 4U, &used, &diagnostic);
                    if (selector == 6U)
                        accepted = status == UMC_FUEL_EXHAUSTED && used == 3U &&
                            steps[2].opcode == UMC_LESS && steps[2].stack_depth == 2U &&
                            steps[2].top == INT32_MAX && steps[2].status == UMC_FUEL_EXHAUSTED &&
                            result.value == 19 && result.instructions_executed == 0U;
                    else
                        accepted = status == UMC_OK && used == 4U && result.value == answers[selector] &&
                            result.instructions_executed == 4U && steps[2].opcode == operations[selector] &&
                            steps[2].stack_depth == 1U && steps[2].top == answers[selector] &&
                            steps[3].opcode == UMC_RETURN && steps[3].status == UMC_OK;
                }
            }
        }
        if (!accepted) {
            fprintf(stderr, "UMICOM_CVM_COMPARISON_FAILED case=%zu status=%s\n", selector, umc_status_text(status));
            return 1;
        }
    }
    puts("UMICOM_CVM_COMPARISON_READY cases=8");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1) return RunProbe();
    if (argc == 2 && strcmp(argv[1], "--trace-check") == 0) return RunTraceChecks();
    if (argc == 2 && strcmp(argv[1], "--comparison-check") == 0) return RunComparisonChecks();
    /* Preserve the trace-only usage; both fixed probes are now explicit. */
#if 0
    fputs("Usage: umicom-cvm-probe [--trace-check]\n", stderr);
#endif
    fputs("Usage: umicom-cvm-probe [--trace-check|--comparison-check]\n", stderr);
    return 2;
}
