#!/bin/bash

echo UPDATING GEOIP DATABASE...
/etc/cron.daily/geoip_update.sh &> /dev/null
systemctl reload crond
