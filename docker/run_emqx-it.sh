DATADIR=/home/alexeyp/docker_data/emqx
docker run --rm -v /build:/build -v $DATADIR/data:/opt/emqx/data -v $DATADIR/log:/opt/emqx/log -it emqx /bin/bash