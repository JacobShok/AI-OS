#!/usr/bin/env python3
"""
Convert argtable3 commands to thread-safe stack-allocated structures.

This script transforms commands from using static global argtable variables
to using stack-allocated structures, making them safe for parallel execution
in threaded pipelines.
"""

import re
import sys
from pathlib import Path


def convert_command_file(filepath):
    """Convert a single command file to use thread-safe argtables."""

    with open(filepath, 'r') as f:
        content = f.read()

    # Extract command name from filename (e.g., cmd_echo.c -> echo)
    cmd_name = filepath.stem.replace('cmd_', '')

    # Step 1: Find all static struct declarations
    static_vars = re.findall(r'static struct (arg_\w+) \*(\w+);', content)
    static_arrays = re.findall(r'static void \*(\w+_argtable)\[(\d+)\];', content)

    if not static_vars and not static_arrays:
        print(f"  ⏭️  {filepath.name}: No static argtable found, skipping")
        return False

    # Extract argtable array name and size
    if not static_arrays:
        print(f"  ⚠️  {filepath.name}: Found static vars but no argtable array")
        return False

    argtable_name, argtable_size = static_arrays[0]

    print(f"  🔧 {filepath.name}: Converting {len(static_vars)} static vars")

    # Step 2: Build the typedef struct
    struct_name = f"{cmd_name}_args_t"
    struct_def = f"/*\n * Thread-safe argtable structure - allocated on stack for each invocation\n"
    struct_def += f" * This prevents race conditions when multiple threads execute '{cmd_name}' in parallel\n */\n"
    struct_def += f"typedef struct {{\n"

    for arg_type, var_name in static_vars:
        # Remove cmd_ prefix if present
        clean_name = var_name.replace(f'{cmd_name}_', '')
        struct_def += f"    struct {arg_type} *{clean_name};\n"

    struct_def += f"    void *argtable[{argtable_size}];\n"
    struct_def += f"}} {struct_name};"

    # Step 3: Replace SECTION 1 with new typedef
    section1_pattern = r'/\* ===== SECTION 1: ARGTABLE STRUCTURES ===== \*/.*?(?=/\* ===== SECTION 2:)'
    replacement1 = f"/* ===== SECTION 1: ARGTABLE STRUCTURES (THREAD-SAFE) ===== */\n\n{struct_def}\n\n"
    content = re.sub(section1_pattern, replacement1, content, flags=re.DOTALL)

    # Step 4: Update build function signature
    build_func_old = f"static void build_{cmd_name}_argtable(void)"
    build_func_new = f"static void build_{cmd_name}_argtable({struct_name} *args)"
    content = content.replace(build_func_old, build_func_new)

    # Step 5: Update references inside build function
    for arg_type, var_name in static_vars:
        clean_name = var_name.replace(f'{cmd_name}_', '')
        # Replace assignments: cmd_name_var = ... with args->var = ...
        content = re.sub(
            rf'\b{var_name}\s*=\s*',
            f'args->{clean_name} = ',
            content
        )
        # Replace references in argtable array: argtable[0] = cmd_name_var with args->argtable[0] = args->var
        content = re.sub(
            rf'{argtable_name}\[(\d+)\]\s*=\s*{var_name}\b',
            rf'args->argtable[\1] = args->{clean_name}',
            content
        )

    # Replace argtable references with args->argtable
    content = re.sub(
        rf'\b{argtable_name}\b',
        'args->argtable',
        content
    )

    # Step 6: Update run function
    # Find the run function and add stack allocation
    run_func_pattern = rf'int {cmd_name}_run\(int argc, char \*\*argv, FILE \*in, FILE \*out\)\s*\{{'
    def add_stack_alloc(match):
        return match.group(0) + f"\n    {struct_name} args;  /* Stack-allocated - each thread gets its own copy */"

    content = re.sub(run_func_pattern, add_stack_alloc, content)

    # Update build_X_argtable() calls to pass &args
    content = re.sub(
        rf'build_{cmd_name}_argtable\(\);',
        f'build_{cmd_name}_argtable(&args);',
        content
    )

    # Update variable references to use args. notation
    for arg_type, var_name in static_vars:
        clean_name = var_name.replace(f'{cmd_name}_', '')
        # Replace var->count with args.var->count
        content = re.sub(
            rf'\b{var_name}->',
            f'args.{clean_name}->',
            content
        )
        # Handle special cases like arg_print_errors(stderr, cmd_end, ...)
        content = re.sub(
            rf'\b{var_name}\b(?!->)',  # var_name not followed by ->
            f'args.{clean_name}',
            content
        )

    # Step 7: Update print_usage function similarly
    usage_func_pattern = rf'void {cmd_name}_print_usage\(FILE \*out\)\s*\{{'
    def add_usage_stack_alloc(match):
        return match.group(0) + f"\n    {struct_name} args;  /* Stack-allocated for thread safety */"

    content = re.sub(usage_func_pattern, add_usage_stack_alloc, content)

    # Write back
    with open(filepath, 'w') as f:
        f.write(content)

    return True


def main():
    commands_dir = Path('src/commands')

    if not commands_dir.exists():
        print("Error: src/commands directory not found")
        print("Run this script from the picobox root directory")
        sys.exit(1)

    command_files = sorted(commands_dir.glob('cmd_*.c'))

    # Skip already converted files
    skip_files = {'cmd_true.c', 'cmd_false.c'}  # Already manually converted

    print(f"\n🔄 Converting {len(command_files)} command files to thread-safe argtables\n")

    converted = 0
    skipped = 0

    for filepath in command_files:
        if filepath.name in skip_files:
            print(f"  ✅ {filepath.name}: Already converted")
            skipped += 1
            continue

        if convert_command_file(filepath):
            converted += 1
        else:
            skipped += 1

    print(f"\n✨ Conversion complete!")
    print(f"   Converted: {converted} files")
    print(f"   Skipped:   {skipped} files")
    print(f"\n📝 Next step: Run 'make' to rebuild")


if __name__ == '__main__':
    main()
