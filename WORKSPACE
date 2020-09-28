# -*- mode: python -*-
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "bazel_compilation_db",
    strip_prefix = "bazel-compilation-database-master",
    urls = [
        "https://github.com/grailbio/bazel-compilation-database/archive/master.tar.gz"
    ],
)

git_repository(
    name = "GSL",
    commit = "f790f0b3a74930d9ea7751459393fae5124159b8",
    remote = "https://github.com/hcoona/GSL.git",
)

new_git_repository(
    name = "structopt",
    build_file = "//thirdparty:BUILD.structopt",
    branch = "master",
    remote = "https://github.com/p-ranav/structopt",
)

new_git_repository(
    name = "magic-enum",
    build_file = "//thirdparty:BUILD.magic-enum",
    branch = "master",
    remote = "https://github.com/Neargye/magic_enum",
)

new_git_repository(
    name = "ctre",
    build_file = "//thirdparty:BUILD.ctre",
    tag = "v2.8.4",
    remote = "https://github.com/hanickadot/compile-time-regular-expressions.git",
)

new_git_repository(
    name = "tl-expected",
    build_file = "//thirdparty:BUILD.tl-expected",
    tag = "v1.0.0",
    remote = "https://github.com/TartanLlama/expected",
)

new_git_repository(
    name = "fmt",
    build_file = "//thirdparty:BUILD.fmt",
    branch = "master",
    remote = "https://github.com/fmtlib/fmt.git",
)
