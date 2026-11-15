# Hash table analysis

## Design and correctness
The table stores string keys and integer values directly in an array. Its FNV-style hash processes every key byte, then reduces modulo capacity. Capacities are rounded upward to a prime; zero capacity is rejected. Quadratic probes visit `(hash + i*i) % capacity`. Searches are bounded by capacity, so a missing key cannot loop forever. Before a new insertion would raise the live load above one half, the array grows to the next prime at least twice its current size. With a prime capacity and fewer than half the slots live, a quadratic sequence has enough distinct positions to find space.

Deletion leaves a tombstone instead of making the slot never-used. Otherwise a lookup could stop before a colliding key. Probing remembers the first tombstone but keeps searching for an existing key before reusing it. Duplicate insertion returns false without changing the original value; update modifies only an existing key. Missing get throws out_of_range. Keys and values contain only live entries, in matching bucket order.

## Complexity
Let m be capacity, n the live entry count, and L the key length. Hashing costs O(L), and string comparisons can cost O(L). For bounded-length keys and well-distributed hashes at moderate load, contains, get, update, and remove take expected O(1); their best case is one probe. Insert has expected amortized O(1) cost with geometric growth. For variable-length keys, include hashing and comparison costs rather than treating them as constants.

A lookup or insertion may inspect O(m) positions in the worst case. Immediately around normal growth m is proportional to n, giving the usual O(n) worst-case probe bound for bounded-length keys, but after many deletions m can greatly exceed n. Tombstones can make unsuccessful searches expensive even when the live load is low. Rehash scans O(m) slots and reinserts n entries: expected O(m+n), with collision-heavy reinsertion potentially O(n*m_new). It uses O(m_new) temporary storage; allocation and copying finish before swapping the replacement into place. keys and values scan O(m) slots and allocate O(n) output elements. size, capacity, empty, and loadFactor are O(1).

## Trade-offs
High load and a poor hash increase collisions and probe lengths. Linear probing has good locality but primary clustering. Quadratic probing reduces primary clustering, but keys with the same initial hash still share a sequence (secondary clustering). Double hashing gives different probe strides. Separate chaining stores a list or other auxiliary structure per bucket and can support load above one, but adds allocation and pointer overhead. Open addressing is compact and cache-friendly but needs spare capacity, tombstones, and careful probe termination. Rehashing removes tombstones and spreads entries over a larger array. A long-running deletion-heavy workload could benefit from a same-capacity tombstone cleanup policy; this implementation rebuilds during growth or probe exhaustion.

## Validation
The original tests are retained. Additional tests exercise real same-bucket collisions, lookup past a tombstone, duplicate prevention beyond a tombstone, empty keys, small and composite starting capacities, and deterministic insert/update/remove sequences against a standard map.
