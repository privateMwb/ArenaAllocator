# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:8a9ddfa32864f269c7cf052ddc11843645f7353a321d0399bb50327c6167770e

COPY . $SRC/ArenaAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/ArenaAllocator
