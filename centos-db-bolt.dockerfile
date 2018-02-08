FROM glorpen/puppet-base:db-centos7
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

RUN /bin/sh /opt/puppetizer/share/provision.sh puppet=no bolt=yes puppetdb=no
