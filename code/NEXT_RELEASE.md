# Next synchronized SPTK / XMQ release

## SPTK 5.6.13: allocation-free `BufferStorage` move construction

- Replace the `malloc(1)` used to reset the source in `BufferStorage(BufferStorage&&)` with a null empty state. Move assignment already uses that state.
- Define and preserve the moved-from contract. Audit operations that must work on an empty buffer, including `reset(0)`, copy assignment from an empty buffer, and `c_str()`. Initialize storage lazily where a writable byte is required, without checks on every hot-path call or a public `isNull()` requirement.
- Test moving empty and nonempty buffers, then destroying, assigning to, resetting, and reading the moved-from source. Run the SPTK unit tests and verify the per-move allocation count.
- Coordinate delivery with the corresponding XMQ release. XMQ commit `9c4266c` avoids the two one-byte allocations locally for each received PUBLISH until SPTK has a general fix. The XMQ P2P latency comparison did not show a repeatable gain, so retain the allocation count as the measured benefit.
