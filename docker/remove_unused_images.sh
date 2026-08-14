docker rmi $(docker images | grep -E '^<none>' | sed -re 's/^<none>  *<none>  * ([a-f0-9]*) .*/\1/')
