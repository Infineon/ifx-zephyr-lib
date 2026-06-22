# Copyright (c) 2026 Infineon Technologies AG,
# or an affiliate of Infineon Technologies AG.
# SPDX-License-Identifier: Apache-2.0

# Root CMakeLists.txt - include Zephyr integration from zephyr subdirectory
add_subdirectory_ifdef(CONFIG_MWUTILS zephyr)