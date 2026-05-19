#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOG_ROOT="$ROOT_DIR/testbench/logs"
RUN_ID="$(date +%Y%m%d_%H%M%S)"
RUN_DIR="$LOG_ROOT/$RUN_ID"
SUMMARY_LOG="$RUN_DIR/recap.log"
QEMU_LOG="$ROOT_DIR/debug/test.txt"
DURATION="${DURATION:-1}"
TOTAL_TESTS=0
PASSED_TESTS=0

mkdir -p "$RUN_DIR" "$ROOT_DIR/debug"
: > "$SUMMARY_LOG"

apply_static_conf() {
	local profile="$1"
	local source_base=""

	case "$profile" in
		process)
			source_base="static_conf_process_test"
			;;
		partition)
			source_base="static_conf_partition_test"
			;;
		time)
			source_base="static_conf_time_timed_wait_test"
			;;
		sampling_port)
			source_base="static_conf_arinc_sampling_port_test"
			;;
		buffer)
			source_base="static_conf_arinc_buffer_test"
			;;
		event)
			source_base="static_conf_arinc_event_test"
			;;
		semaphore)
			source_base="static_conf_arinc_semaphore_test"
			;;
		queuing_port)
			source_base="static_conf_arinc_queuing_port_test"
			;;
		*)
			echo "Unknown test profile: $profile" >&2
			exit 1
			;;
	esac

	cp "$ROOT_DIR/arinc/static/${source_base}.c" "$ROOT_DIR/arinc/static_conf.c"
	cp "$ROOT_DIR/include/arinc/static/${source_base}.h" "$ROOT_DIR/include/arinc/static_conf.h"
}

run_test() {
	local target="$1"
	local label="$2"
	local profile="$3"
	local build_log="$RUN_DIR/${label}.build.log"
	local serial_log="$RUN_DIR/${label}.serial.log"
	local test_total=0
	local test_passed=0

	echo "==> $label"

	: > "$build_log"
	rm -f "$QEMU_LOG"

	{
		cd "$ROOT_DIR"
		apply_static_conf "$profile"
		make veryclean
		make ucx ARCH=riscv/riscv32-qemu
		make "$target"
		timeout "$DURATION" qemu-system-riscv32 -smp 4 -machine virt -bios none -kernel ./build/target/image.elf -display none -serial file:./debug/test.txt
	} >"$build_log" 2>&1 || true

	if [[ -f "$QEMU_LOG" ]]; then
		cp "$QEMU_LOG" "$serial_log"
		test_total="$(grep -c '^--- END SUITE ' "$QEMU_LOG" || true)"
		test_passed="$(grep -c '^--- END SUITE .* PASS ---$' "$QEMU_LOG" || true)"
		TOTAL_TESTS=$((TOTAL_TESTS + test_total))
		PASSED_TESTS=$((PASSED_TESTS + test_passed))
		{
			echo "--- TEST $label ---"
			grep -E '^--- (START|END) SUITE |^\[ARINC_TEST\] completed suite ' "$QEMU_LOG" | awk '
				{ print }
				/^\[ARINC_TEST\] completed suite / { print "" }
			' || true
			echo
		} >> "$SUMMARY_LOG"
	else
		{
			echo "--- TEST $label ---"
			echo "No serial log produced for $label"
			echo
		} >> "$SUMMARY_LOG"
	fi
}

# run_test arinc_test_api_partition api_partition partition
# run_test arinc_test_process process process
# run_test arinc_test_time_timed_wait time_timed_wait time
# run_test arinc_test_sampling_port sampling_port sampling_port
# run_test arinc_test_event event event
# run_test arinc_test_queuing_port queuing_port queuing_port
# run_test arinc_test_buffer buffer buffer
# run_test arinc_test_semaphore semaphore semaphore
run_test arinc_test_event event event

{
	echo "Tests reussis: $PASSED_TESTS/$TOTAL_TESTS"
	echo
} >> "$SUMMARY_LOG"

echo "Summary written to: $SUMMARY_LOG"
echo "Logs stored in: $RUN_DIR"
echo "Tests reussis: $PASSED_TESTS/$TOTAL_TESTS"

cat "$SUMMARY_LOG"