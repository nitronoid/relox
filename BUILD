load("@bazel_compilation_db//:aspects.bzl", "compilation_database")
compilation_database(
    name = "compilation_commands.json",
    targets = [
        "//:lox",
    ],
    # ideally should be the same as `bazel info execution_root`.
    exec_root = "/home/nitronoid/dev/git/nitronoid/lox/AST",
)

cc_library(
    name = "lox-private",
    hdrs = glob(["include/**/*.hpp"]),
    srcs = glob(["src/**/*.cpp"]),
    deps = [
        "@GSL//include/gsl:gsl_library",
        "@ctre//:ctre",
        "@fmt//:fmt",
        "@tl-expected//:expected",
        "@magic-enum//:magic-enum",
        "@structopt//:structopt",
    ],
    include_prefix = "lox",
    strip_include_prefix = "include",
    copts = ["-Wno-type-limits"],
)

cc_binary(
    name = "lox",
    srcs = ["main.cpp"],
    deps = [
        ":lox-private",
    ],
)

