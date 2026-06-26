AiTOS Coding Style
==================

AiTOS follows a consistent kernel-style coding convention for readability and maintainability.

Indentation
-----------

- Use tabs, width 8.
- Keep lines within 80 columns when practical.

Naming
------

- Functions and variables: ``snake_case``.
- Kernel structs: ``struct aitos_*``.
- Public macros: ``AITOS_*`` or project conventions where appropriate.

Types
-----

- Use ``include/aitos/types.h`` (``u8``, ``u64``, ``uintptr_t``, ...).

Initialization
--------------

- Mark boot-time helpers with ``__init``.
- Subsystems expose stable ops tables (``mm_ops``, ``sched_class``, ...).

Logging
-------

- Prefer ``pr_info`` / ``pr_err`` over ad-hoc console writes.
- ``printk`` routes through the console layer.

Errors
------

- Return negative errno values (``-EINVAL``, ``-ENOSYS``).

Headers
-------

- Public kernel headers: ``#include <aitos/...>``.
- AiTOS subsystem APIs: ``#include <aitos/...>``.
