*This project has been created as part of the 42 curriculum by heychong.*

## Description

**codexion** simulates N coders sitting in a circular co-working hub, sharing
N USB dongles (one dongle per seat, positioned between each pair of
neighbours). Each coder cycles endlessly through three phases —
**compiling**, **debugging**, **refactoring** — and compiling requires
holding both the left and right dongle at once. Coders never talk to each
other; the only thing that decides who gets a dongle next is a scheduler
(**fifo** or **edf**) enforcing fair, starvation-free access.

The simulation is a themed variant of the classic dining philosophers
problem: dongles are forks, compiling is eating, and burning out (failing to
start a compile within `time_to_burnout` ms of the last one) is a
philosopher starving to death. It stops the moment a coder burns out, or the
moment every coder has compiled at least `number_of_compiles_required`
times.

## Instructions

```sh
make
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown \
           scheduler
```

- `scheduler` must be exactly `fifo` or `edf`.
- All numeric arguments must be non-negative integers; `number_of_coders`
  must additionally be strictly positive. Invalid input is rejected with an
  error message and exit code 1.

Example:

```sh
./codexion 5 800 200 200 200 4 50 edf
```

`make`, `make bonus`, `make clean`, `make fclean`, `make re` are all
supported (no bonus features are implemented; `bonus` is an alias for
`all`).

## Blocking cases handled

- **Deadlock (circular wait):** every coder acquires its left dongle then
  its right dongle, **except the highest-numbered coder**, who acquires
  right-then-left. This single asymmetry makes a circular hold-and-wait
  chain around the table structurally impossible, regardless of timing.
- **Starvation / fair arbitration:** each dongle owns its own small
  priority queue (a hand-rolled min-heap, no standard library priority
  queue). Only the coder at the front of a dongle's queue may take it; the
  key is either arrival timestamp (`fifo`) or
  `last_compile_start + time_to_burnout` (`edf`), with coder id as a
  deterministic tie-breaker. Because a dongle is only ever contested by its
  two neighbours, this guarantees each contested dongle is granted in
  scheduler order — no coder can be perpetually skipped.
- **Dongle cooldown:** on release, a dongle stores
  `now + dongle_cooldown` as its `cooldown_until` timestamp. It cannot be
  granted again — even to the front-of-queue coder — until that time has
  passed. Waiting coders self-wake near the cooldown deadline via
  `pthread_cond_timedwait` rather than being told, since no other thread
  proactively signals a mere cooldown expiry.
- **Precise burnout detection:** a dedicated monitor thread polls every
  coder's `last_compile_start` every 1ms and declares burnout the instant
  `now - last_compile_start > time_to_burnout`, logging within ~1-2ms of
  the real event (well inside the 10ms requirement).
- **Log serialization:** all output goes through one function guarded by a
  single mutex, so two state-change lines can never interleave.
- **Single-coder edge case:** with one coder there is only one dongle on
  the table (as the subject specifies), but compiling still requires two.
  The second acquisition attempt (on that same, already-held dongle) can
  therefore never succeed. The coder blocks on it until the monitor
  detects the burnout and flips `stop_flag`, which unblocks the wait via
  `is_stopped()` and the coder cleanly releases its one dongle and exits.
  A single coder can never compile and always burns out — this follows
  directly from the subject's "two dongles simultaneously" requirement,
  not from any special-cased shortcut in the code.

## Thread synchronization mechanisms

- **`pthread_mutex_t` per dongle** protects that dongle's `in_use` flag,
  `cooldown_until` timestamp and its private wait-queue heap. Every read or
  mutation of that state happens with the lock held, so two coders can
  never both believe they've taken the same dongle.
- **`pthread_cond_t` per dongle** lets a coder sleep instead of busy-waiting
  while it cannot yet take the dongle. `pthread_cond_broadcast` is used
  (not `signal`) on release, because more than one waiter may need to
  re-check whether it is now the eligible one; `pthread_cond_timedwait` is
  used so a waiter also wakes on cooldown expiry even with nobody actively
  signalling it, capped to a ~50ms poll ceiling as a safety net.
- **`state_lock`** guards simulation-wide shared state: each coder's
  `compile_count`, the global `stop_flag`, and `burnout_id`. Both the
  "all coders reached the required compile count" check (run by whichever
  coder finishes last) and the monitor's burnout write go through this
  lock, so the two possible end conditions can never corrupt each other or
  race to a double-stop.
- **`log_lock`** serializes every `printf` state-change line.
- **Coordination coder ↔ monitor:** coders never talk to each other or to
  the monitor directly; they only expose `last_compile_start` (updated the
  instant a compile phase begins) and `compile_count`, both behind
  `state_lock`. The monitor reads `last_compile_start` under no additional
  coordination beyond that lock's read, decides burnout, and — the moment
  it does — flips `stop_flag`. Every coder checks `stop_flag` between
  phases and while waiting for a dongle (bounded by the ~50ms wait cap
  above), so the whole simulation unwinds cleanly without any thread
  needing to be cancelled or killed.

## Resources

- Dijkstra, E. W. — *Dining Philosophers Problem* (original formulation of
  circular resource contention and deadlock).
- `man pthread_mutex_lock`, `man pthread_cond_timedwait`, `man gettimeofday`
  — POSIX threading reference used throughout.
- Earliest Deadline First scheduling — classic real-time scheduling
  literature (Liu & Layland, 1973) for the theoretical basis of the `edf`
  priority key used here.

**AI usage:** an AI assistant (Claude) was used throughout design and
implementation — to work through the concurrency design from first
principles (state machine, mutex/cond usage, deadlock-avoidance ordering,
per-dongle fair queues, cooldown handling, burnout detection), to write the
initial C implementation of every file in this repository, and to build and
run smoke/stress tests (repeated executions, forced-burnout timing checks,
manual memory-allocation audit) to validate the design. No external code
was copied; all logic was reasoned through and written specifically for
this subject.