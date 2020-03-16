#!/bin/bash

export PATH=/opt/sonar-scanner/bin:$PATH

build-wrapper-linux-x86-64 --out-dir bw-output make clean all

# Project options
OPTIONS="-Dsonar.projectKey=HappySquirrel_sptk5 -Dsonar.organization=happysquirrel-bitbucket -Dsonar.sources=."

# Sonar connection
OPTIONS="${OPTIONS} -Dsonar.host.url=https://sonarcloud.io -Dsonar.login=638f3e4b9ea57879e595dbfddb91d47c9a90bf64"

# Cache options
OPTIONS="${OPTIONS} -Dsonar.cfamily.cache.enabled=true -Dsonar.cfamily.cache.path=relative_or_absolute_path_to_cache_location"

sonar-scanner ${BPROXY} ${OPTIONS} -Dsonar.cfamily.build-wrapper-output=bw-output  -Dsonar.report.export.path=sonar-report.json
echo Completed: $(date)
