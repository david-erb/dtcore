# Virtual-table pattern used across dt* modules

This page explains the common "vtable" mechanism shared by several function-groups. 
The goals are portability, testability, and a stable C API surface
while allowing multiple back-end implementations (mock, OS-specific, hardware-backed).

The design pattern is for polymorphic objects that expose a stable identifier without exposing structure layout,
allowing late binding across independent modules.

The registry backing this mechanism is implemented in the [dtvtable function group](../function_groups/dtvtable.md).

See [dtguidable](../function_groups/dtguidable.md) for a simple example of a facade.

## Mini-guide
- The public API is a thin facade that dispatches via a module-specific vtable struct.
- Implementations define a concrete struct whose **first field** is a common header containing an int32_t model_number.
- You publish an implementation by calling the module's `*_set_vtable()` once at init.
- Errors: functions return NULL on success. Otherwise a dterr_t pointer that the caller must dispose.
- You must treat published vtables as **immutable**. Don't publish and access from multiple threads simultaneously.

---

## Why this pattern?

- **Stable C ABI:** Call sites never see platform details; the facade and typedefs remain fixed.
- **Pluggable backends:** Select a backend at runtime (e.g., POSIX, Zephyr, Win32, Raspberry Pi Pico, or a deterministic fake for tests).
- **Testability:** Swap in a mock backend by setting the vtable during tests.
- **Separation of concerns:** Public headers document behavior; backends focus solely on mechanics.

---

## Anatomy of a vtable module
A typical module (using [dtguidable](../function_groups/dtguidable.md) as an example) defines:

- A vtable type:
```c
typedef struct dtguidable_vt_t {
  dtguidable_get_guid_fn  get_guid;
  dtguidable_dispose_fn   dispose;
} dtguidable_vt_t;
```

- A small static instance in each implementation:
```c
static dtguidable_vt_t my_vt = {
  .get_guid = (dtguidable_get_guid_fn)my_get_guid,
  .dispose = (dtguidable_dispose_fn)my_dispose,
};
```

- A publish call during initialization:
```c
dtguidable_set_vtable(&my_vt);
```

- A facade call site that looks up and dispatches:
```c
dtguid_t guid;
dterr_t* dtguidable_get_guid(dtguidable_handle self, &guid) {
  return self->vt->get_guid(self);
}
```

---

## Runtime performance

The call through the facade involves extra instructions to dispatch to the correct function from the vtable registry.
Normally the extra cost is insignificant.
However, to avoid it while in hot loops or other performance-critical code sections, your code can prepare the function pointer in advance.

For example:
```c
// Reference the static vtable for your object handle.
dtguidable_vt_t *vtable;
DTERR_C(dtguidable_get_vtable(*((int32_t*)guidable_handle), &vtable));
// Store a pointer to the function of interest.
dtguidable_wait_fn guidable_wait_fn = vtable->dtguidable_wait_fn;
while(true)
{
  // Call the function with no vtable dispatch overhead.
  dterr = guidable_wait_fn(guidable_handle, 1000);
}
```
---

## Implementation checklist
- Decide a world-unique integer module number for your implementation.
- Declare your implementation functions in the module's public header.  A macro such as DTguidable_DECLARE_API automates this.
- Use a macro like DTGUIDABLE_INIT_VTABLE to populate the vtable with function pointers from your implementation.
- Register your vtable in the module's init function by calling for example `dtguidable_set_vtable()`,

