# Display launch policy checks

This independent Qt Core/Test target checks the Milestone 4 fresh-launch gate.
It uses synthetic capability XML, never contacts a host and does not create or
change a display. From the source root:

```bash
mkdir -p ../perigee-build/tests/display-launch
cd ../perigee-build/tests/display-launch
qmake6 ../../../perigee/tests/display-launch/display-launch.pro
make -j2
./tst_displaylaunch -o results.txt,txt
```

Cases cover default-off behavior, standard Sunshine/GFE, paired HTTPS provenance,
launch denial/invalid permissions, missing/invalid/unready driver, unsupported
display flags, resume/running-app rejection, an eligible new launch and revoked
metadata. Only an eligible state produces `virtualDisplay=1`. This does not
establish real host display creation, mode selection or Sunshine streaming
regression acceptance; those remain live Milestone 4 checks.
