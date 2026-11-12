# Next synchronized SPTK / XMQ release

## A WebSockets transport that a server can build on

- Rewrite the frame codec. `WSWebSocketsMessage::encode()` wrote no length marker for payloads of 126 bytes or more, and wrote the 64-bit length in host byte order, so a server could send only frames under 126 bytes; `decode()` read lengths and payloads through unaligned casts, and had no bound to check them against. Both now follow RFC 6455, take the bytes received as an explicit limit, and report "not complete yet" rather than assuming a whole frame arrived.
- Add `WSWebSocketStream`, a byte stream over frames: partial frames, several frames in one read, fragmentation with control frames interleaved, ping answered, close echoed once, text validated as UTF-8, and a configurable message limit. It touches no socket, so it is tested without a network and works over TCP or TLS.
- Add `ws::acceptKey()`, `ws::negotiateSubprotocol()` and `ws::writeUpgradeResponse()`, so a server with its own connection loop can answer the upgrade without owning an instance of the WSDL protocol class.
- `WSWebSocketsProtocol::process()` now runs on the stream and echoes what it receives, instead of replying "Hello" and "World" to every frame.

## SPTK 5.6.13: allocation-free `BufferStorage` move construction

- Replace the `malloc(1)` used to reset the source in `BufferStorage(BufferStorage&&)` with a null empty state. Move assignment already uses that state.
- Define and preserve the moved-from contract. Audit operations that must work on an empty buffer, including `reset(0)`, copy assignment from an empty buffer, and `c_str()`. Initialize storage lazily where a writable byte is required, without checks on every hot-path call or a public `isNull()` requirement.
- Test moving empty and nonempty buffers, then destroying, assigning to, resetting, and reading the moved-from source. Run the SPTK unit tests and verify the per-move allocation count.
- Coordinate delivery with the corresponding XMQ release. XMQ commit `9c4266c` avoids the two one-byte allocations locally for each received PUBLISH until SPTK has a general fix. The XMQ P2P latency comparison did not show a repeatable gain, so retain the allocation count as the measured benefit.
