#!/bin/sh
for i in 1 2 3 4 5 6 7 8 9 10
do
update-alternatives --get-selections | awk '{print $3}'
done
