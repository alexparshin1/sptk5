#!/bin/bash

export PATH=/opt/sonar-scanner/bin:$PATH

build-wrapper-linux-x86-64 --out-dir bw-output make clean all
sonar-scanner   -Dsonar.projectKey=HappySquirrel_sptk5   -Dsonar.organization=happysquirrel-bitbucket   -Dsonar.sources=.   -Dsonar.cfamily.build-wrapper-output=bw-output   -Dsonar.host.url=https://sonarcloud.io   -Dsonar.login=638f3e4b9ea57879e595dbfddb91d47c9a90bf64
