FROM centos:centos7
LABEL maintainer="Arkadiusz Dzięgiel <arkadiusz.dziegiel@glorpen.pl>"

# install puppet repo and puppet agent
RUN yum -y --nogpgcheck install https://yum.puppetlabs.com/puppet5/puppet5-release-el-7.noarch.rpm \
    && yum -y install puppet-agent puppetdb-termini \
    && yum clean all \
    && mkdir -p /opt/puppetizer/sources /var/opt/puppetizer/{modules,vendor,data} \
    && rm -rf /etc/puppetlabs/code/environments/production/*

ADD opt /opt/puppetizer/
