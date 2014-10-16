#!/bin/sh
[ -e /home/$USER/cruft-ng.log ] && mv /home/$USER/cruft-ng.log /home/$USER/cruft-ng.log.old
sudo /home/$USER/cruft-ng/cruft > /home/$USER/cruft-ng.log
ls -l /home/$USER/cruft-ng.log*
[ -e /home/$USER/cruft-ng.log.old ] && colordiff -u /home/$USER/cruft-ng.log.old /home/$USER/cruft-ng.log
