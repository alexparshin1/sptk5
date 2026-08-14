For some reasone, oraclelinux refuses regular installation of gtest:

$> yum install -y gtest-devel

- when it's called from make_builder.sh.

The files in this directory are downloaded from within oraclelinux:

$> yum download gtest gtest-devel gmock gmock-devel

and copied to this directory.