# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:fa91d623389589a099c27600abaca3d1dd31299b12715c37ce386ec27fb46f42

COPY . $SRC/ArenaAllocator
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/ArenaAllocator
