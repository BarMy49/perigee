# Controlled release performance and recovery checks

Use the accepted clean AppImage on Fedora 45 through:

```bash
bash /home/barmy/.cache/perigee-build/appimage-clean-final-20261111/run-fedora45-clean-check.sh
```

This verifies artifact 4ceee61c1ae51c3901b3927b56fb72853ddd23b5f795e41e41811e1ee2085675
and applies the stricter 896 MiB RAM/128 MiB swap guard. Use existing pairing.
Do not copy credentials or change system networking. The launcher records resource
samples; its aggregate memory figure includes extraction and is not streaming RSS.

Choose one repeatable five-minute host workload. Keep host, client, codec,
resolution, FPS, bitrate, HDR, display scaling and network unchanged. Record the
actual decoder/renderer and host/client GPU/driver versions. Warm up for one minute
before measuring. Alternate three runs with optional clipboard services disabled
and three with the normal enabled configuration (off/on/off/on/off/on). Use only
synthetic clipboard text when exercising transfers. Keep the stream statistics
visible in every run so instrumentation is the same. Record received FPS, dropped
frames, network latency and decode/render timing as exposed by the client, plus
CPU/GPU/RAM readings from the same monitoring tool. Mark unavailable metrics;
do not substitute configured FPS for observed FPS.

For each run record configuration, duration, workload, metric values and visible
stalls/audio/input problems. Report the range across repeated runs and compare the
paired off/on results before choosing a regression threshold. This compares the
optional feature configuration in one release; it is not an upstream-baseline
comparison or an end-to-end input-latency measurement.

Afterward perform five normal disconnect/reconnect cycles. Confirm video, audio,
input and capture recovery each time and observe whether memory/thread counts
settle rather than growing each cycle. A network-loss test needs a separately
chosen session-only interruption method; do not disable the machine's network or
stop a shared host service implicitly. Preserve the logs locally; do not publish
raw logs containing host identifiers or credentials.

Record user observations separately from agent-run synthetic lifecycle checks.
Synthetic timeout, cancellation and rapid-restart cases do not measure live
streaming performance or physical network-loss recovery.
