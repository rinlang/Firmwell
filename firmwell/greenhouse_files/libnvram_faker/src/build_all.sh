#!/bin/sh
# Build all libnvram-faker.so targets.
set -e

cd "$(dirname "$0")"
./buildarm.sh
./buildmips.sh
./buildmipsel.sh
./buildx86.sh
./buildx86_64.sh
