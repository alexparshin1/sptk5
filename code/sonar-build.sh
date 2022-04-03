#!/bin/bash

export PATH=/opt/sonar-scanner/bin:/usr/lib/ccache:$PATH

CORES=$(grep 'cpu MHz' /proc/cpuinfo | wc -l)
sed -i "s/sonar.cfamily.threads=.*$/sonar.cfamily.threads=$CORES/" sonar-project.properties

build-wrapper-linux-x86-64 --out-dir bw-output make -j $CORES clean install

# Project options
OPTIONS="-Dsonar.projectKey=HappySquirrel_sptk5 -Dsonar.organization=happysquirrel-bitbucket -Dsonar.sources=."

# Sonar connection
OPTIONS="${OPTIONS} -Dsonar.host.url=https://sonarcloud.io -Dsonar.login=638f3e4b9ea57879e595dbfddb91d47c9a90bf64"

# Cache options
OPTIONS="${OPTIONS} -Dsonar.cfamily.cache.enabled=true -Dsonar.cfamily.cache.path=/home/alexeyp/.sonar/cache"

sonar-scanner ${BPROXY} ${OPTIONS} -Dsonar.cfamily.build-wrapper-output=bw-output  -Dsonar.report.export.path=sonar-report.json
echo Completed: $(date)
