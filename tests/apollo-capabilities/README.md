# Apollo-compatible metadata checks

This standalone Qt Core/Qt Test target covers the Milestone 2 value parser,
transport provenance policy and user-configured administration URL validation.
It does not identify a host product/version or contact any host. The application
build does not depend on the test target.

From the source root:

```bash
mkdir -p ../perigee-build/tests/apollo-capabilities
cd ../perigee-build/tests/apollo-capabilities
qmake6 ../../../perigee/tests/apollo-capabilities/apollo-capabilities.pro
make -j2
./tst_apollocapabilities
```

Fixtures cover ordinary serverinfo without extensions; authenticated, missing,
invalid and denied permissions; unknown bits; paired/pinned provenance; ordered
and duplicate command names; display availability independent of permission;
malformed/oversized/nested/DTD XML; stale-state invalidation; and HTTPS URL limits.
They validate the provenance predicate, not a live TLS handshake or host-grant flow.

Implementation follows the source fields in the pinned Apollo snapshot recorded in
[APOLLO_PROTOCOL.md](../../docs/APOLLO_PROTOCOL.md). Real Sunshine/Apollo sessions,
permission revocation and compositor/UI interaction remain separate acceptance.
