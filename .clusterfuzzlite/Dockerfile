# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:32a88ae7628e937252cdaaead897f8da4083dd08d862e27f449b59e96972cad0

COPY . $SRC/ArenaAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/ArenaAllocator
